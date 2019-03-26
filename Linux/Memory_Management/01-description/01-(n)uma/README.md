***3种系统架构与2种存储器共享方式***

- 1 3种系统架构与2种存储器共享方式
    - 1.1 架构概述
    - 1.2 SMP（Symmetric Multi-Processor）
    - 1.3 NUMA（Non-Uniform Memory Access）
        - 1.3.1 NUMA Hierarchy
        - 1.3.2 NUMA Node内部
        - 1.3.3 本地内存
        - 1.3.4 本地I/O资源
        - 1.3.5 NUMA Node互联
        - 1.3.6 NUMA Affinity
        - 1.3.7 Firmware接口
    - 1.4 MPP（Massive Parallel Processing）
- 2 三种体系架构之间的差异
    - 2.1 NUMA、MPP、SMP之间性能的区别
    - 2.2 NUMA、MPP、SMP之间扩展的区别
    - 2.3 NUMA、MPP、SMP应用之间的区别
        - 2.3.1 MPP的优势
        - 2.3.2 SMP的优势
        - 2.3.3 NUMA的优势
- 3 总结

# 1. 3种系统架构与2种存储器共享方式

## 1.1 架构概述

从系统架构来看，目前的商用服务器大体可以分为三类：
- 对称多处理器结构（SMP：Symmetric Multi-Processer）
- 非一致性存储访问结构（NUMA：Non-Uniform Memeory Access）
- 海量并行处理结构（MPP：Massive Parallel Processing）

共享存储型多处理器有两种模型：
- 均匀存储器存取（Uniform Memory Access，简称UMA）模型
- 非均匀存储器存取（Non-Uniform Memory Access，简称NUMA）模型

而我们后面所提到的COMA和ccNUMA模型都是NUMA模型的改进。

## 1.2 SMP（Symmetric Multi-Processor）

所谓对称多处理器结构，是指服务器中多个CPU对等工作，无主次或从属关系。各CPU共享相同的物理存储，每个CPU访问内存中的任何地址所需时间是相同的，因此SMP也被称为一致性存储器访问结构即UMA结构。

对SMP服务器进程扩展的方式包括增加内存、使用更快的CPU、增加CPU个数，扩充I/O（槽口数与总线数）以及添加更多的外部设备（通常是磁盘存储）。

SMP服务器的主要特征是共享，系统中所有资源（CPU、内存、I/O等）都是共享的。也正是由于这种特征，导致了SMP服务器的主要问题，那就是它的扩展能力非常有限。

对于SMP服务器而言，每一个共享的环节都可能造成SMP服务器扩展时的瓶颈，而最受限制的则是内存。由于每个CPU必须通过相同的内存总线访问相同的内存资源，因此随着CPU个数的增加，内存访问冲突（设计内存一致性模型）将迅速增加，最终会造成CPU资源的浪费，使CPU性能的有效性大大降低。实验证明，SMP服务器CPU利用率最好的情况是2到4个CPU。

![image](./images/0x01.gif)

图中，物理存储器被所有处理器均匀共享。所有处理器对所有存储自具有相同的存取时间，这就是为什么称它为均匀存储器存取的原因。每个处理器可以有自己的私有高速缓存，外围设备也以一定形式共享。

## 1.3 NUMA（Non-Uniform Memory Access）

由于SMP的扩展能力上的限制，人们开始探究如何进行有效地扩展而构建大型系统的计数，NUMA就是这种努力下的结果之一。利用NUMA计数，可以把几十个CPU（甚至上百个CPU）组合在一个服务器内。

![image](./images/0x02.png)

NUMA多处理器模型如图所述，其访问时间随存储的位置不同而变化。其共享存储器物理上是分布在所有处理器的本地存储器上。所有本地存储器的集合组成了全局地址空间，可被所有处理器访问。处理器访问本地存储器是比较快的，但访问属于另一个处理器的远程存储器则比较慢，因为通过互联网络会产生附加延迟。

NUMA服务器的基本特征是具有多个CPU模块，每个CPU模块由多个CPU（如4个）组成，并且具有独立的本地内存、I/O插槽等。

![image](./images/0x03.gif)

由于其节点之间可以通过互联模块（称为Crossbar Swtich）互相连接和信息交互，因此每个CPU可以访问整个系统的内存（这是NUMA系统与MPP系统的重要差别）。显然，访问本地内存的速度将远远高于访问非本地内存（系统内其他节点的内存）的速度，这也是非一致存储访问NUMA的由来。

由于这个特点，为了更好地发挥系统性能，开发应用程式时需要尽量减少不同CPU模块之间的信息交互。利用NUMA技术，可以较好地解决原来SMP系统的扩展问题，在一个物理服务器内可以支持上百个CPU。比较经典的NUMA服务器的例子包括，HP的Superdome、Sun Fire 15K、IBM p690等。

但NUMA技术同样有一定缺陷，由于访问非本地内存的延迟远远超过本地内存，因此当CPU数量增加时，系统性能无法线程增加。如HP公司发布Superdome服务器时，曾公布了它与HP其他UNIX服务器的相对性能值，结果发现，64路CPU的Superdome（NUMA结构）的相对性能值是20，而8路N4000（共享的SMP结构）的相对性能值是6.3。
从这个结果可以看到，8倍数量的CPU换来的只是3倍性能的提升。

### 1.3.1 NUMA Hierarchy

NUMA Hierarchy就是NUMA的层次结构。一个Intel x86 NUMA系统就是由多个NUMA Node组成。

### 1.3.2 NUMA Node内部

一个NUMA node内部是由一个物理CPU和它所有的本地内存（Local Memory）组成的。广义来说，一个NUMA node内部还包含本地I/O资源，对大多数Intel x86 NUMA平台来说，主要是PCIe总线资源。ACPI规范就是这么抽象一个NUMA Node的。

Intel X86 NUMA node组成部分：
- Socket
- Local Memory
- Local I/O Resource(PCIe Bus)

一个物理CPU：一个CPU Socket里由多个CPU Core和一个Uncore部分组成，每个CPU Core内部又可以由两个CPU Thread组成，每个CPU Thread都是一个操作系统可见的logic Processor。对于大多数操作系统来说，一个8 core的Socket开启HT后被识别为16个logic processor，下面就说一说这里面的相关概念。

- Socket：
这个词大概就是从CPU主板上的物理连接方式上来的。处理器通过主板的Socket来插到主板上。尤其是有了多核（Multi-Core）系统以后，Multi-Socket系统被用来指明系统到底存在多少个物理CPU。

- Core：
CPU的运算核心。x86的core包含了CPU运算的基本部件，如逻辑运算单元（ALU），浮点运算单元（FPU），L1和L2缓存。一个Socket可以有多个Core。在如今的多核时代，即使是Single Socket的系统，也是逻辑上的SMP系统。但是，一个物理CPU的系统不存在非本地内存，因此相当于UMA系统。

- Uncore：
Intel x86物理CPU里没有放在Core里的部件都被叫做unCore。unCore里集成了过去x86 UMA架构时代北桥芯片的基本功能。在Nehalem时代，内存控制器被集成到CPU里，叫做IMC（Intergrated Memory Controller）。而PCIe Root Complex还作为独立部件在I/O Hub芯片里。到了SandyBridge时代，PCIe Root Complex也被集成到CPU里。现如今的unCore部分，除了IMC、PCIe Root Complex还有QPI（QuickPath Interconnect）控制器，L3缓存，CBox（缓存一致性部件），以及其他外设控制器。

- Thread：
这里特指CPU的多线程计数。在Intel x86架构下，CPU的多线程技术被称作超线程（Hyper-Threading）技术。
Intel的超线程技术在一个处理器Core内部引入了额外的硬件，设计模拟了两个逻辑处理器（Logical Processor），每个逻辑处理器都有独立的处理器状态，但共享Core内部的计算单元，如ALU、FPU、L1、L2等。
这样，在最小的硬件投入下提高了CPU在多线程软件工作负载下的性能，提高了硬件使用率。x86的超线程技术出现早于NUMA架构。

### 1.3.3 本地内存

在Intel x86平台上，所谓本地内存，就是CPU经过unCore部件里的IMC（内存控制器）访问到的内存。而那些非本地的远程内存（Remote Memory），则需要经过QPI的链路到该内存所在的本地CPU的IMC来访问。
曾经在Intel lvyBridge的NUMA平台上做的内存访问性能测试显示，远程内存的延迟是本地内存的一倍。

可以假设，操作系统应尽量利用本地内存的低访问延迟特性来优化应用和系统的性能。

### 1.3.4 本地I/O资源

如前所述，Intel自从SandyBridge处理器开始，已经把PCIe Root Complex集成到CPU里。正因如此，从CPU直接引出PCIe Root Port的PCIe 3.0的链路可以直接与PCIe Switch或者PCIe Endpoint相连。一个PCIe Endpoint就是一个PCIe外设。这意味着，对某个PCIe外设来说，如果它直接与哪个CPU相连，它就属于哪个CPU所在的NUMA Node。

与本地内存一样，所谓本地I/O资源，就是CPU可以经过unCore部件里的PCIe Root Complex直接访问到的I/O资源。如果是非本地I/O资源，则需要经过QPI链路到该I/O资源所属的CPU，在通过该CPU PCIe Root Complex访问。如果同一个NUMA Node内的CPU和内存和另外一个NUMA Node的I/O资源发生交互，因为要跨QPI链路，会存在额外的访问延迟问题。

其他体系结构里，为降低外设访问延迟，也有将IB（InfiniBand）总线集成到CPU里的。这样IB设备也属于NUMA Node的一部分了。

可以假设，操作系统如果是NUMA Aware的话，应该会尽量针对本地I/O资源低延迟的优点进行优化。

### 1.3.5 NUMA Node互联

在Intel x86上，NUMA Node之间的互联是通过QPI（QuickPath Interconnect）Link的。CPU的unCore部分有QPI的控制器来控制CPU到QPI的数据访问。

不借助第三方的Node Controller，2/4/8个NUMA Node（取决于具体架构）可以通过QPI（QuickPath Interconnect）总线互联起来，构成一个NUMA系统。

例如，SIG UV计算机系统，他就是借助自家的SIG NUMAlink®互联技术来达到4到256个CPU Socket扩展的能力的。
这是一个SMP系统，所以支持运行一个Linux操作系统实例去管理系统。

### 1.3.6 NUMA Affinity

NUMA Affinity（亲和性）是和NUMA Hierarchy（层次结构）直接相关的。对于系统软件来说，以下两个概念至关重要即CPU
NUMA Affinity和Device NUMA Affinity。

- CPU NUMA Affinity
CPU NUMA的亲和性是指从CPU角度看，哪些内存访问更快，有更低的延迟。如前所述，和该CPU直接相关的本地内存时更快的。操作系统如果可以根据任务所在CPU去分配本地内存，就是基于CPU NUMA亲和性的考虑。因此，CPU NUMA亲和性就是要尽量让任务运行在本地的NUMA Node里。

- Device NUMA Affinity
设备NUMA亲和性是指从PCIe外设的角度看，如果和CPU和内存相关的I/O活动都发生在外设所属的NUMA Node，将会有更低延迟。这里有两种设备NUMA亲和性的问题：
    - 1. DMA Buffer NUMA Affinity
    大部分PCIe设备支持DMA功能。也就是说，设备可以直接把数据写入到位于内存中的DMA缓冲区。显然，如果DMA缓冲区在PCIe外设所属的NUMA Node里分配，那么将会有最低的延迟。否则，外设的DMA操作要跨越QPI链接去读写另外一个NUMA Node里的DMA缓冲区。因此，操作系统如果根据PCIe设备所属的NUMA node分配DMA缓冲区，将会有最好的DMA操作的性能。
    - 2. Interrupt NUMA Affinity
    设备DMA操作完成后，需要在CPU上触发中断来通知驱动程序的中断处理例程（ISR）来读写DMA缓冲区。有很多时候，ISR触发下半部机制（SoftIRQ）来进入到协议栈相关（Network、Storage）的代码路径来传递数据。</br></br>对大部分操作系统来说，硬件中断（HardIRQ）和下半部机制（SoftIRQ）的代码在同一CPU上发生。因此，DMA缓冲区的读写操作发生的位置和设备硬件中断（HardIRQ）密切相关。假设操作系统可以把设备的硬件中断绑定到自己所属的NUMA Node，那之后中断处理函数和协议栈代码对DMA缓冲区的读写将会有更低的延迟。

### 1.3.7 Firmware接口

由于NUMA的亲和性对应用的性能非常重要，那么硬件平台就需要给操作系统提供接口机制来感知硬件的NUMA层级结构。
在x86平台，ACPI规范提供了以下接口来让操作系统来检测系统的NUMA层次结构。

ACPI 5.0a规范的第17章是有关NUMA的章节。ACPI规范里，NUMA Node被第9章定义的Module Device所描述。
ACPI规范里用Proximity Domain对NUMA Node做了抽象，两者的概念大多数时候等同。

- SRAT（System Resouce Affinity Table）：
主要描述了系统Boot时的CPU和Memory都属于哪个Proximity Domain（NUMA Node）。这个表格的信息时静态的，如果是启动后热插拔，需要用OSPM的\_PXM方法去获得相关信息。

- SLIT（System Locality Information Table）：
提供CPU和Memory的位置远近信息。在SRAT表格里，只能告诉给定的CPU和Memory是否在一个NUMA Node。对某个CPU来说，不再本NUMA Node里的内存，即远程内存是否都一样的访问延迟取决于NUMA的拓扑由多复杂（QPI的跳数）。总之，对于不能简单yoghurt远近来描述的NUMA系统（QPI存在0、1、2等不同跳数），需要SLIT表格给出进一步的说明。同样的，这个表格也是静态表格，热插拔需要使用OSPM的\_SLI方法。

- DSDT（Differentiated System Description Table）：
从Device NUMA角度看，这个表格给出了系统Boot时的外设属于哪个Proximity Domain（NUMA Node）。

ACPI规范OSPM（Operating System-directed configuration and Power Management）和OSPM各种方法就是操作系统的ACPI驱动和ACPI Firmware之间的一个交互的接口。
x86启动OS后，没有ACPI之前，Firmware（BIOS）的代码是无法被执行的，除非通过SMI中断处理程序。
但有了ACPI，BIOS提前把ACPI的一些静态表格和AML的bytecode代码装载到内存，然后ACPI驱动就会加载AML的解释器，这样OS就可以通过ACPI驱动调用预先装载的AML代码。

AML（ACPI Machine Language）：是和Java类似的一种虚拟机解释型语言，所有不同操作系统的ACPI驱动，只要有相同的虚拟机解释器，就可以直接从操作系统调用ACPI写好的AML的代码了。所以，前文所述的所有热插拔的OSPM方法，其实就是对应ACPI Firmware的AML的一段函数代码而已。

至此，x86 NUMA平台所需的一些硬件知识基本就覆盖到了。需要说明的是，虽然文本以Intel平台为例，但AMD平台的差异也只是CPU总线和内部结构的差异而已。其他方面的NUMA概念AMD也是类似的。所以，下一步就是看OS如何利用这些NUMA特定做各种优化了。

## 1.4 MPP（Massive Parallel Processing）

和NUMA不同，MPP提供了另外一种进行系统扩展的方式，它由多个SMP服务器通过一定的节点互联网络进行连接，协同工作，完成相同的任务，从用户的角度来看是一个服务器系统。其基本特性是由多个SMP服务器（每个SMP服务器称节点）通过节点互联网络连接而成，每个节点只访问自己的本地资源（内存、存储等），是一种完全无共享（Share Nothing）结构，因而扩展能力最好，理论上其扩展无限制，目前的技术可实现512个节点互联，数千个CPU。目前业界对节点互联网络暂无标准，如NCR的Bynet，IBM的SPSwitch，它们都采用了不同的内部实现机制。但节点互联网仅供MPP服务器内部使用，对用户而言是透明的。

在MPP系统中，每个SMP节点也可以运行自己的操作系统、数据库等。但和NUMA不同的是，它不存在异地内存访问的问题。换言之，每个节点内的CPU不能访问另一个节点的内存。节点之间的信息交互式通过节点互联网络实现的，这个过程一般称为数据重分配（Data Redistrbution）。

但是MPP服务器需要一种复杂的机制来调度和平衡各个节点的负载及并行处理过程。目前一些基于MPP技术的服务器往往通过系统级软件（如数据库）来屏蔽这种复杂性。举例来说，NCR的Teradata就是基于MPP技术的一个关系数据库软件，基于此数据库来开发应用时，不管后台服务器由多少个节点组成，开发人员所面对的都是同一个数据库系统，而不需要考虑如何调度其中某几个节点的负载。

# 2. 三种体系架构之间的差异

## 2.1 NUMA、MPP、SMP之间性能的区别

NUMA的节点互联机制是在同一个物理服务器内部实现的，当某个CPU需要进行远地内存访问时，它必须等待，这也是NUMA服务器无法实现CPU增加时性能线性扩展。

MPP的节点互联机制是在不同的SMP服务器外部通过I/O实现（所以Linux不用考虑这种）的，每个节点只访问本地内存和存储，节点之间的信息交互与节点本身的处理时并行进行的。因此MPP在增加节点时性能基本上可以实现线性扩展。

SMP所有的CPU资源是共享的，因此完全实现线性扩展。

## 2.2 NUMA、MPP、SMP之间扩展的区别

NUMA理论上可以无限扩展，目前技术比较成熟的能够支持上百个CPU进行扩展。例如，HP的Superdome。

MPP理论上也可以实现无限扩展，目前技术比较成熟的能够支持512个节点，数千个CPU进行扩展。

SMP扩展能力很差，目前2个到4个CPU的利用率最好，但是IMB的Book技术，能够将CPU扩展到8个。

MPP是由多个SMP组成，多个SMP服务器通过一定的节点互联网络进行连接，协同工作，完成相同的任务。

## 2.3 NUMA、MPP、SMP应用之间的区别

### 2.3.1 MPP的优势

MPP系统不共享资源，因此对它而言，资源比SMP要多，当需要处理的事务达到一定规模时，MPP的效率要比SMP号。由于MPP系统因为要在不同处理单元之间传递信息，在通讯时间少的时候，那MPP系统可以充分发挥资源的优势，达到高效率。也就是说：操作互联之间没有什么关系，处理单元之间需要进行的通信比较少，那采用MPP系统就要好。因此，MPP系统在决策支持和数据挖掘方面显示了优势。

### 2.3.2 SMP的优势

MPP系统因为要在不同处理单元之间传递信息，所以它的效率要比SMP要差一点。在通讯时间多的时候，那MPP系统可以充分发挥资源的优势。因此当使用的OTLP程序中，用户访问一个中心数据库，如果采用SMP系统结构，它的效率要比采用MPP结构要快的多。

### 2.3.3 NUMA的优势

NUMA结构来看，它可以在一个物理服务器内集成许多CPU，使系统具有较高的事务处理能力，由于远地内存访问时延迟远长于本地内存访问，因此需要尽量减少不同CPU模块之间的数据交互。显然，NUMA架构更适用于OLTP事务处理环境，当用于数据仓库环境时，由于大量复杂的数据处理必然导致大量的数据交互，将使CPU的利用率大大降低。

# 3. 总结

传统的多核运算是使用SMP（Symmetric Multi-Processor）模式：将多个处理器与一个集中的存储器和I/O总线相连。
所有处理器只能访问同一个物理存储器，因此SMP系统有时也被称为一致存储器访问(UMA)结构体系，
一致性意指无论在什么时候，处理器只能为内存的每个数据保持或共享唯一一个数值。
很显然，SMP的缺点是可伸缩性有限，因为在存储器和I/O接口达到饱和的时候，
增加处理器并不能获得更高的性能，与之相对应的有AMP架构，不同核之间有主从关系，
如一个核控制另外一个核的业务，可以理解为多核系统中控制平面和数据平面。

NUMA模式是一种分布式存储器访问方式，处理器可以同时访问不同的存储器地址，大幅度提高并行性。 NUMA模式下，处理器被划分成多个"节点"（node）， 每个节点被分配有的本地存储器空间。 所有节点中的处理器都可以访问全部的系统物理存储器，但是访问本节点内的存储器所需要的时间，比访问某些远程节点内的存储器所花的时间要少得多。

NUMA 的主要优点是伸缩性。NUMA 体系结构在设计上已超越了SMP体系结构在伸缩性上的限制。
通过SMP，所有的内存访问都传递到相同的共享内存总线。
这种方式非常适用于CPU数量相对较少的情况，但不适用于具有几十个甚至几百个CPU的情况，
因为这些CPU会相互竞争对共享内存总线的访问。
NUMA通过限制任何一条内存总线上的CPU数量并依靠高速互连来连接各个节点，从而缓解了这些瓶颈状况。

- **名词解释：**

概念 | 描述
---|:---
SMP | 称为共享存储型多处理器(Shared Memory mulptiProcessors)，也称为对称型多处理器（Symmetry MultiProcessors)。
UMA | 称为均匀存储器存取（Uniform Memory Access)
NUMA | 非均匀存储器存取（Nonuniform Memory Access）
COMA | 只用高速缓存的存储器结构（Cache-Only Memory Architecture）
ccNUMA | 高速缓存相关的非一致性内存访问（Cache Coherent Non-UniformMemory Access）

- **UMA：**
物理存储器被所有处理器均匀共享。所有处理器对所有存储字具有相同的存取时间，这就是为什么称它为均匀存储器存取的原因。每台处理器可以有私用高速缓存，外围设备也以一定形式共享。

- **NUMA**
其访问时间随存储字的位置不同而变化。其共享存储器物理上是分布在所有处理器的本地存储器上。所有本地存储器的集合组成了全局地址空间，可被所有的处理机访问。处理器访问本地存储器是比较快的，但访问属于另一台处理器的远程存储器则比较慢，因为通过互连网络会产生附加时延。

- **COMA**
一种只用高速缓存的多处理器。COMA模型是NUMA机的一种特例，只是将后者中分布主存储器换成了高速缓存， 在每个处理器结点上没有存储器层次结构，全部高速缓冲存储器组成了全局地址空间。
远程高速缓存访问则借助于分布高速缓存目录进行。

是CC-NUMA体系结构的竞争者，两者拥有相同的目标，但实现方式不同。
COMA节点不对内存部件进行分布，也不通过互连设备使整个系统保持一致性。
COMA节点没有内存，只在每个Quad中配置大容量的高速缓存。

- **ccNUMA**
在CC-NUMA系统中，分布式内存相连接形成单一内存，内存之间没有页面复制或数据复制，也没有软件消息传送。
CC-NUMA只有一个内存映象，存储部件利用铜缆和某些智能硬件进行物理连接。
CacheCoherent是指不需要软件来保持多个数据拷贝的一致性，也不需要软件来实现操作系统与应用系统的数据传输。如同在SMP模式中一样，单一操作系统和多个处理器完全在硬件级实现管理。