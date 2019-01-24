- 1 概述
- 2 使用Sparse工具做类型检查
- 3 Kernel Macro
- 4 在Kernel编译Sparse静态检查

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




# 4. 在Kernel编译Sparse静态检查

- "make C=1"命令，来编译内核，会对所有重新编译的 C 文件使用 sparse 工具。
- "make C=2"命令，无论文件是否被重新编译都会对其使用 sparse 工具。


如果你已经编译了内核，用后一种方式可以很快地检查整个源码树。

> make 的可选变量 CHECKFLAGS 可以用来向 sparse 工具传递参数。编译系统会自动向 sparse 工具传递 -Wbitwise 参数。
