***Linux CFS调度器之队列操作***

- 1 CFS进程入队和出队
- 2 enqueue_task_fair入队操作
    - 2.1 enque_task_fair函数
    - 2.2 enque_task_fair函数的实现
    - 2.3 for_each_sched_entity
    - 2.4 enqueue_entity插入进程
    - 2.5 place_entity处理睡眠进程er
    - 2.6 __enqueue_entity完成红黑树的插入
- 3 dequeue_task_fair出队操作
    - 3.1 dequeue_task_fair函数
    - 3.2 dequeue_entity将调度实体出队
    - 3.3 __dequeue_entity完成真正的出队操作

# 1. CFS进程入队和出队


完全公平调度器CFS中有两个方法可以用来增加/删除队列的成员（进程）：
- 增加/入队：enqueue\_task_fair
- 删除/出队：dequeue\_task_fair

# 2. enqueue_task_fair入队操作

## 2.1 enque_task_fair函数

向就绪队列中放入新进程的工作，由enqueue\_task\_fair函数完成，该函数定义在kernel/sched/fair.c中，其函数原型如下：
```c
static void enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
```
该函数将task\_struct *p所指向的进程插入到rq所在的就绪队列中，除了指向所述的就绪队列rq和task\_struct的指针外，该函数还有另外一个参数wakeup。这使得可以指定入队的进程是否最近才被唤醒并转换为运行状态（此时需指定wakeup=1），还是此前就是可运行的（那么wakeup=0）。

enqueue\_task\_fair的执行流程如下：
- 如果struct sched\_entity的on\_rq成员判断进程已经在就绪队列上，则无事可做。
- 否则，具体的工作委托给enqueue\_entity完成，其中内核会借机用update\_curr更新统计量，在queue\_entity内部如果需要会调用\_\_enqueue\_entity将进程插入到CFS红黑树中合适的结点。

## 2.2 enque_task_fair函数的实现

```c
/*
 * The enqueue_task method is called before nr_running is
 * increased. Here we update the fair scheduling stats and
 * then put the task into the rbtree:
 */
static void
enqueue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;

    for_each_sched_entity(se) {
        if (se->on_rq)
            break;
        cfs_rq = cfs_rq_of(se);
        enqueue_entity(cfs_rq, se, flags);

        /*
         * end evaluation on encountering a throttled cfs_rq
         *
         * note: in the case of encountering a throttled cfs_rq we will
         * post the final h_nr_running increment below.
        */
        if (cfs_rq_throttled(cfs_rq))
            break;
        cfs_rq->h_nr_running++;

        flags = ENQUEUE_WAKEUP;
    }

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        cfs_rq->h_nr_running++;

        if (cfs_rq_throttled(cfs_rq))
            break;

        update_load_avg(se, 1);
        update_cfs_shares(cfs_rq);
    }

    if (!se)
        add_nr_running(rq, 1);

    hrtick_update(rq);
}
```

## 2.3 for_each_sched_entity

首先，内核查找到待添加进程p所在的调度实体信息，然后通过for\_each\_sched\_entity循环所有调度实体：
```c
//  enqueue_task_fair函数
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;

    for_each_sched_entity(se)
    {
    
        /*  ......  */
    
    }
```
但是有个疑问是，进程p所在的调度实体就一个为什么要循环遍历呢？这是因为为了支持组调度。组调度下调度实体是有层次结构的，我们将进程加入的时候，同时要更新其父调度实体的信息，而非组调度情况下，就不需要调度实体的层次结构。

Linux对组调度的支持可以通过CONFIG\_FAIR\_GROUP\_SCHED来启用，在启用和不启用的条件下，内核对很多函数的实现也不会因条件而异，这点对for\_each\_sched\_entity函数尤为明显。
```c
#ifdef CONFIG_FAIR_GROUP_SCHED

/* An entity is a task if it doesn't "own" a runqueue */
#define entity_is_task(se)      (!se->my_q)

/* Walk up scheduling entities hierarchy */
#define for_each_sched_entity(se) \
		for (; se; se = se->parent)

#else   /* !CONFIG_FAIR_GROUP_SCHED */

#define entity_is_task(se)      1

#define for_each_sched_entity(se) \
                for (; se; se = NULL)
#endif
```

- 如果通过struct sched\_entity的on\_rq成员判断进程已经在就绪队列上，则无事可做。
- 否则，具体的工作委托给enqueue\_entity完成，其中内核会借机用update\_curr更新统计量。

```c
//  enqueue_task_fair函数
{
        /*  如果当前进程已经在就绪队列上  */
        if (se->on_rq)
            break;

        /*  获取到当前进程所在的cfs_rq就绪队列  */
        cfs_rq = cfs_rq_of(se);
        
        /*  内核委托enqueue_entity完成真正的插入工作  */
        enqueue_entity(cfs_rq, se, flags);
}
```

## 2.4 enqueue_entity插入进程

enqueue\_entity完成了进程真正的入队操作，其流程如下所示：
- 更新一些统计量，update\_curr、update\_cfs\_shares等
- 如果进程此前是睡眠状态，则调用place\_entity函数，该函数会调整进程的vruntime
- 最后如果进程最近在运行，其vruntime仍然有效，那么直接调用\_\_enqueue\_entity加入红黑树

首先，如果进程最近正在运行，其vruntime仍然有效，那么（除非它当前正在执行中）它可以直接调用\_\_enqueue\_entity插入到红黑树，该函数处理一些红黑树的机制，这可以依靠内核的标准实现。
```c
static void
enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    /*
     * Update the normalized vruntime before updating min_vruntime
     * through calling update_curr().
     *
     * 如果当前进程之前已经是可运行状态不是被唤醒的那么其虚拟运行时间要增加
     */
    if (!(flags & ENQUEUE_WAKEUP) || (flags & ENQUEUE_WAKING))
        se->vruntime += cfs_rq->min_vruntime;

    /*
     * Update run-time statistics of the 'current'.
     * 更新进程的统计量信息
     */
    update_curr(cfs_rq);
    enqueue_entity_load_avg(cfs_rq, se);
    account_entity_enqueue(cfs_rq, se);
    update_cfs_shares(cfs_rq);

    /*  如果当前进行之前在睡眠刚被唤醒  */
    if (flags & ENQUEUE_WAKEUP)
    {
        /*  调整进程的虚拟运行时间  */
        place_entity(cfs_rq, se, 0);
        if (schedstat_enabled())
            enqueue_sleeper(cfs_rq, se);
    }

    check_schedstat_required();
    if (schedstat_enabled()) {
        update_stats_enqueue(cfs_rq, se);
        check_spread(cfs_rq, se);
    }

    /*  将进程插入到红黑树中  */
    if (se != cfs_rq->curr)
        __enqueue_entity(cfs_rq, se);
    se->on_rq = 1;

    if (cfs_rq->nr_running == 1) {
        list_add_leaf_cfs_rq(cfs_rq);
        check_enqueue_throttle(cfs_rq);
    }
}
```

## 2.5 place_entity处理睡眠进程er

如果，进程此前在睡眠，那么则调用place\_entity处理器vruntime。

设想一下，如果休眠进程的vruntime保持不变，而其他运行进程的vruntime一直在推进，那么等到休眠进程终于唤醒的时候，它的vruntime比别人小很多，会使它获得长时间抢占CPu的优势，其他进程就要饿死了。
这显然是另一种形式的不公平，因此CFS是这样做的：</br>在优先进程被唤醒时将重新设置vruntime，以min\_vruntime为基础，给予一定的补偿，但是不能补偿太多。这个重新设置其vruntime的工作就是通过pace\_entity来完成的，另外，新进程创建完成后，也是通过pace\_entity完成其vruntime的设置。pace\_entity通过其第三个参数inital来标识新进程创建和休眠进程苏醒的两种不同情形。

place\_entity函数定义在kernel/sched/fair.c，首先会调整进程的vruntime：
```c
static void
place_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int initial)
{
    u64 vruntime = cfs_rq->min_vruntime;

    /*
     * The 'current' period is already promised to the current tasks,
     * however the extra weight of the new task will slow them down a
     * little, place the new task so that it fits in the slot that
     * stays open at the end.
     *
     * 如果是新进程第一次要入队, 那么就要初始化它的vruntime
     * 一般就把cfsq的vruntime给它就可以
     * 但是如果当前运行的所有进程被承诺了一个运行周期
     * 那么则将新进程的vruntime后推一个他自己的slice
     * 实际上新进程入队时要重新计算运行队列的总权值
     * 总权值显然是增加了，但是所有进程总的运行时期并不一定随之增加
     * 则每个进程的承诺时间相当于减小了，就是减慢了进程们的虚拟时钟步伐。 
     */
    /*  initial标识了该进程是新进程  */
    if (initial && sched_feat(START_DEBIT))
        vruntime += sched_vslice(cfs_rq, se);

    /* sleeps up to a single latency don't count. 
     * 休眠进程  */
    if (!initial)
    {
        /*  一个调度周期  */
        unsigned long thresh = sysctl_sched_latency;

        /*
         * Halve their sleep time's effect, to allow
         * for a gentler effect of sleepers:
         */
        /*  若设了GENTLE_FAIR_SLEEPERS  */
        if (sched_feat(GENTLE_FAIR_SLEEPERS))
            thresh >>= 1;   /*  补偿减为调度周期的一半  */

        vruntime -= thresh;
    }

    /* ensure we never gain time by being placed backwards.
     * 如果是唤醒已经存在的进程，则单调附值
     */
    se->vruntime = max_vruntime(se->vruntime, vruntime);
}
```
我们可以看到enqueue\_task\_fair调用pace\_entity传递的initial参数为0。
```c
place_entity(cfs_rq, se, 0);
```
所以会执行`if (!inital)`后的语句。因为进程睡眠后，vruntime就不会增加了，当它醒来后不知道过了多长事件，可能vruntime已经比min\_vruntime小很多，如果只是简单的将其插入就绪队列中，它将拼命追赶min\_vruntime，因为它总是在红黑树的最左面。

如果这样，它就会占用大量的CPU事件，导致红黑树右边的进程被饿死。但是我们又必须及时响应醒来的进程，因为它们可能有一些工作需要立刻处理，所以系统采取了一种折中的办法，将当前`cfs_rq->min_vruntime`时间减去`sysctl_sched_latency`赋给vruntime，这时它被插入到就绪队列的最左边。这样刚唤醒的进程在当前执行进程时间耗尽时就会被调度上处理器执行。当然如果进程没有睡眠那么多时间，我们只需保留原来的时间`vruntime = max_vruntime(se->vruntime, vruntime)`。这有什么好处呢，我觉得它可以将所有唤醒的进程排个队，睡眠越久的越快得到响应。

对于新进程创建时inital=1时，它会执行`vruntime += sched\_vslice(cfs_rq, se);`这句，而这里的vruntime就是当前CFS就绪队列的min\_vruntime，新进程应该在很快被调度，这样减少系统的相应时间。我们已经知道当前进程的vruntime越小，它在红黑树中就会越靠左，就会被很快调度到处理器上执行。但是，Linux内核需要根据新加入的进程的权重决策一下应该合适调度该进程，而不能任意进程都来抢占当前队列中靠左的进程，因此必须保证就绪队列中所有进程尽量得到它们应得的时间相应，sched\_vslice函数就将其负荷权重转换为等价的vruntime。

place\_entity函数就是根据inital的值来区分两种情况，一般来说只有在新进程被加到系统中，才会首次设置该参数，但是这里的情况并非如此：由于内核已经承诺在当前的延迟周期内使所有进程都至少执行一次，队列的min\_vruntime用作基准虚拟时间，通过减去sys\_sched\_latency，则可以确保新唤醒的进程只有在当前延迟周期结束后才能运行。

但是如果进程在睡眠的过程中积累了比较大的不公平值（即se->vruntime值比较大），则内核必须考虑这一点。
如果se->vruntime比先前的差值更大，则将其作为进程的vruntime，这会导致高进程在红黑树中靠左的位置，具有较小vruntime值，从而进程可以更早调度执行。

## 2.6 __enqueue_entity完成红黑树的插入

如果进程最近在运行，其vruntime是有效的，那么它可以直接通过\_\_enqueue\_entity加入到红黑树
```c
//  enqueue_entity函数解析
    /*  将进程插入到红黑树中  */
    if (se != cfs_rq->curr)
        __enqueue_entity(cfs_rq, se);
    se->on_rq = 1;
```
\_\_enqueue\_entity函数定义在kernel/sched/fair.c中，其实就是一个机械性地红黑树插入操作：
```c
/*
 * Enqueue an entity into the rb-tree:
 */
static void __enqueue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    struct rb_node **link = &cfs_rq->tasks_timeline.rb_node;
    struct rb_node *parent = NULL;
    struct sched_entity *entry;
    int leftmost = 1;

    /*
     * Find the right place in the rbtree:
     * 从红黑树中找到se所应该在的位置
     * 同时leftmost标识其位置是不是最左结点
     * 如果在查找结点的过程中向右走了, 则置leftmost为0
     * 否则说明一直再相左走, 最终将走到最左节点, 此时leftmost恒为1
     */
    while (*link) {
        parent = *link;
        entry = rb_entry(parent, struct sched_entity, run_node);
        /*
         * We dont care about collisions. Nodes with
         * the same key stay together.
         * 以se->vruntime值为键值进行红黑树结点的比较
         */
        if (entity_before(se, entry)) {
            link = &parent->rb_left;
        } else {
            link = &parent->rb_right;
            leftmost = 0;
        }
    }
    /*
     * Maintain a cache of leftmost tree entries (it is frequently
     * used):
     * 如果leftmost为1, 说明se是红黑树当前的最左结点, 即vruntime最小
     * 那么把这个节点保存在cfs就绪队列的rb_leftmost域中
     */
    if (leftmost)
        cfs_rq->rb_leftmost = &se->run_node;

	/*  将新进程的节点加入到红黑树中  */
    rb_link_node(&se->run_node, parent, link);
    /*  为新插入的结点进行着色  */
    rb_insert_color(&se->run_node, &cfs_rq->tasks_timeline);
}
```

# 3. dequeue_task_fair出队操作

dequeue\_task\_fair函数在完成睡眠等情况下调度，将任务从就绪队列中移出，其执行的过程正好跟enqueue\_task\_fair的思路相同，只是操作刚好相反。

dequeue\_task\_fair的执行流程如下：
- 如果通过struct sched\_entity的on\_rq成员判断进程不在就绪队列上，则无事可做。
- 否则，具体工作委托给dequeue\_entity完成，其中内核会借机用update\_curr更新统计量，在dequeue\_entity内部如果需要调用\_\_dequeue\_entity将进程从CFS红黑树中删除。

## 3.1 dequeue_task_fair函数

dequeue\_task\_fair定义在kernel/sched/fair.c：
```c
/*
 * The dequeue_task method is called before nr_running is
 * decreased. We remove the task from the rbtree and
 * update the fair scheduling stats:
 */
static void dequeue_task_fair(struct rq *rq, struct task_struct *p, int flags)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se;
    int task_sleep = flags & DEQUEUE_SLEEP;

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        dequeue_entity(cfs_rq, se, flags);

        /*
         * end evaluation on encountering a throttled cfs_rq
         *
         * note: in the case of encountering a throttled cfs_rq we will
         * post the final h_nr_running decrement below.
        */
        if (cfs_rq_throttled(cfs_rq))
            break;
        cfs_rq->h_nr_running--;

        /* Don't dequeue parent if it has other entities besides us */
        if (cfs_rq->load.weight) {
            /* Avoid re-evaluating load for this entity: */
            se = parent_entity(se);
            /*
             * Bias pick_next to pick a task from this cfs_rq, as
             * p is sleeping when it is within its sched_slice.
             */
            if (task_sleep && se && !throttled_hierarchy(cfs_rq))
                set_next_buddy(se);
            break;
        }
        flags |= DEQUEUE_SLEEP;
    }

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        cfs_rq->h_nr_running--;

        if (cfs_rq_throttled(cfs_rq))
            break;

        update_cfs_shares(cfs_rq);
        update_entity_load_avg(se, 1);
    }

    if (!se) {
        dec_nr_running(rq);
        update_rq_runnable_avg(rq, 1);
    }
    hrtick_update(rq);
}
```

## 3.2 dequeue_entity将调度实体出队

```c
static void
dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se, int flags)
{
    /*
     * Update run-time statistics of the 'current'.
     */
    update_curr(cfs_rq);
    dequeue_entity_load_avg(cfs_rq, se, flags & DEQUEUE_SLEEP);

    if (schedstat_enabled())
        update_stats_dequeue(cfs_rq, se, flags);

    clear_buddies(cfs_rq, se);

    if (se != cfs_rq->curr)
        __dequeue_entity(cfs_rq, se);
    se->on_rq = 0;
    account_entity_dequeue(cfs_rq, se);

    /*
     * Normalize the entity after updating the min_vruntime because the
     * update can refer to the ->curr item and we need to reflect this
     * movement in our normalized position.
     */
    if (!(flags & DEQUEUE_SLEEP))
        se->vruntime -= cfs_rq->min_vruntime;

    /* return excess runtime on last dequeue */
    return_cfs_rq_runtime(cfs_rq);

    update_min_vruntime(cfs_rq);
    update_cfs_shares(cfs_rq);
}
```

## 3.3 __dequeue_entity完成真正的出队操作

```c
static void __dequeue_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    if (cfs_rq->rb_leftmost == &se->run_node) {
        struct rb_node *next_node;

        next_node = rb_next(&se->run_node);
        cfs_rq->rb_leftmost = next_node;
    }

    rb_erase(&se->run_node, &cfs_rq->tasks_timeline);
}
```