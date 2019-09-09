<!-- TOC -->

- [1. What is vmcall?](#1-what-is-vmcall)
    - [1.1 opcode define](#11-opcode-define)
    - [1.2 kvm_hypercall implement](#12-kvm_hypercall-implement)
    - [1.3 kvm_emulate_hypercall in kernel](#13-kvm_emulate_hypercall-in-kernel)
- [2. implementation step](#2-implementation-step)
- [3. Example](#3-example)
- [4. Runing effect](#4-runing-effect)
    - [4.1 Normal](#41-normal)
    - [4.2 Execption](#42-execption)

<!-- /TOC -->


# 1. What is vmcall?

```
VMCALL op code:0F 01 C1 -- VMCALL Call to VM monitor by causing VM exit.
```

## 1.1 opcode define
arch/x86/include/asm/kvm_para.h：
```
#define KVM_HYPERCALL ".byte 0x0f,0x01,0xc1"
```

## 1.2 kvm_hypercall implement
```c
static inline long kvm_hypercall0(unsigned int nr)
{
    long ret;
    asm volatile(KVM_HYPERCALL
             : "=a"(ret)
             : "a"(nr)
             : "memory");
    return ret;
}
```

## 1.3 kvm_emulate_hypercall in kernel

```c
int kvm_emulate_hypercall(struct kvm_vcpu *vcpu)
{
    nr = kvm_register_read(vcpu, VCPU_REGS_RAX);
    a0 = kvm_register_read(vcpu, VCPU_REGS_RBX);
    a1 = kvm_register_read(vcpu, VCPU_REGS_RCX);
    a2 = kvm_register_read(vcpu, VCPU_REGS_RDX);
    a3 = kvm_register_read(vcpu, VCPU_REGS_RSI);

    if (kvm_x86_ops->get_cpl(vcpu) != 0) {
        ret = -KVM_EPERM;
        goto out;
    }   

    switch (nr) {

        CASE KVM_HC_KICK_CPU: 
            kvm_pv_kick_cpu_op(vcpu->kvm, a0, a1);
            ret = 0;
            break;

        default:
            ret = -KVM_ENOSYS;
            break;
    }
}
```

# 2. implementation step 

1. add Hyperall number. `include/uapi/linux/kvm_para.h`
2. add struct argment define. `arch/x86/include/uapi/asm/kvm_para.h`
3. register hypercall handler. `arch/x86/kvm/x86.c`

# 3. Example

0001-PCM-Add-Hypercall-to-get-PCM-information.patch
```patch
From 11e7626347d1f68d32a8ae45e892cb79d0698085 Mon Sep 17 00:00:00 2001
From: Daizhiyuan <daizhiyuan@tsinghua.edu.cn>
Date: Fri, 16 Aug 2019 09:54:13 +0800
Subject: [PATCH] PCM: Add Hypercall to get PCM information

---
 arch/x86/include/uapi/asm/kvm_para.h |  6 ++++++
 arch/x86/kvm/x86.c                   | 28 ++++++++++++++++++++++++++++
 include/uapi/linux/kvm_para.h        |  1 +
 3 files changed, 35 insertions(+)

diff --git a/arch/x86/include/uapi/asm/kvm_para.h b/arch/x86/include/uapi/asm/kvm_para.h
index 23f966f..c8a6d58 100644
--- a/arch/x86/include/uapi/asm/kvm_para.h
+++ b/arch/x86/include/uapi/asm/kvm_para.h
@@ -3,6 +3,7 @@

 #include <linux/types.h>
 #include <asm/hyperv.h>
+#include <cicada/pcm_stats.h>

 /* This CPUID returns the signature 'KVMKVMKVM' in ebx, ecx, and edx.  It
  * should be used to determine that a VM is running under KVM.
@@ -106,5 +107,10 @@ struct kvm_vcpu_pv_apf_data {
 #define KVM_PV_EOI_ENABLED KVM_PV_EOI_MASK
 #define KVM_PV_EOI_DISABLED 0x0

+struct pcm_statistics_info {
+    struct pcm_sampling_statistics sample_info;
+    struct pcm_replay_stats replay_info;
+    struct pcm_event_stats  event_info;
+};

 #endif /* _UAPI_ASM_X86_KVM_PARA_H */
diff --git a/arch/x86/kvm/x86.c b/arch/x86/kvm/x86.c
index a93c0a8..0b1a2eb 100644
--- a/arch/x86/kvm/x86.c
+++ b/arch/x86/kvm/x86.c
@@ -148,6 +148,10 @@ static bool __read_mostly backwards_tsc_observed = false;
 #ifdef CONFIG_KVM_CICADA
 unsigned int max_prefault_read = 204800;
 module_param(max_prefault_read, uint, S_IRUGO | S_IWUSR);
+
+extern struct pcm_sampling_statistics pcm_sampling_stats;
+extern struct pcm_replay_stats pcm_replay_stats;
+extern struct pcm_event_stats pcm_event_stats;
 #endif

 #define KVM_NR_SHARED_MSRS 16
@@ -6697,6 +6701,21 @@ void kvm_vcpu_deactivate_apicv(struct kvm_vcpu *vcpu)
        kvm_x86_ops->refresh_apicv_exec_ctrl(vcpu);
 }

+static int kvm_pv_get_pcm_info(struct kvm_vcpu *vcpu, gpa_t paddr)
+{
+    int ret = 0;
+    struct pcm_statistics_info info;
+
+    memcpy(&info.sample_info, &pcm_sampling_stats, sizeof(info.sample_info));
+    memcpy(&info.replay_info, &pcm_replay_stats, sizeof(info.replay_info));
+    memcpy(&info.event_info, &pcm_event_stats, sizeof(info.event_info));
+
+    if (kvm_write_guest(vcpu->kvm, paddr, &info, sizeof(struct pcm_statistics_info)))
+        ret = -KVM_EFAULT;
+
+    return ret;
+}
+
 int kvm_emulate_hypercall(struct kvm_vcpu *vcpu)
 {
        unsigned long nr, a0, a1, a2, a3, ret;
@@ -6774,6 +6793,15 @@ int kvm_emulate_hypercall(struct kvm_vcpu *vcpu)
                }
                ret = 0;
                break;
+
+    case PCM_HC_GET_PCM_INFO:
+        if (!pcm_hypercall) {
+            ret = -KVM_ENOSYS;
+            break;
+        }
+
+        ret = kvm_pv_get_pcm_info(vcpu, a0);
+        break;
 #endif
 #ifdef CONFIG_KVM_CICADA_PMU_BRANCH_VERIFY
        case PCM_HC_PMU_START_BRANCH:   /* start pmu branch counter */
diff --git a/include/uapi/linux/kvm_para.h b/include/uapi/linux/kvm_para.h
index cf4c4b8..17914b0 100644
--- a/include/uapi/linux/kvm_para.h
+++ b/include/uapi/linux/kvm_para.h
@@ -31,6 +31,7 @@
 #define PCM_HC_PMU_STOP_BRANCH    101
 #define PCM_HC_START_SAMPLING     102
 #define PCM_HC_STOP_SAMPLING      103
+#define PCM_HC_GET_PCM_INFO       104
 #endif

 /*
--
1.8.3.1
```

guest_aware.c

```
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/miscdevice.h>

#define GUEST_AWARE_NAME        "guest-aware"
#define PCM_HC_GET_PCM_INFO     104
#define KVM_HYPERCALL           ".byte 0x0f,0x01,0xc1"

static inline long kvm_hypercall(unsigned int nr, unsigned long args)
{
    long ret;

    __asm__ __volatile__ (KVM_HYPERCALL
        : "=a"(ret)
        : "a"(nr), "b"(args)
        : "memory");

    return ret;
}

struct pcm_sampling_statistics {
    unsigned long smi_fired;
    unsigned long buf_overflow;
    unsigned long error_occurred;
    unsigned long itr_overflow;
    unsigned long start_sample_hw_failure;
    unsigned long stop_sample_hw_failure;
    unsigned long send_payload_failure;
};

struct pcm_replay_stats {
    unsigned long success;
    unsigned long failed;
    unsigned long dropped;
};

struct pcm_event_stats {
    unsigned long evt_dma_memory_write;
    unsigned long evt_dma_memory_read;
    unsigned long evt_external_interrupt;
    unsigned long evt_switch_exec_cpu;
    unsigned long evt_rflags_change;
    unsigned long evt_itr_dma;
    unsigned long evt_itr_p2p;
    unsigned long evt_mmio_compl;
    unsigned long evt_itr_dq;
    unsigned long log_exception_pagefault;
    unsigned long log_ins_exec_rdtsc;
    unsigned long log_ins_exec_rdtscp;
    unsigned long log_ins_exec_rdpmc;
    unsigned long log_ins_exec_rdseed;
    unsigned long log_ins_exec_pio_in;
    unsigned long log_ins_exec_pio_out;
    unsigned long log_ins_exec_mmio_in;
    unsigned long log_ins_exec_mmio_out;
    unsigned long log_ins_vmcall;
    unsigned long log_ins_cpuid_rd;
    unsigned long log_ins_msr_read;
    unsigned long log_ins_msr_write;
    unsigned long log_infield_debug;
    unsigned long replay_abort;
    unsigned long replay_end;
    unsigned long log_oss_memmap_change;
};

struct pcm_statistics_info {
    struct pcm_sampling_statistics sample_info;
    struct pcm_replay_stats replay_info;
    struct pcm_event_stats  event_info;
};

static int guest_aware_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int guest_aware_close(struct inode *inode, struct file *file)
{
    return 0;
}

static long guest_aware_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

#define GUEST_AWARE_MAGIC       'Z'
#define GUEST_IOCTL_GET_PCM_INFO   _IOWR(GUEST_AWARE_MAGIC, 0x01, struct pcm_statistics_info)
    switch (cmd) {
        case GUEST_IOCTL_GET_PCM_INFO:
        {
            struct pcm_statistics_info *stat;
            stat = kmalloc(sizeof(struct pcm_statistics_info), GFP_KERNEL);
            if (kvm_hypercall(PCM_HC_GET_PCM_INFO, virt_to_phys(stat))) {
                kfree(stat);
                return -EFAULT;
            }

            if (copy_to_user((struct pcm_statistics_info*)arg, stat,\
                            sizeof(struct pcm_statistics_info))) {
                kfree(stat);
                return -EFAULT;
            }

            kfree(stat);
            return 0;
        }

        default:
            return -EINVAL;
    }
}

static const struct file_operations guest_aware_fops = {
    .owner = THIS_MODULE,
    .open = guest_aware_open,
    .release = guest_aware_close,
    .unlocked_ioctl = guest_aware_ioctl,
};

static struct miscdevice guest_aware_misc = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = GUEST_AWARE_NAME,
    .fops  = &guest_aware_fops,
};

static int __init guest_aware_init(void)
{
    pr_info("guest_aware_init.\n");

    if (misc_register(&guest_aware_misc)) {
        pr_err("[GuestAware]: cannot regsiter misc device.");
        return -EINVAL;
    }

    return 0;
}

static void __exit guest_aware_exit(void)
{
    pr_info("guest_aware_exit.\n");

    if (misc_deregister(&guest_aware_misc)) {
        pr_err("[GuestAware]: cannot unregsiter misc device.");
    }

    return;
}

module_init(guest_aware_init);
module_exit(guest_aware_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry Dai <daizhiyuan@tsinghua.edu.cn>");
MODULE_DESCRIPTION("Guest enhancements tools");
MODULE_VERSION("1.0");
```

dpi.c
```
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/ioctl.h>

#define GUEST_AWARE_DEV         "/dev/guest-aware"
#define GUEST_AWARE_NAME        "display_pcm_infomation"
#define GUEST_AWARE_VERSION     "1.0"
#define RUN_CASE(module)        module(),exit(0)

static struct pcm_statistics_info g_stat;

struct pcm_sampling_statistics {
    unsigned long smi_fired;
    unsigned long buf_overflow;
    unsigned long error_occurred;
    unsigned long itr_overflow;
    unsigned long start_sample_hw_failure;
    unsigned long stop_sample_hw_failure;
    unsigned long send_payload_failure;
};

struct pcm_replay_stats {
    unsigned long success;
    unsigned long failed;
    unsigned long dropped;
};

struct pcm_event_stats {
    unsigned long evt_dma_memory_write;
    unsigned long evt_dma_memory_read;
    unsigned long evt_external_interrupt;
    unsigned long evt_switch_exec_cpu;
    unsigned long evt_rflags_change;
    unsigned long evt_itr_dma;
    unsigned long evt_itr_p2p;
    unsigned long evt_mmio_compl;
    unsigned long evt_itr_dq;
    unsigned long log_exception_pagefault;
    unsigned long log_ins_exec_rdtsc;
    unsigned long log_ins_exec_rdtscp;
    unsigned long log_ins_exec_rdpmc;
    unsigned long log_ins_exec_rdseed;
    unsigned long log_ins_exec_pio_in;
    unsigned long log_ins_exec_pio_out;
    unsigned long log_ins_exec_mmio_in;
    unsigned long log_ins_exec_mmio_out;
    unsigned long log_ins_vmcall;
    unsigned long log_ins_cpuid_rd;
    unsigned long log_ins_msr_read;
    unsigned long log_ins_msr_write;
    unsigned long log_infield_debug;
    unsigned long replay_abort;
    unsigned long replay_end;
    unsigned long log_oss_memmap_change;
};

struct pcm_statistics_info {
    struct pcm_sampling_statistics sample_info;
    struct pcm_replay_stats replay_info;
    struct pcm_event_stats  event_info;
};

void get_pcm_info(void)
{

#define GUEST_AWARE_MAGIC       'Z'
#define GUEST_IOCTL_GET_PCM_INFO   _IOWR(GUEST_AWARE_MAGIC, 0x01, struct pcm_statistics_info)

    int ret, fd;

    fd = open(GUEST_AWARE_DEV, O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "Try run \"modprobe gustaware\"\n");
        exit(errno);

    }

    ret = ioctl(fd, GUEST_IOCTL_GET_PCM_INFO, &g_stat);
    if (ret < 0 ) {
        if (errno == EFAULT)
            fprintf(stderr, "PCM hypercall is disable.\n"), exit(errno);
        else
            perror("ioctl()"), exit(errno);
    }

    close(fd);
}

void display_pcm_sample(void)
{
        printf("        ****** display pcm sample ******\n");
        printf("              smi_fired : %lu\n", g_stat.sample_info.smi_fired);
        printf("           buf_overflow : %lu\n", g_stat.sample_info.buf_overflow);
        printf("         error_occurred : %lu\n", g_stat.sample_info.error_occurred);
        printf("           itr_overflow : %lu\n", g_stat.sample_info.itr_overflow);
        printf("start_sample_hw_failure : %lu\n", g_stat.sample_info.start_sample_hw_failure);
        printf(" stop_sample_hw_failure : %lu\n", g_stat.sample_info.stop_sample_hw_failure);
        printf("   send_payload_failure : %lu\n", g_stat.sample_info.send_payload_failure);
}

void display_pcm_replay(void)
{
        printf("        ****** display pcm replay ******\n");
        printf("                success : %lu\n", g_stat.replay_info.success);
        printf("                 failed : %lu\n", g_stat.replay_info.failed);
        printf("                dropped : %lu\n", g_stat.replay_info.dropped);

}

void display_pcm_event(void)
{
        printf("        ****** display pcm event ******* \n");
        printf("   evt_dma_memory_write : %lu\n",  g_stat.event_info.evt_dma_memory_write);
        printf("    evt_dma_memory_read : %lu\n",  g_stat.event_info.evt_dma_memory_read);
        printf(" evt_external_interrupt : %lu\n",  g_stat.event_info.evt_external_interrupt);
        printf("    evt_switch_exec_cpu : %lu\n",  g_stat.event_info.evt_switch_exec_cpu);
        printf("      evt_rflags_change : %lu\n",  g_stat.event_info.evt_rflags_change);
        printf("            evt_itr_dma : %lu\n",  g_stat.event_info.evt_itr_dma);
        printf("            evt_itr_p2p : %lu\n",  g_stat.event_info.evt_itr_p2p);
        printf("         evt_mmio_compl : %lu\n",  g_stat.event_info.evt_mmio_compl);
        printf("             evt_itr_dq : %lu\n",  g_stat.event_info.evt_itr_dq);
        printf("log_exception_pagefault : %lu\n",  g_stat.event_info.log_exception_pagefault);
        printf("     log_ins_exec_rdtsc : %lu\n",  g_stat.event_info.log_ins_exec_rdtsc);
        printf("    log_ins_exec_rdtscp : %lu\n",  g_stat.event_info.log_ins_exec_rdtscp);
        printf("     log_ins_exec_rdpmc : %lu\n",  g_stat.event_info.log_ins_exec_rdpmc);
        printf("    log_ins_exec_rdseed : %lu\n",  g_stat.event_info.log_ins_exec_rdseed);
        printf("    log_ins_exec_pio_in : %lu\n",  g_stat.event_info.log_ins_exec_pio_in);
        printf("   log_ins_exec_pio_out : %lu\n",  g_stat.event_info.log_ins_exec_pio_out);
        printf("   log_ins_exec_mmio_in : %lu\n",  g_stat.event_info.log_ins_exec_mmio_in);
        printf("  log_ins_exec_mmio_out : %lu\n",  g_stat.event_info.log_ins_exec_mmio_out);
        printf("         log_ins_vmcall : %lu\n",  g_stat.event_info.log_ins_vmcall);
        printf("       log_ins_cpuid_rd : %lu\n",  g_stat.event_info.log_ins_cpuid_rd);
        printf("       log_ins_msr_read : %lu\n",  g_stat.event_info.log_ins_msr_read);
        printf("      log_ins_msr_write : %lu\n",  g_stat.event_info.log_ins_msr_write);
        printf("      log_infield_debug : %lu\n",  g_stat.event_info.log_infield_debug);
        printf("           replay_abort : %lu\n",  g_stat.event_info.replay_abort);
        printf("             replay_end : %lu\n",  g_stat.event_info.replay_end);
        printf("  log_oss_memmap_change : %lu\n",  g_stat.event_info.log_oss_memmap_change);
}

void display_all_info(void)
{
        display_pcm_sample();
        printf("\n");
        display_pcm_replay();
        printf("\n");
        display_pcm_event();
        printf("\n");
}

static inline void usage(void)
{
       fprintf(stderr,
"Usage: %s [OPTION]\n\n"
"Get PCM information.\n\n"
"    -v : version\n"
"    -h : help\n"
"    -s : display PCM sampling\n"
"    -r : display PCM replay\n"
"    -e : display PCM event\n"
"    -a : display PCM all information (defalut)\n\n"
"Report bugs to <daizhiyuan@tsinghua.edu.cn>.\n"
, GUEST_AWARE_NAME);
}

static inline void version(void)
{
       fprintf(stderr,
"%s (v%s) Jerry Dai <daizhiyuan@tsinghua.edu.cn>\n\n"
"This program is free software; you can redistribute it and/or\n"
"modify it under the terms of the GNU General Public License as\n"
"published by the Free Software Foundation; version 2 of the\n"
"License.\n\n", GUEST_AWARE_NAME, GUEST_AWARE_VERSION);
}


int main(int argc, char **argv)
{
    int opt;

    get_pcm_info();

    while ((opt = getopt(argc,argv,"vhsrea")) != -1) {
        switch(opt) {
            case 'v': RUN_CASE(version);
            case 'h': RUN_CASE(usage);
            case 's': RUN_CASE(display_pcm_sample);
            case 'r': RUN_CASE(display_pcm_replay);
            case 'e': RUN_CASE(display_pcm_event);
            case 'a': RUN_CASE(display_all_info);
        }
        exit(fprintf(stderr, "usage %s -h for help!\n", GUEST_AWARE_NAME));
    } RUN_CASE(display_all_info);

    return 0;
}
```

# 4. Runing effect

## 4.1 Normal
Host kernel cmdline:
```bash
[root@localhost ~]# cat /proc/cmdline
BOOT_IMAGE=/vmlinuz-3.10.0-693.24.el7.tsinghua_aware.x86_64 root=LABEL=Primary-root ro memmap=0x4000000$0x1c0000000 
pcm=0xa7d0000000@0x1870000000,0xa800000000@0xd840000000 mtr_init=proc pcm_sample_window=100000 pcmtest=all-replay intel_iommu=on intremap=off crashkernel=auto rhgb quiet loglevel=1 pcm_v2_payload module_blacklist=be2net,bnxt_en,mlx5_core,igb,i40e,qla2xxx,ixgbe,mpt3sas
```

testcase:
```
[root@localhost userland]# ./gpi
Try run "modprobe gustaware"

[root@localhost kernel]# insmod ./guest_aware.ko
[root@localhost Kernel]# dmesg
[  493.639180] guest_aware: loading out-of-tree module taints kernel.
[  493.639259] guest_aware: module verification failed: signature and/or required key missing - tainting kernel
[  493.642084] guest_aware_init.

[root@localhost userland]# ./gpi -v
display_pcm_infomation (v1.0) Jerry Dai <daizhiyuan@tsinghua.edu.cn>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; version 2 of the
License.

[root@localhost userland]# ./gpi -h
Usage: display_pcm_infomation [OPTION]

Get PCM information.

    -v : version
    -h : help
    -s : display PCM sampling
    -r : display PCM replay
    -e : display PCM event
    -a : display PCM all information (defalut)

Report bugs to <daizhiyuan@tsinghua.edu.cn>.

[root@localhost userland]# ./gpi -s
        ****** display pcm sample ******
              smi_fired : 0
           buf_overflow : 0
         error_occurred : 0
           itr_overflow : 5
start_sample_hw_failure : 0
 stop_sample_hw_failure : 0
   send_payload_failure : 0

[root@localhost userland]# ./gpi -r
        ****** display pcm replay ******
                success : 484
                 failed : 0
                dropped : 0

[root@localhost userland]# ./gpi -e
        ****** display pcm event *******
   evt_dma_memory_write : 0
    evt_dma_memory_read : 0
 evt_external_interrupt : 11
    evt_switch_exec_cpu : 0
      evt_rflags_change : 526
            evt_itr_dma : 94
            evt_itr_p2p : 0
         evt_mmio_compl : 0
             evt_itr_dq : 0
log_exception_pagefault : 0
     log_ins_exec_rdtsc : 9
    log_ins_exec_rdtscp : 0
     log_ins_exec_rdpmc : 0
    log_ins_exec_rdseed : 0
    log_ins_exec_pio_in : 28
   log_ins_exec_pio_out : 0
   log_ins_exec_mmio_in : 3
  log_ins_exec_mmio_out : 0
         log_ins_vmcall : 0
       log_ins_cpuid_rd : 2
       log_ins_msr_read : 0
      log_ins_msr_write : 0
      log_infield_debug : 549
           replay_abort : 0
             replay_end : 512
  log_oss_memmap_change : 0

[root@localhost userland]# ./gpi -a
        ****** display pcm sample ******
              smi_fired : 0
           buf_overflow : 0
         error_occurred : 0
           itr_overflow : 5
start_sample_hw_failure : 0
 stop_sample_hw_failure : 0
   send_payload_failure : 0

        ****** display pcm replay ******
                success : 532
                 failed : 0
                dropped : 0

        ****** display pcm event *******
   evt_dma_memory_write : 0
    evt_dma_memory_read : 0
 evt_external_interrupt : 11
    evt_switch_exec_cpu : 0
      evt_rflags_change : 547
            evt_itr_dma : 94
            evt_itr_p2p : 0
         evt_mmio_compl : 0
             evt_itr_dq : 0
log_exception_pagefault : 0
     log_ins_exec_rdtsc : 9
    log_ins_exec_rdtscp : 0
     log_ins_exec_rdpmc : 0
    log_ins_exec_rdseed : 0
    log_ins_exec_pio_in : 28
   log_ins_exec_pio_out : 0
   log_ins_exec_mmio_in : 3
  log_ins_exec_mmio_out : 0
         log_ins_vmcall : 0
       log_ins_cpuid_rd : 2
       log_ins_msr_read : 0
      log_ins_msr_write : 0
      log_infield_debug : 570
           replay_abort : 0
             replay_end : 533
  log_oss_memmap_change : 0
```

## 4.2 Execption

Host kernel args: add pcm_hypercall=off (defalut on)

Host kernel cmdline:
```bash
[root@localhost ~]# cat /proc/cmdline
BOOT_IMAGE=/vmlinuz-3.10.0-693.24.el7.tsinghua_aware.x86_64 root=LABEL=Primary-root ro memmap=0x4000000$0x1c0000000 
pcm=0xa7d0000000@0x1870000000,0xa800000000@0xd840000000 mtr_init=proc pcm_sample_window=100000 pcmtest=all-replay intel_iommu=on intremap=off crashkernel=auto rhgb quiet loglevel=1 pcm_v2_payload module_blacklist=be2net,bnxt_en,mlx5_core,igb,i40e,qla2xxx,ixgbe,mpt3sas pcm_hypercall=off
```

testcase:
```
[root@localhost userland]# ./gpi
Try run "modprobe gustaware"

[root@localhost kernel]# insmod ./guest_aware.ko
[root@localhost Kernel]# dmesg
[  493.639180] guest_aware: loading out-of-tree module taints kernel.
[  493.639259] guest_aware: module verification failed: signature and/or required key missing - tainting kernel
[  493.642084] guest_aware_init.

[root@localhost userland]# ./gpi
PCM hypercall is disable.
```
