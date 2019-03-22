***Linux进程调度CFS调度器***

- 1 CFS完全公平调度器
    - 1.1 CFS调度器类
    - 1.2 CFS的就绪队列

# 1. CFS完全公平调度器

## 1.1 CFS调度器类

CFS完全公平调度器的调度类叫fair\_sched\_class，其定义在kernel/sched/fair.c，它是我们熟知的strcut sched\_class调度器类类型，将CFS调度器与一些特定的函数关联起来。

```c
/*
 * All the scheduling class methods:
 */
const struct sched_class fair_sched_class = {
        /* 下个优先级的调度类, 所有的调度类通过next链接在一个链表中 */
        .next                   = &idle_sched_class,  
        .enqueue_task           = enqueue_task_fair,
        .dequeue_task           = dequeue_task_fair,
        .yield_task             = yield_task_fair,
        .yield_to_task          = yield_to_task_fair,

        .check_preempt_curr     = check_preempt_wakeup,

        .pick_next_task         = pick_next_task_fair,
        .put_prev_task          = put_prev_task_fair,

#ifdef CONFIG_SMP
        .select_task_rq         = select_task_rq_fair,
        .migrate_task_rq        = migrate_task_rq_fair,

        .rq_online              = rq_online_fair,
        .rq_offline             = rq_offline_fair,

        .task_waking            = task_waking_fair,
        .task_dead              = task_dead_fair,
        .set_cpus_allowed       = set_cpus_allowed_common,
#endif

        .set_curr_task          = set_curr_task_fair,
        .task_tick              = task_tick_fair,
        .task_fork              = task_fork_fair,

        .prio_changed           = prio_changed_fair,
        .switched_from          = switched_from_fair,
        .switched_to            = switched_to_fair,

        .get_rr_interval        = get_rr_interval_fair,

        .update_curr            = update_curr_fair,

#ifdef CONFIG_FAIR_GROUP_SCHED
        .task_move_group        = task_move_group_fair,
#endif
};
```

成员 | 描述
---|:---
enqueue\_task | 向就绪队列中添加一个进程，某个任务进入可运行状态时，该函数将得到调用。</br>它将调度实体放入红黑树中个，并对nr\_running变量加1。
dequeue\_task | 将一个进程从就绪队列中删除，当某个任务退出可运行状态时调用该函数。</br>它将从红黑树中去掉对应的调度实体，并从nr\_running变量中减1。
yield\_task | 在进程想要资源放弃对处理器的控制权时，可使用在sched\_yield系统调用，</br>会调用内核API yield\_task完成此工作。compat\_yield sysctl关闭的情况下，</br>该函数实际上执行先出队后入队；在这种情况下，它将调度实体放在红黑树的最右端。
check\_preempt\_curr | 该函数检查当前运行的任务释放被抢占。在实际抢占正在运行的任务之前，</br>CFS调度模块将执行公平性测试。这将驱动唤醒式（wakeup）抢占。
pick\_next\_task | 该函数选择接下来要运行的最合适的进程
put\_prev\_task | 用另一个进程代替当前运行的进程
set\_curr\_task | 当任务修改其调度类或修改其任务组时，将调用这个函数
task\_tick | 在每次激活周期性调度器时，有周期性调度器调用，该函数通常调用自time\_tick函数；</br>它可能引起进程切换。这将驱动运行时（running）抢占。
task\_new | 内核调度程序为调度模块提供了管理新任务启动的机会，</br>用于建立fork系统调用和调度器之间的关联，每次新进程建立完成后，则用new\_task通知调度器，</br>CFS调度模块使用它进行组调度，用于实时任务的模块则不会使用这个函数。

## 1.2 CFS的就绪队列

就绪队列是全局调度器许多操作的起点，但是进程并不是由就绪队列直接管理的，调度管理是各个调度器的职责，因此在各个就绪队列中嵌入了特定调度类的子就绪队列（CFS调度类的就绪队列struct cfs\_rq，实时调度类的就绪队列struct rq\_rq，Deadline调度类的就绪队列struct dl\_rq）。

```c
/* CFS-related fields in a runqueue */
/* CFS调度的运行队列，每个CPU的rq会包含一个cfs_rq，而每个组调度的sched_entity也会有自己的一个cfs_rq队列 */
struct cfs_rq {
	/* CFS运行队列中所有进程的总负载 */
    struct load_weight load;
	/*
     *  nr_running: cfs_rq中调度实体数量
     *  h_nr_running: 只对进程组有效，其下所有进程组中cfs_rq的nr_running之和
	*/
    unsigned int nr_running, h_nr_running;

    u64 exec_clock;
    
	/*
     * 当前CFS队列上最小运行时间，单调递增
     * 两种情况下更新该值: 
     * 1、更新当前运行任务的累计运行时间时
     * 2、当任务从队列删除去，如任务睡眠或退出，这时候会查看剩下的任务的vruntime是否大于min_vruntime，如果是则更新该值。
     */

    u64 min_vruntime;
#ifndef CONFIG_64BIT
    u64 min_vruntime_copy;
#endif
	/* 该红黑树的root */
    struct rb_root tasks_timeline;
     /* 下一个调度结点(红黑树最左边结点，最左边结点就是下个调度实体) */
    struct rb_node *rb_leftmost;

    /*
     * 'curr' points to currently running entity on this cfs_rq.
     * It is set to NULL otherwise (i.e when none are currently running).
	 * curr: 当前正在运行的sched_entity（对于组虽然它不会在cpu上运行，但是当它的下层有一个task在cpu上运行，那么它所在的cfs_rq就把它当做是该cfs_rq上当前正在运行的sched_entity）
     * next: 表示有些进程急需运行，即使不遵从CFS调度也必须运行它，调度时会检查是否next需要调度，有就调度next
     *
     * skip: 略过进程(不会选择skip指定的进程调度)
     */
    struct sched_entity *curr, *next, *last, *skip;

#ifdef  CONFIG_SCHED_DEBUG
    unsigned int nr_spread_over;
#endif

#ifdef CONFIG_SMP
    /*
     * CFS load tracking
     */
    struct sched_avg avg;
    u64 runnable_load_sum;
    unsigned long runnable_load_avg;
#ifdef CONFIG_FAIR_GROUP_SCHED
    unsigned long tg_load_avg_contrib;
#endif
    atomic_long_t removed_load_avg, removed_util_avg;
#ifndef CONFIG_64BIT
    u64 load_last_update_time_copy;
#endif

#ifdef CONFIG_FAIR_GROUP_SCHED
    /*
     *   h_load = weight * f(tg)
     *
     * Where f(tg) is the recursive weight fraction assigned to
     * this group.
     */
    unsigned long h_load;
    u64 last_h_load_update;
    struct sched_entity *h_load_next;
#endif /* CONFIG_FAIR_GROUP_SCHED */
#endif /* CONFIG_SMP */

#ifdef CONFIG_FAIR_GROUP_SCHED
    /* 所属于的CPU rq */
    struct rq *rq;  /* cpu runqueue to which this cfs_rq is attached */

    /*
     * leaf cfs_rqs are those that hold tasks (lowest schedulable entity in
     * a hierarchy). Non-leaf lrqs hold other higher schedulable entities
     * (like users, containers etc.)
     *
     * leaf_cfs_rq_list ties together list of leaf cfs_rq's in a cpu. This
     * list is used during load balance.
     */
    int on_list;
    struct list_head leaf_cfs_rq_list;
    /* 拥有该CFS运行队列的进程组 */
    struct task_group *tg;  /* group that "owns" this runqueue */

#ifdef CONFIG_CFS_BANDWIDTH
    int runtime_enabled;
    u64 runtime_expires;
    s64 runtime_remaining;

    u64 throttled_clock, throttled_clock_task;
    u64 throttled_clock_task_time;
    int throttled, throttle_count;
    struct list_head throttled_list;
#endif /* CONFIG_CFS_BANDWIDTH */
#endif /* CONFIG_FAIR_GROUP_SCHED */
};
```

成员 | 描述
---|:---
nr\_running | 队列上可运行进程的数目
load | 就绪队列上可运行进程的累计负荷权重
min\_vruntime | 跟踪记录队列上所有进程的最小虚拟运行时间。</br>这个值是实现与就绪队列相关的虚拟时钟的基础。
task\_timeline | 用于在按时间排序的红黑树中管理所有进程
rb\_leftmost | 总是设置为指向红黑树最左边的节点，即需要被调度的进程。</br>该值其实可以通过遍历红黑树获得，但是将这个值保存下来可以减少搜索红黑树花费的平均时间。
curr | 当前正在运行的sched\_entity
next | 表示有些进程急需运行，即使不遵从CFS调度也必须运行它，</br>调度时会检查是否next需要调度，有就调度next。
skip | 忽略进程（不会选择skip指定的进程调度）
