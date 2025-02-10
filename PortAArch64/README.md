<!-- TOC -->

- [1. 软件迁移指南（From X86 to ARM）](#1-软件迁移指南from-x86-to-arm)
    - [1.1 Instruction Set Architecture(ISA)](#11-instruction-set-architectureisa)
- [2. X86与ARM架构编程模型对比](#2-x86与arm架构编程模型对比)
    - [2.1 64-bit Data Model](#21-64-bit-data-model)
    - [2.2 char的默认类型](#22-char的默认类型)
    - [2.3 Endianess](#23-endianess)
    - [2.4 Platform Macro](#24-platform-macro)
    - [2.5 GCC Machine-Dependent Option](#25-gcc-machine-dependent-option)
        - [2.5.1 X86 Option](#251-x86-option)
        - [2.5.2 AArch64 Options](#252-aarch64-options)
    - [2.6 GCC Extensions](#26-gcc-extensions)
        - [2.6.1 常用的atomic原语的Built-in函数原型](#261-常用的atomic原语的built-in函数原型)
        - [2.6.2 常用的sync原语的Built-in函数原型](#262-常用的sync原语的built-in函数原型)
        - [2.6.3 案例：PostgreSQL是如何使用Atomic与Sync原语替换X86汇编](#263-案例postgresql是如何使用atomic与sync原语替换x86汇编)
            - [2.6.3.1 内存屏障](#2631-内存屏障)
            - [2.6.3.2 自旋锁](#2632-自旋锁)
            - [2.6.3.3 TAS](#2633-tas)
            - [2.6.3.4 CAS](#2634-cas)
            - [2.6.3.5 clear](#2635-clear)
            - [2.6.3.6 fetch-add](#2636-fetch-add)
        - [2.6.4 平台相关的built-in函数案例](#264-平台相关的built-in函数案例)
- [3. rdtsc指令案例](#3-rdtsc指令案例)

<!-- /TOC -->

# 1. 软件迁移指南（From X86 to ARM）

## 1.1 Instruction Set Architecture(ISA)

High-Level language:
```c
int add(int x, int y)
{
    return x + y;
}
```

aarch64:
```asm
sub     sp, sp, #0x10
str     w0, [sp,#12]
str     w1, [sp,#8]

ldr     w1, [sp,#12]
ldr     w0, [sp,#8]
add     w0, w1, w0

add     sp, sp, #0x10
ret
```

amd64:
```asm
push   %rbp
mov    %rsp,%rbp
mov    %edi,-0x4(%rbp)
mov    %esi,-0x8(%rbp)

mov    -0x8(%rbp),%eax
mov    -0x4(%rbp),%edx
add    %edx,%eax

pop    %rbp
retq
```
> 类似C这样的编译型高级语言，需要通过编译器rebuild源代码，产生不同架构的指令集，来达到软件移植到不同架构的目的。

# 2. X86与ARM架构编程模型对比

## 2.1 64-bit Data Model

Data Model | short int | int | long int | long long | pointer
---|---|---|---|---|---
LP64 | 16 | 32 | 64 | 64 | 64
LLP64 | 16 | 32 | 32 | 64 | 64

Macro | ARMv8-A(aarch64) |  AMD64(IA32e)
---|---|---
__UINT8_TYPE__ | unsigned char | unsigned char
__UINT16_TYPE__ | short unsigned int | short unsigned int
__UINT32_TYPE__ | unsigned int | unsigned int
__UINT64_TYPE__ | long unsigned int | long unsigned int
__UINTPTR_TYPE__ | long unsigned int | long unsigned int

> Armv8架构的aarch64执行状态与AMD64架构的IA32e运行模式的数据模型一致（即`LP64`）。

## 2.2 char的默认类型
Macro | ARMv8(aarch64) |  AMD64(IA32e)
---|---|---
__CHAR_UNSIGNED__ | true | false

> Armv8架构的aarch64执行状态char类型默认是无符号，而AMD64架构的IA32e运行模式char类型默认是有符号的。

Example:
```c
#include <stdio.h>
int main(void)
{
    char value = -1;
    printf("value = %d\n", value);

    return 0;
}
```

- AMD64(IA32e)执行结果为：`value = -1`。
- ARMv8(aarch64)执行结果为：`value = 255`。

## 2.3 Endianess

Example:
```c
uint32_t value = 0x0A0B0C0D;
uint32_t *pos = &value;
```
in memory view(Big-endian):
```
pos[0] == 0x0A; 
pos[1] == 0x0B;
pos[2] == 0x0C;
pos[3] == 0x0D;
```
in memory view(Little-endian)
```
pos[0] == 0x0D;
pos[1] == 0x0C;
pos[2] == 0x0B;
pos[3] == 0x0A;
```

get_cpu_endian:
```c
#include <stdio.h>
#include <stdint.h>

union {
    uint8_t u8; 
    uint16_t u16; 
    uint32_t u32; 
    uint64_t u64;
} u = { .u64 = 0x4A };

int main(void)
{
   
    puts(u.u8 == u.u16 && u.u8 == u.u32 && u.u8 == u.u64 ? "little-endian" : "big-endian");
    
    return 0;
}
```
- AMD64(IA32e)执行结果为：`little-endian`。
- ARMv8(aarch64)执行结果为：`little-endian`。

> ARMv8体系结构文档描述Endianess是现实定义的，Arm Cortex系列处理器实现为little-endian与X86一致。

## 2.4 Platform Macro 

ARMv8-A(aarch64) | AMD64(IA32e)
---|---
\_\_aarch64\_\_ | \_\_x86\_64\_\_ 

Example:
```c
uint32_t arch_crc32_checksum(const unsigned char *text)
{
#ifdef (__x86_64__)
    return do_x86_crc32();
#elifef (__aarch64__)
    return do_arm_crc32();
#endif
}

int main(void)
{
    const unsigned char *text = "Some Text";
    
    uint32_t checksum = arch_crc32_checksum(text);
    
    return 0;
}
```

## 2.5 GCC Machine-Dependent Option

### 2.5.1 X86 Option

-march=skylake -mtune=intel

> Intel Skylake CPU with 64-bit extensions, MOVBE, MMX, SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, POPCNT, AVX, AVX2, AES, PCLMUL, FSGSBASE, RDRND, FMA, BMI, BMI2, F16C, RDSEED, ADCX, PREFETCHW, CLFLUSHOPT, XSAVEC and XSAVES instruction set support.

### 2.5.2 AArch64 Options

-march=armv8-a/armv8.1-a/armv8.2-a/armv8.3-a/armv8.4-a/armv8.5-a

-mtune=cortex-a57/cortex-a72/thunderx/tsv110

> cavium/thunderx (microarch)

Feature | Description
---|---
crc | Enable CRC extension.
crypto | Enable Crypto extension.
fp | Enable floating-point instructions. 
simd | Enable Advanced SIMD instructions. 
sve | Enable Scalable Vector Extension instructions. 
lse | Enable Large System Extension instructions. 
rdma | Enable Round Double Multiply Accumulate instructions.
fp16 | Enable FP16 extension. 
fp16fml | Enable FP16 fmla extension. 
rcpc | Enable the RcPc extension. 
dotprod | Enable the Dot Product extension. 
aes | Enable the Armv8-a aes and pmull crypto extension.
sha2 | Enable the Armv8-a sha2 crypto extension. 
sha3 | Enable the sha512 and sha3 crypto extension.
sm4 | Enable the sm3 and sm4 crypto extension.
profile | Enable the Statistical Profiling extension.
rng | Enable the Random Number instructions.
memtag | Enable the Memory Tagging Extensions.
sb | Enable the Armv8-a Speculation Barrier instruction.
ssbs | Enable the Armv8-a Speculative Store Bypass Safe instruction. 
predres | Enable the Armv8-a Execution and Data Prediction Restriction instructions.

## 2.6 GCC Extensions

GCC提供了C语言版本的原子操作，GCC可保证跨平台的编译问题。

GCC 4.7前，__sync同步原语中默认的内存模型为`full barrier`模型，__sync原语前后的
读写操作均不可做指令重排。为提高流水线执行效率，GCC 4.7合入和C11的内存模
型，通过__atomic同步原语，由使用者控制需要的屏障级别。

C11的内存模型:
- __ATOMIC_RELAXED：对其它读写操作没有同步，只保证本操作是原子的。
- __ATOMIC_CONSUME：load操作，当前线程依赖该原子变量的访存操作不能reorder到该指令之前，对其他线程store操作(release)可见。
- __ATOMIC_ACQUIRE：load操作，当前线程所有访存操作不能reorder到该指令之前，对其他线程store操作(release)可见。
- __ATOMIC_RELEASE：store操作，当前线程所有访存操作不能reorder到该指令之后，对其他线程load操作(consume)可见。
- __ATOMIC_ACQ_REL：load/store操作，memory_order_acquire + memory_order_release。
- __ATOMIC_SEQ_CST：memory_order_acq_rel + 顺序一致性(sequential consisten)。

### 2.6.1 常用的atomic原语的Built-in函数原型

```c
// read
type __atomic_load_n (type *ptr, int memorder)
void __atomic_load (type *ptr, type *ret, int memorder)

// write
void __atomic_store (type *ptr, type *val, int memorder)

// swap
void __atomic_exchange (type *ptr, type *val, type *ret, int memorder)

// CAS
bool __atomic_compare_exchange (type *ptr, type *expected, type *desired, bool weak, int success_memorder, int failure_memorder)

// Logic 
type __atomic_add_fetch (type *ptr, type val, int memorder)
type __atomic_sub_fetch (type *ptr, type val, int memorder)
type __atomic_and_fetch (type *ptr, type val, int memorder)
type __atomic_xor_fetch (type *ptr, type val, int memorder)
type __atomic_or_fetch (type *ptr, type val, int memorder)
type __atomic_nand_fetch (type *ptr, type val, int memorder)
type __atomic_fetch_add (type *ptr, type val, int memorder)
type __atomic_fetch_sub (type *ptr, type val, int memorder)
type __atomic_fetch_and (type *ptr, type val, int memorder)
type __atomic_fetch_xor (type *ptr, type val, int memorder)
type __atomic_fetch_or (type *ptr, type val, int memorder)
type __atomic_fetch_nand (type *ptr, type val, int memorder)

// TAS
bool __atomic_test_and_set (void *ptr, int memorder)

// Clear
void __atomic_clear (bool *ptr, int memorder)
```

### 2.6.2 常用的sync原语的Built-in函数原型

```c
type __sync_add_and_fetch (type *ptr, type value, ...)
type __sync_sub_and_fetch (type *ptr, type value, ...)
type __sync_or_and_fetch (type *ptr, type value, ...)
type __sync_and_and_fetch (type *ptr, type value, ...)
type __sync_xor_and_fetch (type *ptr, type value, ...)
type __sync_nand_and_fetch (type *ptr, type value, ...)

bool __sync_bool_compare_and_swap (type *ptr, type oldval, type newval, ...)
type __sync_val_compare_and_swap (type *ptr, type oldval, type newval, ...)

type __sync_lock_test_and_set (type *ptr, type value, ...)

void __sync_lock_release (type *ptr, ...)
```

### 2.6.3 案例：PostgreSQL是如何使用Atomic与Sync原语替换X86汇编

#### 2.6.3.1 内存屏障

x86_64:
```c
#if defined(__x86_64__)
#define pg_memory_barrier_impl()                \
        __asm__ __volatile__ ("lock; addl $0,0(%%rsp)" : : : "memory", "cc")
#endif
```

aarch64:
```c
#define pg_memory_barrier_impl()    __sync_synchronize()
```

#### 2.6.3.2 自旋锁 

x86_64:
```
static __inline__ void
pg_spin_delay_impl(void)
{
        __asm__ __volatile__(" rep; nop \n");
}
```
aarch64:
```
#define pg_spin_delay_impl()    ((void)0)
```

#### 2.6.3.3 TAS

x86_64(Intel compiler):
```c
static inline bool
pg_atomic_test_set_flag_impl(volatile pg_atomic_flag *ptr)
{
        register char _res = 1;

        __asm__ __volatile__(
                "       lock                    \n"
                "       xchgb   %0,%1   \n"
:               "+q"(_res), "+m"(ptr->value)
:
:               "memory");
        return _res == 0;
}
```
aarch64(GNU compiler):
```c
pg_atomic_test_set_flag_impl(volatile pg_atomic_flag *ptr)
{
        return __sync_lock_test_and_set(&ptr->value, 1) == 0;
}
```

#### 2.6.3.4 CAS

x86_64(Intel compiler):
```c
static inline bool
pg_atomic_compare_exchange_u64_impl(volatile pg_atomic_uint64 *ptr,
                                                                        uint64 *expected, uint64 newval)
{
        char    ret;

        /*
         * Perform cmpxchg and use the zero flag which it implicitly sets when
         * equal to measure the success.
         */
        __asm__ __volatile__(
                "       lock                            \n"
                "       cmpxchgq        %4,%5   \n"
                "   setz                %2              \n"
:               "=a" (*expected), "=m"(ptr->value), "=q" (ret)
:               "a" (*expected), "r" (newval), "m"(ptr->value)
:               "memory", "cc");
        return (bool) ret;
}
```

aarch64(GNU compiler):
```c
static inline bool
pg_atomic_compare_exchange_u64_impl(volatile pg_atomic_uint64 *ptr,
                                                                        uint64 *expected, uint64 newval)
{
        return __atomic_compare_exchange_n(&ptr->value, expected, newval, false,
                                                                           __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
}
```


#### 2.6.3.5 clear

x86_64(Intel compiler):
```c
static inline void
pg_atomic_clear_flag_impl(volatile pg_atomic_flag *ptr)
{
        /*
         * On a TSO architecture like x86 it's sufficient to use a compiler
         * barrier to achieve release semantics.
         */
        __asm__ __volatile__("" ::: "memory");
        ptr->value = 0;
}
```
aarch64(GNU compiler):
```c
static inline void
pg_atomic_clear_flag_impl(volatile pg_atomic_flag *ptr)
{
        __sync_lock_release(&ptr->value);
}
```

#### 2.6.3.6 fetch-add

x86_64(Intel compiler):
```c
static inline uint64
pg_atomic_fetch_add_u64_impl(volatile pg_atomic_uint64 *ptr, int64 add_)
{
        uint64 res;
        __asm__ __volatile__(
                "       lock                            \n"
                "       xaddq   %0,%1           \n"
:               "=q"(res), "=m"(ptr->value)
:               "0" (add_), "m"(ptr->value)
:               "memory", "cc");
        return res;
}
```
aarch64(GNU compiler):
```c
static inline uint64
pg_atomic_fetch_add_u64_impl(volatile pg_atomic_uint64 *ptr, int64 add_)
{
        return __sync_fetch_and_add(&ptr->value, add_);
}
```

### 2.6.4 平台相关的built-in函数案例

x86_64:
```c
#include <nmmintrin.h>

pg_crc32c
pg_comp_crc32c_sse42(pg_crc32c crc, const void *data, size_t len)
{
        const unsigned char *p = data;
        const unsigned char *pend = p + len;

        while (p + 8 <= pend) {
                crc = (uint32) _mm_crc32_u64(crc, *((const uint64 *) p));
                p += 8;
        }

        if (p + 4 <= pend) {
                crc = _mm_crc32_u32(crc, *((const unsigned int *) p));
                p += 4;
        }

        while (p < pend) {
                crc = _mm_crc32_u8(crc, *p);
                p++;
        }

        return crc;
}
```

aarch64:
```c
#include <arm_acle.h>

pg_crc32c
pg_comp_crc32c_armv8(pg_crc32c crc, const void *data, size_t len)
{
        const unsigned char *p = data;
        const unsigned char *pend = p + len;

        if (!PointerIsAligned(p, uint16) &&
                p + 1 <= pend) {
                crc = __crc32cb(crc, *p);
                p += 1;
        }
        if (!PointerIsAligned(p, uint32) &&
                p + 2 <= pend) {
                crc = __crc32ch(crc, *(uint16 *) p);
                p += 2;
        }
        if (!PointerIsAligned(p, uint64) &&
                p + 4 <= pend) {
                crc = __crc32cw(crc, *(uint32 *) p);
                p += 4;
        }

        while (p + 8 <= pend) {
                crc = __crc32cd(crc, *(uint64 *) p);
                p += 8;
        }

        if (p + 4 <= pend) {
                crc = __crc32cw(crc, *(uint32 *) p);
                p += 4;
        }
        if (p + 2 <= pend) {
                crc = __crc32ch(crc, *(uint16 *) p);
                p += 2;
        }
        if (p < pend) {
                crc = __crc32cb(crc, *p);
        }

        return crc;
}
```



# 3. rdtsc指令案例 

x86_64:
```c
static inline uint64_t
rdtsc64(void)
{
    uint64_t low, high;
    __asm__ volatile("rdtsc" : "=a"(low), "=d"(high));
    return (high << 32) | low;
}
```

aarch64:
```c
static inline uint64_t
rdtsc64(void)
{
        uint64_t value = 0;
        // processor clock cycles
        asm volatile("mrs %0, PMCCNTR_EL0" : "=r"(value));
        return value;
}

int main(void)
{
    uint64_t time_start, time_end;

    time_start = rdtsc64();
    // workload
    time_end = rdtsc64();

    printf("Time delta = %u\n", time_end - time_start);
    return 0;
}
```
> UserMode(EL0) access PMCCNTR\_EL0 register will generate `Illegal instruction` exectpion

kernel module:
```c
/*
 * Enable user-mode ARM performance counter access.
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/smp.h>

/** -- Configuration stuff ------------------------------------------------- */

#define DRVR_NAME "enable_arm_pmu"

#if !defined(__aarch64__)
#error Module can only be compiled on ARM64 machines.
#endif

/** -- Initialization & boilerplate ---------------------------------------- */

#define PERF_DEF_OPTS (1 | 16)

static void
enable_cpu_counters(void* data)
{
        printk(KERN_INFO "[" DRVR_NAME "] enabling user-mode PMU access on CPU #%d",
                smp_processor_id());

        // at EL0, software can access all of the specified registers
        asm volatile("msr PMUSERENR_EL0, %0" :: "r"(1));
        
        // Control Register
        // Enable
        // Export events where not prohibited
        asm volatile("msr PMCR_EL0, %0" :: "r"(PERF_DEF_OPTS));
        
        // Count Enable Set register
        // When read, means the cycle counter is enabled. When written,
        // enables the cycle counter.
        // Enable [0 ~ 3] Performance Monitors Event Count Registers
        asm volatile("msr PMCNTENSET_EL0, %0" :: "r"(0x8000000f));
        
        // Cycle Count Filter Register
        // Every processor clock cycle
        asm volatile("msr PMCCFILTR_EL0, %0" :: "r"(0));
}

static void
disable_cpu_counters(void* data)
{
        printk(KERN_INFO "[" DRVR_NAME "] disabling user-mode PMU access on CPU #%d",
                smp_processor_id());

        /* Program PMU and disable all counters */
        asm volatile("msr PMCR_EL0, %0" :: "r"(0));
        asm volatile("msr PMCNTENSET_EL0, %0" :: "r"(0x8000000f));
        /* Disable user-mode access to counters. */
        asm volatile("msr PMUSERENR_EL0, %0" :: "r"(0));
}

static int __init
init(void)
{
        on_each_cpu(enable_cpu_counters, NULL, 1);
        printk(KERN_INFO "[" DRVR_NAME "] initialized");
        return 0;
}

static void __exit
fini(void)
{
        on_each_cpu(disable_cpu_counters, NULL, 1);
        printk(KERN_INFO "[" DRVR_NAME "] unloaded");
}

MODULE_LICENSE("Dual MIT/GPL");
MODULE_DESCRIPTION("Enables user-mode access to ARM PMU counters");
module_init(init);
module_exit(fini);
```
