***Linux核心调度器之周期性调度器scheduler_tick***

- 1 两个调度器
- 2 周期性调度器
    - 2.1 周期性调度器主流程
    - 2.2 更新统计量
    - 2.3 激活进程所属调度类的周期性调度器方法
- 3 周期性调度器的激活
    - 3.1 定时器周期性的激活调度器
    - 3.2 早期实现
    - 3.3 目前实现

# 1. 两个调度器

我们前面提到Linux有两种方法激活调度器：
1. 一种是直接的，比如，进程打算睡眠或者出于其他原因放弃CPU
2. 另一种是通过周期性的机制，以固定的频率运行，不时的检测是否有必要

因而内核提供了两个调度器：主调度器和周期性调度器，分别实现如上工作，两者合在一起就组成了核心调度器（core scheduler）也叫通用调度器（generic scheduler）。

它们都根据进程的优先级分配CPU时间，因此这个过程就优先调度，我们在本节主要讲解核心调度器的设计和优先调度的实现方式。

而我们的周期性调度器以固定频率激活，负责当前进程调度类的周期性调度方法以保证系统的并发性。

# 2. 周期性调度器

周期性调度器在scheduler\_tick()中实现，如果系统正在活动中，内核会按照频率HZ自动调用该函数。如果没有进程在等待调度，那么在计算机电力供应不足的情况下，内核将关闭该调度器以减少功耗。
这对我们的嵌入式设备或者手机终端设备的电源管理很重要。

## 2.1 周期性调度器主流程

scheduler\_tick函数定义在kernel/sched/core.c中，它由两个主要任务：
1. 更新相关统计量：管理内中中的整个系统和各个进程调度相关的统计量，其间执行的主要操作是对各计数器+1。
2. 激活负责当前进程调度类的周期性：检查进程执行的时间是否超过了它对应的idel\_runtime，如果超过了，则告诉系统，需要启动主调度器（schedule）进行进程切换。

```c
void scheduler_tick(void)
{
    /*  1.  获取当前cpu上的全局就绪队列rq和当前运行的进程curr  */

    /*  1.1 在于SMP的情况下，获得当前CPU的ID。如果不是SMP，那么就返回0  */
    int cpu = smp_processor_id();

    /*  1.2 获取cpu的全局就绪队列rq, 每个CPU都有一个就绪队列rq  */
    struct rq *rq = cpu_rq(cpu);

    /*  1.3 获取就绪队列上正在运行的进程curr  */
    struct task_struct *curr = rq->curr;

    sched_clock_tick();

	/*  2 更新rq上的统计信息, 并执行进程对应调度类的周期性的调度  */

    /*  加锁 */
    raw_spin_lock(&rq->lock);

    /*  2.1 更新rq的当前时间戳.即使rq->clock变为当前时间戳  */
    update_rq_clock(rq);

    /*  2.2 执行当前运行进程所在调度类的task_tick函数进行周期性调度  */
    curr->sched_class->task_tick(rq, curr, 0);

    /*  2.3 更新rq的负载信息,  即就绪队列的cpu_load[]数据
     *  本质是讲数组中先前存储的负荷值向后移动一个位置，
     *  将当前负荷记入数组的第一个位置 */
    update_cpu_load_active(rq);

    /*  2.4 更新cpu的active count活动计数
     *  主要是更新全局cpu就绪队列的calc_load_update*/
    calc_global_load_tick(rq);

    /* 解锁 */
    raw_spin_unlock(&rq->lock);

    /* 与perf计数事件相关 */
    perf_event_task_tick();

#ifdef CONFIG_SMP

     /* 当前CPU是否空闲 */
    rq->idle_balance = idle_cpu(cpu);

    /* 如果到是时候进行周期性负载平衡则触发SCHED_SOFTIRQ */
    trigger_load_balance(rq);

#endif

    rq_last_tick_reset(rq);
}
```

## 2.2 更新统计量

函数 | 描述 | 定义
---|---|---
update\_rq\_clock | 处理就绪队列的更新，本质上是增加struct rq当前实例的时钟时间戳。 | sched/core.c
update\_cpu\_load\_active | 负责更新就绪队列的cpu\_load数组，</br>其本质相当于将数组中先前存储的负荷值向后移动一个位置，</br>将当前就绪队列的负荷记入数组的第一个位置，</br>另外该函数还引入一些取平均值的技巧，</br>以确保负荷数组的内容不会呈现太多的不连续跳读。| kernel/sched/fair.c
calc\_global\_load\_tick | 更新CPU的活动计数，主要是更新全局CPU就绪队列的calc\_load\_update。 | kernel/sched/loadavg.c

## 2.3 激活进程所属调度类的周期性调度器方法

由于调度器的模块化结构，主体工程其实很简单，在更新统计信息的同时，内核将真正的调度工作委托给了特定的调度类方法。

内核先找到了就绪队列上当前运行的进程curr，然后调用curr所属调度类sched\_class的周期性调度方法task\_tick，即：
```c
    curr->sched_class->task_tick(rq, curr, 0);
```
task\_tick的实现方法取决于低层的调度类，例如，CFS会在该方法中检测是否进程已经运行了太长的时间，以免过长的延迟，注意，此处的做法与之前旧的基于时间片的调度方法有本质区别，旧的方法我们称之为到期的时间片，而CFS中则不存在所谓的时间片的概念。

目前内核中的3个调度实体struct sched\_entity，struct sched\_rt\_entity和struct sched\_dl\_entity，我们针对当前内核中实现的调度类分别列出周期性调度函数task\_tick：


调度类 | task\_tick操作 | task\_tick函数定义
---|---|---
stop\_sched\_class | task\_tick\_stop | kernel/sched/stop_task.c
dl\_sched\_class | task\_tick_dl | kernel/sched/deadline.c
rt\_sched\_class | task\_tick_rt | kernel/sched/rt.c
fair\_sched\_class | task\_tick_fair | kernel/sched/fair.c
idle\_sched\_class | task\_tick\_idle | kernel/sched/idle_task.c

- 如果当前进程是完全公平队列中的进程，则首先根据当前就绪队列中的进程个数算出一个延迟时间间隔，大概每个进程分配2ms时间，然后按照该进程在队列中的总权重中占的比例，算出它该执行的时间X，如果该进程执行物理时间超过了X，则激发延迟调度；如果没有超过X，但是红黑树就绪队列中下一个进程优先级更高，即curr->vruntime->leftmost->vruntime > X，也将延迟调度。

> 延迟调度的真正调度过程在：schedule中实现，会按照调度类的顺序和优先级挑选出一个最高优先级的进程执行

- 如果当前进程是实时调度类中的进程：则如果该进程是SCHED\_RR，则递减时间片为[HZ/10]，到期，插入到队列尾部，并激发延迟调度，如果是SCHED\_FIFO，则什么也不做，直到该进程执行完成。

如果当前进程希望被重新调度，那么调度类方法会在task\_struct中设置TIF\_NEED\_RESCHED标志，以表示该请求，而内核将会在接下来的适当时候完成此请求。

# 3. 周期性调度器的激活

## 3.1 定时器周期性的激活调度器

定时器是Linux提供的一种定时服务机制。它在某个特定的时间唤醒某个进程，来做一些工作。

在低分辨率定时器的每次时钟中断完成全局统计量更新后，每个CPU在SoftIRQ中执行以下操作：
- 更新该CPU上当前进程内核态、用户态使用事件xtime\_update
- 调用该cpu上的定时器函数
- 启动周期性定时器（scheduler\_tick）完成该cpu上任务的周期性调度工作

在支持动态定时器的系统中，可以关闭该调度器（没有进程在等待调度时候），从而进入深度睡眠过程；
scheduler\_tick查看当前进程是否运行太长时间，如果是，将进程的TIF\_NEED\_RESCHED置位，然后在中断返回时，调用schedule，进行进程切换操作。

```c
/*
* Kernel system timer support.
*/
void timer_tick(void)
{
    profile_tick(CPU_PROFILING);
    xtime_update(1);
#ifndef CONFIG_SMP
    update_process_times(user_mode(get_irq_regs()));
#endif
}

/*
 * Called from the timer interrupt handler to charge one tick to the current
 * process.  user_tick is 1 if the tick is user time, 0 for system.
 */
void update_process_times(int user_tick)
{
    struct task_struct *p = current;

    /* Note: this timer irq context must be accounted for as well. */
    account_process_tick(p, user_tick);
    run_local_timers();
    rcu_check_callbacks(user_tick);
#ifdef CONFIG_IRQ_WORK
    if (in_irq())
        irq_work_tick();
#endif
    // 周期性调度器
    scheduler_tick();
    run_posix_cpu_timers(p);
}
```

## 3.2 早期实现

Linux初始化时，init\_IRQ()函数设定8253的定时周期为10ms（一个tick值）。
同样，在初始化时，time\_init()用setup\_irq()设置时钟中断向量IRQ0，中断服务程序为timer\_interrupt。

在Linux 2.4版本中，定时器的中断处理采用下半部机制，下半部处理函数的注册在start\_kernel()函数中调用sched\_init()，在这个函数中又调用init\_bh(TIMER\_BH, timer\_bh)注册了定时器的下半部处理函数。
然后系统才调用time\_init()来注册定时器的中断向量和中断处理函数。

在中断处理函数timer\_interrupt()中，主要是调用do\_timer()函数完成工作。
do\_timer()函数的主要功能就是调用mark\_bg()产生软中断，随后处理器会在合适的时候调用定时器的下半部处理函数timer\_bh()。在timer\_bh()中，实现了更新定时器的功能。

Linux 2.4.23版的do\_timer()函数代码如下：
```c
void do_timer(struct pt_regs *regs)
{
       (*(unsigned long *)&jiffies)++;
       update_process_times(user_mode(regs));
       mark_bh(TIMER_BH);
}
```

## 3.3 目前实现

而在Linux 2.6.x版本，定时器中断处理采用了SoftIRQ机制，而不是下半部机制。
时钟中断的处理函数依然为：
```
    timer_interrupt()
        -> do_timer_interrupt()
            -> do_timer_interrupt_hook()
                -> do_timer()
```
不过，do\_timer()函数的实现有所不同：
```c
void do_timer(struct pt_regs *regs)
{
       jiffies_64++;
       update_process_times(user_mode(regs));
       update_times();
}
```