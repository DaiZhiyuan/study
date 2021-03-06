***QEMU架构概述***

<!-- TOC -->

- [1. 简介](#1-简介)
- [2. QEMU与KVM的关系](#2-qemu与kvm的关系)
- [3. KVM架构](#3-kvm架构)
    - [3.1 EPT](#31-ept)
- [4. QEMU的代码结构](#4-qemu的代码结构)
- [5. 设备模拟](#5-设备模拟)
- [6. QEMU中的虚拟机管理命令](#6-qemu中的虚拟机管理命令)
- [7. 块操作](#7-块操作)
- [8. QEMU源码编译](#8-qemu源码编译)

<!-- /TOC -->

# 1. 简介

QEMU的以下几个特点： 
- QEMU可以被当作模拟器，也可以被当作虚拟机。 
- 当QEMU被当作模拟器时，我们可以在一台机器上通过模拟设备，运行针对不同于本机上CPU的程序或者操作系统。（使用了动态翻译技术，在我们的虚拟化环境中并没有使用，因此不展开叙述） 
- 当QEMU被当作虚拟机使用时，QEMU必须基于Xen Hypervisor或者KVM内核模块才能支持虚拟化。在这种条件下QEMU虚拟机可以通过直接在本机CPU上运行客户机代码获得接近本机的性能。

# 2. QEMU与KVM的关系

当QEMU在模拟器模式下，运行操作系统时，我们可以认为这是一种软件实现的虚拟化技术，它的效率比真机差很多，用户可以明显地感觉出来。当QEMU在虚拟机模式下，QEMU必须在Linux上运行，并且需要借助KVM或者Xen，利用Intel或者Amd提供的硬件辅助虚拟化技术，才能使虚拟机达到接近真机的性能。QEMU与KVM内核模块协同工作，在虚拟机进程中，各司其职，又相互配合，最终实现高效的虚拟机应用。

QEMU与KVM的关系如下：

![image](./images/0x01.png)


KVM在物理机启动时创建/dev/kvm设备文件，当创建虚拟机时，KVM为该虚拟机进程创建一个VM的文件描述符，当创建vCPU时，KVM为每个vCPU创建一个文件描述符。同时，KVM向用户空间提供了一系列针对特殊设备文件的ioctl系统调用。QEMU主要是通过ioctl系统调用与KVM进行交互的。

那么QEMU和KVM具体都实现了哪些功能呢？我们用一张图说明：

![image](./images/0x02.png)

QEMU所实现的功能包括：虚拟机的配置和创建、虚拟机运行依赖的虚拟设备、虚拟机运行时用户操作环境和交互（vnc）以及一些针对虚拟机的特殊技术（如动态迁移），都是QEMU自己实现的。同时QEMU还实现了利用KVM提供的接口实现虚拟机硬件加速。 
而KVM的主要功能在于初始化CPU硬件，打开虚拟化模式，然后将虚拟客户机运行在虚拟机模式下，并对虚拟客户机的运行提供支持，这些支持主要是以针对相关的特殊设备文件的ioctl系统调用。外设的模拟一般不会由KVM负责，只有对性能要求较高的虚拟设备，如虚拟中断控制器和虚拟时钟，是由KVM模拟的，这样可以大量减少处理器的模式转换的开销。


# 3. KVM架构

![image](./images/0x03.png)


## 3.1 EPT

![image](./images/0x04.png)


# 4. QEMU的代码结构

线程事件驱动模型

![image](./images/0x05.png)


QEMU的体系结构正如上图展示的——每个vCPU都是一个线程，这些vCPU线程可以运行客户机的代码，以及虚拟中断控制器、虚拟时钟的模拟。而Main loop主线程则是Event-driver的，通过轮询文件描述符，调用对应的回调函数，处理由Monitor发出的命令、Timers超时，并且实现VNC、完成IO等功能。 
QEMU事件驱动的代码主要可以查看include/qemu/main-loop.h，以及相关的实现代码。

# 5. 设备模拟

QEMU为了实现大量设备的模拟，实现了比较完备的面向对象模型——QOM（QEMU Object Model）。QEMU对于CPU、内存、总线以及主板的模拟都是依赖于QOM的，QEMU中设备相关的数据结构的初始化工作都是依赖于QOM的初始化实现机制。对于它的实现主要可以查看include/qom/object.h。对于具体的CPU、内存等设备的模拟，可以查看include/qom/cpu.h、include/exec/memory.h、include/hw/qdev-core.h


# 6. QEMU中的虚拟机管理命令

QEMU中可以使用hmp command对虚拟机进行管理，在虚拟机环境中同时按住ctrl、Alt、2就可以进入QEMU的命令模式。通过输入命令，就可以进行虚拟机的管理。比如savevm命令可以把虚拟机的当前状态保存到虚拟机的磁盘中。这些命令的实现函数都有一个统一的命名方式：hmp_xxx，比如hmp_savevm就是savevm的实现函数的起始位置，hmp_migrate就是migrate的实现函数的起始位置。 
因此对于QEMU中的每一条命令都可以很快找到相关的实现函数。

# 7. 块操作

QEMU实现了大量的块设备驱动，从而支持了包括qcow2、qed、raw等格式的镜像，这些格式的实现代码都在block的文件夹下以及主目录下的block.c中。QEMU设计了BlockDriver数据结构，其中包含了大量的回调函数指针，对于每一种磁盘格式，都有一个对应的BlockDriver的对象，并且实现了BlockDriver中的回调函数，然后将这个BlockDriver的对象注册，即添加到一个全局的BlockDriver的链表中。


# 8. QEMU源码编译

QEMU的编译过程并不复杂，首先进入QEMU的代码目录后，首先运行./configure –help，查看qemu支持的特性。然后选择相关的特性进行编译。 
由于我们使用的X86_64的平台，并且主要查看的是QEMU与KVM协同工作实现虚拟化的代码，我们使用下列命令配置： 
```
$> ./configure --target-list=x86_64-softmmu --enable-sdl --enable-kvm --enable-uuid --enable-bzip2 --enable-numa --enable-libusb
```
上述命令会生成Makefile文件，然后直接make就可以了，为了加速编译可以使用多线程：make -j 4。