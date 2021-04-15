# 1. ARMV8 virtualization related registers

The registers related to virtualization in Armv8 are included in the list below.

Register | Types of | Length | Description
---|---|---|---
VPIDR_EL2 | RW | 64 | `V`irtualization `P`rocessor `ID` `R`egister
VMPIDR_EL2 | RW | 64 | `V`irtualization `M`ultiprocessor `ID` `R`egister
SCTLR_EL2 | RW |64 | `S`ystem `C`on`t`ro`l` `R`egister (EL2)
HCR_EL2 | RW | 64 | `H`ypervisor `C`onfiguration `R`egister
MDCR_EL2 | RW |64 | `M`onitor `D`ebug `C`onfiguration `R`egister (EL2)
CPTR_EL2 | RW | 64 | Architectural Feature `T`rap `R`egister (EL2)
HSTR_EL2 | RW | 64 | `H`ypervisor `S`ystem `T`rap `R`egister
HACR_EL2 | RW | 64 | `H`ypervisor `A`uxiliary `C`ontrol `R`egister
TTBR0_EL2 | RW | 64 | `T`ranslation `T`able `B`ase `R`egister `0` (EL2)
TCR_EL2 | RW | 64 |  `T`ranslation `C`ontrol `R`egister (EL2)
VTTBR_EL2 | RW | 64 |  `V`irtualization `T`ranslation `T`able `B`ase `R`egister
VTCR_EL2 | RW | 64 | `V`irtualization `T`ranslation `C`ontrol `R`egister
DACR32_EL2 | RW | 64 | `D`omain `A`ccess `C`ontrol `R`egister
AFSR0_EL2 | RW | 64 | `A`uxiliary `F`ault `S`tatus `R`egister `0` (EL2)
AFSR1_EL2 | RW | 64 | `A`uxiliary `F`ault `S`tatus `R`egister `1` (EL2)
ESR_EL2| RW | 64 | `E`xception `S`yndrome `R`egister (EL2)
FAR_EL2| RW | 64 | `F`ault `A`ddress `R`egister (EL2)
HPFAR_EL2 | RW | 64 | `H`ypervisor IPA `F`ault `A`ddress `R`egister
AMAIR_EL2 | RW | 64 | `A`uxiliary `M`emory `A`ttribute `I`ndirection `R`egister (EL2)
VBAR_EL2 | RW | 64 | `V`ector `B`ase `A`ddress `R`egister (EL2)

## VPIDR_EL2 && VMPIDR_EL2

VPIDR_EL2: 
- Bits [63:32]
- `Implementer`, bits [31:24]
- `Variant`, bits [23:20]
- `Architecture`, bits [19:16]
- `PartNum`, bits [15:4]
- `Revision`, bits [3:0]

VMPIDR_EL2:
- Bits [63:40]
- `Aff3`, bits [39:32]
- Bit [31]
- `U`, bit [30]
- Bits [29:25]
- `MT`, bit [24]
- `Aff2`, bits [23:16]
- `Aff1`, bits [15:8]
- `Aff0`, bits [7:0]

include/asm/el2_setup.h
```
/* Virtual CPU ID registers */
.macro __init_el2_nvhe_idregs
    mrs x0, midr_el1
    mrs x1, mpidr_el1
    msr vpidr_el2, x0
    msr vmpidr_el2, x1
.endm
```

## SCTLR_EL2

SCTLR_EL2:
...
- SA, bit [3]
- C, bit [2]
- A, bit [1]
- M, bit [0]

include/asm/el2_setup.h
```
.macro __init_el2_sctlr
    mov_q   x0, INIT_SCTLR_EL2_MMU_OFF
    msr sctlr_el2, x0
    isb
.endm
```

## HCR_EL2

为虚拟化提供配置控制，包括定义各种操作是否被捕获到EL2中。

- TPCP, bit [23]
- TSW, bit [22]
- TACR, bit [21]
- TIDCP, bit [20]
- TSC, bit [19]
- TID3, bit [18]
- TID2, bit [17]
- TID1, bit [16]
- TID0, bit [15]
- TWE, bit [14]
- TWI, bit [13]
- DC, bit [12]
- BSU, bits [11:10]
- FB, bit [9]
- VSE, bit [8]
- VI, bit [7]
- VF, bit [6]
- AMO, bit [5]
- IMO, bit [4]
- FMO, bit [3]
- PTW, bit [2]
- SWIO, bit [1]
- VM, bit [0]

### kvm_vcpu_fault_info

```c
struct kvm_vcpu_fault_info {
    u32 esr_el2;        /* Hyp Syndrom Register */
    u64 far_el2;        /* Hyp Fault Address Register */
    u64 hpfar_el2;      /* Hyp IPA Fault Address Register */
    u64 disr_el1;       /* Deferred [SError] Status Register */
};
```
ESR_EL2:
- Bits [63:32] = 0
- EC, bits [31:26] = Exception Class.
- IL, bit [25] = Instruction Length.
- ISS, bits [24:0] = Instruction Specific Syndrome.

FAR_EL2:
- Bits [63:0] = Faulting Virtual Address for synchronous exceptions taken to EL2.

FPFAR_EL2: 
- NS, bit [63]
- Bits [62:44] = 0
- FIPA[51:48], bits [43:40]
- FIPA[47:12], bits [39:4]
- Bits [3:0] = 0

DISR_EL1:
- Bits [63:32] = 0
- A, bit [31] = Set to 1 when an ESB instruction defers an asynchronous SError interrupt.
- Bits [30:25] = 0
- IDS, bit [24] = Indicates the deferred SError interrupt type.
- Bits [23:13] = 0
- AET, bits [12:10] = Asynchronous Error Type.
- EA, bit [9] = External abort Type.
- Bits [8:6] = 0
- DFSC, bits [5:0] = Fault Status Code.


# VGIC

kvm_arch_init
    init_subsystems
        kvm_vgic_hyp_init
            vgic_v3_probe
                kvm_register_vgic_device
                    kvm_register_device_ops
                    kvm_vgic_register_its_device


## KVM vs QEMU

QEMU_CMDLINE:
```
qemu-kvm -name guest=example,debug-threads=on 
    -machine virt-4.0,accel=kvm,usb=off,dump-guest-core=off,gic-version=3 
    -cpu host -smp 4,sockets=4,cores=1,threads=1
    -drive file=/usr/share/edk2/aarch64/QEMU_EFI-pflash.raw,if=pflash,format=raw,unit=0,readonly=on 
    -drive file=/var/lib/libvirt/qemu/nvram/example_VARS.fd,if=pflash,foqrmat=raw,unit=1 
    -m 16384 
    -overcommit mem-lock=off  

-device pcie-root-port,port=0x8,chassis=1,id=pci.1,bus=pcie.0,multifunction=on,addr=0x1 [Red Hat, Inc. QEMU PCIe Host bridge] <Speed 16GT/s,Width x32>
-device pcie-root-port,port=0x9,chassis=2,id=pci.2,bus=pcie.0,addr=0x1.0x1 [Virtio: Virtio network device] <Speed 16GT/s, Width x32>
-device pcie-root-port,port=0xa,chassis=3,id=pci.3,bus=pcie.0,addr=0x1.0x2 [Red Hat, Inc. QEMU XHCI Host Controller] <Speed 16GT/s, Width x32>
-device pcie-root-port,port=0xb,chassis=4,id=pci.4,bus=pcie.0,addr=0x1.0x3 [Virtio: Virtio SCSI] <Speed 16GT/s, Width x32 >
-device pcie-root-port,port=0xc,chassis=5,id=pci.5,bus=pcie.0,addr=0x1.0x4 [Virtio: Virtio console] <Speed 16GT/s, Width x32>
-device pcie-root-port,port=0xd,chassis=6,id=pci.6,bus=pcie.0,addr=0x1.0x5 [Virtio: Virtio block device] <Speed 16GT/s, Width x32>
```

## Exit status

/sys/kernel/debug/kvm
```
#> cat mmio_exit_kernel
3444931
#> cat mmio_exit_user
446488
#> cat wfe_exit_stat
3939831
#> cat wfi_exit_stat
693175646
#> cat hvc_exit_stat
176
#>  cat exits
1191370759
```

vgic-state
```
Distributor
===========
vgic_model:     GICv3
nr_spis:        256
nr_lpis:        33
enabled:        1

P=pending_latch, L=line_level, A=active
E=enabled, H=hw, C=config (level=1, edge=0)
G=group

VCPU 0 TYP   ID TGT_ID PLAEHCG     HWID   TARGET SRC PRI VCPU_ID
----------------------------------------------------------------
       SGI    0      0 0001001        0        0   0 160      -1
       SGI    1      0 0001001        0        0   0 160      -1
       SGI    2      0 0001001        0        0   0 160      -1
       SGI    3      0 0001001        0        0   0 160      -1
       SGI    4      0 0001001        0        0   0 160      -1
       SGI    5      0 0001001        0        0   0 160      -1
       SGI    6      0 0001001        0        0   0 160      -1
       SGI    7      0 0001001        0        0   0 160      -1
       SGI    8      0 0001001        0        0   0 160      -1
       SGI    9      0 0001001        0        0   0 160      -1
       SGI   10      0 0001001        0        0   0 160      -1
       SGI   11      0 0001001        0        0   0 160      -1
       SGI   12      0 0001001        0        0   0 160      -1
       SGI   13      0 0001001        0        0   0 160      -1
       SGI   14      0 0001001        0        0   0 160      -1
       SGI   15      0 0001001        0        0   0 160      -1
       PPI   16      0 0000011        0        0   0 160      -1
       PPI   17      0 0000011        0        0   0 160      -1
       PPI   18      0 0000011        0        0   0 160      -1
       PPI   19      0 0000011        0        0   0 160      -1
       PPI   20      0 0000011        0        0   0 160      -1
       PPI   21      0 0000011        0        0   0 160      -1
       PPI   22      0 0000011        0        0   0 160      -1
       PPI   23      0 0001011        0        0   0 160      -1
       PPI   24      0 0000011        0        0   0 160      -1
       PPI   25      0 0000011        0        0   0 160      -1
       PPI   26      0 0000011        0        0   0 160      -1
       PPI   27      0 0001111       27        0   0 160      -1
       PPI   28      0 0000011        0        0   0 160      -1
       PPI   29      0 0000011        0        0   0 160      -1
       PPI   30      0 0000011        0        0   0 160      -1
       PPI   31      0 0000011        0        0   0 160      -1
```

GuestOS Interrupt:
```
[root@test77 ~]# cat /proc/interrupts
           CPU0       CPU1       CPU2       CPU3
  2:   10943457   10847070   10958236   10124633     GICv3  27 Level     arch_timer
  4:          0          0          0         34     GICv3  33 Level     uart-pl011
 42:          0          0          0          0     GICv3  23 Level     arm-pmu
 43:          0          0          0          0  ARMH0061:00   3 Edge      ACPI:Event
 44:          1          0          0          0   ITS-MSI 16384 Edge      PCIe PME, pciehp
 45:          1          0          0          0   ITS-MSI 18432 Edge      PCIe PME, pciehp
 46:          1          0          0          0   ITS-MSI 20480 Edge      PCIe PME, pciehp
 47:          1          0          0          0   ITS-MSI 22528 Edge      PCIe PME, pciehp
 48:          1          0          0          0   ITS-MSI 24576 Edge      PCIe PME, pciehp
 49:          7          0          0          0   ITS-MSI 26624 Edge      PCIe PME, pciehp
 50:          0          0          0          0   ITS-MSI 2621440 Edge      virtio3-config
 51:     140726          0          0          0   ITS-MSI 2621441 Edge      virtio3-req.0
 52:          0          0          0          0   ITS-MSI 1048576 Edge      xhci_hcd
 53:          0          0          0          0   ITS-MSI 1048577 Edge      xhci_hcd
 54:          0          0          0          0   ITS-MSI 1048578 Edge      xhci_hcd
 55:          0          0          0          0   ITS-MSI 1048579 Edge      xhci_hcd
 56:          0          0          0          0   ITS-MSI 1048580 Edge      xhci_hcd
 57:          0          0          0          0   ITS-MSI 524288 Edge      virtio0-config
 58:          2          0     713983          0   ITS-MSI 524289 Edge      virtio0-input.0
 59:          1          0          0          0   ITS-MSI 524290 Edge      virtio0-output.0
 60:          0          0          0          0   ITS-MSI 2097152 Edge      virtio2-config
 61:         13          0          0          0   ITS-MSI 2097153 Edge      virtio2-virtqueues
 62:          0          0          0          0   ITS-MSI 1572864 Edge      virtio1-config
 63:          0          0          0          0   ITS-MSI 1572865 Edge      virtio1-control
 64:          0          0          0          0   ITS-MSI 1572866 Edge      virtio1-event
 65:     329396          0          0          0   ITS-MSI 1572867 Edge      virtio1-request
IPI0:  18801217   18797238   17781347   17210721       Rescheduling interrupts
IPI1:       215      51588      31205      37345       Function call interrupts
IPI2:         0          0          0          0       CPU stop interrupts
IPI3:         0          0          0          0       CPU stop (for crash dump) interrupts
IPI4:         0          0          0          0       Timer broadcast interrupts
IPI5:         0          0          0          0       IRQ work interrupts
IPI6:         0          0          0          0       CPU wake-up interrupts
IPI7:         0          0          0          0       backtrace interrupts
Err:          0
```


GuestOS Boot:
```
arch_timer: cp15 timer(s) running at 50.00MHz (virt).
CPU: All CPU(s) started at EL1
kvm [1]: HYP mode not available
```
HostOS Boot:
```
arch_timer: cp15 timer(s) running at 50.00MHz (phys).
CPU: All CPU(s) started at EL2
kvm [1]: 8-bit VMID
kvm [1]: IPA Size Limit: 44bits
kvm [1]: vgic-v2@80029c20000
kvm [1]: GIC system register CPU interface enabled
kvm [1]: vgic interrupt IRQ1
kvm [1]: Hyp mode initialized successfully
```

Guest OS a_inode:
```
/dev/kvm
    kvm-vm
        kvm-vcpu:0
        kvm-vcpu:1
        kvm-vcpu:2
        kvm-vcpu:3
        kvm-arm-vgic-its
```

kvm-vm: IOCTL
```c
long kvm_arch_vm_ioctl(struct file *filp,
               unsigned int ioctl, unsigned long arg)
{
    struct kvm *kvm = filp->private_data;
    void __user *argp = (void __user *)arg;

    switch (ioctl) {
    case KVM_CREATE_IRQCHIP: {
        int ret;
        if (!vgic_present)
            return -ENXIO;
        mutex_lock(&kvm->lock);
        ret = kvm_vgic_create(kvm, KVM_DEV_TYPE_ARM_VGIC_V2);
        mutex_unlock(&kvm->lock);
        return ret;
    }
    case KVM_ARM_SET_DEVICE_ADDR: {
        struct kvm_arm_device_addr dev_addr;

        if (copy_from_user(&dev_addr, argp, sizeof(dev_addr)))
            return -EFAULT;
        return kvm_vm_ioctl_set_device_addr(kvm, &dev_addr);
    }
    case KVM_ARM_PREFERRED_TARGET: {
        int err;
        struct kvm_vcpu_init init;

        err = kvm_vcpu_preferred_target(&init);
        if (err)
            return err;

        if (copy_to_user(argp, &init, sizeof(init)))
            return -EFAULT;

        return 0;
    }
    default:
        return -EINVAL;
    }
}
```
> VGIC_V3_MAX_CPUS    512       VGIC_V2_MAX_CPUS    8

cond_resched

Receive Packet Steering (RPS), Transmit Packet Steering (XPS), and IRQ affinity.

wire speed

[root@node1 ~]# ethtool -T eth1
Time stamping parameters for eth1:
Capabilities:
        hardware-transmit     (SOF_TIMESTAMPING_TX_HARDWARE)
        software-transmit     (SOF_TIMESTAMPING_TX_SOFTWARE)
        hardware-receive      (SOF_TIMESTAMPING_RX_HARDWARE)
        software-receive      (SOF_TIMESTAMPING_RX_SOFTWARE)
        software-system-clock (SOF_TIMESTAMPING_SOFTWARE)
        hardware-raw-clock    (SOF_TIMESTAMPING_RAW_HARDWARE)
PTP Hardware Clock: 2
Hardware Transmit Timestamp Modes:
        off                   (HWTSTAMP_TX_OFF)
        on                    (HWTSTAMP_TX_ON)
Hardware Receive Filter Modes:
        none                  (HWTSTAMP_FILTER_NONE)
        ptpv1-l4-sync         (HWTSTAMP_FILTER_PTP_V1_L4_SYNC)
        ptpv1-l4-delay-req    (HWTSTAMP_FILTER_PTP_V1_L4_DELAY_REQ)
        ptpv2-event           (HWTSTAMP_FILTER_PTP_V2_EVENT)


## CEPH OSD

[Global]
cluster_network
public_network
osd_pool_default_size
osd_memory_target --> 4294967296(4GB)
osd_pool_default_min_size --> 1

[OSD]
osd_journal_size 5120 -> 20000
osd_max_write_size  90 --> 512MB
osd_client_message_size_cap    2147483648
osd_deep_scrub_stride           131072
osd_map_cache_size              1024
osd_recovery_op_priority         2
osd_recovery_max_active          10
osd_max_backfiils                4
osd_min_pg_log_entries          30000
ods_max_pg_log_entries          100000
osd_mon_heartbeat_interval         40

ceph --admin-daemon /var/run/ceph/ceph-osd.1.asok config show

```
[root@node1 ~]# ceph mgr module enable balancer
module 'balancer' is already enabled (always-on)
[root@node1 ~]# ceph balancer status
{
    "last_optimize_duration": "",
    "plans": [],
    "mode": "none",
    "active": false,
    "optimize_result": "",
    "last_optimize_started": ""
}
[root@node1 ~]# ceph balancer eval
current cluster score 0.084294 (lower is better)
```

```
[root@node1 ~]# ceph osd erasure-code-profile ls
default
[root@node1 ~]# ceph osd erasure-code-profile get default
k=2
m=1
plugin=jerasure
technique=reed_sol_van

```


```
static struct kvm_sys_reg_target_table genericv8_target_table = {
        .table64 = {
                .table = genericv8_sys_regs,
                .num = ARRAY_SIZE(genericv8_sys_regs),
        },
        .table32 = {
                .table = genericv8_cp15_regs,
                .num = ARRAY_SIZE(genericv8_cp15_regs),
        },
};
```

arm_cpu_type_info



QEMU Workfolow:

softmmu/main.c
```c
int main(int argc, char **argv, char **envp)
{
    qemu_init(argc, argv, envp);
    qemu_main_loop();
    qemu_cleanup();

    return 0;
}
```


QEMU:
```c
int kvm_arm_vcpu_init(CPUState *cs)
{
    ARMCPU *cpu = ARM_CPU(cs);
    struct kvm_vcpu_init init;

    init.target = cpu->kvm_target;
    memcpy(init.features, cpu->kvm_init_features, sizeof(init.features));

    return kvm_vcpu_ioctl(cs, KVM_ARM_VCPU_INIT, &init);
}

kvm_vcpu_ioctl(cs, KVM_GET_ONE_REG, &reg);
```

KERNEL:
```c
long kvm_arch_vcpu_ioctl(struct file *filp,
             unsigned int ioctl, unsigned long arg)
{
    struct kvm_vcpu *vcpu = filp->private_data;
    void __user *argp = (void __user *)arg;
    struct kvm_device_attr attr;
    long r;

switch (ioctl) {
    case KVM_ARM_VCPU_INIT: {
        struct kvm_vcpu_init init;

        r = -EFAULT;
        if (copy_from_user(&init, argp, sizeof(init)))
            break;

        r = kvm_arch_vcpu_ioctl_vcpu_init(vcpu, &init);
        break;
    }
    default:
        r = -EINVAL;
    
    return r;
}
```
kvm_arch_vcpu_ioctl_vcpu_init
    --> kvm_vcpu_set_target
        --> kvm_target_cpu <-- return KVM_ARM_TARGET_GENERIC_V8;
        --> kvm_reset_vcpu
            --> 1. Reset core registers
            --> 2. Reset system registers
            --> 3. Set endianness
            --> 4. Reset Timer

ARM IP:
- CCI: `The Arm CoreLink Cache Coherent Interconnect`
- CCN: `The Arm CoreLink Cache Coherent Network`
- CSS: `ARM Compute Subsystem SCP Message Interface Protocols`
- SCU: Snoop Control Unit (SCU)
- sp805: ARM Watchdog Module
- pl061: PrimeCell General Purpose Input/Output 
- pl011: PrimeCell UART (PL011)
- sbsa: ARM SBSA defined generic UART
- tzc: The CoreLink TZC-400 TrustZone Address Space Controller (TZC-400) 
- CoreLink GIC-400
- DMC:Arm CoreLink DMC-620 Dynamic Memory Controller
- SMMU: ARM CoreLink MMU-500


MB/s=MiB/s *1.024 = 2285MB


jira | Confluence | Git | Gerrit | Jenkins


```
/* Switch to the guest for VHE systems running in EL2 */
static int __kvm_vcpu_run_vhe(struct kvm_vcpu *vcpu)
{
    struct kvm_cpu_context *host_ctxt;
    struct kvm_cpu_context *guest_ctxt;
    u64 exit_code;

    host_ctxt = &this_cpu_ptr(&kvm_host_data)->host_ctxt;
    host_ctxt->__hyp_running_vcpu = vcpu;
    guest_ctxt = &vcpu->arch.ctxt;

    sysreg_save_host_state_vhe(host_ctxt);

    /*
     * ARM erratum 1165522 requires us to configure both stage 1 and
     * stage 2 translation for the guest context before we clear
     * HCR_EL2.TGE.
     *
     * We have already configured the guest's stage 1 translation in
     * kvm_vcpu_load_sysregs_vhe above.  We must now call
     * __load_guest_stage2 before __activate_traps, because
     * __load_guest_stage2 configures stage 2 translation, and
     * __activate_traps clear HCR_EL2.TGE (among other things).
     */
    __load_guest_stage2(vcpu->arch.hw_mmu);
        | write_sysreg(kern_hyp_va(mmu->kvm)->arch.vtcr, vtcr_el2);
        | write_sysreg(kvm_get_vttbr(mmu), vttbr_el2);

    __activate_traps(vcpu);
        ___activate_traps
        __activate_traps_common
    __adjust_pc(vcpu);

    sysreg_restore_guest_state_vhe(guest_ctxt);
    __debug_switch_to_guest(vcpu);

    do {
        /* Jump in the fire! */
        exit_code = __guest_enter(vcpu);

        /* And we're baaack! */
    } while (fixup_guest_exit(vcpu, &exit_code));

    sysreg_save_guest_state_vhe(guest_ctxt);

    __deactivate_traps(vcpu);

    sysreg_restore_host_state_vhe(host_ctxt);

    if (vcpu->arch.flags & KVM_ARM64_FP_ENABLED)
        __fpsimd_save_fpexc32(vcpu);

    __debug_switch_to_host(vcpu);

    return exit_code;
}
NOKPROBE_SYMBOL(__kvm_vcpu_run_vhe);
```