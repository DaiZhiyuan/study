```sh
#!/bin/bash

/usr/libexec/qemu-kvm -M virt,gic-version=2,accel=tcg -cpu cortex-a57 -smp 8 -m 4096 -drive if=pflash,format=raw,readonly,file=/usr/share/AAVMF/AAVMF_CODE.fd -drive if=pflash,format=raw,file=/usr/share/AAVMF/AAVMF_VARS.fd -hda /root/kvm.qcow2 -cdrom /usr/local/src/CentOS-8.2.2004-aarch64-variant.iso -boot order=cd --nographic
```

dump\_regs.c

```c
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/cputype.h>

static char* comment[7] = {
    "32 bits, 4GB.",
    "36 bits, 64GB.",
    "40 bits, 1TB.",
    "42 bits, 4TB.",
    "44 bits, 16TB.",
    "48 bits, 256TB.",
    "52 bits, 4PB."
};

static int dump_init(void)
{
    u64 mmfr0 = read_cpuid(ID_AA64MMFR0_EL1);
    printk("Physical Address range = %s\n", comment[mmfr0 & 0xf]);
    return 0;
}

static void dump_exit(void)
{
    // nothing
}

module_init(dump_init)
module_exit(dump_exit)
MODULE_LICENSE("GPL");
```
Makefile
```makefile
CONFIG_MODULE_SIG=n

obj-m += dump_regs.o

all:
    make -C /usr/src/kernels/4.19.166-80.el8.aarch64 M=$(PWD) modules

clean:
    make -C /usr/src/kernels/4.19.166-80.el8.aarch64  M=$(PWD) clean
```

