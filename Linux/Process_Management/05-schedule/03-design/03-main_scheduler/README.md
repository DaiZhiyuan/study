***Linux进程核心调度器之主调度器schedule***

- 1 主调度器
    - 1.1 调度函数的__sched前缀
    - 1.2 schedule函数
        - 1.2.1 schedule主框架
        - 1.2.2 sched_submit_work()避免死锁
        - 1.2.3 preempt_disbale和sched_preempt_enable_no_resched开关内核抢占
    - 1.3 __schedule开始进程调度
        - 1.3.1 __schedule函数主框架
        - 1.3.2 pick_next_task选择抢占的进程
    - 1.4 context_switch进程上下文切换
        - 1.4.1 进程上下文切换
        - 1.4.2 context_switch流程
        - 1.4.3 switch_mm切换进程虚拟地址空间
        - 1.4.4 switch_to切换进程堆栈和寄存器
    - 1.5 need_resched，TIF_NEED_RESCHED标识与用户抢占
        - 1.5.1 用户抢占和内核抢占
- 2 总结
    - 2.1 schedule调度流程
    - 2.2 __schedule如何完成内核抢占
    - 2.3 调度的内核抢占和用户抢占

# 1. 主调度器

在内核中的许多地方，如果将CPU分配给与当前活动进程不同的另一个进程，都会直接调用主调度器函数schedule()。

从系统调用返回后，内核也会检查当前进程是否设置了重调度标志TLF\_NEED\_RESCHED。
例如，前述的周期性调度器scheduler\_tick()就会设置该标志，如果是这样则内核会调用schedule()，该函数假定当前活动进程一定会被另一个进程取代。

## 1.1 调度函数的__sched前缀

在详细讨论schedule之前，需要说明一下\_\_sched前缀，该前缀用于可能调用schedule的函数，包括schedule本身。

\_\_sched前缀的声明，在include/linux/sched.h，如下所示：
```c
    /* Attach to any functions which should be ignored in wchan output. */
    #define __sched     __attribute__((__section__(".sched.text")))
```
其中\_\_attribute\_\_((\_\_section\_\_("......")))是一个gcc的编译属性，其目的在于将相关联函数的代表编译之后放到目标文件的特定段内，即.sched.text段中。该信息使得内核在显示栈存储类似的信息，忽略所有与调度相关的调用。由于调度函数调用不是普通代码流程的一部分，因此在这种情况下是没有意义的。

用它修饰函数的方式如下：
```c
void __sched some_function(args, ...)
{
	......
	
	schedule();
	
    ......
}
```

## 1.2 schedule函数

### 1.2.1 schedule主框架

schedule()就是主调度器的函数，在内核中的许多地方，如果要将CPU分配给与当前活动进程不同的另一个进程，就会直接调用主调度器函数schedule()。

该函数完成如下工作：
1. 确定当前就绪队列，并在保存一个指向当前（仍然）活动进程的task\_struct指针。
2. 检查死锁，关闭内核抢占后调用\_\_schedule完成内核调度。
3. 恢复内核抢占，然后检查当前进程是否设置了重新调度标志TIL\_NEED\_RESCHED，如果该进程被其他进程设置了TIL\_NEED\_RESCHED标志，则函数重新执行进行调度。

该函数定义在kernel/sched/core.c，如下所示：
```c
asmlinkage __visible void __sched schedule(void)
{

	/*  获取当前的进程  */
    struct task_struct *tsk = current;

    /*  避免死锁 */
    sched_submit_work(tsk);
    do {
        /* 关闭内核抢占 */
        preempt_disable();									
        /* 完成调度 */
        __schedule(false);									
        /* 开启内核抢占 */
        sched_preempt_enable_no_resched();	 
    /* 如果该进程被其他进程设置了TIF_NEED_RESCHED标志，则函数重新执行进行调度 */
    } while (need_resched());
}
EXPORT_SYMBOL(schedule);
```

### 1.2.2 sched_submit_work()避免死锁

该函数定义在kernel/sched/core.c，如下图所示：
```c
static inline void sched_submit_work(struct task_struct *tsk)
{
	/* 检测tsk->state是否为0 （runnable）, 若为运行态时则返回，
	 * tsk_is_pi_blocked(tsk),检测tsk的死锁检测器是否为空，若非空的话就return */
	 
    if (!tsk->state || tsk_is_pi_blocked(tsk))
        return;
        
    /* 然后检测是否需要刷新plug队列，用来避免死锁 */
    if (blk_needs_flush_plug(tsk))
        blk_schedule_flush_plug(tsk);
}
```

### 1.2.3 preempt_disbale和sched_preempt_enable_no_resched开关内核抢占

Linux除了内核态外还有用户态。用户程序的上下文属于用户态，系统调用和中断处理例程上下文属于内核态。
如果一个进程在用户态时被其他进程抢占了CPU则发生了用户态抢占，而如果此时进程进入了内核态，则内核此时代替进程执行，如果此时发生了抢占，我们就说发生了内核抢占。

内核抢占时Linux 2.6以后引入的一个重要的概念。

我们说：如果进程正在执行内核函数时，即它在内核态运行时，允许发生内核切换（被替换的进程是正执行内核函数的进程），这个内核就是抢占的。

抢占式内核的主要特点是：一个内核态运行的进程，当且仅当在执行内核函数期间被另一个进程取代。这与用户态的抢占有本质区别。

内核为了支撑内核抢占，提供了很多机制和结构，必要时候开关内核抢占也是必须的，这些函数定义在include/linux/preempt.h。
```c
#define preempt_disable() \
do { \
    preempt_count_inc(); \
    barrier(); \
} while (0)

#define sched_preempt_enable_no_resched() \
do { \
    barrier(); \
    preempt_count_dec(); \
} while (0)
```

## 1.3 __schedule开始进程调度

\_\_schedule完成了真正的调度工作，其定义在kernel/sched/core.c，如下所示。

### 1.3.1 __schedule函数主框架

```c
static void __sched notrace __schedule(bool preempt)
{
    struct task_struct *prev, *next;
    unsigned long *switch_count;
    struct rq *rq;
    int cpu;

    /*  ==1==  
        找到当前cpu上的就绪队列rq
        并将正在运行的进程curr保存到prev中  */
    cpu = smp_processor_id();
    rq = cpu_rq(cpu);
    prev = rq->curr;

    if (unlikely(prev->state == TASK_DEAD))
        preempt_enable_no_resched_notrace();
    
    /*  如果禁止内核抢占，而又调用了cond_resched就会出错
     *  这里就是用来捕获该错误的  */
    schedule_debug(prev);

    if (sched_feat(HRTICK))
        hrtick_clear(rq);

    /*  关闭本地中断  */
    local_irq_disable();

    /*  更新全局状态，
     *  标识当前CPU发生上下文的切换  */
    rcu_note_context_switch();

    smp_mb__before_spinlock();
    /*  锁住该队列  */
    raw_spin_lock(&rq->lock);
    lockdep_pin_lock(&rq->lock);

    rq->clock_skip_update <<= 1; /* promote REQ to ACT */

    /*  切换次数记录, 默认认为非主动调度计数(抢占)  */
    switch_count = &prev->nivcsw;
    
    /*
     *  scheduler检查prev的状态state和内核抢占表示
     *  如果prev是不可运行的, 并且在内核态没有被抢占
     *  
     *  此时当前进程不是处于运行态, 并且不是被抢占
     *  此时不能只检查抢占计数
     *  因为可能某个进程(如网卡轮询)直接调用了schedule
     *  如果不判断prev->stat就可能误认为task进程为RUNNING状态
     *  到达这里，有两种可能，一种是主动schedule, 另外一种是被抢占
     *  被抢占有两种情况, 一种是时间片到点, 一种是时间片没到点
     *  时间片到点后, 主要是置当前进程的need_resched标志
     *  接下来在时钟中断结束后, 会preempt_schedule_irq抢占调度
     *  
     *  那么我们正常应该做的是应该将进程prev从就绪队列rq中删除, 
     *  但是如果当前进程prev有非阻塞等待信号, 
     *  并且它的状态是TASK_INTERRUPTIBLE
     *  我们就不应该从就绪队列总删除它 
     *  而是配置其状态为TASK_RUNNING, 并且把他留在rq中

    /*  如果内核态没有被抢占, 并且内核抢占有效
        即是否同时满足以下条件：
        1  该进程处于停止状态
        2  该进程没有在内核态被抢占 */
    if (!preempt && prev->state)
    {

        /*  如果当前进程有非阻塞等待信号，并且它的状态是TASK_INTERRUPTIBLE  */
        if (unlikely(signal_pending_state(prev->state, prev)))
        {
            /*  将当前进程的状态设为：TASK_RUNNING  */
            prev->state = TASK_RUNNING;
        }
        else   /*  否则需要将prev进程从就绪队列中删除*/
        {
            /*  将当前进程从runqueue(运行队列)中删除  */
            deactivate_task(rq, prev, DEQUEUE_SLEEP);

            /*  标识当前进程不在runqueue中  */
            prev->on_rq = 0;

            /*
             * If a worker went to sleep, notify and ask workqueue
             * whether it wants to wake up a task to maintain
             * concurrency.
             */
            if (prev->flags & PF_WQ_WORKER) {
                struct task_struct *to_wakeup;

                to_wakeup = wq_worker_sleeping(prev);
                if (to_wakeup)
                    try_to_wake_up_local(to_wakeup);
            }
        }
        /*  如果不是被抢占的，就累加主动切换次数  */
        switch_count = &prev->nvcsw;
    }

    /*  如果prev进程仍然在就绪队列上没有被删除  */
    if (task_on_rq_queued(prev))
        update_rq_clock(rq);  /*  跟新就绪队列的时钟  */

    /*  挑选一个优先级最高的任务将其排进队列  */
    next = pick_next_task(rq, prev);
    /*  清除pre的TIF_NEED_RESCHED标志  */
    clear_tsk_need_resched(prev);
    /*  清楚内核抢占标识  */
    clear_preempt_need_resched();

    rq->clock_skip_update = 0;

    /*  如果prev和next非同一个进程  */
    if (likely(prev != next))
    {
        rq->nr_switches++;  /*  队列切换次数更新  */
        rq->curr = next;    /*  将next标记为队列的curr进程  */
        ++*switch_count;    /* 进程切换次数更新  */

        trace_sched_switch(preempt, prev, next);
        /*  进程之间上下文切换    */
        rq = context_switch(rq, prev, next); /* unlocks the rq */
    }
    else    /*  如果prev和next为同一进程，则不进行进程切换  */
    {
        lockdep_unpin_lock(&rq->lock);
        raw_spin_unlock_irq(&rq->lock);
    }

    balance_callback(rq);
}
STACK_FRAME_NON_STANDARD(__schedule); /* switch_to() */
```

### 1.3.2 pick_next_task选择抢占的进程

内核从CPU的就绪队列中选择一个最合适的进程来抢占CPU
```c
    next = pick_next_task(rq);
```
全局的pick\_next\_task函数会从按照优先级遍历所有调度类的pick\_next\_task函数，去查找最优的那个进程，当然因为大多数情况下，系统中全是CFS调度的非实时进程，因而Linux内核也有一些优化的策略。

其执行流程如下：
- 如果当前CPU上所有的进程都是CFS调度的普通非实时进程，则直接用CFS调度，如果无程序可调度则调度idle进程
- 否则从优先级最高的调度类sched\_class\_highest（目前是stop\_sched\_class）开始依次遍历所有调度器类的pick\_next\_task函数，选择最优的那个进程执行。

其定义在kernel/sched/core.c，如下所示：

```c
static inline struct task_struct *
pick_next_task(struct rq *rq, struct task_struct *prev)
{
    const struct sched_class *class = &fair_sched_class;
    struct task_struct *p;

    /*
     * 如果待被调度的进程prev是隶属于CFS的普通非实时进程
     * 而当前cpu的全局就绪队列rq中的进程数与cfs_rq的进程数相等
     * 则说明当前cpu上的所有进程都是由cfs调度的普通非实时进程
     *
     * 那么我们选择最优进程的时候
     * 就只需要调用cfs调度器类fair_sched_class的选择函数pick_next_task
     * 就可以找到最优的那个进程p
     */
    /*  如果当前所有的进程都被cfs调度, 没有实时进程  */
    if (likely(prev->sched_class == class &&
           rq->nr_running == rq->cfs.h_nr_running))
    {
        /*  调用cfs的选择函数pick_next_task找到最优的那个进程p*/
        p = fair_sched_class.pick_next_task(rq, prev);
        /*  #define RETRY_TASK ((void *)-1UL)有被其他调度气找到合适的进程  */
        if (unlikely(p == RETRY_TASK))
            goto again; /*  则遍历所有的调度器类找到最优的进程 */

        /* assumes fair_sched_class->next == idle_sched_class */
        if (unlikely(!p))   /*  如果没有进程可被调度  */
            p = idle_sched_class.pick_next_task(rq, prev); /*  则调度idle进程  */

        return p;
    }

/*  进程中所有的调度器类, 是通过next域链接域链接在一起的
 *  调度的顺序为stop -> dl -> rt -> fair -> idle 
 *  again出的循环代码会遍历他们找到一个最优的进程  */
again:
    for_each_class(class)
    {
        p = class->pick_next_task(rq, prev);
        if (p)
        {
            if (unlikely(p == RETRY_TASK))
                goto again;
            return p;
        }
    }

    BUG(); /* the idle class will always have a runnable task */
}
```
进程中所有的调度器类，是通过next域连接域链接在一起的，调度的顺序为：
```
stop -> dl -> rt -> fair -> idle
```
其中for\_each\_class遍历所有的调度器类，依次执行pick\_next\_task操作选择最优的进程，它会从优先级最高的sched\_class\_highest（目前是stop\_sched\_class）查起，依次按照调度器类的优先级从高到低的顺序调用调度器类对应的pick\_next\_task\_fair函数直到查找一个能够被调度的进程。

for\_each\_class定义在kernel/sched/sched.h，如下所示：
```c
#define sched_class_highest (&stop_sched_class)
#define for_each_class(class) \
   for (class = sched_class_highest; class; class = class->next)

extern const struct sched_class stop_sched_class;
extern const struct sched_class dl_sched_class;
extern const struct sched_class rt_sched_class;
extern const struct sched_class fair_sched_class;
extern const struct sched_class idle_sched_class;
```
除了全局的pick\_next\_task函数，每个调度器类都提供了pick\_next\_task函数用于查找对应调度器下的最优进程，如定义如下所示：

调度器类 | pick\_next策略 | pick\_next\_task函数
---|---|---
stop\_sched\_class | none |  pick\_next\_task\_stop
dl\_sched\_class | none | pick\_next\_task\_dl
rt\_sched\_class | 取出合适的进程后，dequeue\_pushable\_task从pushbale队列里取出来 | pick\_next\_task\_rt
fair\_sched\_class | pick\_next\_task\_fair，从红黑树里选出vtime最小的哪个进程，</br>调用set\_next\_entity将其出队 | pick\_next\_task\_fair
idle\_sched\_class | 直接调度idle进程 | pick\_next\_task\_idle

> 实际上，对于RT进程，put和pick并不操作运行队列
>
> 对于FIFO和RR的区别，在scheduler\_tick中通过curr->sched\_class->task\_tick进入到task\_tick\_rt的处理，如果是非RR的进程则直接返回，否则递减时间片，如果时间片耗完，则需要将当前进程放到运行队列的末尾，这个时候才操作运行队列，时间片到点，会重新移动当前进程requeue\_task\_rt，进程会被加到队列尾，接下来set\_tsk\_need\_resched触发调度，进程被抢占进入schedule。

***为什么要多此一举判断所有的进程是否全是cfs调度的普通非实时进程呢？***

加快经常性事件，是程序开发中的一个优化的准则，那么Linux系统中最普遍的进程是什么呢？肯定是非实时进程啊，其调度器必然是CFS，因此：
```c
    rev->sched_class == class && rq->nr_running == rq->cfs.h_nr_running
```
这种情况发生的概率非常大，也就是说多数情况下，我们的Linux中进程全是CFS调度的。

而likely这个宏也表明了这点，这也是gcc内建的一个编译选项，它其实就是告诉编译器表达式很大的情况下为真，编译器可以对此作出优化：
```c
#ifndef likely
# define likely(x)     (__builtin_expect(!!(x), 1))
#endif
 
#ifndef unlikely
# define unlikely(x)   (__builtin_expect(!!(x), 0))
#endif
```

## 1.4 context_switch进程上下文切换

### 1.4.1 进程上下文切换

上下文切换（有时也称做进程切换或任务切换）是指CPU从一个进程或线程切换到另一个进程或线程。

稍微详细描述一下，上下文切换可以认为是内核（操作系统的核心）在CPU上对于进程（线程）进行以下的活动：
1. 挂起一个进程，将这个进程在CPU中的状态（上下文）存储于内存中的某处
2. 在内存中检索下一个进程的上下文并将其在CPU的寄存器中恢复
3. 跳转到程序计数器所指向的位置（即跳转到进程被中断时的代码行），以恢复该进程

因此上下文是指某一时间点CPU寄存器和程序计数器的内容，广义上还包括内存中进程的虚拟地址映射的信息。

上下文切换只能发生在内核态中，上下文切换通常是CPU密集型的。也就是说，它需要相当可观的处理器事件，在每秒几十上百次的切换中，每次切换都需要纳秒级的时间。所以，上下文切换对系统来说意味着消耗大量的CPU时间，事实上，可能是操作系统中时间消耗最大的操作。

Linux相比与其他操作系统有很多优点，其中有一项就是，其上下文切换和模式切换的时间消耗非常少。

### 1.4.2 context_switch流程

context\_switch函数完成了进程上下文的切换，其定义在kernel/sched/core.c。
```c
/*
 * context_switch - switch to the new MM and the new
 * thread's register state.
 */
static inline void
context_switch(struct rq *rq, struct task_struct *prev,
           struct task_struct *next)
{
    struct mm_struct *mm, *oldmm;

    prepare_task_switch(rq, prev, next);

    mm = next->mm;
    oldmm = prev->active_mm;
    /*
     * For paravirt, this is coupled with an exit in switch_to to
     * combine the page table reload and the switch backend into
     * one hypercall.
     */
    arch_start_context_switch(prev);

    if (!mm) {
        next->active_mm = oldmm;
        atomic_inc(&oldmm->mm_count);
        enter_lazy_tlb(oldmm, next);
    } else
        switch_mm(oldmm, mm, next);

    if (!prev->mm) {
        prev->active_mm = NULL;
        rq->prev_mm = oldmm;
    }
    
    /*
     * Since the runqueue lock will be released by the next
     * task (which is an invalid locking op but in the case
     * of the scheduler it's an obvious special-case), so we
     * do an early lockdep release here:
     */
#ifndef __ARCH_WANT_UNLOCKED_CTXSW
    spin_release(&rq->lock.dep_map, 1, _THIS_IP_);
#endif

    context_tracking_task_switch(prev, next);
    /* Here we just switch the register state and the stack. */
    switch_to(prev, next, prev);

    barrier();
    /*
     * this_rq must be evaluated again because prev may have moved
     * CPUs since it called schedule(), thus the 'rq' on its stack
     * frame will be invalid.
     */
    finish_task_switch(this_rq(), prev);
}
```
context\_switch()函数建立next进程的地址空间。进程描述符的active\_mm字段指向进程所使用的内存描述符，而mm字段指向进程所拥有的用户空间内存描述符。对于一般的进程，这两个字段有相同的地址，但是，内核线程没有它自己的地址空间而且它的mm字段总是被设置为NULL；active\_mm成员被初始化为前一个运行进程的actvie\_mm值，如果当前内核线程被调度之前运行的也是另外一个内核线程的时候，那么其mm和active\_mm都是NULL。

context\_switch()函数保证：如果next是一个内核线程，它使用prev所使用的地址空间。
它主要执行如下操作：
- 调用switch\_mm()，把虚拟内存从一个进程映射切换到新进程中
- 调用switch\_to()，从上一个进程的处理器状态切换到新进程的处理状态。这些包括恢复栈信息和寄存器信息

由于不同体系结构下地址映射的机制有所不同，而寄存器等信息也是依赖于体系结构的，因此，switch\_mm和switch\_to两个函数均是体系结构相关的。

### 1.4.3 switch_mm切换进程虚拟地址空间

switch\_mm主要完成了进程prev和next虚拟地址空间的映射，由于内核虚拟地址空间是不许切换的，因此切换的主要是用户态的虚拟地址空间。

下面我们看看x86体系结构的实现，定义在arch/x86/include/asm/mmu_context.h中：
```c
static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next,
                 struct task_struct *tsk)
{
    unsigned cpu = smp_processor_id();

    if (likely(prev != next)) {
#ifdef CONFIG_SMP
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        this_cpu_write(cpu_tlbstate.active_mm, next);
#endif
        cpumask_set_cpu(cpu, mm_cpumask(next));

        /*
         * Re-load page tables.
         *
         * This logic has an ordering constraint:
         *
         *  CPU 0: Write to a PTE for 'next'
         *  CPU 0: load bit 1 in mm_cpumask.  if nonzero, send IPI.
         *  CPU 1: set bit 1 in next's mm_cpumask
         *  CPU 1: load from the PTE that CPU 0 writes (implicit)
         *
         * We need to prevent an outcome in which CPU 1 observes
         * the new PTE value and CPU 0 observes bit 1 clear in
         * mm_cpumask.  (If that occurs, then the IPI will never
         * be sent, and CPU 0's TLB will contain a stale entry.)
         *
         * The bad outcome can occur if either CPU's load is
         * reordered before that CPU's store, so both CPUs must
         * execute full barriers to prevent this from happening.
         *
         * Thus, switch_mm needs a full barrier between the
         * store to mm_cpumask and any operation that could load
         * from next->pgd.  TLB fills are special and can happen
         * due to instruction fetches or for no reason at all,
         * and neither LOCK nor MFENCE orders them.
         * Fortunately, load_cr3() is serializing and gives the
         * ordering guarantee we need.
         *
         */
        load_cr3(next->pgd);

        /* Stop flush ipis for the previous mm */
        cpumask_clear_cpu(cpu, mm_cpumask(prev));

        /* Load the LDT, if the LDT is different: */
        if (unlikely(prev->context.ldt != next->context.ldt))
            load_LDT_nolock(&next->context);
    }
#ifdef CONFIG_SMP
      else {
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        BUG_ON(this_cpu_read(cpu_tlbstate.active_mm) != next);

        if (!cpumask_test_cpu(cpu, mm_cpumask(next))) {
            /*
             * On established mms, the mm_cpumask is only changed
             * from irq context, from ptep_clear_flush() while in
             * lazy tlb mode, and here. Irqs are blocked during
             * schedule, protecting us from simultaneous changes.
             */
            cpumask_set_cpu(cpu, mm_cpumask(next));

            /*
             * We were in lazy tlb mode and leave_mm disabled
             * tlb flush IPI delivery. We must reload CR3
             * to make sure to use no freed page tables.
             *
             * As above, load_cr3() is serializing and orders TLB
             * fills with respect to the mm_cpumask write.
             */
            load_cr3(next->pgd);
            load_LDT_nolock(&next->context);
        }
    }
#endif
}
```
其主要工作就是切换进程的CR3。

控制寄存器（CR0 ~ CR3）用于控制和确定处理器的操作模式以及当前执行任务的特性：
-  CR0：含有控制处理器操作模式和状态的系统控制标志
-  CR1：保留不用
-  CR2：导致Page Falt的线性地址
-  CR3：页目录表物理内存基地址，因此该寄存器也被称为PDBR（Page-Directory Base Address Register）

### 1.4.4 switch_to切换进程堆栈和寄存器

执行环境的切换是在switch\_to()中完成的，switch\_to()完成最终的进程切换，它保存原进程的所有寄存器信息，恢复新进程的所有寄存器信息，并执行新的进程。

调度过程可能选择了一个新的进程，而清理工作则是针对此前的活动进程。
请注意，这不是发起上下文切换的那个进程，而是系统中随机的某个其他进程，内核必须想办法使得进程能够与context\_switch例程通信，这就可以通过switch\_to宏实现。因此switch\_to()函数通过3个参数提供2个变量。

在新进程被选中时，低层的进程切换例程必须将此前执行的进程提供给context\_switch例程，由于控制流会回到该函数的中间，这无法用普通的函数返回值做到，因此提供了3个参数的宏。
```c
/*
 * Saving eflags is important. It switches not only IOPL between tasks,
 * it also protects other tasks from NT leaking through sysenter etc.
 */
#define switch_to(prev, next, last)                 \
do {                                    \
    /*                              \
     * Context-switching clobbers all registers, so we clobber  \
     * them explicitly, via unused output variables.        \
     * (EAX and EBP is not listed because EBP is saved/restored \
     * explicitly for wchan access and EAX is the return value of   \
     * __switch_to())                       \
     */                             \
    unsigned long ebx, ecx, edx, esi, edi;              \
                                    \
    asm volatile("pushfl\n\t"       /* save    flags */ \
             "pushl %%ebp\n\t"      /* save    EBP   */ \
             "movl %%esp,%[prev_sp]\n\t"    /* save    ESP   */ \
             "movl %[next_sp],%%esp\n\t"    /* restore ESP   */ \
             "movl $1f,%[prev_ip]\n\t"  /* save    EIP   */ \
             "pushl %[next_ip]\n\t" /* restore EIP   */ \
             __switch_canary                    \
             "jmp __switch_to\n"    /* regparm call  */ \
             "1:\t"                     \
             "popl %%ebp\n\t"       /* restore EBP   */ \
             "popfl\n"          /* restore flags */ \
                                    \
             /* output parameters */                \
             : [prev_sp] "=m" (prev->thread.sp),        \
               [prev_ip] "=m" (prev->thread.ip),        \
               "=a" (last),                 \
                                    \
               /* clobbered output registers: */        \
               "=b" (ebx), "=c" (ecx), "=d" (edx),      \
               "=S" (esi), "=D" (edi)               \
                                        \
               __switch_canary_oparam               \
                                    \
               /* input parameters: */              \
             : [next_sp]  "m" (next->thread.sp),        \
               [next_ip]  "m" (next->thread.ip),        \
                                        \
               /* regparm parameters for __switch_to(): */  \
               [prev]     "a" (prev),               \
               [next]     "d" (next)                \
                                    \
               __switch_canary_iparam               \
                                    \
             : /* reloaded segment registers */         \
            "memory");                  \
} while (0)
```
内核在switch\_to中执行如下操作：
1. 进程切换，即esp的切换，由于从esp可以找到进程的描述符
2. 硬件上下文切换，设置eip寄存器的值，并jmp到\_\_switch\_to函数
3. 退栈的切换，即ebp的切换，ebp是栈底指针，它确定了当前用户空间属于哪个进程

## 1.5 need_resched，TIF_NEED_RESCHED标识与用户抢占

内核在即将返回用户空间时检查进程是否需要重新调度，如果设置了，就会发生调度，这被称为用户抢占，
因此内核在thread\_info的flag中设置了一个标识来标志进程是否需要重新调度。

并提供了一些设置和检测的函数：

函数 | 描述
---|---
set\_tsk\_need\_resched | 设置指定进程中的need_resched标志
clear\_tsk\_need\_resched | 清除指定进程中的need_resched标志
test\_tsk\_need\_resched | 检查指定进程need_resched标志

而我们内核中调度时常用的need\_resched()函数检查进程是否需要被重新调度，其实就是通过test\_tsk\_need\_resched实现的，其定义如下所示：
```c
static inline int need_resched(void)
{
    return unlikely(test_thread_flag(TIF_NEED_RESCHED));
}
```

### 1.5.1 用户抢占和内核抢占

当内核即将返回用户空间时，内核会检查need\_resched是否设置，如果设置，则调用schedule()，此时，发生用户抢占。

一般来说，用户抢占发生以下情况：
- 从系统调用返回用户空间
- 从中断（异常）处理程序返回用户空间

当kernel（系统调用或者中断处理）返回用户态时，系统可以安全的执行当前的任务，或者切换到另外一个任务。

当中断处理例程或者系统调用完成后，kernel返回用户态时，need\_resched标志的值会被检查，假如它为1，调度器会选择一个新的任务并执行。中断和系统调用的返回路径（return path）实现在entry.S中（entry.S不仅包括kernel entry code，也包括kernel exit code）。

抢占时伴随着schedule()的执行，因此内核提供了一个TIF\_NEED\_RESCHED标志来表明是否需要schedule()调度一次。

根据抢占繁盛的时机分为用户抢占和内核抢占：
- 用户抢占：发生在内核即将返回到用户空间的时候
- 内核抢占：发生在陷入内核空间的时候

抢占类型 | 抢占时机 | 描述
---|---|---
用户抢占 | 内核在即将返回用户空间时检查进程是否设置了TIF\_NEED\_RESCHED标志，</br>如果设置了，就会发生用户抢占。| 从系统调用或中断处理程序返回用户空间的时候
内核抢占 | Linux 2.6的内核增加了内核抢占，同是为了某些地方不被抢占，又添加了自旋锁。</br>在进程的thread\_info结构中添加了preempt\_count该数值为0，</br>当进程使用一个自旋锁时就+1，释放一个自旋锁时就减1。</br>为0时表示内核可以抢占。| 1.从中断处理程序返回内核空间时，内核会检查preempt\_count和TIF\_NEED\_RESCHED标志，如果进程设置了TIF\_NEED\_RESCHED标志，并且preempt\_count为0，发生内核抢占。</br>2.当进程所有的自旋锁都释放了，释放程序会检查TIF\_NEED\_RESCHED标志，如果设置了就会调用schedule。</br>3.显示调用schedule时<br>4.内核中的进程被阻塞的时候

> 抢占内核虽然牺牲了上下文切换的开销，但获得了更大的吞吐量和响应时间。

# 2. 总结

## 2.1 schedule调度流程

schedule就是主调度器函数，在内核中的许多地方，如果将CPU分配给与当前活动进程不同的另一个进程，都会直接调用主调度器函数schedule，该函数定义在kernel/sched/core.c。如下所示：
```c
 do {
        preempt_disable();						/*  关闭内核抢占  */
        __schedule(false);						/*  完成调度  */
        sched_preempt_enable_no_resched();	    /*  开启内核抢占  */
    } while (need_resched());	/* 如果该进程被其他进程设置了TIF_NEED_RESCHED标志，则函数重新执行进行调度 */
```
该函数完成如下工作：
1. 确定当前就绪队列，并在保存一个指向当前（仍然）活动进程的task\_struct指针
2. 检查死锁，关闭内核抢占后调用\_\_schedule完成内核调度
3. 恢复内核抢占，然后检查当前进程是否设置了重新调度标志TIL\_NEED\_RESCHED，如果该进程被其他进程设置了TIF\_NEED\_RESCHED标志，则函数重新执行进行调度

## 2.2 __schedule如何完成内核抢占

1. 完成一些必要的检查，并设置进程状态，处理进程所在的就绪队列
2. 调度全局的pick\_next\_task选择抢占的进程
    1. 如果当前CPU上所有的进程都是CFS调度的普通非实时进程，则直接用CFS调度，如果无程序可调度则调度idle进程
    2. 否则从优先级最高的调度类sched\_class\_highest（目前是stop\_sched\_class）开始依次遍历所有调度器类的pick\_next\_task函数，选择最优的哪个进程执行
3. context\_switch完成进程上下文切换
    1. 调用switch\_mm()，把虚拟内存从上一个进程映射切换到新进程中
    2. 调用switch\_to()，从上一个进程的处理器状态切换到新进程的处理器状态。这些包括恢复栈信息和寄存器信息。

## 2.3 调度的内核抢占和用户抢占

内核在完成调度的过程中总是先关闭内核抢占，等待内核完成调度的工作后，在把内核抢占开启，如果在内核完成调度器过程红，这时候如果发生了内核抢占，我们的调度会被中断，而调度却还没完成，这样会丢失我们调度的信息。

而同样我们可以看到，在调度完成后，内核回去判断need\_resched条件，如果这个时候为真，内核会重新调度进程一次，此次调度由于发生在内核态，因此，仍然是一次内核抢占。

need\_resched条件其实是判断need\_resched标识TIF\_NEED\_RESCHED的值，内核在thread\_info的flag中设置了一个标识来标识进程是否需要重新调度，即重新调度need\_resched标识TIF\_NEED\_RESCHED，内核在即将返回用户空间时会检查标识TIF\_NEED\_RESCHED标志进程是否需要重新调度，如果设置了，就会发生调度，这被称为用户抢占。

而内核抢占时通过自旋锁preempt\_count实现的，同样当内核可以进行内核抢占的时候（比如，中断处理程序返回内核空间、内核进程被阻塞的时候），内核会检查preempt\_count和TIF\_NEED\_RESCHED标志，如果设置了TIF\_NEED\_RESCHED标志，并且preempt\_count为0，发生内核抢占。