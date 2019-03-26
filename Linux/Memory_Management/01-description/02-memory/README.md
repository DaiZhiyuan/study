***Linux内存描述之概述***

- 1 (N)UMA模型中Linux内存结构
- 2 Linux物理内存的组织形式
- 3 内存节点Node
- 4 物理内存区域Zone
- 5 内存页Page
- 6 高端内存

# 1. (N)UMA模型中Linux内存结构

Linux适用于各种不同的体系结构，而不同体系结构在内存管理方面差别很大。因此，Linux内核需要用一种体系结构无关的方式表示内存。

Linux内核通过插入一些兼容层，使得不同体系结构的差异很好的被隐藏起来，内核对一致和非一致内存访问使用相同的数据结构。

**NUMA系统：**
- 处理器被划分成多个Node，每个Node被分配有本地存储器空间。所有Node中的处理器都可以访问全部的系统物理存储器，但是访问Local Memory所需要的时间比访问Remote Memory所花的时间要少很多。
- 内存被分割成多区域（BANK，也叫簇），根据簇与处理器的“距离”不同，访问不同簇的代码也会不同。比如，可能把内存的一个簇指派给每个处理器，或则某个簇和设备卡很近，很适合DMA，那么就指派给该设备。因此当前的多数系统会把内存系统分割成2块区域，一块是专门给CPU去访问，一块是给外围设备板卡的DMA去访问。

**UMA系统：**
- 内存就相当于一个只使用一个NUMA节点来管理整个系统的内存。而内存管理的其他地方则认为它们就是在处理一个NUMA Node的系统。

# 2. Linux物理内存的组织形式

Linux把物理内存划分为三个层次来管理：

层次 | 描述
---|:---
节点（Node）| CPU被划分为多个Node，内存则被分Bank。每个CPU对应一个本地内存，</br>即一个CPU-Node对应一个内存簇Bank，即每个内存簇被认为是一个Node。
区域（Zone）| 每个物理内存节点Node被划分为多个内存管理区域Zone，用于表示不同范围的内存，</br>内核可以使用不同的映射方式映射物理内存。
页面（Page）| 内存被细分为多个Page Frame，Page是最基本的页面分配单位。

为了支持NUMA模型，也即CPU对不同内存单元的访问时间可能不同，此时系统的物理内存被划分为几个节点（Node），一个Node对应一个内存簇Bank，即每个内存簇被认为是一个节点Node。

- 首先，内存被划分为Node。每个Node关联到系统中的一个CPU，内核中表示为`pg_data_t`的实例。定义了大小为MAX\_NUMNODES类型为pg\_data\_t的数组node\_data，数组的大小根据CONFIG\_NODES\_SHIFT的配置决定。对于UMA来说，NODES\_SHIFT为0，所以MAX\_NUMNODES的值为1。而对于PC这种UMA结构的机器来说，只使用了一个称为contig\_page_data的静态`pg_data_t`结构。
- 各个节点Node又被划分为内存管理区域zone，一个管理区域通过`struct zone_struct`描述，其被定义为`zone_t`，用以表示内存的某个范围，低端范围的16MB被描述为ZONE\_DMA，某些工业标准体系结构中的（ISA）设备需要用到它，然后是可直接映射到内核的普通内存区域ZONE\_NORMAL，最后是超出了内核段的物理地址域ZONE\_HIGHMEM，被称为高端内存。是系统中预留的可用内存空间，不能被内核直接映射。
- 最后页帧（page frame）代表了系统内存的最小单位，堆内存中的每个页都会创建一个`struct page`的实例。传统上，把内存视为连续的字节，即内存为字节数组，内存单元的编号可作为字节数组的索引。分页管理时，将若干字节视为一页，比如4K byte。此时，内存变成了连续的页，即内存为页数组，每一页物理内存叫页帧，以页为单位对内存进行编号，该编号可作为页数组的索引，又称为页帧号。

在一个单独的节点内，任一给定CPU访问页面所需要的时间都是相同的。然而你，在不同的CPU，这个时间可能就不同。对每个CPU而言，内核都试图把耗时节点的访问次数减到最少这就要小心地选择CPU最常引用的内核数据结构的存放位置。

# 3. 内存节点Node

> CPU被划分为多个节点（Node），内存则被分簇，每个CPU对应一个本地物理内存，即一个CPU-Node对应一个内存簇Bank，即每个内存簇被认为是一个节点。

在Linux中引入一个数据结构`struct pglist_data`来描述一个Node，定义在include/linux/mmzone.h文件中。（这个结构被typedef pg\_data\_t）。
- 对于NUMA系统来说，整个系统的内存由一个node\_data的pg\_data\_t指针数组来管理
```c
struct pglist_data *node_data[MAX_NUMNODES] __read_mostly;
```
- 对于UMA系统，使用`struct pglist_data contig_page_data`，作为系统唯一的Node管理所有的内存区域。（UMA系统中只有一个Node）
```c
struct pglist_data __refdata contig_page_data = {
    .bdata = &bootmem_node_data[0]
};
```

可以使用NODE\_DATA(node\_id)来查找系统中编号为node\_id的节点，参见NODE\_DATA的定义：
```c
extern struct pglist_data *node_data[];
#define NODE_DATA(nid)      (node_data[nid])
```
UMA结构下由于只有一个结点，因此该宏总是返回全局的contig\_page\_data，而与参数node\_id无关：
```c
#ifndef CONFIG_NEED_MULTIPLE_NODES

extern struct pglist_data contig_page_data;
#define NODE_DATA(nid)      (&contig_page_data)
#define NODE_MEM_MAP(nid)   mem_map

#else /* CONFIG_NEED_MULTIPLE_NODES */

#include <asm/mmzone.h>

#endif /* !CONFIG_NEED_MULTIPLE_NODES */
```
在分配一个页面时，Linux采用节点局部分配策略，从最靠近运行中的CPU的节点分配内存，由于进程往往是在同一个CPU上运行，因此从当前节点得到的内存很可能被用到。

# 4. 物理内存区域Zone

因为实际的计算机体系结构有硬件的诸多限制，这限制了页框可以使用的方式。尤其是，Linux内核必须处理x86体系结构的两种硬件约束。
- ISA总线的直接内存存储DMA操作，处理器有一个严格的限制：它们只能对RAM的前16MB进行寻址。
- 在具有大容量RAM的现代32位计算机中，CPU不能直接访问所有的物理地址，因为线性地址空间太小，内核不可能直接映射所有的物理内存到线性地址空间，我们会在后面典型架构（x86）上内存区域划分详细讲解x86_32上的内存区域规划。

因此，Linux内核对不同Zone（即不同范围）的内存（物理内存）需要采用不同的管理方式和映射方式。

为了解决这些制约条件，Linux使用了三种类型：
- ZONE\_DMA：这个区包含页用来执行DMA操作。
- ZONE\_NOMAL：这个区包含的都是正常映射的页。
- ZONE\_HIGHMEM：这个区“高端内存”，其中的页不能永久映射到内核地址空间（虚拟地址空间）。

而为了兼容一些设备的热插拔支持以及内存碎片化的处理，内核也进入了一些逻辑上的内存区：
- ZONE\_MOVABLE：内核定义了一个伪内存域ZONE\_MOVABLE，在防止物理内存碎片的机制memory migration中使用该内存区域。供防止物理内存碎片的极致使用。
- ZONE\_DEVICE：为支持热插拔设备而分配的Non-Volatile-Memory非易失性内存。

内核将每个簇所对应的Node又被分成称为管理区（Zone）的块，它们各自描述在内存中的范围。一个管理区Zone由`struct zone`结构体来描述。

对于x86\_32的机器，管理区类型如下分布：

类型 | 区域
:---|---:
ZONE\_DMA | 0 ~ 15MB
ZONE\_NORMAL | 16MB ~ 895MB
ZONE\_HIGHMEM | 896MB ~ 物理内存结束

内核在初始化内存管理区时，首先建立管理区表zone\_table。
```c
/*
 *
 * The zone_table array is used to look up the address of the
 * struct zone corresponding to a given zone number (ZONE_DMA,
 * ZONE_NORMAL, or ZONE_HIGHMEM).
 */
zone_t *zone_table[MAX_NR_ZONES*MAX_NR_NODES];
EXPORT_SYMBOL(zone_table);
```
该表处理起来就像一个多为数组：
- MAX\_NR\_ZONES是一个节点中所能容纳的管理区的最大数，例如3个，与Zone区域的类型（ZONE\_DMA，ZONE\_NORMAL，ZONE\_HIGHMEM）定义在一起。当然这时候我们这些标志都是通过宏的方式来实现的，而不是如今的枚举类型。
- MAX\_NR\_NODES是可以存在的节点的最大数
- 函数EXPORT\_SYMBOL使得内核的变量或者函数可以被载入的模块（例如，我们的驱动模块）所访问。

# 5. 内存页Page

大多数内核（Kernel）的操作只使用ZONE\_NORMAL区域，系统内存由很多固定大小的内存块组成的，这样的内存块称作为“页”即Page。

在x86体系结构中，默认情况Page的大小为4096个字节（4KB）。

每个物理页由一个`struct page`的数据结构对象来描述。页的数据结构对象都保存在`mem_map`全局数组中，该数组通常被存放在ZONE\_NORMAL的首部，或者就在小内存系统中为装入内核映像而预留的区域之后。从载入内核的低地址内存区域的后面内存区域，也就是ZONE\_NORMAL开始的地方的内存的页的数据结构对象，都保存在这个全局数组中。

# 6. 高端内存

由于能够被Linux内核直接访问的ZONE\_NORMAL区域的内存空间也是有限的，所以Linux提出了高端内存（High memory）的概念，并且允许对高端内存的访问。