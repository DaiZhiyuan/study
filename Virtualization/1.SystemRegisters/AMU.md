# 1. ARMv8访问系统寄存器

ARMv8，取消了协处理器，之前协处理器实现的功能，全部由系统寄存器来是实现。

两大类协处理器：
- CP14: DCC(the Debug Communications Channel)
- CP15: SCC(the System Control Coprocessor)

系统寄存器的编码由`{op0, op1, CRn, CRm, op2}`构成，对于系统寄存器的访问，使用`mrs`与`msr`指令来访问。


例如：`MIDR_EL1`系统寄存器的编码为
```
Register selectors(16bit)      Name       Description
op0  op1   CRn   CRm  op2                 
11   000  0000  0000  000      MIDR_EL1   Main ID Register         
```

# 2. Activity Monitors Unit (AMU) extension in AArch64 Linux

`AMU extension`是`ARMv8.4`体系结构引入的可选扩展。

在每个CPU中实现的AMU，提供了供系统管理使用的性能计数器。

AMU扩展为计数器寄存器提供了系统寄存器接口，并且还支持可选的外部`memory-mapped`接口。

AMUv1架构实现了一个组计数器，该组由`4个`固定功能与`16个`架构定义的`64-bit`的事件计数器。

- `CPU cycle counter`：以CPU的频率递增。
- `Constant counter`：以系统时钟的固定频率递增。
- `Instructions retired`：每条架构执行的指令都增加。
- `Memory stall cycles`：计算由时钟域内最后一级缓存中的未命中引起的指令调度停顿周期。

> 在`WFI`或`WFE`中时，这些计数器不会递增。
冷重置时，所有计数器重置为`0`。


```c
/* Definitions for system register interface to AMU for ARMv8.4 onwards */
#define SYS_AM_EL0(crm, op2)        sys_reg(3, 3, 13, (crm), (op2))
#define SYS_AMCR_EL0                SYS_AM_EL0(2, 0)
#define SYS_AMCFGR_EL0              SYS_AM_EL0(2, 1)
#define SYS_AMCGCR_EL0              SYS_AM_EL0(2, 2)
#define SYS_AMUSERENR_EL0           SYS_AM_EL0(2, 3)
#define SYS_AMCNTENCLR0_EL0         SYS_AM_EL0(2, 4)
#define SYS_AMCNTENSET0_EL0         SYS_AM_EL0(2, 5)
#define SYS_AMCNTENCLR1_EL0         SYS_AM_EL0(3, 0)
#define SYS_AMCNTENSET1_EL0         SYS_AM_EL0(3, 1)

#define SYS_AMEVCNTR0_EL0(n)        SYS_AM_EL0(4 + ((n) >> 3), (n) & 7)
#define SYS_AMEVTYPER0_EL0(n)       SYS_AM_EL0(6 + ((n) >> 3), (n) & 7)
#define SYS_AMEVCNTR1_EL0(n)        SYS_AM_EL0(12 + ((n) >> 3), (n) & 7)
#define SYS_AMEVTYPER1_EL0(n)       SYS_AM_EL0(14 + ((n) >> 3), (n) & 7)

/* Macro to expand the AMU counter and type registers*/
#define AMU_AMEVCNTR0_EL0(n) { SYS_DESC(SYS_AMEVCNTR0_EL0(n)), undef_access }
#define AMU_AMEVTYPER0_EL0(n) { SYS_DESC(SYS_AMEVTYPER0_EL0(n)), undef_access }
#define AMU_AMEVCNTR1_EL0(n) { SYS_DESC(SYS_AMEVCNTR1_EL0(n)), undef_access }
#define AMU_AMEVTYPER1_EL0(n) { SYS_DESC(SYS_AMEVTYPER1_EL0(n)), undef_access }


/* vcpu_arch flags field values: */
#define KVM_ARM64_DEBUG_DIRTY       (1 << 0)
#define KVM_ARM64_FP_ENABLED        (1 << 1) /* guest FP regs loaded */
#define KVM_ARM64_FP_HOST       (1 << 2) /* host FP regs loaded */
#define KVM_ARM64_HOST_SVE_IN_USE   (1 << 3) /* backup for host TIF_SVE */
#define KVM_ARM64_HOST_SVE_ENABLED  (1 << 4) /* SVE enabled for EL0 */
#define KVM_ARM64_GUEST_HAS_SVE     (1 << 5) /* SVE exposed to guest */
#define KVM_ARM64_VCPU_SVE_FINALIZED    (1 << 6) /* SVE config completed */
#define KVM_ARM64_GUEST_HAS_PTRAUTH (1 << 7) /* PTRAUTH exposed to guest */
#define KVM_ARM64_PENDING_EXCEPTION (1 << 8) /* Exception pending */
#define KVM_ARM64_EXCEPT_MASK       (7 << 9) /* Target EL/MODE */

/*
 * When KVM_ARM64_PENDING_EXCEPTION is set, KVM_ARM64_EXCEPT_MASK can
 * take the following values:
 *
 * For AArch32 EL1:
 */
#define KVM_ARM64_EXCEPT_AA32_UND   (0 << 9)
#define KVM_ARM64_EXCEPT_AA32_IABT  (1 << 9)
#define KVM_ARM64_EXCEPT_AA32_DABT  (2 << 9)
/* For AArch64: */
#define KVM_ARM64_EXCEPT_AA64_ELx_SYNC  (0 << 9)
#define KVM_ARM64_EXCEPT_AA64_ELx_IRQ   (1 << 9)
#define KVM_ARM64_EXCEPT_AA64_ELx_FIQ   (2 << 9)
#define KVM_ARM64_EXCEPT_AA64_ELx_SERR  (3 << 9)
#define KVM_ARM64_EXCEPT_AA64_EL1   (0 << 11)
#define KVM_ARM64_EXCEPT_AA64_EL2   (1 << 11)

static void inject_undef64(struct kvm_vcpu *vcpu)
{
    u32 esr = (ESR_ELx_EC_UNKNOWN << ESR_ELx_EC_SHIFT);

    vcpu->arch.flags |= (KVM_ARM64_EXCEPT_AA64_EL1      |
                 KVM_ARM64_EXCEPT_AA64_ELx_SYNC |
                 KVM_ARM64_PENDING_EXCEPTION);

    vcpu_write_sys_reg(vcpu, esr, ESR_EL1);
}

static bool undef_access(struct kvm_vcpu *vcpu, struct sys_reg_params *p,
              const struct sys_reg_desc *r)
{
    kvm_inject_undefined(vcpu);
        -> inject_undef64(vcpu);

    return false;
}

static const struct sys_reg_desc sys_reg_descs[] = {
    AMU_AMEVCNTR0_EL0(0),  // Group 0 of activity monitors (architected)
    AMU_AMEVTYPER0_EL0(0), // Group 0 of activity monitors (architected)
    AMU_AMEVCNTR1_EL0(0),  // Group 1 of activity monitors (auxiliary)
    AMU_AMEVTYPER1_EL0(0), // Group 1 of activity monitors (auxiliary)
};
```
宏展开后：
```c
{
    // AMU_AMEVCNTR0_EL0(0),
    .name = "SYS_AMEVCNTR0_EL0(0)", 
    .Op0 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((4 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((4 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((4 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((4 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((4 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 5) & 0x7), 
    undef_access 
},

{ 
    // AMU_AMEVTYPER0_EL0(0),
    .name = "SYS_AMEVTYPER0_EL0(0)", 
    .Op0 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((6 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((6 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((6 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((6 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((6 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 5) & 0x7), 
    undef_access 
},
    
{ 
    // AMU_AMEVCNTR1_EL0(0),
    .name = "SYS_AMEVCNTR1_EL0(0)", 
    .Op0 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((12 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((12 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((12 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((12 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((12 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 5) & 0x7), 
    undef_access 
},

{ 
    // AMU_AMEVTYPER1_EL0(0)
    .name = "SYS_AMEVTYPER1_EL0(0)", 
    .Op0 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((14 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((14 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((14 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((14 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((3) << 16) | ((13) << 12) | (((14 + ((0) >> 3))) << 8) | ((((0) & 7)) << 5))) >> 5) & 0x7), 
    undef_access 
}
```
# 3. Pointer authentication in AArch64 Linux

`ARMv8.3`指针身份验证扩展，可用于减轻某些类型的攻击的原语，在这些类别中，攻击者可能破坏某些内存（例如堆栈）的内容。

该扩展使用`Pointer Authentication Code (PAC)`来确定指针指向的数据是否已被恶意踹改。它供了五个单独的密钥来生成`PAC`:
- 两个用于指令地址（APIAKey，APIBKey）
- 两个用于数据地址（APDAKey，APDBKey）
- 一个用于通用身份验证（APGAKey）


```c
#define REG_HIDDEN      (1 << 0) /* hidden from userspace and guest */
#define REG_RAZ         (1 << 1) /* RAZ from userspace and guest */

static unsigned int ptrauth_visibility(const struct kvm_vcpu *vcpu,
            const struct sys_reg_desc *rd)
{
    return vcpu_has_ptrauth(vcpu) ? 0 : REG_HIDDEN;
}

/* Reset functions */
static inline void reset_unknown(struct kvm_vcpu *vcpu,
                 const struct sys_reg_desc *r)
{
    BUG_ON(!r->reg);
    BUG_ON(r->reg >= NR_SYS_REGS);
    __vcpu_sys_reg(vcpu, r->reg) = 0x1de7ec7edbadc0deULL;
}

static bool undef_access(struct kvm_vcpu *vcpu, struct sys_reg_params *p,
             const struct sys_reg_desc *r)
{
    kvm_inject_undefined(vcpu);

    return false;
}

#define __PTRAUTH_KEY(k)                        \
    { SYS_DESC(SYS_## k), undef_access, reset_unknown, k,       \
    .visibility = ptrauth_visibility}

#define PTRAUTH_KEY(k)                          \
    __PTRAUTH_KEY(k ## KEYLO_EL1),                  \
    __PTRAUTH_KEY(k ## KEYHI_EL1)

static const struct sys_reg_desc sys_reg_descs[] = {
    PTRAUTH_KEY(APIA),
    PTRAUTH_KEY(APIB),
    PTRAUTH_KEY(APDA),
    PTRAUTH_KEY(APDB),
    PTRAUTH_KEY(APGA),
};
```
宏展开后：
```c
{ 
    .name = "SYS_APIAKEYLO_EL1", 
    .Op0 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((0) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((0) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((0) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((0) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((0) << 5))) >> 5) & 0x7), 
    undef_access, 
    reset_unknown, 
    APIAKEYLO_EL1, 
    .visibility = ptrauth_visibility
}, 
{ 
    .name = "SYS_APIAKEYHI_EL1", 
    .Op0 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((1) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((1) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((1) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((1) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((1) << 8) | ((1) << 5))) >> 5) & 0x7), 
    undef_access, 
    reset_unknown, 
    APIAKEYHI_EL1, 
    .visibility = ptrauth_visibility
},
{ 
    .name = "SYS_APDAKEYLO_EL1", 
    .Op0 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((0) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((0) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((0) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((0) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((0) << 5))) >> 5) & 0x7), 
    undef_access, 
    reset_unknown,
    APDAKEYLO_EL1, 
    .visibility = ptrauth_visibility
}, 
{ 
    .name = "SYS_APDAKEYHI_EL1", .Op0 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((1) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((1) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((1) << 5))) >> 12) & 0xf), .
    CRm = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((1) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((2) << 8) | ((1) << 5))) >> 5) & 0x7), 
    undef_access, 
    reset_unknown, 
    APDAKEYHI_EL1, 
    .visibility = ptrauth_visibility
},
{ 
    .name = "SYS_APGAKEYLO_EL1", 
    .Op0 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((0) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((0) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((0) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((0) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((0) << 5))) >> 5) & 0x7), 
    undef_access, 
    reset_unknown, 
    APGAKEYLO_EL1, 
    .visibility = ptrauth_visibility}, 
{ 
    .name = "SYS_APGAKEYHI_EL1", 
    .Op0 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((1) << 5))) >> 19) & 0x3), 
    .Op1 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((1) << 5))) >> 16) & 0x7), 
    .CRn = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((1) << 5))) >> 12) & 0xf), 
    .CRm = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((1) << 5))) >> 8) & 0xf), 
    .Op2 = ((((((3) << 19) | ((0) << 16) | ((2) << 12) | ((3) << 8) | ((1) << 5))) >> 5) & 0x7), 
    undef_access, 
    reset_unknown, 
    APGAKEYHI_EL1, 
    .visibility = ptrauth_visibility
}
```