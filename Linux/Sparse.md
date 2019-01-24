# 1. 概述

Sparse 诞生于 2004 年, 是由linux之父Linus开发的, 目的就是提供一个静态检查代码的工具, 从而减少linux内核的隐患。
内核代码中有一个简略的关于Sparse的说明文件:Documentation/sparse.txt。

这里我们主要关注__attribute__((bitwise))的作用。通过阅读资料以及实验，关于bitwise有获得如下认知：

- Sparse是一个独立于gcc的工具，虽然__attribute__((bitwise))看起来是gcc的属性声明格式，实际上gcc并不会处理这个属性。
- 在代码里经常会看到用\_\_le16, \_\_le32, \_\_le64, \_\_be16, \_\_be32, \_\_be64, 修饰某些变量，其实这些都是宏定义，最终会使用\_\_attribute\_\_((bitwise))。le与be分别表示little endian和big endian。
- "bitwise"是用来确保不同位方式类型不会被弄混（小端，大端，native CPU）它提供了非常强的类型检查，如果不同类型之间进行赋值，即便强制类型转换，Sparse仍然会发出警告。
- 在网络编程里面面，对字节序的要求非常高，bitwise可以很好的帮助检查潜在的错误。
- bitwise的典型用法是利用typedef定义一个有bitwise属性的基类型，之后凡是利用该基类型声明的变量都将被强制类型检查。
- Sparse的典型用法是：sparse file.c

# 2. 使用Sparse工具做类型检查

Sparse除了能够用在内核代码的静态分析上，其实也可以用在一般的C语言程序中，例如：

```c
#include <stdio.h>

#define __acquire(x) __context__(x,1)
#define __release(x) __context__(x,-1)

int main(int argc, char **argv)
{
    int lock = 1;
    __acquire(lock);

    printf("todo something");

    //__release(lock);

    return 0;
}
```

如果\_\_acquire与\_\_release不是成对出现，Sparse则会发出警告：
```
[root@lab workspace]# sparse demo.c
demo.c:6:5: warning: context imbalance in 'main' - wrong count at exit
```

# 3. Kernel Macro

SOURCE/include/linux/compiler_types.h

```c
#ifdef __CHECKER__
    # define __user     __attribute__((noderef, address_space(1)))
    # define __kernel   __attribute__((address_space(0)))
    # define __safe     __attribute__((safe))
    # define __force    __attribute__((force))
    # define __nocast   __attribute__((nocast))
    # define __iomem    __attribute__((noderef, address_space(2)))
    # define __must_hold(x) __attribute__((context(x,1,1)))
    # define __acquires(x)  __attribute__((context(x,0,1)))
    # define __releases(x)  __attribute__((context(x,1,0)))
    # define __acquire(x)   __context__(x,1)
    # define __release(x)   __context__(x,-1)
    # define __cond_lock(x,c)   ((c) ? ({ __acquire(x); 1; }) : 0)
    # define __percpu   __attribute__((noderef, address_space(3)))
    # define __rcu      __attribute__((noderef, address_space(4)))
    # define __private  __attribute__((noderef))
    extern void __chk_user_ptr(const volatile void __user *);
    extern void __chk_io_ptr(const volatile void __iomem *);
    # define ACCESS_PRIVATE(p, member) (*((typeof((p)->member) __force *) &(p)->member))
#else /* __CHECKER__ */
    # ifdef STRUCTLEAK_PLUGIN
    #  define __user __attribute__((user))
    # else
    #  define __user
    # endif
    # define __kernel
    # define __safe
    # define __force
    # define __nocast
    # define __iomem
    # define __chk_user_ptr(x) (void)0
    # define __chk_io_ptr(x) (void)0
    # define __builtin_warning(x, y...) (1)
    # define __must_hold(x)
    # define __acquires(x)
    # define __releases(x)
    # define __acquire(x) (void)0
    # define __release(x) (void)0
    # define __cond_lock(x,c) (c)
    # define __percpu
    # define __rcu
    # define __private
    # define ACCESS_PRIVATE(p, member) ((p)->member)
#endif /* __CHECKER__ */
```

宏名称 | 宏定义 | 检查点
---|---|---
__bitwise | __attribute__((bitwise)) | 确保变量是相同的位方式(比如 bit-endian, little-endian)
__user | __attribute__((noderef, address_space(1))) | 指针地址必须在用户地址空间
__kernel | __attribute__((noderef, address_space(0))) | 指针地址必须在内核地址空间
__iomem | __attribute__((noderef, address_space(2))) | 指针地址必须在设备地址空间
__safe | __attribute__((safe)) | 变量可以为空
__force | __attribute__((force)) | 变量可以进行强制转换
__nocast | __attribute__((nocast)) | 参数类型与实际参数类型必须一致
__acquires(x) | __attribute__((context(x, 0, 1))) | 参数x 在执行前引用计数必须是0，执行后，引用计数必须为1
__releases(x) | __attribute__((context(x, 1, 0))) | 参数x 在执行前引用计数必须是1，执行后，引用计数必须为1
__acquire(x) | __context__(x,  1) | 参数x 的引用计数 +1
__release(x) | __context__(x, -1) | 参数x 的引用计数 -1
__cond_lock(x,c) | ((c) ? ({ __acquire(x); 1; }) : 0) | 参数c 不为0时,引用计数 +1, 并返回1

kernel代码实例：

```c
typedef __u32 __bitwise     __le32;
typedef __u32 __bitwise     __be32;

static inline bool virtio_legacy_is_little_endian(void)
{
#ifdef __LITTLE_ENDIAN
    return true;
#else
    return false;
#endif
}
```

```
static inline u16 __virtio16_to_cpu(bool little_endian, __virtio16 val)
{
    if (little_endian)
        return le16_to_cpu((__force __le16)val);
    else
        return be16_to_cpu((__force __be16)val);
}

static inline __virtio16 __cpu_to_virtio16(bool little_endian, u16 val)
{
    if (little_endian)
        return (__force __virtio16)cpu_to_le16(val);
    else
        return (__force __virtio16)cpu_to_be16(val);
}
```

Gcc预定义宏：
```
[root@lab workspace]# cpp -dM /dev/null
#define __DBL_MIN_EXP__ (-1021)
#define __UINT_LEAST16_MAX__ 65535
#define __ATOMIC_ACQUIRE 2
#define __FLT_MIN__ 1.17549435082228750797e-38F
#define __UINT_LEAST8_TYPE__ unsigned char
#define __INTMAX_C(c) c ## L
#define __CHAR_BIT__ 8
#define __UINT8_MAX__ 255
#define __WINT_MAX__ 4294967295U
#define __ORDER_LITTLE_ENDIAN__ 1234
#define __SIZE_MAX__ 18446744073709551615UL
#define __WCHAR_MAX__ 2147483647
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_1 1
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_2 1
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_4 1
#define __DBL_DENORM_MIN__ ((double)4.94065645841246544177e-324L)
#define __GCC_HAVE_SYNC_COMPARE_AND_SWAP_8 1
#define __GCC_ATOMIC_CHAR_LOCK_FREE 2
#define __FLT_EVAL_METHOD__ 0
#define __unix__ 1
#define __GCC_ATOMIC_CHAR32_T_LOCK_FREE 2
#define __x86_64 1
#define __UINT_FAST64_MAX__ 18446744073709551615UL
#define __SIG_ATOMIC_TYPE__ int
#define __DBL_MIN_10_EXP__ (-307)
#define __FINITE_MATH_ONLY__ 0
#define __GNUC_PATCHLEVEL__ 5
#define __UINT_FAST8_MAX__ 255
#define __DEC64_MAX_EXP__ 385
#define __INT8_C(c) c
#define __UINT_LEAST64_MAX__ 18446744073709551615UL
#define __SHRT_MAX__ 32767
#define __LDBL_MAX__ 1.18973149535723176502e+4932L
#define __UINT_LEAST8_MAX__ 255
#define __GCC_ATOMIC_BOOL_LOCK_FREE 2
#define __UINTMAX_TYPE__ long unsigned int
#define __linux 1
#define __DEC32_EPSILON__ 1E-6DF
#define __unix 1
#define __UINT32_MAX__ 4294967295U
#define __LDBL_MAX_EXP__ 16384
#define __WINT_MIN__ 0U
#define __linux__ 1
#define __SCHAR_MAX__ 127
#define __WCHAR_MIN__ (-__WCHAR_MAX__ - 1)
#define __INT64_C(c) c ## L
#define __DBL_DIG__ 15
#define __GCC_ATOMIC_POINTER_LOCK_FREE 2
#define __SIZEOF_INT__ 4
#define __SIZEOF_POINTER__ 8
#define __USER_LABEL_PREFIX__
#define __STDC_HOSTED__ 1
#define __LDBL_HAS_INFINITY__ 1
#define __FLT_EPSILON__ 1.19209289550781250000e-7F
#define __LDBL_MIN__ 3.36210314311209350626e-4932L
#define __DEC32_MAX__ 9.999999E96DF
#define __INT32_MAX__ 2147483647
#define __SIZEOF_LONG__ 8
#define __STDC_IEC_559__ 1
#define __STDC_ISO_10646__ 201103L
#define __UINT16_C(c) c
#define __DECIMAL_DIG__ 21
#define __gnu_linux__ 1
#define __LDBL_HAS_QUIET_NAN__ 1
#define __GNUC__ 4
#define __MMX__ 1
#define __FLT_HAS_DENORM__ 1
#define __SIZEOF_LONG_DOUBLE__ 16
#define __BIGGEST_ALIGNMENT__ 16
#define __DBL_MAX__ ((double)1.79769313486231570815e+308L)
#define __INT_FAST32_MAX__ 9223372036854775807L
#define __DBL_HAS_INFINITY__ 1
#define __DEC32_MIN_EXP__ (-94)
#define __INT_FAST16_TYPE__ long int
#define __LDBL_HAS_DENORM__ 1
#define __DEC128_MAX__ 9.999999999999999999999999999999999E6144DL
#define __INT_LEAST32_MAX__ 2147483647
#define __DEC32_MIN__ 1E-95DF
#define __DBL_MAX_EXP__ 1024
#define __DEC128_EPSILON__ 1E-33DL
#define __SSE2_MATH__ 1
#define __ATOMIC_HLE_RELEASE 131072
#define __PTRDIFF_MAX__ 9223372036854775807L
#define __amd64 1
#define __STDC_NO_THREADS__ 1
#define __ATOMIC_HLE_ACQUIRE 65536
#define __LONG_LONG_MAX__ 9223372036854775807LL
#define __SIZEOF_SIZE_T__ 8
#define __SIZEOF_WINT_T__ 4
#define __GCC_HAVE_DWARF2_CFI_ASM 1
#define __GXX_ABI_VERSION 1002
#define __FLT_MIN_EXP__ (-125)
#define __INT_FAST64_TYPE__ long int
#define __DBL_MIN__ ((double)2.22507385850720138309e-308L)
#define __LP64__ 1
#define __DECIMAL_BID_FORMAT__ 1
#define __DEC128_MIN__ 1E-6143DL
#define __REGISTER_PREFIX__
#define __UINT16_MAX__ 65535
#define __DBL_HAS_DENORM__ 1
#define __UINT8_TYPE__ unsigned char
#define __NO_INLINE__ 1
#define __FLT_MANT_DIG__ 24
#define __VERSION__ "4.8.5 20150623 (Red Hat 4.8.5-28)"
#define __UINT64_C(c) c ## UL
#define _STDC_PREDEF_H 1
#define __GCC_ATOMIC_INT_LOCK_FREE 2
#define __FLOAT_WORD_ORDER__ __ORDER_LITTLE_ENDIAN__
#define __STDC_IEC_559_COMPLEX__ 1
#define __INT32_C(c) c
#define __DEC64_EPSILON__ 1E-15DD
#define __ORDER_PDP_ENDIAN__ 3412
#define __DEC128_MIN_EXP__ (-6142)
#define __INT_FAST32_TYPE__ long int
#define __UINT_LEAST16_TYPE__ short unsigned int
#define unix 1
#define __INT16_MAX__ 32767
#define __SIZE_TYPE__ long unsigned int
#define __UINT64_MAX__ 18446744073709551615UL
#define __INT8_TYPE__ signed char
#define __ELF__ 1
#define __FLT_RADIX__ 2
#define __INT_LEAST16_TYPE__ short int
#define __LDBL_EPSILON__ 1.08420217248550443401e-19L
#define __UINTMAX_C(c) c ## UL
#define __GNUC_RH_RELEASE__ 28
#define __SSE_MATH__ 1
#define __k8 1
#define __SIG_ATOMIC_MAX__ 2147483647
#define __GCC_ATOMIC_WCHAR_T_LOCK_FREE 2
#define __SIZEOF_PTRDIFF_T__ 8
#define __x86_64__ 1
#define __DEC32_SUBNORMAL_MIN__ 0.000001E-95DF
#define __INT_FAST16_MAX__ 9223372036854775807L
#define __UINT_FAST32_MAX__ 18446744073709551615UL
#define __UINT_LEAST64_TYPE__ long unsigned int
#define __FLT_HAS_QUIET_NAN__ 1
#define __FLT_MAX_10_EXP__ 38
#define __LONG_MAX__ 9223372036854775807L
#define __DEC128_SUBNORMAL_MIN__ 0.000000000000000000000000000000001E-6143DL
#define __FLT_HAS_INFINITY__ 1
#define __UINT_FAST16_TYPE__ long unsigned int
#define __DEC64_MAX__ 9.999999999999999E384DD
#define __CHAR16_TYPE__ short unsigned int
#define __PRAGMA_REDEFINE_EXTNAME 1
#define __INT_LEAST16_MAX__ 32767
#define __DEC64_MANT_DIG__ 16
#define __INT64_MAX__ 9223372036854775807L
#define __UINT_LEAST32_MAX__ 4294967295U
#define __GCC_ATOMIC_LONG_LOCK_FREE 2
#define __INT_LEAST64_TYPE__ long int
#define __INT16_TYPE__ short int
#define __INT_LEAST8_TYPE__ signed char
#define __DEC32_MAX_EXP__ 97
#define __INT_FAST8_MAX__ 127
#define __INTPTR_MAX__ 9223372036854775807L
#define linux 1
#define __SSE2__ 1
#define __LDBL_MANT_DIG__ 64
#define __DBL_HAS_QUIET_NAN__ 1
#define __SIG_ATOMIC_MIN__ (-__SIG_ATOMIC_MAX__ - 1)
#define __code_model_small__ 1
#define __k8__ 1
#define __INTPTR_TYPE__ long int
#define __UINT16_TYPE__ short unsigned int
#define __WCHAR_TYPE__ int
#define __SIZEOF_FLOAT__ 4
#define __UINTPTR_MAX__ 18446744073709551615UL
#define __DEC64_MIN_EXP__ (-382)
#define __INT_FAST64_MAX__ 9223372036854775807L
#define __GCC_ATOMIC_TEST_AND_SET_TRUEVAL 1
#define __FLT_DIG__ 6
#define __UINT_FAST64_TYPE__ long unsigned int
#define __INT_MAX__ 2147483647
#define __amd64__ 1
#define __INT64_TYPE__ long int
#define __FLT_MAX_EXP__ 128
#define __ORDER_BIG_ENDIAN__ 4321
#define __DBL_MANT_DIG__ 53
#define __INT_LEAST64_MAX__ 9223372036854775807L
#define __GCC_ATOMIC_CHAR16_T_LOCK_FREE 2
#define __DEC64_MIN__ 1E-383DD
#define __WINT_TYPE__ unsigned int
#define __UINT_LEAST32_TYPE__ unsigned int
#define __SIZEOF_SHORT__ 2
#define __SSE__ 1
#define __LDBL_MIN_EXP__ (-16381)
#define __INT_LEAST8_MAX__ 127
#define __SIZEOF_INT128__ 16
#define __LDBL_MAX_10_EXP__ 4932
#define __ATOMIC_RELAXED 0
#define __DBL_EPSILON__ ((double)2.22044604925031308085e-16L)
#define _LP64 1
#define __UINT8_C(c) c
#define __INT_LEAST32_TYPE__ int
#define __SIZEOF_WCHAR_T__ 4
#define __UINT64_TYPE__ long unsigned int
#define __INT_FAST8_TYPE__ signed char
#define __DBL_DECIMAL_DIG__ 17
#define __FXSR__ 1
#define __DEC_EVAL_METHOD__ 2
#define __UINT32_C(c) c ## U
#define __INTMAX_MAX__ 9223372036854775807L
#define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#define __FLT_DENORM_MIN__ 1.40129846432481707092e-45F
#define __INT8_MAX__ 127
#define __UINT_FAST32_TYPE__ long unsigned int
#define __CHAR32_TYPE__ unsigned int
#define __FLT_MAX__ 3.40282346638528859812e+38F
#define __INT32_TYPE__ int
#define __SIZEOF_DOUBLE__ 8
#define __FLT_MIN_10_EXP__ (-37)
#define __INTMAX_TYPE__ long int
#define __DEC128_MAX_EXP__ 6145
#define __ATOMIC_CONSUME 1
#define __GNUC_MINOR__ 8
#define __UINTMAX_MAX__ 18446744073709551615UL
#define __DEC32_MANT_DIG__ 7
#define __DBL_MAX_10_EXP__ 308
#define __LDBL_DENORM_MIN__ 3.64519953188247460253e-4951L
#define __INT16_C(c) c
#define __STDC__ 1
#define __PTRDIFF_TYPE__ long int
#define __ATOMIC_SEQ_CST 5
#define __UINT32_TYPE__ unsigned int
#define __UINTPTR_TYPE__ long unsigned int
#define __DEC64_SUBNORMAL_MIN__ 0.000000000000001E-383DD
#define __DEC128_MANT_DIG__ 34
#define __LDBL_MIN_10_EXP__ (-4931)
#define __SIZEOF_LONG_LONG__ 8
#define __GCC_ATOMIC_LLONG_LOCK_FREE 2
#define __LDBL_DIG__ 18
#define __FLT_DECIMAL_DIG__ 9
#define __UINT_FAST16_MAX__ 18446744073709551615UL
#define __GNUC_GNU_INLINE__ 1
#define __GCC_ATOMIC_SHORT_LOCK_FREE 2
#define __UINT_FAST8_TYPE__ unsigned char
#define __ATOMIC_ACQ_REL 4
#define __ATOMIC_RELEASE 3
```


# 4. 在Kernel编译Sparse静态检查

- "make C=1"命令，来编译内核，会对所有重新编译的 C 文件使用 sparse 工具。
- "make C=2"命令，无论文件是否被重新编译都会对其使用 sparse 工具。


如果你已经编译了内核，用后一种方式可以很快地检查整个源码树。

> make 的可选变量 CHECKFLAGS 可以用来向 sparse 工具传递参数。编译系统会自动向 sparse 工具传递 -Wbitwise 参数。
