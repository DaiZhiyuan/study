***Linux下0号进程的前世(init_task进程)今生(idle进程)***

- 1 Linux中3个特殊的进程
- 2 idle的创建
    - 2.1 0号进程上下文信息——init_task描述符
        - 2.1.1 进程堆栈init_thread_union
        - 2.1.2 进程内存空间
- 3 0号进程的演化
    - 3.1 rest_init创建init进程（PID=1）和Kthread进程（PID=2）
        - 3.1.1 创建kernel_init
        - 3.1.2 创建kthreadd
    - 3.2 0号进程演变为idle
- 4 idle的运行与调度
    - 4.1 idle的workload——cpu_idle_loop
    - 4.2 idle的调度和运行时机
- 5 idle进程总结
- 6 附录——rest_init的执行解析

# 1. Linux中3个特殊的进程

Linux下有三个特殊的进程：
- idle进程，PID=0
- init进程，PID=1
- kthreadd，PID=2


1. **idle进程由系统自动创建，运行在内核态**

idle进程其pid=0，其前身是系统创建的第一个进程，也是唯一一个没有通过fork或者kernel\_thread产生的进程。
完成加载系统后，演变为进程调度、交换。

2. **init进程由idle通过kernel\_thread创建，在内核空间完成初始化后，加载用户态/sbin/init程序，并最终在用户空间运行**

由0进程创建，完成系统的初始化，是系统中所有其他用户进程的祖先进程。Linux中的所有进程都是由init进程创建并运行的。首先Linux内核启动，然后在用户空间启动init进程，在由init进程启动其他系统进程。
在系统启动完成后，init进程将变为守护进程监视系统其他进程。

3. **kthreadd进程由idle通过kernel\_thread创建，并最终运行在内核空间，负责所有内核线程的调度和管理**

它的任务就是管理和调度其他内核线程kernel\_thread，会循环执行一个kthread的函数，该函数的作用就是运行kthread\_create\_list全局链表中维护kthread，当我们调用kernel\_thread创建的内核线程会被加入到此链表中，因此threadd为父进程。

我们下面就详解分析0号进程的前世（init\_task）今生（idle）

# 2. idle的创建

idle进程其pid=0，其前身是系统创建的第一个进程，也是唯一一个没有通过fork()产生的进程。
在SMP系统中，每个处理器单元有独立的一个运行队列，而每个运行队列上又有一个idle进程，即有多少处理器单元，
就有多少idle进程。系统的空闲时间，其实就是指idle进程的运行时间。
既然是idle是进程，那我们看看idle是如何被创建，有具体做了哪些事情？

## 2.1 0号进程上下文信息——init_task描述符

init\_task是内核所有进程、线程的task\_struct雏形，在内核初始化过程中，通过静态定义构造出了一个task\_struct结构，取名为init\_task，然后在内核初始化的后期，通过rest\_init()函数新建了内核init线程、kthreadd线程。

- 内核init线程，最终执行/sbin/init进程，变为所有用户态的根程序，即用户空间的init进程

开始的init时有kthread\_thread创建的内核线程，它在完成初始化工作后，转向用户空间，并且生成所有用户进程的祖先。

- 内核kthreadd内核线程，变为所有内核态其他线程的父线程

它的任务就是管理和调度其他内核线程kernel\_thread，会循环执行一个kthread的函数，该函数的作用就是运行kernel\_create\_list全局变量中维护的kthread，当我们调用kernel\_thread创建的内核线程会被加入到此链表中，因为所有内核线程都是直接或者间接的以kthreadd为父进程。

```
/sbin/init: symbolic link to `../lib/systemd/systemd'

UID        PID  PPID  C STIME TTY          TIME CMD
root         1     0  0 Feb19 ?        00:06:35 /usr/lib/systemd/systemd
root         2     0  0 Feb19 ?        00:00:00 [kthreadd]
```
所以init\_task决定了系统所有进程、线程的基因，它完成初始化后，最终演变为0号进程idle，并且运行在内核态。

内核在初始化过程中，当创建完init和kthreadd线程后，内核会发生调度执行，此时内核将使用该init\_task作为其task\_struct结构体描述符，当系统无事可做时，会调度其执行，此时该内核会变为idle进程，让出CPU，自己进入睡眠，不停地循环。查看init\_task结构体，其comm字段为swapper，作为idle进程的描述符。

> idle的运行时机：</br>idle进程优先级为MAX_PRIO-20。早先版本中，idle是参与调度的，所以将其优先级设置最低，当没有其他进程可以运行时，才会执行idle。而目前的版本中，idle并不在运行队列中参与调度，而是在运行队列结构中含idle指针，指向idle进程，在调度器发现运行队列为空的时候调入运行。

简言之，内核中init\_task变量就是进程0使用的进程描述符，也是Linux系统中第一个进程描述符，init\_task并不是系统通过kernel\_thread的方式（当然更不可能使fork）创建的，而是由内核静态创建的。

该进程的描述符在init/init\_task.c中定义，代码片段如下：
```c
/* Initial task structure */
struct task_struct init_task = INIT_TASK(init_task);
EXPORT_SYMBOL(init_task);
```
init\_task描述符使用宏INIT\_TASK对init\_task的进程描述符进程初始化，宏INIT\_TASK在include/linux/init\_task.h文件中定义。

init\_task是Linux内核中的第一个线程，它贯穿于整个Linux系统的初始化过程中，该进程也是Linux系统唯一一个没用kernel_thread()函数创建的内核线程。

在init\_task进程执行后期，它会调用kernel\_thread()函数创建第一个核心进程kernel\_init，同时init\_task进程继续对Linux系统初始化。
在完成初始化后，init\_task会退化为cpu\_idle进程，当core 0的就绪队列中没有其他进程时，该进程将会获得CPU运行。
新创建的1号进程kernel\_init将会逐个启动其他CPU，并最终创建用户进程。

> core 0上的idle进程由init\_task进程退化而来，而AP的idle进程则是BSP在后面调用fork()函数逐个创建的。

### 2.1.1 进程堆栈init_thread_union

init\_task进程使用init\_thread\_union数据结构描述的内存区域作为该进程的堆栈空间，并且和自身的thread\_info参数共用这一内存空间。

INIT\_TASK宏定义在include/linux/init_task.h中

```c
#define INIT_TASK(tsk)  \
{                                   
    ......
    
    .stack      = &init_thread_info,
    .mm     = NULL, 
    .active_mm  = &init_mm,      
    ......
}
```
而init\_thread\_info则是一段体系结构相关的定义，被定义在arch/x86/asm/thread\_info.h中，但是它们大多数为如下定义：
```c
#define init_thread_info    (init_thread_union.thread_info)
#define init_stack      (init_thread_union.stack)
```
其中init\_thread\_union被定义在init/init\_task.c
```c
/*
 * Initial thread structure. Alignment of this is handled by a special
 * linker map entry.
 */
union thread_union init_thread_union __init_task_data =
    { INIT_THREAD_INFO(init_task) };
```
我们可以发现init\_task使用INIT\_THREAD\_INFO宏进行初始化，这个才是我们**真正体系结构相关的部分**，它与init\_thread\_info定义在一起，被定义在arch/x86/include/asm/thread\_info.h中：
```c
#define INIT_THREAD_INFO(tsk)           \
{                       \
    .task       = &tsk,         \
    .exec_domain    = &default_exec_domain, \
    .flags      = 0,            \
    .cpu        = 0,            \
    .preempt_count  = INIT_PREEMPT_COUNT,   \
    .addr_limit = KERNEL_DS,        \
    .restart_block = {          \
        .fn = do_no_restart_syscall,    \
    },                  \
}
```
init\_thread\_info定义中的**\_\_init\_task\_data**表明该内核栈所在的区域**位于kernel images的init data区**，我们可以通过编译完的内核后所产生的System.map来看到该变量及其对应的逻辑地址。
```shell
[root@lab boot]# cat System.map-3.10.0-693.el7.centos.x86_64.debug | grep init_thread_union
ffffffff81ae0000 D init_thread_union
```

### 2.1.2 进程内存空间

init\_task的虚拟地址空间，也采用同样的方法被定义。

由于init\_task是一个运行在内核空间的内核线程，因此成员mm为NULL，但是必要的时它还是需要使用虚拟地址的，因此avtive\_mm被设置为init\_mm。

其中init\_mm被定义在mm/init-mm.c中：

```c
struct mm_struct init_mm = {
    .mm_rb      = RB_ROOT,
    .pgd        = swapper_pg_dir,
    .mm_users   = ATOMIC_INIT(2),
    .mm_count   = ATOMIC_INIT(1),
    .mmap_sem   = __RWSEM_INITIALIZER(init_mm.mmap_sem),
    .page_table_lock =  __SPIN_LOCK_UNLOCKED(init_mm.page_table_lock),
    .mmlist     = LIST_HEAD_INIT(init_mm.mmlist),
    INIT_MM_CONTEXT(init_mm)
};
```

# 3. 0号进程的演化

## 3.1 rest_init创建init进程（PID=1）和Kthread进程（PID=2）

Linux在无进程概念的情况下将一直从初始化部分的代码执行到start\_kernel()，然后再到其最后一个函数调用reset\_init()。

大致是vmlinux的入口start\_32(head.S)中为pid号为0的原始进程设置了执行幻境，然后原始进程开始执行start\_kernel()完成Linux内核的初始化工作。包括初始化页表，初始化中断向量表，初始化系统时间等。

从rest\_init开始，Linux开始产生进程，因为init\_task是静态制造出来的，pid=0，它试图将从最早的汇编代码一直到start\_kernel的执行都纳入到init\_task进程上下文中。

reset\_init函数其实是由0号进程执行的，它就是在这个函数中，创建了init进程和kthreadd进程。
```c
static noinline void __init_refok rest_init(void)
{
    int pid;

    rcu_scheduler_starting();
    /*
     * We need to spawn init first so that it obtains pid 1, however
     * the init task will end up wanting to create kthreads, which, if
     * we schedule it before we create kthreadd, will OOPS.
     */
    kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);
    numa_default_policy();
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
    rcu_read_lock();
    kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
    rcu_read_unlock();
    complete(&kthreadd_done);

    /*
     * The boot idle thread must execute schedule()
     * at least once to get things moving:
     */
    init_idle_bootup_task(current);
    schedule_preempt_disabled();
    /* Call into cpu_idle with preempt disabled */
    cpu_startup_entry(CPUHP_ONLINE);
}
```
1. 调用kernel\_thread()创建1号内核线程，该线程随后转向用户空间，演变为init进程
2. 调用kernel_thrad()创建kthreadd内核线程
3. init\_idle\_bootup\_task()：当前0号进程init\_task最终会退化成idle进程，所以这样调用init\_idle\_bootup\_task()函数，让init\_task进程隶属于idle调度类中，即选择idle的相关调度函数。
4. 调用schedule()函数切换当前进程，在调用该函数之前，Linux系统中只有两个进程，即0号进程init\_task和1号进程kernel\_init，起哄kernel\_init进程也是刚刚被创建的。调用该函数后，1号进程kernel\_init将会运行。
5. 调用cpu\_idle()，0号线程进入idle函数的循环，该循环中会周期性地检查。

### 3.1.1 创建kernel_init

在rest\_init函数中，内核通过下面的代码产生第一个真正的进程（pid=1）：
```c
kernel_thread(kernel_init, NULL, CLONE_FS);
```
这个进程就是著名的pid为1的init进程，它会继续完成剩下的初始化工作，然后execve('/sbin/init')，称为系统中其他所有进程的祖先。

但是这里我们发现一个问题，init进程应该是一个用户空间的进程，但是这里却是通过kernel\_thread的方式创建的，那岂不是一个永远运行在内核态的内核线程么，它是怎么演变成真正意义上用户空间的init进程的？

1号kernel\_init进程完成Linux的各项配置（包括启动AP）后，就会在/sbin/、/etc、/bin寻找init程序来运行。
该init程序会替换kernel\_init进程（注意：并不是创建一个新的进程来运行init程序，而是一次变身，使用sys\_execve函数改变核心进程的正文段，将核心进程kernel\_init转换成用户进程init），此时处于内核态的1号kernel\_init进程将会转换为用户空间的1号进程init。用户进程init将根据/etc/inittab中提供的信息完成应用程序的初始化调用。然后init进程会执行/bin/sh产生shell页面提供给用户来与Linux系统进行交互。

> 调用init_post()创建用户模式1号进程。

关于init其他的信息我们这次先不研究，因为我们这篇旨在探究0号进程的详细过程。

### 3.1.2 创建kthreadd

在reset\_init函数中，内核将通过下面的代码产生第一个kthreadd（pid=2）
```c
pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
```
它的任务就是管理和调度其他内核线程kernel\_thread，会循环执行一个kthread的函数，该函数的作用就是运行
kthread\_create\_list全局链表中维护的kthread，当我们调用kernel\_thread创建的内核线程会被加入到此链表中，
因此所有的内核线程都是直接或者间接的以kthreadd为父进程。

## 3.2 0号进程演变为idle

```c
    init_idle_bootup_task(current);
	schedule_preempt_disabled();
	/* Call into cpu_idle with preempt disabled */
	cpu_startup_entry(CPUHP_ONLINE);
```
因此我们回过头来看pid=0的进程，在创建了init进程后，pid=0的进程调用cpu\_idle()演变成了idle进程。

0号进程首先执行init\_idle\_bootup\_task，让init\_task进程隶属于idle调度类中。即选择idle的调度方法。

这个函数被定义在kernel/sched/core.c中，如下：
```c
void init_idle_bootup_task(struct task_struct *idle)
{
    idle->sched_class = &idle_sched_class;
}
```
接着通过schedule\_preempt\_disabled来执行调用schedule()函数切换当前进程，在调用该函数之前，Linux系统中只有两个进程，即0号进程init\_task和1号进程kernel\_init，其中kernel\_init进程也是刚刚被创建的。
调用该函数后，1号进程kernel\_init将会运行。

这个函数定义在kernel/sched/core.c中，如下：
```c
/**
 * schedule_preempt_disabled - called with preemption disabled
 *
 * Returns with preemption disabled. Note: preempt_count must be 1
 */
void __sched schedule_preempt_disabled(void)
{
    sched_preempt_enable_no_resched();
    schedule();
    preempt_disable();
}
```
最后cpu\_startup\_entry调用cpu\_idle\_loop()，0号进程进入idle函数的循环，在该循环中会周期性地检查。

cpu\_startup\_entry定义在kernel/sched/idle.c中：
```c
void cpu_startup_entry(enum cpuhp_state state)
{
    /*
     * This #ifdef needs to die, but it's too late in the cycle to
     * make this generic (arm and sh have never invoked the canary
     * init for the non boot cpus!). Will be fixed in 3.11
     */
#ifdef CONFIG_X86
    /*
     * If we're the non-boot CPU, nothing set the stack canary up
     * for us. The boot CPU already has it initialized but no harm
     * in doing it again. This is a good place for updating it, as
     * we wont ever return from this function (so the invalid
     * canaries already on the stack wont ever trigger).
     */
    boot_init_stack_canary();
#endif
    arch_cpu_idle_prepare();
    cpu_idle_loop();
}
```
其中cpu\_idle\_loop就是idle进程的时间循环，定义在kernel/sched/idle.c。

整个过程简单来说就是，原始进程（pid=0）创建init进程（pid=1），然后原始进程演化成idle进程（pid=0）。
init进程为每个AP处理器（每个运行队列）创建出一个idle进程（pid=0），然后演化成/sbin/init。

# 4. idle的运行与调度

## 4.1 idle的workload——cpu_idle_loop

从上面的分析我们知道，idle在系统没有其他就绪的进程可执行的时候才会被调度。不管是BSP处理器还是AP处理器，最后都是执行的cpu\_idle\_loop()函数。

其中cpu\_idle\_loop就是idle进程的事件循环，定义在kernel/cpu/idle.c，早期的版本中提供的是cpu\_idle，但是这个函数是完全依赖于体系结构的，不利于架构的分层，因此在新的内核中更新为更加通用的cpu\_idle\_loop，由它来调用体系结构相关的代码。

所以我们来看看cpu\_idle\_loop做了什么事情。

因为idle进程中并不执行什么有意义的任务，所以通常考虑的是两点：
1. 节能
2. 低退出延迟

其他代码如下：
```c
/*
 * Generic idle loop implementation
 *
 * Called with polling cleared.
 */
static void cpu_idle_loop(void)
{
    while (1) {
        /*
         * If the arch has a polling bit, we maintain an invariant:
         *
         * Our polling bit is clear if we're not scheduled (i.e. if
         * rq->curr != rq->idle).  This means that, if rq->idle has
         * the polling bit set, then setting need_resched is
         * guaranteed to cause the cpu to reschedule.
         */

        __current_set_polling();
        quiet_vmstat();
        tick_nohz_idle_enter();

        while (!need_resched()) {
            check_pgt_cache();
            rmb();

            if (cpu_is_offline(smp_processor_id()))
                arch_cpu_idle_dead();

            local_irq_disable();
            arch_cpu_idle_enter();
            /*
             * In poll mode we reenable interrupts and spin.
             *
             * Also if we detected in the wakeup from idle
             * path that the tick broadcast device expired
             * for us, we don't want to go deep idle as we
             * know that the IPI is going to arrive right
             * away
             */
            if (cpu_idle_force_poll || tick_check_broadcast_expired()) {
                cpu_idle_poll();
            } else {
                if (!current_clr_polling_and_test()) {
                    stop_critical_timings();
                    rcu_idle_enter();
                    arch_cpu_idle();
                    WARN_ON_ONCE(irqs_disabled());
                    rcu_idle_exit();
                    start_critical_timings();
                } else {
                    local_irq_enable();
                }
                __current_set_polling();
            }
            arch_cpu_idle_exit();
        }
        tick_nohz_idle_exit();
        __current_clr_polling();
        /*
         * We promise to call sched_ttwu_pending and reschedule
         * if need_resched is set while polling is set.  That
         * means that clearing polling needs to be visible
         * before doing these things.
         */
        smp_mb__after_atomic();

        sched_ttwu_pending();
        schedule_preempt_disabled();
    }
```
循环判断need\_resched以降低退出延迟，用idle()来节能。

默认的idle实现是hlt指令，hlt指令使CPU处于暂停状态，等待硬件中断发生的时候恢复，从而达到节能的目的。即从处理器C0状态转换到C1状态（详见ACPI标准）。这也是早些年windows平台上各种处理器降温工具的主要手段。
当然idle也可以在别的ACPI或者APM模块中定义的，甚至是自定义的一个idle（比如说nop）。
1. idle是一个进程，其pid为0。
2. BSP处理器上的idle由原始进程（pid=0）演变而来。AP处理器上的idle由init进程fork得到，它是它们的pid都为0。
3. idle进程为最低优先级，且不参与导读，指示在运行队列为空的时候才被调度。
4. idle循环等待need\_resched置位。默认使用hlt节能。

## 4.2 idle的调度和运行时机

我们知道，Linux进程的调度顺序是按照rt实时进程（rt调度器），normal普通进程（cfs调度器），和idle的顺序来调度的，那么可以试想如果rt和cfs都没有可运行的任务，那么idle才可以被调度，那么它是通过怎样的方式实现的呢？

由于我们还没有讲解调度器的指示，所以我们只是简单讲解一下：
在normal的调度类，cfs调度器sched\_fair.c中，我们可以看到
```c
const struct sched_class fair_sched_class = {
    .next           = &idle_sched_class,
    
    ......
}
```
也就是说，如果系统中没有普通进程，那么会选择下个调度类优先级的进程，即使用idle\_sched\_class调度类进程调度的进程。

当系统空闲的时候，最后的就是调用idle的pick\_next\_task\_idle函数，被定义在kernel/sched/idle\_task.c中：
```c
static struct task_struct *pick_next_task_idle(struct rq *rq)
{
    schedstat_inc(rq, sched_goidle);
#ifdef CONFIG_SMP
    /* Trigger the post schedule to do an idle_enter for CFS */
    rq->post_schedule = 1;
#endif
    return rq->idle;    // 可以看到就是返回rq中idle进程。
}
```
这idle进程在启动start\_kernel函数的时候调用init\_idle函数的时候，就把当前进程（0号进程）设伪每个rq运行队列上的idle上。
```c
    rq->curr = iq->idle = idle;
```
这里idle就是调用start\_kernel函数的进程，就是0号进程。

# 5. idle进程总结

系统允许一个进程创建新进程，新进程即为子进程，子进程还可以创建新的子进程，形成进程树结构模型。
整个Linux系统的所有进程也是一个树状结构。树根是系统静态构造的，即在内核态下执行的0号进程，
它是所有进程的远古先祖。

在SMP系统中，每个处理器单元有独立的一个运行队列，而每个运行队列上有一个idle进程，即有多少个处理器单元，就有多少idle进程。

1. idle进程其pid=0，其前身是系统创建的第一个进程（我们称之为init\_task），也是唯一一个没有通过fork或者kernel\_thread产生的进程。
2. init\_task是内核中所有进程、线程的task\_struct雏形，它是内核初始化过程中，通过静态定义构造出一个task\_struct结构，取名为init\_task，然后在内核初始化的后期，在rest\_init()函数中通过kernel\_thread创建了两个内核线程即init线程与kthreadd线程，init线程后来通过演变，进入用户空间，成为所有用户进程的先祖，而后者则称为所有内核态线程的守护线程，负责接手内核线程的创建工作。
3. 然后init\_task通过变更调度类为sched\_idle等操作便成为idle进程，此时系统中只有idle(0)、init(1)、ktreadd(2)3个进程，然后执行一次进程调度，必然切换当前进程到init。

# 6. 附录——rest_init的执行解析

```c
static noinline void __init_refok rest_init(void)
{
    int pid;

    rcu_scheduler_starting();
    /*
     * We need to spawn init first so that it obtains pid 1, however
     * the init task will end up wanting to create kthreads, which, if
     * we schedule it before we create kthreadd, will OOPS.
     */
    kernel_thread(kernel_init, NULL, CLONE_FS | CLONE_SIGHAND);
    numa_default_policy();
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
    rcu_read_lock();
    kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
    rcu_read_unlock();
    complete(&kthreadd_done);

    /*
     * The boot idle thread must execute schedule()
     * at least once to get things moving:
     */
    init_idle_bootup_task(current);
    schedule_preempt_disabled();
    /* Call into cpu_idle with preempt disabled */
    cpu_startup_entry(CPUHP_ONLINE);
}
```

- rcu\_scheduler\_starting();

启动Read-Copy Update，会调用num\_online\_cpus确认目前只有bootstrap处理器在运作，以及调用nr\_context\_switches确认在启动RCU前，没有进行过Contex-Switch，最后就是设定rcu\_scheduler\_active=1启动RCU机制。

RCU在多核心架构下，不同的线程要读取同一笔资料内容/结构，可以提供高效率的同步与正确性。在这之后就可以使用 rcu\_read\_lock/rcu\_read\_unlock了。

- kernel\_thread(kernel\_init, NULL, CLONE\_FS | CLONE\_SIGHAND);

kernel\_init实例在init/main.c中，init Task PID=1，是内核第一个产生的Task。 产生后，会阻塞在wait\_for\_completion处，等待kthreadd\_done Signal，以便往后继续执行下去。

- kernel\_thread(kthreadd, NULL, CLONE\_FS | CLONE\_FILES);

kthreadd实例在kernel/kthread.c中，kthreadd Task PID=2，是内核第二个产生的Task。

- kthreadd\_task = find\_task\_by\_pid\_ns(pid, &init\_pid_ns);

实例在kernel/pid.c中，调用函数find\_task\_by\_pid\_ns,并传入参数kthreadd的PID 2与PID NameSpace (struct pid\_namespace init\_pid\_ns)取回PID 2的Task Struct。

- complete(&kthreadd_done);

实例在kernel/sched.c中，会发送kthreadd\_done Signal，让kernel_init(也就是 init task)可以往后继续执行。

- schedule\_preempt_disabled();

启动一次Linux Kernel Process的排程Context-Switch调度机制, 从而使得kernel_init即1号进程获得处理机。

-  cpu\_startup\_entry(CPUHP_ONLINE);

完成工作后，调用cpu\_idle\_loop()使得idle进程进入自己的事件处理循环。