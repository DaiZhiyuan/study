<!-- TOC -->

- [1. What is KVM API ?](#1-what-is-kvm-api-)
    - [1.1 /dev/kvm](#11-devkvm)
    - [1.2 kvm-vm](#12-kvm-vm)
    - [1.3 kvm-vcpu](#13-kvm-vcpu)
    - [1.4 Summary](#14-summary)
- [2. abort mini-vmm](#2-abort-mini-vmm)
    - [2.1 feature](#21-feature)
    - [2.2 Used KVM API](#22-used-kvm-api)
    - [2.3 Runing effect](#23-runing-effect)
    - [2.4 Source Code](#24-source-code)
- [3. Appendix](#3-appendix)
    - [3.1 Architecture of 8086](#31-architecture-of-8086)

<!-- /TOC -->


# 1. What is KVM API ?

## 1.1 /dev/kvm

```c
#define KVM_API_VERSION 12

static long kvm_dev_ioctl(struct file *filp,
              unsigned int ioctl, unsigned long arg)
{
    long r = -EINVAL;

    switch (ioctl) {
    case KVM_GET_API_VERSION:
        if (arg)
            goto out;
        r = KVM_API_VERSION;
        break;
    case KVM_CREATE_VM:
        r = kvm_dev_ioctl_create_vm(arg);
        break;
    case KVM_CHECK_EXTENSION:
        r = kvm_vm_ioctl_check_extension_generic(NULL, arg);
        break;
    case KVM_GET_VCPU_MMAP_SIZE:
        if (arg)
            goto out;
        r = PAGE_SIZE;     /* struct kvm_run */
#ifdef CONFIG_X86
        r += PAGE_SIZE;    /* pio data page */
#endif
#ifdef KVM_COALESCED_MMIO_PAGE_OFFSET
        r += PAGE_SIZE;    /* coalesced mmio ring page */
#endif
        break;
    case KVM_TRACE_ENABLE:
    case KVM_TRACE_PAUSE:
         case KVM_TRACE_DISABLE:
         r = -EOPNOTSUPP;
         break;
     default:
         return kvm_arch_dev_ioctl(filp, ioctl, arg);
     }
 out:
     return r;
 }

static struct file_operations kvm_chardev_ops = {
    .unlocked_ioctl = kvm_dev_ioctl,
    .compat_ioctl   = kvm_dev_ioctl,
    .llseek     = noop_llseek,
};

static struct miscdevice kvm_dev = {
    KVM_MINOR,
    "kvm",
    &kvm_chardev_ops,
};

int kvm_init(void *opaque, unsigned vcpu_size, unsigned vcpu_align,
          struct module *module)
{
    ......
    
    r = misc_register(&kvm_dev);
    if (r) {
        pr_err("kvm: misc device register failed\n");
        goto out_unreg;
        
    ......
}
```


## 1.2 kvm-vm

```c
int __kvm_set_memory_region(struct kvm *kvm,
                const struct kvm_userspace_memory_region *mem)
{

    struct kvm_memory_slot new;
    ......
    
    kvm_arch_create_memslot(kvm, &new, npages);
    ......

}

int kvm_set_memory_region(struct kvm *kvm,
              const struct kvm_userspace_memory_region *mem)
{
    int r;

    mutex_lock(&kvm->slots_lock);
    r = __kvm_set_memory_region(kvm, mem);
    mutex_unlock(&kvm->slots_lock);
    return r;
}
EXPORT_SYMBOL_GPL(kvm_set_memory_region);

static long kvm_vm_ioctl(struct file *filp,
                unsigned int ioctl, unsigned long arg)
{
    struct kvm *kvm = filp->private_data;
    void __user *argp = (void __user *)arg;
    int r;

    if (kvm->mm != current->mm)
        return -EIO;
    switch (ioctl) {
    case KVM_CREATE_VCPU:
        r = kvm_vm_ioctl_create_vcpu(kvm, arg);
        break;
        
    case KVM_SET_USER_MEMORY_REGION: {
        struct kvm_userspace_memory_region kvm_userspace_mem;

        r = -EFAULT;
        if (copy_from_user(&kvm_userspace_mem, argp,
                         sizeof(kvm_userspace_mem)))
            goto out;

        r = kvm_vm_ioctl_set_memory_region(kvm, &kvm_userspace_mem);
         break;
     
        ......
         
    default:
         r = kvm_arch_vm_ioctl(filp, ioctl, arg);
     }
 out:
     return r;
}


static struct file_operations kvm_vm_fops = {
    .release        = kvm_vm_release,
    .unlocked_ioctl = kvm_vm_ioctl,
    .llseek     = noop_llseek,
};


static int kvm_dev_ioctl_create_vm(unsigned long type)
{

    ......
    
    file = anon_inode_getfile("kvm-vm", &kvm_vm_fops, kvm, O_RDWR);
    if (IS_ERR(file)) {
        put_unused_fd(r);
        kvm_put_kvm(kvm);
        return PTR_ERR(file);
    }
    ......
}

```

## 1.3 kvm-vcpu 
```c
static long kvm_vcpu_ioctl(struct file *filp,
               unsigned int ioctl, unsigned long arg)
{
    struct kvm_vcpu *vcpu = filp->private_data;
    void __user *argp = (void __user *)arg;
    int r;
    struct kvm_fpu *fpu = NULL;
    struct kvm_sregs *kvm_sregs = NULL;

    if (vcpu->kvm->mm != current->mm)
        return -EIO;

    if (unlikely(_IOC_TYPE(ioctl) != KVMIO))
        return -EINVAL;

    r = vcpu_load(vcpu);
    if (r)
        return r;
    switch (ioctl) {
    case KVM_RUN:
        r = -EINVAL;
        if (arg)
            goto out;
        if (unlikely(vcpu->pid != current->pids[PIDTYPE_PID].pid)) {
            /* The thread running this VCPU changed. */
            struct pid *oldpid = vcpu->pid;
            struct pid *newpid = get_task_pid(current, PIDTYPE_PID);
    
            rcu_assign_pointer(vcpu->pid, newpid);
            if (oldpid)
                synchronize_rcu();
            put_pid(oldpid);
        }
        r = kvm_arch_vcpu_ioctl_run(vcpu, vcpu->run);
        trace_kvm_userspace_exit(vcpu->run->exit_reason, r);
        break;

     case KVM_GET_REGS: {
         struct kvm_regs *kvm_regs;

         r = -ENOMEM;
         kvm_regs = kzalloc(sizeof(struct kvm_regs), GFP_KERNEL);
         if (!kvm_regs)
             goto out;
         r = kvm_arch_vcpu_ioctl_get_regs(vcpu, kvm_regs);
         if (r)
             goto out_free1;
         r = -EFAULT;
         if (copy_to_user(argp, kvm_regs, sizeof(struct kvm_regs)))
             goto out_free1;
         r = 0;
 out_free1:
         kfree(kvm_regs);
         break;
     }
     
          case KVM_SET_REGS: {
         struct kvm_regs *kvm_regs;

         r = -ENOMEM;
         kvm_regs = memdup_user(argp, sizeof(*kvm_regs));
         if (IS_ERR(kvm_regs)) {
             r = PTR_ERR(kvm_regs);
             goto out;
         }
         r = kvm_arch_vcpu_ioctl_set_regs(vcpu, kvm_regs);
         kfree(kvm_regs);
         break;
     }
     
     default:
         r = kvm_arch_vcpu_ioctl(filp, ioctl, arg);
     }
 out:
     vcpu_put(vcpu);
     kfree(fpu);
     kfree(kvm_sregs);
     return r;
 }
 
 static struct file_operations kvm_vcpu_fops = {
    .release        = kvm_vcpu_release,
    .unlocked_ioctl = kvm_vcpu_ioctl,
    .mmap           = kvm_vcpu_mmap,
    .llseek     = noop_llseek,
};

static int create_vcpu_fd(struct kvm_vcpu *vcpu)
{
    return anon_inode_getfd("kvm-vcpu", &kvm_vcpu_fops, vcpu, O_RDWR | O_CLOEXEC);
}

static int kvm_vm_ioctl_create_vcpu(struct kvm *kvm, u32 id)
{
    ......

    r = create_vcpu_fd(vcpu);
    if (r < 0) {
        kvm_put_kvm(kvm);
        goto unlock_vcpu_destroy;
    }
    
    ......
}
```

## 1.4 Summary

The KVM API is Set(ioctl syscall). 

name | inode | function | comment
---|---|---|---
kvm | /dev/kvm  | kvm_dev_ioctl  | filesystem `/dev/kvm` by open syscall.
kvm-vm | anonymous | kvm_vm_ioctl   |  ioctl(`KVM_CREATE_VM`) by `kvm file descriptor`
kvm-vcpu | anonymous | kvm_vcpu_ioctl |  ioctl(`KVM_CREATE_VCPU`) by `kvm-vm file descriptor`

# 2. abort mini-vmm

To understand how the VMM works.

## 2.1 feature 

- only suport 1 vcpu.
- fixed guest memory.
- is very simple.

```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C                                1             44             10            184
make                             1              4              0              9
Assembly                         1              3              0              8
-------------------------------------------------------------------------------
SUM:                             3             51             10            201
-------------------------------------------------------------------------------
```

## 2.2 Used KVM API

- KVM_GET_API_VERSION：This identifies the API version as the stable kvm API.
- KVM_CREATE_VM：The new VM has no virtual cpus and no memory.
- KVM_SET_USER_MEMORY_REGION：This ioctl allows the user to create or modify a guest physical memory slot. 
- KVM_CREATE_VCPU：This API adds a vcpu to a virtual machine.
- KVM_GET_VCPU_MMAP_SIZE： communicates with userspace via a shared memory region.
- KVM_SET_SREGS：Writes special registers into the vcpu.
- KVM_SET_REGS：Writes the general purpose registers into the vcpu.
- KVM_RUN：used to run a guest virtual cpu.

## 2.3 Runing effect 

```bash
#> ./vmm
VM Starting...
VMM enter guest...

Handle PIO
Hardware decode:
-- port: 255
-- direction: in
-- data: 0
-- size: 2
VMM simulate

VMM enter guest...

Handle PIO
Hardware decode:
-- port: 255
-- direction: in
-- data: 1
-- size: 2
VMM simulate

VMM enter guest...

Handle PIO
Hardware decode:
-- port: 255
-- direction: in
-- data: 2
-- size: 2
VMM simulate

VMM enter guest...

Handle PIO
Hardware decode:
-- port: 255
-- direction: in
-- data: 3
-- size: 2
VMM simulate

VMM enter guest...

Handle PIO
Hardware decode:
-- port: 255
-- direction: in
-- data: 4
-- size: 2
VMM simulate

VMM enter guest...

Handle PIO
Hardware decode:
-- port: 255
-- direction: in
-- data: 5
-- size: 2
VMM simulate
```

## 2.4 Source Code

vm.S
```asm
.globl _start
    .code16

_start:
    xorw %ax, %ax

loop:
    out %ax, $0xFF
    inc %ax
    jmp loop
```

vmm.c
```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/kvm.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

/*
 * Guest specification
 */
#define GUEST_MEMORY_SIZE   (256*1024*1024)
#define GUEST_CPU_NUMBER    1
#define GUEST_SYSTEM_IMAGE  "vm.bin"

#define die(messages)       fprintf(stderr, "%s\n", messages),exit(errno);

/*
 * For 20-bit Addresses, selector is 4bits, base/offset is 16 bits.
 */
#define RESET_SEGMENT(regs) \
    do { \
        vcpu->sregs.regs.selector = 0; \
        vcpu->sregs.regs.base = 0x1000 << 4; \
    } while(0)

struct vcpu {
    int vcpu_id;
    int vcpu_fd;
    pthread_t thread_id;
    void *(*thread_routine) (void *);
    struct kvm_regs regs;
    struct kvm_sregs sregs;
    struct kvm_run *kvm_run;
    int kvm_run_mmap_size;
};

struct virtual_machine_manager {
    int kvm_fd;
    int vm_fd;
    __u64 guest_ram_start;
    __u64 guest_ram_size;
    int kvm_version;
    struct kvm_userspace_memory_region mem;
    struct vcpu *vcpus;
    int nr_vcpu;
};

void vmm_reset_vcpu(struct vcpu *vcpu)
{
    if (ioctl(vcpu->vcpu_fd, KVM_GET_SREGS, &(vcpu->sregs)) < 0)
        die("Can not get sregs.");

    RESET_SEGMENT(cs);
    RESET_SEGMENT(ds);
    RESET_SEGMENT(es);
    RESET_SEGMENT(fs);
    RESET_SEGMENT(gs);
    RESET_SEGMENT(ss);

    if (ioctl(vcpu->vcpu_fd, KVM_SET_SREGS, &vcpu->sregs) < 0)
        die("Can not set segment regs.");

    vcpu->regs.rflags = 0x2;
    vcpu->regs.rip = 0;
    vcpu->regs.rsp = 0;
    vcpu->regs.rbp = 0;

    if (ioctl(vcpu->vcpu_fd, KVM_SET_REGS, &(vcpu->regs)) < 0)
        die("Can not set regs.");
}

void vmm_handle_pio(struct vcpu *vcpu)
{
    printf("\n");
    printf("Handle PIO\n");
    printf("Hardware decode:\n");
    printf("-- port: %d\n", vcpu->kvm_run->io.port);
    printf("-- direction: %s\n",
           vcpu->kvm_run->io.direction ? "in" : "out");
    printf("-- data: %d\n",
           *(int *) ((char *) (vcpu->kvm_run) +
                     vcpu->kvm_run->io.data_offset));
    printf("-- size: %d\n", vcpu->kvm_run->io.size);
    printf("VMM simulate\n");
    sleep(1);
    printf("\n");

}

void *vmm_cpu_thread(void *args)
{
    struct virtual_machine_manager *vmm =
        (struct virtual_machine_manager *) args;

    vmm_reset_vcpu(vmm->vcpus);

    int ret;
    printf("VM Starting...\n");

    while (1) {

        printf("VMM enter guest...\n");
        ret = ioctl(vmm->vcpus->vcpu_fd, KVM_RUN, 0);
        if (ret < 0)
            die("KVM_RUN failed.");

            switch (vmm->vcpus->kvm_run->exit_reason) {
            case KVM_EXIT_IO:
                vmm_handle_pio(vmm->vcpus);
                break;
            case KVM_EXIT_SHUTDOWN:
                printf("KVM_EXIT_SHUTDOWN\n");
                goto exit_kvm;
                break;
            default:
                printf("KVM PANIC\n");
                goto exit_kvm;
            }
    }

exit_kvm:
    return 0;
}

struct virtual_machine_manager *virtual_machine_manager_init(void)
{
    struct virtual_machine_manager *vmm =
        malloc(sizeof(struct virtual_machine_manager));

    vmm->kvm_fd = open("/dev/kvm", O_RDWR);
    if (vmm->kvm_fd < 0)
        return NULL;

    vmm->kvm_version = ioctl(vmm->kvm_fd, KVM_GET_API_VERSION, 0);

    return vmm;
}

int virtual_machine_manager_create_vm(struct virtual_machine_manager *vmm,
                                      int nr_vcpu, int memory_size,
                                      char *guest_image)
{

    // 1. Create Virtual Machine
    vmm->vm_fd = ioctl(vmm->kvm_fd, KVM_CREATE_VM, 0);
    if (vmm->vm_fd < 0)
            return -1;

    // 2. Settings Virtual Machine Memory(GPA:0 ~ 256MB)
    vmm->guest_ram_size = memory_size;
    vmm->guest_ram_start =
        (__u64) mmap(NULL, vmm->guest_ram_size, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if ((void *) vmm->guest_ram_start == MAP_FAILED)
            return -1;

    vmm->mem.slot = 0;
    vmm->mem.guest_phys_addr = 0;
    vmm->mem.userspace_addr = vmm->guest_ram_start;
    vmm->mem.memory_size = vmm->guest_ram_size;

    if (ioctl(vmm->vm_fd, KVM_SET_USER_MEMORY_REGION, &(vmm->mem)))
        return -1;

    // 3. Load System Images
    int sys_image = open(guest_image, O_RDONLY);
    if (sys_image < 0)
            die("Can not open binary file.");

    int nbyte = 0;
    char *pos = (char *) vmm->guest_ram_start;
    while (1) {
            nbyte = read(sys_image, pos, 4096);
            if (nbyte <= 0) {
                break;
            }
            pos += nbyte;
    }

    // 4. Settings Virtual Machine vCPU
    struct vcpu *vcpu = malloc(sizeof(struct vcpu) * nr_vcpu);
    vmm->nr_vcpu = nr_vcpu;
    vcpu->vcpu_id = 0;
    vcpu->vcpu_fd = ioctl(vmm->vm_fd, KVM_CREATE_VCPU, vcpu->vcpu_id);
    if (vcpu->vcpu_fd < 0)
            die("Can not create vcpu.");

    vcpu->kvm_run_mmap_size =
        ioctl(vmm->kvm_fd, KVM_GET_VCPU_MMAP_SIZE, 0);
    if (vcpu->kvm_run_mmap_size < 0)
            die("Can not get vcpu mmap size.");

    vcpu->kvm_run =
        mmap(NULL, vcpu->kvm_run_mmap_size, PROT_READ | PROT_WRITE,
             MAP_SHARED, vcpu->vcpu_fd, 0);
    if (vcpu->kvm_run == MAP_FAILED)
            die("Can not mmap kvm_run.");

    vcpu->thread_routine = vmm_cpu_thread;
    vmm->vcpus = vcpu;

    return 0;
}

void virtual_machine_manager_run_vm(struct virtual_machine_manager *vmm)
{
    int i = 0;

    for (i = 0; i < vmm->nr_vcpu; i++)
        if (pthread_create
            (&(vmm->vcpus->thread_id), NULL, vmm->vcpus[0].thread_routine,
             vmm) != 0)
            die("Can not create vmm thread.");

    pthread_join(vmm->vcpus->thread_id, NULL);
}

int main(void)
{
    struct virtual_machine_manager *vmm;

    vmm = virtual_machine_manager_init();
    if (!vmm)
            die("Can not initialize virtual machine manager.");

    if (virtual_machine_manager_create_vm(vmm, GUEST_CPU_NUMBER,
                                          GUEST_MEMORY_SIZE,
                                          GUEST_SYSTEM_IMAGE))
            die("Can not create virtual machine.")


    virtual_machine_manager_run_vm(vmm);

    return 0;
}
```

Makefile:
```makefile
all: vmm vm.bin

vmm: vmm.o
        @gcc vmm.c -o vmm -lpthread

vm.bin: vm.o
        @ld -m elf_i386 --oformat binary -N -e _start -Ttext 0x10000 -o vm.bin vm.o

vm.o: vm.S
        @as -32 vm.S -o vm.o

clean:
        @rm -rf vm.o vmm.o vmm vm.bin
```

# 3. Appendix

## 3.1 Architecture of 8086

It generates the 20 bit physical address using Segment(4bits) and Offset addresses(16bits) using the formula:
```
    Physical Address = Segment Address x 10H + Offset Address
```