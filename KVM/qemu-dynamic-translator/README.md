***QEMU, a Fast and Portable Dynamic Translator***

<!-- TOC -->

- [1. 背景](#1-背景)
- [2. 概述](#2-概述)
- [3. 可移植的动态翻译](#3-可移植的动态翻译)
    - [3.1 描述](#31-描述)
    - [3.2 例子](#32-例子)
    - [3.3 Dyngen实现](#33-dyngen实现)
- [4 Dyngen实现细节](#4-dyngen实现细节)
    - [4.1 翻译块和翻译缓存](#41-翻译块和翻译缓存)
    - [4.2 寄存器分配](#42-寄存器分配)
    - [4.3 条件码优化](#43-条件码优化)
    - [4.4 直接块链](#44-直接块链)
    - [4.5 内存管理](#45-内存管理)
    - [4.6 自修改代码和被翻译代码失效](#46-自修改代码和被翻译代码失效)
    - [4.7 异常支持](#47-异常支持)
    - [4.8 硬件中断](#48-硬件中断)
    - [4.9 用户模式模拟](#49-用户模式模拟)
- [5. 接口工作](#5-接口工作)
- [6. 性能](#6-性能)
- [7. 结论与未来的工作](#7-结论与未来的工作)

<!-- /TOC -->

# 1. 背景

我们展示QEMU实现的内部原理，QEMU是一个快速的模拟器，它使用了一个原始的可移植的动态翻译器。它可以在多种主机上（x86、PowerPC、ARM、Sparc、Alpha、MIPS）拟多个CPU（x86、PowerPc、ARM、Sparc）。QEMU支持全系统的模拟，在这个模拟器中，一个完整的没有修改的操作系统可以运行在虚拟机和模拟的Linux用户空间中，一个被编译执行在一个目标CPU的Linux的进程可以执行在另一种CPU上。

# 2. 概述

QEMU是一个模拟器：它能在一个虚拟机中运行一个未修改的目标操作系统（比如Windows或者Linux）以及所有这个操作系统上的应用程序。QEMU本身可以运行在多个主机操作系统上比如Linux、Windows和Mac OS X。主机的CPU和目标CPU的类型可以是不同的。

QEMU的主要使用是在其他机器上运行一个操作系统，比如在Linux上运行Windows或者在Windows上运行Linux。另一个使用是调试，因为使用QEMU虚拟机能够容易地被中止，它的状态能够被监控、保存和恢复。而且，通过增加新的机器描述和新的模拟设备，特定的嵌入式设备能够被仿真出来。

QEMU还集成了一个Linux特定的用户模式模拟器。它使模拟器的子集。能够使运行在特定CPU的Linux进程运行在其他CPU上。它被主要用于测试交叉编译器的结果或者在不必开启完整虚拟机的情况下测试CPU的模拟器。

QEMU由下列几个子系统构成：
- CPU 模拟器（目前有x86、PowerPC、ARM和Sparc）
- 模拟设备（比如VGA显示、16450串行端口、PS/2的鼠标和键盘、IDE硬盘、NE2000网卡等等）
- 通用设备（比如块设备、字符设备、网络设备），被用来连接模拟设备和主机设备
- 机器的描述（比如PC、PowerMac、Sun4m），用于实例化模拟设备
- 调试器 
- 用户界面

这篇文章探讨了QEMU使用的动态翻译器的实现。动态翻译器可以在运行时执行由目标CPU指令向主机指令集转换。二进制结果码会被存储到一个翻译的缓存中，从而它能够被重复使用。与解释器比较的优势在于目标指令只会被获取和译码一次。

通常情况下，由一个主机向另一种主机转换的动态翻译器是困难的，因为全部的代码生成器必须被重写。这代表着，与向C的编译器中加入新的目标语言相同数量的工作量。QEMU是更加简单的，因为它只是连接被GNU C编译器离线生成的机器码的片段。

一个CPU的模拟器也要面对其他的更多的经典但是困难的问题： 
- 管理被翻译的代码的缓存
- 寄存器分配 
- 条件代码优化 
- 直接块链接 
- 内存管理 
- 自修改代码的支持 
- 异常的支持 
- 硬件的中断 
- 用户模式模拟

# 3. 可移植的动态翻译

## 3.1 描述

第一步，是将每个目标CPU指令分解成更少的更简单的指令，被称作微操作。每个微操作都通过一小段C代码实现。这段小的C源代码被GCC编译成一个目标文件。微操作会被挑选，从而使它们的数目（典型的有几百）相比于所有目标CPU的指令和操作的集合。由目标CPU的指令向微操作的翻译是全部由手动编码的。为了代码的可读性和紧密性，代码进行了优化，因为这一阶段的速度不比在一个解释器中重要。

一个被称作dyngen的编译时工具使用包含了微操作的object文件作为输入来生成一个动态的代码生成器。这个动态地代码生成器在运行时会被调用来生成一个连接着多个微操作的完整的主机函数。

为了获得更好的性能在编译时做了更多的工作。特别是，一个关键的想法是在QEMU中，常数参数能够被交到微操作中。为了这个目的，需要使用GCC为每个常数参数生成伪代码重定位。这使得dyngen工具能够定位重定位位置，以及在创建动态代码时生成合适的C代码解决它们。支持重定位，从而能够访问微操作中的静态数据和其他的函数。

## 3.2 例子

考虑我们必须翻译系列PowerPC指令到x86指令的情况：
```asm
addi r1,r1,-16  # r1 = r1 - 16
```

下列微操作会被PowerPC的代码翻译器生成：
```asm
movl_T0_r1      # T0 = r1
addl_T0_im -16  # T0 = T0 - 16
movl_r1_T0      # r1 = T0
```

在我们通过大量地对生成代码质量影响，最小化微操作的数目。例如，我们只是在一些临时寄存器中进行move的操作，而不是在32个PowerPC寄存器中的每一个中生成可能的move操作。这些寄存器T0，T1，T2，通常情况下，通过使用GCC静态寄存器变量扩展被存储到主机的寄存器中。

微操作movl_T0_r1在通常情况下被编码为：
```c
void op_movl_T0_r1(void)
{
    T0 = env->regs[1];
}
```

env是一个包含目标CPU状态的结构。32个PowerPC寄存器被被保存到数组env->regs[32]。

addl_T0_im更有趣，因为他使用了一个常量，这个常量的值会在运行时决定。

```c
extern int __op_param1;
void op_addl_T0_im(void)
{
    T0 = T0 + ((long)(&__op_param1));
}
```

被dyngen生成的代码生成器使用了一个被opc_ptr指向的微操作的流，并且在gen_code_ptr位置输出了主机代码。opparam_ptr指向微操作的参数。

```c
[...]
for(;;) {
    switch(*opc_ptr++) {
    [...]
    case INDEX_op_movl_T0_r1:
    {
        extern void op_movl_T0_r1();
        memcpy(gen_code_ptr,
            (char *)&op_movl_T0_r1+0,
            3);
        gen_code_ptr += 3;
        break;
    }
    case INDEX_op_addl_T0_im:
    {
        long param1;
        extern void op_addl_T0_im();
        memcpy(gen_code_ptr,
            (char *)&op_addl_T0_im+0,
        6);
        param1 = *opparam_ptr++;
        *(uint32_t *)(gen_code_ptr + 2) =
            param1;
        gen_code_ptr += 6;
        break;
    }
    [...]
    }
}
[...]
}
```

对于大多数的微操作比如movl_T0_r1,只是拷贝GCC生成的主机代码。当使用常量参数时，dyngen利用重定位到__op_param1是被gcc生成的事实，实现了运行时参数的生成代码。（这里被称作param1）

当代码生成器运行时，下列的主机代码会被输出：
```asm
# movl_T0_r1
# ebx = env->regs[1]
mov 0x4(%ebp),%ebx
# addl_T0_im -16
# ebx = ebx - 16
add $0xfffffff0,%ebx
# movl_r1_T0
# env->regs[1] = ebx
mov %ebx,0x4(%ebp)
```

在x86上，T0被映射到ebx寄存器，CPU状态上下文被映射到ebp寄存器。

## 3.3 Dyngen实现

dyngen工具是QEMU翻译进程的关键。在一个包含微操作的object文件上运行它时，下列任务会被执行。

- object文件会被分析，从而得到它的符号表，它的重定位位置和它的代码部分。这一遍扫描依赖于主机上的object文件的格式（dyngen支持ELF（Linux）、PE-coff（Windows）、MACH-O（MAC OS X））。 
- 使用符号表，微操作被定位在代码部分。一个主机特定的方法会被执行，从而得到被拷贝的代码的开始和结束。通常情况下，函数的头部和尾部会被跳过。 
- 每个微操作的重定位会被检查来获得常数变量的数量。通过常数变量会使用特定的符号名，来检测常数变量的重定位。 
- 在C中一个内存的拷贝会被生成来拷贝微操作的代码。每个微操作的代码的重定位位置会被用来修改被拷贝代码从而使得它能合理地重定位。重定位的修改是具体到每一台主机的。 
- 对于一些主机比如ARM，变量必须被存储到接近生成代码的区域，这些主机才能通过一个小偏移的相对寻址访问这些变量。一个主机的特定的扫描完成，实现在生成代码中重定位这些常量。

当编译微操作代码时，一系列的GCC参数会被用来控制函数头部和尾部代码的格式的生成，从而使得它容易分析。一个伪汇编的宏促使GCC总是能终止每个有单一return指令的宏操作的函数。如果在一个单一的微操作中生成了多个return指令，代码的连接将不会成功。

# 4 Dyngen实现细节

## 4.1 翻译块和翻译缓存

当QEMU第一次遇到一段目标代码时，它将目标代码翻译成主机代码，直到下一条jump指令或者在翻译时间使用了一种无法推断的方式改变了静态CPU状态的指令。我们将这些基本的块称作翻译块（Translated Blocks，TBs）。

一个16M的缓存会用来存储最近最常用的TB。为了简单，它会在满的时候被完整地flush掉。

静态的CPU状态会被定义为CPU状态的一部分，它被认为在进入TB时，在翻译时间是已知的。例如，对所有目标CPU，在翻译时间程序计数器（PC）是已知的。在x86上，静态地CPU状态包括更多的数据，从而能够生成更好的代码。例如直到CPU处于保护模式还是实模式，在用户模式还是内核模式，以及默认的操作数位是16位还是32位，是重要的。

## 4.2 寄存器分配

QEMU使用一个固定的寄存器分配。这意味着每个目标CPU寄存器会被映射到一个固定的主机寄存器或者内存地址。在大多数主机上，我们简单地将所有的目标寄存器映射到内存，并且只存储少量的临时变量到主机寄存器。临时变量的分配会被硬编码到每个目标CPU描述的硬编码中。这种方法的好处是简单和便捷。

未来QEMU的版本会使用动态地临时寄存器分配来消除一些在目标寄存器能直接存储到主机寄存器时非必要的move操作。

## 4.3 条件码优化

好的CPU的条件码模拟（在x86上eflags寄存器）对于得到好的性能是重要的一点。

QEMU使用懒惰条件码评估：它只是存储一个操作数（被称作CC_SRC），结果（被称作CC_DST），操作的类型（被称作CC_OP），而不是在每次x86指令之后计算条件码。对于32位加法，例如R = A + B，我们有：
```c
CC_SRC=A
CC_DST=R
CC_OP=CC_OP_ADDL
```

我们能从CC_OP中存储的常量直到我们进行了32位的加法，我们能够通过CC_SRC和CC_DST得到A、B和R。然后如果下一条指令需要，所有的相关的条件码比如零结果（ZF）、非正结果（SF）、借位（CF）或溢出（OF）也能得到。

条件码评估在翻译时间利用一个完整的TB的代码只会生成一次的事实会被大幅度优化。一个回溯的扫描会在生成的代码中被执行，来查看CC_OP、CC_SRC或者CC_DST是否没有再接下来的代码中使用。在TB的最后我们只考虑会被用到的变量。然后我们删除在之后的代码中不会用到的值的赋值。

## 4.4 直接块链

在每个TB被执行后，QEMU使用仿真的程序计数器（PC）以及其他的静态地CPU状态，使用一个哈希表，来找到下一个TB。如果下一个TB还没有被翻译，一个新的翻译会被启动。否则，跳转到下一个TB。

在大多数情况下，为了加速，新的仿真的PC是已知的时候（例如在一个条件跳转之后），QEMU会修改TB使它能直接跳转到下一个TB。

最便捷的代码使用一个间接跳转。在一些主机上（比如x86、PowerPC），一个分支指令会被直接修改，从而使得块链接没有小号。

## 4.5 内存管理

对于系统模拟，QEMU使用mmap()系统调用来模拟目标MMU。只要模拟的操作系统不使用被主机操作系统保留的区域，它就能正常工作。

为了能在任何操作系统上启动，QEMU也支持一个软件形式的MMU。在这种模式下，在每次内存访问时，MMU虚拟地址到屋里地址的翻译都要完成。QEMU使用一个地址翻译缓冲来加速这种翻译。

为了避免每次MMU映射改变时flush掉被翻译的代码，QEMU使用一个物理的索引翻译缓冲。这意味着每个TB会通过它的物理地址被索引。

## 4.6 自修改代码和被翻译代码失效

在大多数CPU上，自修改代码是容易处理的，因为一个特定的代码缓冲失效指令会被执行来发信号表明代码已经被改变。这足以使相关的被翻译的代码失效。

然而在一些CPU上，比如x86，当代码被改变时，没有代码缓冲失效通过应用程序被发信号表明，自修改的代码时一个特殊的挑战。

当一个TB的翻译的代码被生成时，相关的主机内存也会被写保护，如果这些内存页不是只读的。如果一个写访问到达这个内存页，QEMU会其中所有被翻译的代码失效，然后重新允许写访问它。

正确的翻译代码失效可以通过维持一个包含在一个给定的内存页中的，包含所有翻译快的链表，来实现高效性。其他链表也要维持来取消直接块链接。

当使用一个软件MMU时，代码失效是更加有效的：如果一个给定的代码页也是失效的，经常是因为写访问，那么一个代表所有在该页中创建代码的位图会被创建。每次存储到该页都会检查位图，查看代码是否真的需要失效。它会避免只有该页中数据被修改时代码的失效。

## 4.7 异常支持

当遇到异常，比如除0，longjmp()被用来跳转到异常处理函数。当没有使用软件MMU时，主机信号句柄会用来截获失效的内存访问。

QEMU支持明确的异常，这种异常是指在异常发生时，总是能得到准确的目标CPU的状态。对于大多数目标CPU的状态不行也要做任何事情，因为它会被翻译的代码存储和改变。不会被明确存储的目标CPU状态S（例如当前程序计数器）会通过重新翻译异常出现的TB来得到，在一种S会在每条翻译目标指令之前被记录的模式下。异常发生时的主机程序计数器会被用来发现相关的目标执行和状态S。

## 4.8 硬件中断

为了更快，如果一个硬件中断要执行，QEMU不会在每个TB检查，而是用户必须异步地调用一个特定的函数来通知一个中断要执行。这个函数重置了当前执行TB的链。它保证在CPU模拟器的main循环中会迅速返回。然后main循环测试一个中断是否要执行，然后处理它。

## 4.9 用户模式模拟

为了将一个编译在一个目标CPU的Linux进程运行在其他CPU上，QEMU支持用户模式模拟。

在CPU级别上，用户模式模拟是全系统模拟的子集。由于QEMU假设用户内存映射由主机操作系统完成，没有执行MMU的仿真。QEMU包含一个通用的Linux系统调用转换来处理字节序问题和32位/64位转换。由于QEMU支持异常，它准确地模拟了目标信号。每个目标线程运行在一个主机线程上。

# 5. 接口工作

为了移植QEMU到新的主机CPU，下列的事项必须完成： 
- dyngen必须被移植（查看3.2部分）
- 为了优化性能，通过微操作使用的临时变量可能需要被映射到特定的寄存器上 
- 为了维持指令缓存和内存的相关性，大多数主机CPU需要特定的指令
- 如果通过修改分支指令，直接块链接被实现，一些特定的汇编宏必须被提供

QEMU移植的整体复杂度被评估为与一个动态链接相同。

# 6. 性能

为了衡量由于模拟产生的消耗，我们比较x86平台的Linux机器上，在本机下运行BYTEmark benchmark的性能与x86目标机器用户模式模拟下运行该benchmark的性能。

利用用户模式QEMU（版本0.4.2）测得，运行整数代码其比本机下运行慢4倍，运行浮点数代码，其比本机下运行慢10倍。这可以理解为是由于在静态的CPU状态中缺少x86 FPU栈指针。在全系统模拟中，软件MMU使得速度运行下降了一半。

在全系统模拟中，QEMU大约比Bochs快30倍。

用户模式的QEMU比1.9.6版本的valgrind –skin=none快1.2倍，这是一个正常情况下，用于调试程序的，手动编码的x86到x86的动态翻译器。–skin=none参数保证Valgrind不会生成调试代码。

# 7. 结论与未来的工作

QEMU已经到达对于日常工作有用的程度，特别是对商业的x86操作系统例如Windows。PowerPC的目标接近于启动Mac OS X，Sparc目标接近于启动Linux。没有其他的动态翻译器如今能在如此多的主机上支持如此多的目标，主要是因为移植复杂性被低估。QEMU的方法在性能和复杂性上看起来是一个好的折中。

未来，以下几点仍然需要解决： 
- 移植：QEMU能够很好地支持PowerPC和x86的主机。其他的在Sparc、Alpha、ARM和MIPS的移植需要改进。QEMU也严重依赖于用来编译微操作定义的特定的GCC的版本。 
- 全系统模拟：ARM和MIPS目标CPU需要被增加进来。 
- 性能：软件MMU的性能能够被提高。一些重要的微操作也能够在汇编器中手动编码，而不需要对目前的翻译的框架进行大量的修改。CPU的main循环也能在汇编器中进行手动编码。
- 虚拟化：当主机和目标CPU相同时，可以运行大部分的代码。最简单的实现是像通常情况下运行目标用户代码一样，模拟目标内核的代码。 
- 调试：缓存仿真和循环计数器能够被加入，来制作在SIMICS中的调试器。

> https://www.usenix.org/legacy/event/usenix05/tech/freenix/full_papers/bellard/bellard_html/