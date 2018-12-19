- 1 什么是Watchdog
- 2 Watchdog的实现
    - 2.1 监视一个进程
    - 2.2 监视一个操作系统
- 3 Watchdog in Linux
    - 3.1 The Linux Watchdog driver API
        - 3.1.1 Linux下watchdog的工作原理
        - 3.1.2 硬件与软件watchdog的区别
        - 3.1.3 Linux内核中关于watchdog的配置
        - 3.1.4 watchdog重启时间间隔设定
        - 3.1.5 watchdog启动
        - 3.1.6 watchdog停止
        - 3.1.7 watchdog运行
        - 3.1.8 使用watchdog构建高可用性的Linux系统及应用
    - 3.2 NMI watchdog
        - 3.2.1 I/O APIC watchdog
        - 3.2.1 Local APIC watchdog
        - 3.2.3 两种NMI watchdog

# 1. 什么是Watchdog

Watchdog，又称watchdog timer，是计算机可靠性（dependability）领域中一个极为简单同时非常有效的检测（detection）工具。
其基本思想是针对被监视的目标设置一个计数器和一个阈值，watchdog会自己增加计数值，并等待被监视的目标周期性地重置计数值。
一旦目标发生错误，没来得及重置计数值，watchdog会检测到计数值溢出，并采取恢复措施（通常情况下是重启）。

> 总结一下：就是计数——>溢出——>触发。

Watchdog的工作方式是事件触发的，它可以对任何合理的事件计数（如CPU指令）。
其中时间事件（timeout）最常使用，这也是为什么watchdog又叫做watchdog timer。
但无论watchdog统计什么事件，它的思想都是一样的。

归根结底，Watchdog是一种检测手段，它监视的目标可以是一个进程也可以是整个操作系统。
Watchdog的合理性基于这样的假设：一个正常运行的系统，它的执行流应该是可预测的，因此可以在它正常执行路径上设置一些周期性重置watchdog的点；
但如果系统发生故障，它可能执行不到下一个重置watchdog的点，此时故障将被watchdog捕捉到。
看到这儿，您应该想到watchdog对于检测死循环或死锁这类故障非常有效。

# 2. Watchdog的实现

针对不同的监视目标，watchdog可以采取不同的实现方法。

## 2.1 监视一个进程

如果监视目标只是一个进程，那么利用操作系统提供的定时功能即可实现一个watchdog。

## 2.2 监视一个操作系统

如果要监视操作系统，就得使用操作系统之外的工具，通常是一个附加的计数器。
现代Intel CPU都包含的performance counter也可以提供这样的功能，从而不需要额外的设备就能实现监视操作系统的watchdog。

关于操作系统watchdog的设计策略以及实现方法，可参考论文《Exploring Recovery from Operating System Lockups》。

# 3. Watchdog in Linux

Linux很早就引进了watchdog，事实上，Linux中有两套watchdog体系。

## 3.1 The Linux Watchdog driver API

### 3.1.1 Linux下watchdog的工作原理

Watchdog在实现上可以是硬件电路也可以是软件定时器，能够在系统出现故障时自动重新启动系统。
在Linux 内核下, watchdog的基本工作原理是：当watchdog启动后(即/dev/watchdog 设备被打开后)，如果在某一设定的时间间隔内/dev/watchdog没有被执行写操作, 硬件watchdog电路或软件定时器就会重新启动系统。

/dev/watchdog 是一个主设备号为10， 从设备号130的字符设备节点。 Linux内核不仅为各种不同类型的watchdog硬件电路提供了驱动，还提供了一个基于定时器的纯软件watchdog驱动。 驱动源码位于内核源码树drivers\char\watchdog\目录下。

### 3.1.2 硬件与软件watchdog的区别

1. 硬件watchdog必须有硬件电路支持, 设备节点/dev/watchdog对应着真实的物理设备， 不同类型的硬件watchdog设备由相应的硬件驱动管理。软件watchdog由一内核模块softdog.ko 通过定时器机制实现，/dev/watchdog并不对应着真实的物理设备，只是为应用提供了一个与操作硬件watchdog相同的接口。
2. 硬件watchdog比软件watchdog有更好的可靠性。 软件watchdog基于内核的定时器实现，当内核或中断出现异常时，软件watchdog将会失效。而硬件watchdog由自身的硬件电路控制, 独立于内核。无论当前系统状态如何，硬件watchdog在设定的时间间隔内没有被执行写操作，仍会重新启动系统。
3. 一些硬件watchdog卡如WDT501P 以及一些Berkshire卡还可以监测系统温度，提供了 /dev/temperature接口。

对于应用程序而言, 操作软件、硬件watchdog的方式基本相同：打开设备/dev/watchdog, 在重启时间间隔内对/dev/watchdog执行写操作。即软件、硬件watchdog对应用程序而言基本是透明的。

在任一时刻， 只能有一个watchdog驱动模块被加载，管理/dev/watchdog 设备节点。如果系统没有硬件watchdog电路，可以加载软件watchdog驱动softdog.ko。


### 3.1.3 Linux内核中关于watchdog的配置

在Linux下使用watchdog开发应用之前， 请确定内核已经正确地配置支持watchdog。

内核源码下的drivers/char/watchdog/Kconfig文件提供了各种watchdog配置选项的详细介绍。

特别指出关于‘CONFIG_WATCHDOG_NOWAYOUT’选项的配置，从下面watchdog的模块信息可以看到，nowayout参数的缺省值等于‘CONFIG_WATCHDOG_NOWAYOUT’, 如果‘CONFIG_WATCHDOG_NOWAYOUT’选项在内核配置时设为‘Y’, 缺省情况下，watchdog启动后（即/dev/watchdog被打开后），无论是执行close操作还是写入字符‘V’都不能停止watchdog的运行。

```bash
[root@lab dev]# modinfo softdog
filename:       /lib/modules/3.10.0-693.el7.centos.x86_64.debug/kernel/drivers/watchdog/softdog.ko
alias:          char-major-10-130
license:        GPL
description:    Software Watchdog Device Driver
author:         Alan Cox
rhelversion:    7.4
srcversion:     250C077ED39E75BF25AD8D1
depends:
intree:         Y
vermagic:       3.10.0-693.el7.centos.x86_64.debug SMP mod_unload modversions
signer:         CentOS Linux kernel signing key
sig_key:        0B:01:58:1C:51:83:29:74:0F:DA:22:C6:A8:28:87:34:F0:DC:21:A6
sig_hashalgo:   sha256
parm:           soft_margin:Watchdog soft_margin in seconds. (0 < soft_margin < 65536, default=60) (uint)
parm:           nowayout:Watchdog cannot be stopped once started (default=0) (bool)
parm:           soft_noboot:Softdog action, set to 1 to ignore reboots, 0 to reboot (default=0) (int)
parm:           soft_panic:Softdog action, set to 1 to panic, 0 to reboot (default=0) (int)
```

### 3.1.4 watchdog重启时间间隔设定

在开发应用之前，必须了解所用的watchdog驱动的重启时间间隔。各种硬件以及软件watchdog驱动都设有一个缺省的重启时间间隔，此重启时间间隔也可在加载驱动模块时设置。

从上卖弄的模块信息可以看到，soft_margin参数代表了softdog.ko的重启时间间隔，缺省值是60秒，可以在加载softdog.ko 时指定重启时间间隔， 如 `modprobe softdog soft_margin=100`。

### 3.1.5 watchdog启动

各种硬件以及软件watchdog驱动都为应用提供了相同的操作方式。打开/dev/watchdog设备，watchdog将被启动。如果在指定重启时间间隔内没有对/dev/watchdog执行写操作，系统将重启。
```c
int wdt_fd = -1;

wdt_fd = open("/dev/watchdog", O_WRONLY);
if (wdt_fd == -1)
{
    // fail to open watchdog device
}
```

### 3.1.6 watchdog停止

如果内核配置选项‘CONFIG_WATCHDOG_NOWAYOUT’设为‘Y’, 缺省情况下watchdog启动后不能被停止。 如果模块的nowayout参数设为0， 往/dev/watchdog 写入字符`V’ 可以使watchdog停止工作。可以参考参考文献二2.6内核源码drivers\char\watchdog目录下各种硬件及软件watchdog驱动的write函数得到停止watchdog的逻辑, 譬如软件watchdog驱动softdog.c 中的write函数。

参考文献三watchdog daemon源码 watchdog-5.4.tar.gz的 close_all函数提供了停止watchdog运行的范例。以下是一个简单的停止watchdog的代码段范例:
```c
if (wdt_fd != -1)
{
    write(wdt_fd, "V", 1);
    close(wdt_fd);
    wdt_fd = -1;
}
```

### 3.1.7 watchdog运行

从3.1.6节中提到的softdog模块的write函数可以看到，在watchdog重启时间间隔内执行写操作，softdog_keepalive将被调用，增加定时器定时时间。

所以应用在启动watchdog后，必须在重启时间间隔内，周期性地对/dev/watchdog执行写操作才能使系统不被重启。

参考文献三watchdog daemon源码 watchdog-5.4.tar.gz的keep_alive函数提供了保持watchdog运行的范例。以下是一个简单的保持watchdog运行的代码段范例:
```
if (wdt_fd != -1)
    write(wdt_fd, "a", 1);
```

### 3.1.8 使用watchdog构建高可用性的Linux系统及应用

不同的系统与应用有自己的监控需求，不同的被监控对象有相应的监控方法。下文第一部分介绍了一个Linux下的开源项目watchdog daemon。 这是一个系统监控的后台应用，通过从配置文件中读取监控对象以及监控参数等信息对系统进行内存、负载、进程、网络等方面的监控， 同时将各种监测信息记入系统日志，并能够在系统重启之前发email通知管理员。通过阅读这个项目的源码可以学习到：

1. 如何在一个系统监控程序中使用watchdog，在系统出现故障时重新启动系统，以提高系统的可用性
2. 常用的被监控对象以及相应的监控方法

当Linux用于电信、 嵌入式领域， 一些基于Linux操作系统开发的服务应用在可用性上有较高的要求，下文第二部分介绍了如何在这种类型的应用中加入watchdog机制以提高应用的可用性。

**1. 在监控应用中加入watchdog以提高系统可用性**

watchdog daemon是一个Linux下使用了watchdog的系统监控的后台应用。开发人员可以下载最新的watchdog_5.4.tar.gz源码。 参见参考资源下载源码。

Watchdog daemon主程序运行流程如下：
1. 解析命令行输入，从watchdog.conf 配置文件读取配置信息，检测各种配置参数的有效性。 这些配置信息包括：
- 被监控的对象譬如网络接口eth0,　被监控进程的pid文件如/var/run/syslogd.pid
- 各种检测参数的阙值
- 用户提供的修复程序路径
- 用户提供的测试程序路径

2. 进行一系列初始化：
- 初始化用于检测网络状态的套接口
- 设置自身为后台进程
- 打开日志文件，往日志中记入各种配置信息
- 打开/dev/watchdog设备启动watchdog
- 打开proc/下各种状态信息文件如/proc/meminfo、 /proc/loadavg
- 禁止自身被swap出内存， 设置自身的调度优先级(在系统负载较高时， watchdog 后台进程自身有可能被swap出内存，导致不能及时对/dev/watchdog 执行写操作而使系统重启)

3. 进入while(1) 主循环， 依次执行：
- 对/dev/watchdog执行写操作，表明当前系统运行良好。如果超过一分钟的时间间隔没有对/dev/watchdog执行写操作，系统将会被动重启
- 进行各种检测，下节将介绍被各种检测的对象以及相应的检测方法
- 睡眠一段时间，此睡眠时间在配置文件中设置。如果睡眠时间设置太长，则会出现不必要的重启；设置太短，会导致检测太频繁，增加系统负载。

如果检测返回错误， 检测返回值显示故障可以修复，同时配置文件中也设置了修复程序， 修复程序将会被调用。如果修复故障失败，主程序将会尝试执行一系列清理、记载日志、email通知管理员的操作，然后重新启动系统。必须注意的是，当检测发现错误时，主程序并不是退出while（1）循环，使/dev/watchdog在1分钟内没被执行写操作而导致系统被动重启； 而是执行一系列操作后主动重启系统，这样既可以记录下日志信息，email通知管理员，也可以防止系统被动重启导致文件系统损坏。

主程序在主动重启之前， 会执行以下一系列操作:

- 关闭所有打开的文件
- 如果sendemail应用存在，配置文件中提供了管理员的email地址，将发封email给管理员通知系统重启
- 将重启信息记入系统日志
- kill掉所有进程
- 将重启信息记入wtmp
- 关闭accounting，quota，swap
- 卸载所有非根文件系统
- 以只读方式重新装载根文件系统
- 关闭网络接口
- 重启系统

从以上程序框架可以看到，监控应用检测出系统故障时，在系统可控的状态下，应用会尝试主动重启系统。但是通过启动watchdog，能够在系统不可控，或是监控应用自身不能正常运行时，由watchdog自动启动系统。

**2.监控对象及监控方法**

watchdog daemon根据配置文件对系统进行以下类型的监控，并将每次检测的结果记入系统日志：

- 通过打开一个文件成功与否测试文件表是否已满
- 通过读取/proc/loadavg 以检测1、5、15分钟内系统平均负载是否超过设定值
- 通过读取/proc/meminfo检测系统是否还剩下足够的空闲内存
- 如果一些硬件watchdog卡提供有温度传感器进行温度监控，访问/dev/temperature设备判断温度是否过高
- 通过调用kill (pid, 0)检测某个进程是否仍在运行，如果kill调用返回为0，则进程仍在运行, 通过从配置文件中读取pid文件如/var/run/syslogd.pid来获取被监控进程的pid
- 通过解析/proc/net/dev 的信息， 查看指定的网络接口如eh0的收发包状况
- 通过往一些IP地址发包检测这些IP地址可否被ping通，或是ping广播地址检测子网中是否至少一台机器可以被ping通
- 通过调用fork、execl执行用户传递的测试程序

watchdog daemon提供了一些常用的很有参考价值的监控方法及源码，开发人员也可自行设计开发更丰富的监控方法对系统进行更为细致、全面的监控。

**在服务应用中增加watchdog模块以提高应用的可用性：**

Linux因为其强大的功能已成为很多企业级应用的开发平台。这些应用本身并不是一个监控程序而是一个管理或服务程序， 对可用性有较高的要求。通过在这类应用中加watchdog模块， 一方面监测系统状态， 另一方面监测此应用中其它模块的工作状态。系统出现故障时能自动重启，因为应用已加在系统的init.d启动服务中， 应用会随着系统的启动而启动，自动提供服务。

这类服务应用通常是多线程的，甚至是多进程的。 加入watchdog机制的通常作法是在应用中加入一个watchdog线程，此线程主循环的工作流程与上节介绍的系统监控应用的守护进程的主循环工作流程大致相似。Watchdog线程在进入主循环之前的初始化阶段打开/dev/watchdog 启动watchdog, 然后执行上节中提到的while(1)循环中的操作。

如果此应用是多进程的，子进程的fork可以放在watchdog线程的初始化阶段执行。 Watchdog线程可以获得这些子进程的pid，从而检测这些子进程的工作状态。

Watchdog线程除了可以参照上节watchdog daemon提供的方法对系统状态进行监控， 还可以通过消息队列或套接口与应用中的其它线程进行通信， 获得其它线程的工作状态。不过这需要在其它子线程中加入与watchdog线程进行通信的接口。

## 3.2 NMI watchdog

NMI watchdog是Linux的开发者为了debugging而添加的特性，但也能用来检测和恢复Linux kernel hang，现代多核x86体系都能支持NMI watchdog。
NMI（Non Maskable Interrupt）即不可屏蔽中断，之所以要使用NMI，是因为NMI watchdog的监视目标是整个内核，而内核可能发生在关中断同时陷入死循环的错误，此时只有NMI能拯救它。

NMI watchdog的思想如前所述，仍然是通过计数和阈值进行监控。
要打开NMI watchdog，只需要在grub.cfg中添加一句"nmi_watchdog=N"，重启之后NMI watchdog就已经开启了。

Linux中有两种NMI watchdog，分别是I/O APIC watchdog（nmi_watchdog=1）和Local APIC watchdog（nmi_watchdog=2）。
它们的触发机制不同，但触发NMI之后的操作是几乎一样的。
从名字上看，它们都利用了APIC，所以前面强调这是被现代多核x86体系支持的功能。

下面介绍这两种NMI watchdog的监测机制。

### 3.2.1 I/O APIC watchdog

I/O APIC watchdog的工作方式比较奇葩，一旦开启了I/O APIC watchdog（nmi_watchdog=1），那么每个CPU对应的Local APIC的LINT0线都关联到NMI，这样每个CPU将周期性地接到NMI（这个周期与Local timer的周期相同），接到中断的CPU立即处理NMI。

处理的过程就是检查CPU的irq_stat（统计CPU中断信息的结构体，多核系统中这将是个数组）中的apic_timer_irqs和irq0_irqs这两个字段的和，它们分别代表了当前CPU从启动到现在处理的本地时钟中断次数和全局时钟中断次数。

如果这两个值的和在一定时间（在我的机子上是5秒）之内都没有增加，说明这个CPU已经连续5秒没有处理过一次时钟中断，显然，出大事了。

```c
do_nmi
    -> default_do_nmi
        -> nmi_watchdog_tick
            -> get_timer_irqs
            -> if (last_irq_sum == sum)
                -> local_nc(last_irq_sum)
                -> if (alter_counter ==  5 * nmi_hz)
                    -> die_nmi
                    
                    
arch/x86/kernel/traps.c: do_nmi, default_do_nmi
arch/x86/kernel/dumpstack.c: die_nmi
arch/x86/kernel/apic/nmi.c: nmi_watchdog_tick                    
```

### 3.2.1 Local APIC watchdog

Intel CPU带有performance counter，可以对的许多事件计数（P4系列多达43到45个，具体数目视型号而定），在watchdog中，统计的事件是"UnHalted Core Cycle"，即CPU处于非halt状态下经过的周期数。当计数溢出时，Local APIC会向CPU发出一个PMI（performance counter interrupt）。
PMI的属性可选，理所应当的，在Local APIC watchdog中它被设置成NMI。利用这个特性，就能周期性地产生NMI。

发出NMI之后，Local APIC watchdog的行为就与I/O APIC watchdog几乎一样了，仍然是检查CPU处理时钟中断的次数。
唯一的区别是，在处理NMI的过程中还多执行一项操作：重置performance counter。
由于Local APIC watchdog统计的是UnHalted Cycles，所以系统空闲的时候，NMI触发的频率比较低。


```c
do_nmi
    -> default_do_nmi
        -> nmi_watchdog_tick
            -> get_timer_irqs
            -> if (last_irq_sum == sum)
                -> local_nc(last_irq_sum)
                -> if (alter_counter ==  5 * nmi_hz)
                    -> die_nmi
            -> lapic_wd_event
                -> wd_ops->rearm
                
arch/x86/perfctr_watchdog.c: lapic_wd_event
```

NMI watchdog在发现故障之后调用了die_nmi，我们来看看这个函数到底做了什么：

```c
    die_nmi
        -> notify_die
        -> show_registers
        -> if (do_panice || panic_on_timeout)
            -> panic("Non maskable interrupt")
        -> do_exit
```

一个一个来解析：
- notify_die通知对NMI感兴趣、即订阅了die_chain的模块
- show_registers调用printk输出出当前寄存器信息，别忘了NMI watchdog本来是为了debugging哦
- 根据do_panic和panic_on_timeout的值决定是否要panic，do_panic是die_nmi的参数，panic_on_timeout是启动项参数，当然启动之后也可以通过/proc动态修改它的值
- do_exit就是恢复操作，Linux陷入hang，current进程有很大的嫌疑，所以退出当前进程有一定几率恢复系统运行

### 3.2.3 两种NMI watchdog

I/O APIC watchdog的缺点很明显……每个时钟周期都要触发NMI，这对性能是个不小的影响，
引用Linux Document的原话"...its NMI frequency is much higher, resulting in a more significant hit to the overall system performance"，而且可靠性不太好。
我开启watchdog之后，写了个关中断+死循环的kernel module并且加载到系统中，结果Linux跪了——这意味着I/O APIC watchdog没起作用。

Local APIC watchdog的触发频率要低一些，性能开销也要小一些。但是注意它统计的事件，"UnHalted Core Cycle"，也就是说，万一CPU进入了关中断+halt的状态，那它也没救了，因为这时候计数器根本就不走了。
但是I/O APIC watchdog却没有这个缺陷。但是就可靠性而言，Local APIC watchdog的表现更好，我开启它之后通过module注入了各种spinlock死锁，它居然都恢复了……

