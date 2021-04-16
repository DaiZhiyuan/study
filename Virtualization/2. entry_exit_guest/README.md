```patch
commit b97b66c14b96ab562e4fd516d804c5cd05c0529e
Author: Marc Zyngier <marc.zyngier@arm.com>
Date:   Thu Oct 22 08:32:18 2015 +0100

    arm64: KVM: Implement guest entry

    Contrary to the previous patch, the guest entry is fairly different
    from its assembly counterpart, mostly because it is only concerned
    with saving/restoring the GP registers, and nothing else.

    Reviewed-by: Christoffer Dall <christoffer.dall@linaro.org>
    Signed-off-by: Marc Zyngier <marc.zyngier@arm.com>

diff --git a/arch/arm64/kvm/hyp/Makefile b/arch/arm64/kvm/hyp/Makefile
index ec14cac..1e1ff06 100644
--- a/arch/arm64/kvm/hyp/Makefile
+++ b/arch/arm64/kvm/hyp/Makefile
@@ -7,3 +7,4 @@ obj-$(CONFIG_KVM_ARM_HOST) += vgic-v3-sr.o
 obj-$(CONFIG_KVM_ARM_HOST) += timer-sr.o
 obj-$(CONFIG_KVM_ARM_HOST) += sysreg-sr.o
 obj-$(CONFIG_KVM_ARM_HOST) += debug-sr.o
+obj-$(CONFIG_KVM_ARM_HOST) += entry.o
diff --git a/arch/arm64/kvm/hyp/entry.S b/arch/arm64/kvm/hyp/entry.S
new file mode 100644
index 0000000..ff19695
--- /dev/null
+++ b/arch/arm64/kvm/hyp/entry.S
@@ -0,0 +1,130 @@
+/*
+ * Copyright (C) 2015 - ARM Ltd
+ * Author: Marc Zyngier <marc.zyngier@arm.com>
+ *
+ * This program is free software; you can redistribute it and/or modify
+ * it under the terms of the GNU General Public License version 2 as
+ * published by the Free Software Foundation.
+ *
+ * This program is distributed in the hope that it will be useful,
+ * but WITHOUT ANY WARRANTY; without even the implied warranty of
+ * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
+ * GNU General Public License for more details.
+ *
+ * You should have received a copy of the GNU General Public License
+ * along with this program.  If not, see <http://www.gnu.org/licenses/>.
+ */
+
+#include <linux/linkage.h>
+
+#include <asm/asm-offsets.h>
+#include <asm/assembler.h>
+#include <asm/fpsimdmacros.h>
+#include <asm/kvm.h>
+#include <asm/kvm_arm.h>
+#include <asm/kvm_asm.h>
+#include <asm/kvm_mmu.h>
+
+#define CPU_GP_REG_OFFSET(x)   (CPU_GP_REGS + x)
+#define CPU_XREG_OFFSET(x)     CPU_GP_REG_OFFSET(CPU_USER_PT_REGS + 8*x)
+
+       .text
+       .pushsection    .hyp.text, "ax"
+
+.macro save_callee_saved_regs ctxt
+       stp     x19, x20, [\ctxt, #CPU_XREG_OFFSET(19)]
+       stp     x21, x22, [\ctxt, #CPU_XREG_OFFSET(21)]
+       stp     x23, x24, [\ctxt, #CPU_XREG_OFFSET(23)]
+       stp     x25, x26, [\ctxt, #CPU_XREG_OFFSET(25)]
+       stp     x27, x28, [\ctxt, #CPU_XREG_OFFSET(27)]
+       stp     x29, lr,  [\ctxt, #CPU_XREG_OFFSET(29)]
+.endm
+
+.macro restore_callee_saved_regs ctxt
+       ldp     x19, x20, [\ctxt, #CPU_XREG_OFFSET(19)]
+       ldp     x21, x22, [\ctxt, #CPU_XREG_OFFSET(21)]
+       ldp     x23, x24, [\ctxt, #CPU_XREG_OFFSET(23)]
+       ldp     x25, x26, [\ctxt, #CPU_XREG_OFFSET(25)]
+       ldp     x27, x28, [\ctxt, #CPU_XREG_OFFSET(27)]
+       ldp     x29, lr,  [\ctxt, #CPU_XREG_OFFSET(29)]
+.endm
+
+/*
+ * u64 __guest_enter(struct kvm_vcpu *vcpu,
+ *                  struct kvm_cpu_context *host_ctxt);
+ */
+ENTRY(__guest_enter)
+       // x0: vcpu
+       // x1: host/guest context
+       // x2-x18: clobbered by macros
+
+       // Store the host regs to kvm_cpu_context(x19 ~ X30<LR>)
+       save_callee_saved_regs x1
+
+       // Preserve vcpu & host_ctxt for use at exit time
        // vcpu. host/guest connext .push stack. --> at exit time.
+       stp     x0, x1, [sp, #-16]!
+
+       add     x1, x0, #VCPU_CONTEXT
+
+       // Prepare x0-x1 for later restore by pushing them onto the stack
+       ldp     x2, x3, [x1, #CPU_XREG_OFFSET(0)]
+       stp     x2, x3, [sp, #-16]!
+
+       // x2-x18
+       ldp     x2, x3,   [x1, #CPU_XREG_OFFSET(2)]
+       ldp     x4, x5,   [x1, #CPU_XREG_OFFSET(4)]
+       ldp     x6, x7,   [x1, #CPU_XREG_OFFSET(6)]
+       ldp     x8, x9,   [x1, #CPU_XREG_OFFSET(8)]
+       ldp     x10, x11, [x1, #CPU_XREG_OFFSET(10)]
+       ldp     x12, x13, [x1, #CPU_XREG_OFFSET(12)]
+       ldp     x14, x15, [x1, #CPU_XREG_OFFSET(14)]
+       ldp     x16, x17, [x1, #CPU_XREG_OFFSET(16)]
+       ldr     x18,      [x1, #CPU_XREG_OFFSET(18)]
+
+       // x19-x29, lr
+       restore_callee_saved_regs x1
+
+       // Last bits of the 64bit state
+       ldp     x0, x1, [sp], #16
+
+       // Do not touch any register after this!
+       eret
+ENDPROC(__guest_enter)
+
+ENTRY(__guest_exit)
+       // x0: vcpu
+       // x1: return code
+       // x2-x3: free
+       // x4-x29,lr: vcpu regs
+       // vcpu x0-x3 on the stack
+
+       add     x2, x0, #VCPU_CONTEXT
+
+       stp     x4, x5,   [x2, #CPU_XREG_OFFSET(4)]
+       stp     x6, x7,   [x2, #CPU_XREG_OFFSET(6)]
+       stp     x8, x9,   [x2, #CPU_XREG_OFFSET(8)]
+       stp     x10, x11, [x2, #CPU_XREG_OFFSET(10)]
+       stp     x12, x13, [x2, #CPU_XREG_OFFSET(12)]
+       stp     x14, x15, [x2, #CPU_XREG_OFFSET(14)]
+       stp     x16, x17, [x2, #CPU_XREG_OFFSET(16)]
+       str     x18,      [x2, #CPU_XREG_OFFSET(18)]
+
+       ldp     x6, x7, [sp], #16       // x2, x3
+       ldp     x4, x5, [sp], #16       // x0, x1
+
+       stp     x4, x5, [x2, #CPU_XREG_OFFSET(0)]
+       stp     x6, x7, [x2, #CPU_XREG_OFFSET(2)]
+
+       save_callee_saved_regs x2
+
+       // Restore vcpu & host_ctxt from the stack
+       // (preserving return code in x1)
+       ldp     x0, x2, [sp], #16
+       // Now restore the host regs
+       restore_callee_saved_regs x2
+
+       mov     x0, x1
+       ret
+ENDPROC(__guest_exit)
+
+       /* Insert fault handling here */
diff --git a/arch/arm64/kvm/hyp/hyp.h b/arch/arm64/kvm/hyp/hyp.h
index 454e46f..0809653 100644
--- a/arch/arm64/kvm/hyp/hyp.h
+++ b/arch/arm64/kvm/hyp/hyp.h
@@ -52,5 +52,7 @@ void __debug_restore_state(struct kvm_vcpu *vcpu,
 void __debug_cond_save_host_state(struct kvm_vcpu *vcpu);
 void __debug_cond_restore_host_state(struct kvm_vcpu *vcpu);

+u64 __guest_enter(struct kvm_vcpu *vcpu, struct kvm_cpu_context *host_ctxt);
+
 #endif /* __ARM64_KVM_HYP_H__ */
```

```
stp x1, x2, [x0]
```
等价于
```
STR x1, [x0, #0]	
STR x2, [x0, #8]
```

```c
enum vcpu_sysreg {
    __INVALID_SYSREG__,   /* 0 is reserved as an invalid value */
    MPIDR_EL1,  /* MultiProcessor Affinity Register */
    CSSELR_EL1, /* Cache Size Selection Register */
    SCTLR_EL1,  /* System Control Register */
    ACTLR_EL1,  /* Auxiliary Control Register */
    CPACR_EL1,  /* Coprocessor Access Control */
    ZCR_EL1,    /* SVE Control */
    TTBR0_EL1,  /* Translation Table Base Register 0 */
    TTBR1_EL1,  /* Translation Table Base Register 1 */
    TCR_EL1,    /* Tra:nslation Control Register */
    ESR_EL1,    /* Exception Syndrome Register */
    AFSR0_EL1,  /* Auxiliary Fault Status Register 0 */
    AFSR1_EL1,  /* Auxiliary Fault Status Register 1 */
    FAR_EL1,    /* Fault Address Register */
    MAIR_EL1,   /* Memory Attribute Indirection Register */
    VBAR_EL1,   /* Vector Base Address Register */
    CONTEXTIDR_EL1, /* Context ID Register */
    TPIDR_EL0,  /* Thread ID, User R/W */
    TPIDRRO_EL0,    /* Thread ID, User R/O */
    TPIDR_EL1,  /* Thread ID, Privileged */
    AMAIR_EL1,  /* Aux Memory Attribute Indirection Register */
    CNTKCTL_EL1,    /* Timer Control Register (EL1) */
    PAR_EL1,    /* Physical Address Register */
    MDSCR_EL1,  /* Monitor Debug System Control Register */
    MDCCINT_EL1,    /* Monitor Debug Comms Channel Interrupt Enable Reg
    DISR_EL1,   /* Deferred Interrupt Status Register */

    /* Performance Monitors Registers */
    PMCR_EL0,   /* Control Register */
    PMSELR_EL0, /* Event Counter Selection Register */
    PMEVCNTR0_EL0,  /* Event Counter Register (0-30) */
    PMEVCNTR30_EL0 = PMEVCNTR0_EL0 + 30,
    PMCCNTR_EL0,    /* Cycle Counter Register */
    PMEVTYPER0_EL0, /* Event Type Register (0-30) */
    PMEVTYPER30_EL0 = PMEVTYPER0_EL0 + 30,
    PMCCFILTR_EL0,  /* Cycle Count Filter Register */
    PMCNTENSET_EL0, /* Count Enable Set Register */
    PMINTENSET_EL1, /* Interrupt Enable Set Register */
    PMOVSSET_EL0,   /* Overflow Flag Status Set Register */
    PMSWINC_EL0,    /* Software Increment Register */
    PMUSERENR_EL0,  /* User Enable Register */

    /* Pointer Authentication Registers in a strict increasing order. *
    APIAKEYLO_EL1,
    APIAKEYHI_EL1,
    APIBKEYLO_EL1,
    APIBKEYHI_EL1,
    APDAKEYLO_EL1,
    APDAKEYHI_EL1,
    APDBKEYLO_EL1,
    APDBKEYHI_EL1,
    APGAKEYLO_EL1,
    APGAKEYHI_EL1,

    ELR_EL1,
    SP_EL1,
    SPSR_EL1,

    CNTVOFF_EL2,
    CNTV_CVAL_EL0,
    CNTV_CTL_EL0,
    CNTP_CVAL_EL0,
    CNTP_CTL_EL0,

    /* 32bit specific registers. Keep them at the end of the range */
    DACR32_EL2, /* Domain Access Control Register */
    IFSR32_EL2, /* Instruction Fault Status Register */
    FPEXC32_EL2,    /* Floating-Point Exception Control Register */
    DBGVCR32_EL2,   /* Debug Vector Catch Register */

    NR_SYS_REGS /* Nothing after this line! */
};

struct user_pt_regs {
    __u64       regs[31]; // X0 - X30
    __u64       sp;       // StackPointer
    __u64       pc;       // ProgramCounter
    __u64       pstate;   // Processer status
};

struct user_fpsimd_state {
    __uint128_t vregs[32];  // Neon. 32 x 128bit vector regsiters
    __u32       fpsr;
    __u32       fpcr;
    __u32       __reserved[2];
};

struct kvm_cpu_context {
    struct user_pt_regs regs;   /* sp = sp_el0 */

    u64 spsr_abt;
    u64 spsr_und;
    u64 spsr_irq;
    u64 spsr_fiq;

    struct user_fpsimd_state fp_regs;

    u64 sys_regs[NR_SYS_REGS];

    struct kvm_vcpu *__hyp_running_vcpu;
};


struct kvm_vcpu
    --> struct kvm_vcpu_arch 
        --> struct kvm_cpu_context ctxt;
```