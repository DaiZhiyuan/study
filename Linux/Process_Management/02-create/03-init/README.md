***Linux下1号进程的前世(kernel_init)今生(init进程)***

- 1 概述
- 2 PID=1的进程
    - 2.1 kernel_init
    - 2.2 init进程
    - 2.3 关于init程序
- 3 附录
    - 3.1 kernel_init_freeable流程分析
    - 3.2 kernel_init分析

# 1. 概述

Linux系统中的init进程（PID=1）是除了idle进程（PID=0，也就是init\_task）之外另一个比较特殊的进程，它是Linux内核开始建立起进程概念时的第一个通过kernel\_thread产生的进程，其开始在内核态执行，然后通过一个系统调用，开始执行用户空间的/sbin/init程序，期间Linux内核也经历了从内核态到用户态的特权级转变，/sbin/init极有可能产生出Shell，然后所有的用户进程都由该进程派生出来。

# 2. PID=1的进程

前面我们了解到0号进程是系统所有进程的先祖，它的进程描述符init\_task是静态创建的，而它在进行初始化的时候，通过kernel\_thread的方式创建了两个内核线程，分别是kernel\_init和kthreadd，其中kernel\_init进程号为1。

start\_kernel在其最后一个函数rest\_init的调用中，会通过kernel\_thread生成一个内核线程，后者则会在新进程环境下调用kernel\_init函数，kernel\_init一个让人感兴趣的地方在于它会调用run\_init\_process来执行根文件系统下的/sbin/init程序。

```c
static int run_init_process(const char *init_filename)
{
    argv_init[0] = init_filename;
    return do_execve(getname_kernel(init_filename),
        (const char __user *const __user *)argv_init,
        (const char __user *const __user *)envp_init);
}
```

## 2.1 kernel_init

0号进程创建1号进程的方式如下：
```c
kernel_thread(kernel_init, NULL, CLONE_FS);
```
我们发现1号进程的执行函数就是kernel\_init，这个函数所定义init_main.c中，如下所示：

```c
static int __ref kernel_init(void *unused)
{
    kernel_init_freeable();
    /* need to finish all async __init code before freeing the memory */
    async_synchronize_full();
    free_initmem();
    mark_rodata_ro();
    system_state = SYSTEM_RUNNING;
    numa_default_policy();

    flush_delayed_fput();

    if (ramdisk_execute_command) {
        if (!run_init_process(ramdisk_execute_command))
            return 0;
        pr_err("Failed to execute %s\n", ramdisk_execute_command);
    }

    /*
     * We try each of these until one succeeds.
     *
     * The Bourne shell can be used instead of init if we are
     * trying to recover a really broken machine.
     */
    if (execute_command) {
        if (!run_init_process(execute_command))
            return 0;
        pr_err("Failed to execute %s.  Attempting defaults...\n",
            execute_command);
    }
    if (!run_init_process("/sbin/init") ||
        !run_init_process("/etc/init") ||
        !run_init_process("/bin/init") ||
        !run_init_process("/bin/sh"))
        return 0;

    panic("No init found.  Try passing init= option to kernel. "
          "See Linux Documentation/init.txt for guidance.");
}
```
kernel\_init函数将完成设备驱动程序的初始化，并调用run\_init\_process函数启动用户空间的init进程。

由0号进程创建1号进程，1号内核线程负责执行内核的部分初始化工作及进行系统配置，并创建若干个用户高速缓存和虚拟内存管理的内核线程。

## 2.2 init进程

随后，1号进程调用do\_execve运行可执行程序init，并演变成用户态1号进程，即init进程。

init进程是Linux内核启动的第一个用户级进程。
init由很多重要的任务：
- 启动getty用于用户登录
- 实现运行级别
- 处理孤儿进程

它按照配置文件/etc/inittab的要求，完成系统启动工作，创建编号为1号、2号等若干终端注册进程gettty。

每个getty进程设置其进程组标识号，并监视配置到系统终端的接口线路。当检测到来终端的连接信号时，getty进程将通过函数do\_execve()执行注册程序login，此时用户就可以输入用户名和密码进行登录，如果登录成功，由login程序在通过函数execv()执行shell，该shell接收getty进程的pid，取代原来的getty进程。再由shell直接或间接地产生其他进程。

```
上述过程可描述为： 0号进程 -> 1号内核进程 -> 1号用户进程（init进程） -> getty进程 -> shell进程
```

注意：上述过程中提到：1号内核进程调用执行init函数并演变成1号用户态进程（init进程）。这里前者是init时函数，后者是进程。两者容易混淆，区别如下：
1. kernel\_init函数在内核态运行，是内核代码
2. init进程是内核启动并运行的第一个用户进程，运行在用户态下。
3. 1号内核进程调用execve()从文件/etc/inittab中加载可执行程序init并执行，这个过程并没有调用do\_fork()，因此两个进程都是1号进程。

当内核启动自己之后（已被装入内存、已经开始运行、已经初始化所有设备驱动和数据结构等等）。通过启动用户级程序init来完成起到进程的内核部分。因此，init总是第一个进程（它的进程号总是1）。

当init开始运行，它通过执行一些管理任务来结束引导进程，例如检查文件系统、清理/tmp、启动各种服务以及为每个中断和虚拟控制台启动getty，在这些地方用户将登陆系统。

在系统完全起来后，init为每个用户已退出中断重启getty（这样下一个用户就可以登陆）。init同样也收集孤儿进程：当一个进程启动了一个子进程且在子进程之前终止了，这个子进程立刻成为init的子进程。对于各种技术方面的原因来说这个是很重要的，知道这些也是有好处的，因为这便于理解进程列表和进程树图。
init的变种很少。绝大多数Linux发行版使用sysint，它是基于System V的init设计。Unix的BSD版本有一个不同的init。最主要的不同在于运行级别：System V有而BSD没有。这种区别并不是主要的。再次我们仅讨论sysvinit。配置init以启动getty:/etc/inittab文件。

## 2.3 关于init程序

1号进程通过execve执行init程序来进入用户空间，成为init进程，那么这个init在哪里呢？

内核在几个位置上来寻找init程序，这几个位置以前常用来设置init程序，但是init最合适的位置（在Linux系统上）是/sbin/init。如果内核没有找到init，它就会试着运行/bin/sh，如果还是失败了，那么系统的启动就宣告失败了。


实现方式 | 说明
---|:---
sysvinit | 早期一些版本使用的初始化进程工具, 目前在逐渐淡出linux历史舞台,</br> sysvinit 就是 system V 风格的 init 系统，顾名思义，</br>它源于 System V 系列 UNIX。它提供了比 BSD 风格 init 系统更高的灵活性。</br>是已经风行了几十年的 UNIX init 系统，一直被各类 Linux 发行版所采用。
upstart | debian, Ubuntu等系统使用的initdaemon
systemd | Systemd 是Linux系统中最新的初始化系统（init），</br>它主要的设计目标是克服 sysvinit 固有的缺点，提高系统的启动速度

# 3. 附录

## 3.1 kernel_init_freeable流程分析

```c
static noinline void __init kernel_init_freeable(void)
{
    /*
     * Wait until kthreadd is all set-up.
     */
    wait_for_completion(&kthreadd_done);

    /* Now the scheduler is fully set up and can do blocking allocations */
    gfp_allowed_mask = __GFP_BITS_MASK;

    /*
     * init can allocate pages on any node
     */
    set_mems_allowed(node_states[N_MEMORY]);
    /*
     * init can run on any cpu.
     */
    set_cpus_allowed_ptr(current, cpu_all_mask);

    cad_pid = task_pid(current);

    smp_prepare_cpus(setup_max_cpus);

    do_pre_smp_initcalls();
    lockup_detector_init();

    smp_init();
    sched_init_smp();

    page_alloc_init_late();

    do_basic_setup();

    /* Open the /dev/console on the rootfs, this should never fail */
    if (sys_open((const char __user *) "/dev/console", O_RDWR, 0) < 0)
        pr_err("Warning: unable to open an initial console.\n");

    (void) sys_dup(0);
    (void) sys_dup(0);
    /*
     * check if there is an early userspace init.  If yes, let it do all
     * the work
     */

    if (!ramdisk_execute_command)
        ramdisk_execute_command = "/init";

    if (sys_access((const char __user *) ramdisk_execute_command, 0) != 0) {
        ramdisk_execute_command = NULL;
        prepare_namespace();
    }

    /*
     * Ok, we have completed the initial bootup, and
     * we're essentially up and running. Get rid of the
     * initmem segments and start the user-mode stuff..
     */

    /* rootfs is available now, try loading default modules */
    load_default_modules();
}
```
- wait\_for_completion

实例在kernel/sched/completion.c中, 等待Kernel Thread kthreadd (PID=2)创建完毕。

- gfp\_allowed_mask

\_\_GFP\_BITS_MASK；设置bitmask，使得init进程可以使用PM并且允许I/O阻塞操作。

- set\_mems\_allowed(node\_states[N_MEMORY])

init进程可以分配物理页面。

- set\_cpus\_allowed_ptr

通过设置cpu\_bit\_mask，可以限定task只能在特定的处理器上运行， 而init current进程此时必然是init进程，设置其cpu\_all_mask即使得init进程可以在任意的cpu上运行。

- task_pid

设置到目前运行进程init的pid号给cad\_pid(cad\_pid是用来接收ctrl-alt-del reboot signal的进程，如果设置C\_A\_D=1就表示可以处理来自ctl-alt-del的动作)，最后会调用ctrl\_alt\_del(void)并确认C\_A\_D是否为1，确认完成后将执行cad\_work=deferred\_cad，执行kernel_restart。

- smp\_prepare\_cpus

体系结构相关的函数，实例在arch/arm/kernel/smp.c中，调用smp\_prepare\_cpus时，会以全局变量setup\_max\_cpus为函式参数max_cpus，以表示在编译核心时，设定支援的最大CPU数量。

- do\_pre\_smp_initcalls

实例在init/main.c中，会透过函式do\_one\_initcall，执行Symbol中\_\_initcall\_start与\_\_early\_initcall_end之间的函数。

- smp_init

实例在kernel/smp.c中，函数主要是由Bootstrap处理器，进行Active多核心架构下其它的处理器。
如果发生Online的处理器个数(from num\_online\_cpus)超过在核心编译时，所设定的最大处理器个数setup\_max\_cpus (from NR\_CPUS)，就会终止流程。

如果该处理器目前属于Present（也就是存在系统中)，但尚未是Online的状态，就会调用函数cpu_up(in kernel/cpu.c)来启动该处理器。

- sched\_init_smp

实例在kernel/sched.c中：
1. 调用get\_online\_cpus，如果目前CPU Hotplug Active Write执行例程是自己，就直接返回。反之就把cpu_hotplug.refcount加1 (表示多一个Reader)。
2. 取得Mutex Lock “sched\_domains_mutex”
3. 调用arch\_init\_sched\_domains，设定scheduler domains与groups，参考Linux Documentation/scheduler/sched-domains.txt，一个Scheduling Domain会包含一个或多个CPU Groups，调度的Load-Balance就会根据Domain中的Groups来做调整。
4. 释放Mutex Lock “sched\_domains_mutex”
5. 调用put\_online\_cpus，如果目前CPU Hotplug Active Writer执行例程是自己，就直接返回。反之就把cpu\_hotplug.refcount减1，如果cpu\_hotplug.refcount减到為0，表示没有其他Reader，此时如果有CPU Hotplug Active Writer行程在等待，就会通过wake\_up\_process唤醒该进程，以便让等待中的Writer可以被执行下去。(也可以参考\_cpu\_up中对与函数cpu\_hotplug_begin的说明)。
6. 注册CPU Notifier cpuset\_cpu\_active/cpuset\_cpu\_inactive/update_runtime。
7. 调用set\_cpus\_allowed\_ptr，通过这个函数可以设定CPU bitmask，限定Task只能在特定的处理器上运作。在这会用参数”non\_isolated\_cpus”，也就是会把init指定给non-isolated CPU。Linux Kernel可以在启动时，透过Boot Parameters “isolcpus=“指定CPU编号或是范围，让这些处理器不被包含在Linux Kernel SMP balancing/scheduling算法内，可以在啟动后指派给特定的Task运作。而不在 “isolcpus=“ 指定范围内的处理器就算是non-isolated CPU。
8. 调用sched\_init\_granularity，通过函数update\_sysctl，让sysctl\_sched\_min\_granularity=normalized\_sysctl\_sched\_min\_granularity，sysctl\_sched\_latency=normalized\_sysctl\_sched\_latency，sysctl\_sched\_wakeup\_granularity=normalized\_sysctl\_sched\_wakeup_granularit。

- do\_basic\_setup

实例在init/main.c中，
1. diaousermodehelper_init (in kernel/kmod.c)，产生khelper workqueue。
2. 调用init_tmpfs (in mm/shmem.c)，对VFS注册Temp FileSystem。
3. 调用driver_init (in drivers/base/init.c)，初始化Linux Kernel Driver System Model。
4. 调用init\_irq_proc(in kernel/irq/proc.c)，初始化 “/proc/irq”与其下的File Nodes。
5. 调用do\_ctors (in init/main.c)，执行位於Symbol \_\_ctors\_start 到 \_\_ctors_end间属于Section “.ctors” 的Constructor函数。
6. 通过函数do\_initcalls，执行介于Symbol \_\_early\_initcall\_end与\_\_initcall_end之间的函数。

- sys\_open

实例在fs/fcntl.c中，”SYSCALL\_DEFINE1(dup, unsigned int, fildes)”，在这会连续执行两次sys\_dup，复制两个sys_open开启/dev/console所产生的文件描述符0 (也就是会多生出两个1与2)，只是都对应到”/dev/console”，我们在System V streams下的Standard Stream一般而言会有如下的对应：
```
    0：Standard input (stdin)
    1：Standard output (stdout)
    2：Standard error (stderr)
```

- ramdisk\_execute\_command与prepare_namespace

1. 如果ramdisk\_execute\_command为0，就设定ramdisk\_execute\_command = “/init”
2. 如果sys\_access确认文件ramdisk\_execute\_command 失败，就把ramdisk\_execute\_command设置为0，然后调用prepare_namespace去mount root FileSystem。

- integrity\_load_keys

至此我们初始化工作完成, 文件系统也已经准备好了，那么接下来加载 load integrity keys hook。

- load\_default_modules

加载基本的模块。

## 3.2 kernel\_init分析

```c
static int __ref kernel_init(void *unused)
{
    kernel_init_freeable();
    /* need to finish all async __init code before freeing the memory */
    async_synchronize_full();
    free_initmem();
    mark_rodata_ro();
    system_state = SYSTEM_RUNNING;
    numa_default_policy();

    flush_delayed_fput();

    if (ramdisk_execute_command) {
        if (!run_init_process(ramdisk_execute_command))
            return 0;
        pr_err("Failed to execute %s\n", ramdisk_execute_command);
    }

    /*
     * We try each of these until one succeeds.
     *
     * The Bourne shell can be used instead of init if we are
     * trying to recover a really broken machine.
     */
    if (execute_command) {
        if (!run_init_process(execute_command))
            return 0;
        pr_err("Failed to execute %s.  Attempting defaults...\n",
            execute_command);
    }
    if (!run_init_process("/sbin/init") ||
        !run_init_process("/etc/init") ||
        !run_init_process("/bin/init") ||
        !run_init_process("/bin/sh"))
        return 0;

    panic("No init found.  Try passing init= option to kernel. "
          "See Linux Documentation/init.txt for guidance.");
}
```

- kernel\_init_freeable

调用kernel_init_freeable完成初始化工作，准备文件系统，准备模块信息。

- async\_synchronize_full

用以同步所有非同步函式呼叫的执行，在这函数中会等待List async\_running与async\_pending都清空后，才会返回。
Asynchronously called functions主要设计用来加速Linux Kernel开机的效率，避免在开机流程中等待硬体反应延迟，影响到开机完成的时间。

- free_initmem

free\_initmem(in arch/arm/mm/init.c)，释放Linux Kernel介于\_\_init\_begin到 \_\_init\_end属于init Section的函数的所有内存。
并会把Page个数加到变量totalram\_pages中，作为后续Linux Kernel在配置内存时可以使用的Pages。 (在这也可把TCM范围(\_\_tcm\_start到\_\_tcm_end)释放加入到总Page中，但TCM比外部内存效率，适合多媒体，中断等对效能要求高的执行需求，放到总Page中，成为可供一般目的配置的存储范围。

- system_state

设置运行状态SYSTEM_RUNNING。

- 加载init进程，进入用户空间
    - 如果ramdisk\_execute_command不為0，就执行该命令成為init User Process。
    - 如果execute\_command不為0，就执行该命令成為init User Process。
    - 如果上述都不成立,就依序執行如下指令：
        - run\_init_process("/sbin/init");
        - run\_init_process("/etc/init");
        - run\_init_process("/bin/init");
        - run\_init_process("/bin/sh");
    - 如果都找不到可以執行的init Process，就會進入Kernel Panic。