***Linux CFS调度器之pick_next_task_fair选择下一个被调度的进程***

- 1 CFS如何选择最合适的进程
- 2 simple无组调度最简单的pick_next_task_fair
    - 2.1 simple的基本流程
    - 2.2 put_prev_task
        - 2.2.1 全局put_prev_task函数
        - 2.2.2 CFS的put_prev_task_fair函数
    - 2.3 pick_next_entity
        - 2.3.1 pick_next_entity函数完全注释
        - 2.3.2 从left，second和curr进程中选择最优的进程
        - 2.3.3 cfs_rq的last和next指针域
        - 2.3.4 wakeup_preempt_entity检查是否可以被抢占
    - 2.4 set_next_entity
- 3 idle进程的调度
- 4 组调度策略的支持
- 5 与主调度器schedule进行通信
- 6 总结

# 1. CFS如何选择最合适的进程

每个调度器类sched\_class都必须提供一个pick\_next\_task函数用以在就绪队列中选择一个最优的进程来等待调度， 而我们的CFS调度器类中，选择下一个将要运行的进程由pick\_next\_task_fair函数来完成。

之前我们在将主调度器的时候，主调度器schedule函数在进程调度抢占时， 会通过\_\_schedule函数调用全局pick\_next\_task选择一个最优的进程， 在pick\_next\_task中我们就按照优先级依次调用不同调度器类提供的pick\_next\_task方法。

今天就让我们窥探一下完全公平调度器类CFS的pick\_next\_task方法pick\_next\_task\_fair。

pick\_next\_task\_fair方法，选择下一个将要运行的进程pick\_next\_task\_fair执行，其代码执行流程如下。

对于pick\_next\_task\_fair函数的讲解，我们从simple标签开始，这个是常规状态下pick\_next的思路， 简单的来说pick\_next\_task_fair的函数框架如下。

```c
again:
	控制循环来读取最优进程

#ifdef CONFIG_FAIR_GROUP_SCHED
	完成组调度下的pick_next选择
    返回被选择的调度时实体的指针
#endif

simple:
	最基础的pick_next函数
    返回被选择的调度时实体的指针

idle :
	如果系统中没有可运行的进行, 则需要调度idle进程
```
可见我们会发现：
- simple标签：是CFS中最基础的pick_next操作
- idle标签：是使得在没有进程被调度时, 调度idle进程
- again标签：是用于循环的进行pick_next操作
- CONFIG\_FAIR\_GROUP\_SCHED宏：是指定了组调度情况下的pick\_next操作, 如果不支持组调度, 则pick\_next\_task_fair将直接从simple开始执行


# 2. simple无组调度最简单的pick_next_task_fair

在不支持组调度情况下（选项CONFIG\_FAIR\_GROUP\_SCHED），CFS的pick\_next\_task\_fair函数会直接执行simple标签，优选下一个函数，这个流程清晰而且简单，但是已经足够我们理解cfs的pick_next了。

## 2.1 simple的基本流程

pick\_next\_task\_fair函数的simple标签定义在kernel/sched/fair.c，代码如下所示：
```c
simple:
    cfs_rq = &rq->cfs;
#endif
    /*  如果nr_running计数器为0,
     *  当前队列上没有可运行进程,
     *  则需要调度idle进程  */
    if (!cfs_rq->nr_running)
        goto idle;
    /*  将当前进程放入运行队列的合适位置  */
    put_prev_task(rq, prev);

    do
    {
        /*  选出下一个可执行调度实体(进程)  */
        se = pick_next_entity(cfs_rq, NULL);
        /*  把选中的进程从红黑树移除，更新红黑树  
         *  set_next_entity会调用__dequeue_entity完成此工作  */
        set_next_entity(cfs_rq, se);
        /*  group_cfs_rq return NULL when !CONFIG_FAIR_GROUP_SCHED
         *  在非组调度情况下, group_cfs_rq返回了NULL  */
        cfs_rq = group_cfs_rq(se);
    } while (cfs_rq);  /*  在没有配置组调度选项(CONFIG_FAIR_GROUP_SCHED)的情况下.group_cfs_rq()返回NULL.因此,上函数中的循环只会循环一次  */


    /*  获取到调度实体指代的进程信息  */
    p = task_of(se);

    if (hrtick_enabled(rq))
        hrtick_start_fair(rq, p);

    return p;
```
其基本流程如下：

流程 | 描述
---|:---
如果!cfs\_rq->nr\_running则goto idle;	| 如果nr_running计数器为0，当前队列上没有可运行进程，则需要调度idle进程。
put\_prev\_task(rq, prev); | 将当前进程放入运行队列的合适位置，每次当进程被调度后都会使用set\_next\_entity从红黑树中移除，因此被抢占时需要重新加如红黑树中等待被调度。
se = pick\_next\_entity(cfs\_rq, NULL); | 选出下一个可执行调度实体。
set\_next\_entity(cfs\_rq, se); | set\_next\_entity会调用\_\_dequeue\_entity把选中的进程从红黑树移除，并更新红黑树。

## 2.2 put_prev_task

put\_prev\_task用来将前一个进程prev放回到就绪队列中，这是一个全局的函数， 而每个调度器类也必须实现一个自己的put\_prev\_task函数。例如，CFS的实现即put\_prev\_task\_fair。

### 2.2.1 全局put_prev_task函数

由于CFS调度的时候，prev进程不一定是一个CFS调度的进程， 因此必须调用全局的put\_prev\_task来调用prev进程所属调度器类sched\_class的对应put\_prev\_task方法， 完成将进程放回到就绪队列中。

全局的put\_prev\_task函数定义在kernel/sched/sched.h中，代码如下所示：
```c
static inline void put_prev_task(struct rq *rq, struct task_struct *prev)
{
	prev->sched_class->put_prev_task(rq, prev);
}
```

### 2.2.2 CFS的put_prev_task_fair函数

然后我们来分析一下CFS的put\_prev\_task\_fair函数，其定义在kernel/sched/fair.c，在选中了下一个将被调度执行的进程之后，回到pick\_next\_task\_fair中，执行set\_next\_entity。
```c
/*
 * Account for a descheduled task:
 */
static void put_prev_task_fair(struct rq *rq, struct task_struct *prev)
{
    struct sched_entity *se = &prev->se;
    struct cfs_rq *cfs_rq;

    for_each_sched_entity(se) {
        cfs_rq = cfs_rq_of(se);
        put_prev_entity(cfs_rq, se);
    }
}
```
前面我们说到过函数在组策略情况下，调度实体之间存在父子的层次， for\_each\_sched\_entity会从当前调度实体开始，然后循环向其父调度实体进行更新，非组调度情况下则只执行一次。

而put\_prev\_task\_fair函数最终会调用put\_prev\_entity函数将prev的调度时提se放回到就绪队列中等待下次调度。

## 2.3 pick_next_entity

### 2.3.1 pick_next_entity函数完全注释

```c
/*
 * Pick the next process, keeping these things in mind, in this order:
 * 1) keep things fair between processes/task groups
 * 2) pick the "next" process, since someone really wants that to run
 * 3) pick the "last" process, for cache locality
 * 4) do not run the "skip" process, if something else is available
 *
 *  1. 首先要确保任务组之间的公平, 这也是设置组的原因之一
 *  2. 其次, 挑选下一个合适的（优先级比较高的）进程
 *     因为它确实需要马上运行 
 *  3. 如果没有找到条件2中的进程
 *     那么为了保持良好的局部性
 *     则选中上一次执行的进程 
 *  4. 只要有任务存在, 就不要让CPU空转, 
 *     只有在没有进程的情况下才会让CPU运行idle进程
 */
static struct sched_entity *
pick_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *curr)
{
    /*  摘取红黑树最左边的进程  */
    struct sched_entity *left = __pick_first_entity(cfs_rq);
    struct sched_entity *se;

    /*
     * If curr is set we have to see if its left of the leftmost entity
     * still in the tree, provided there was anything in the tree at all.
     *
     * 如果
     * left == NULL  或者
     * curr != NULL curr进程比left进程更优(即curr的虚拟运行时间更小) 
     * 说明curr进程是自动放弃运行权利, 且其比最左进程更优
     * 因此将left指向了curr, 即curr是最优的进程
     */
    if (!left || (curr && entity_before(curr, left)))
    {
        left = curr;
    }

    /* se = left存储了cfs_rq队列中最优的那个进程  
     * 如果进程curr是一个自愿放弃CPU的进程(其比最左进程更优), 则取se = curr
     * 否则进程se就取红黑树中最左的进程left, 它必然是当前就绪队列上最优的
     */
    se = left; /* ideally we run the leftmost entity */

    /*
     * Avoid running the skip buddy, if running something else can
     * be done without getting too unfair.
     *
     * cfs_rq->skip存储了需要调过不参与调度的进程调度实体
     * 如果我们挑选出来的最优调度实体se正好是skip
     * 那么我们需要选择次优的调度实体se来进行调度
     * 由于之前的se = left = (curr before left) curr left
     * 则如果 se == curr == skip, 则选择left = __pick_first_entity进行即可
     * 否则则se == left == skip, 则选择次优的那个调度实体second
     */
    if (cfs_rq->skip == se)
    {
        struct sched_entity *second;

        if (se == curr) /* se == curr == skip选择最左的那个调度实体left  */
        {
            second = __pick_first_entity(cfs_rq);
        }
        else    /*  否则se == left == skip, 选择次优的调度实体second  */
        {
            /*  摘取红黑树上第二左的进程节点  */
            second = __pick_next_entity(se);
            /*  同时与left进程一样, 
             * 如果
             * second == NULL 没有次优的进程  或者
             * curr != NULL curr进程比left进程更优(即curr的虚拟运行时间更小) 
             * 说明curr进程比最second进程更优
             * 因此将second指向了curr, 即curr是最优的进程*/
            if (!second || (curr && entity_before(curr, second)))
                second = curr;
        }

        /* 判断left和second的vruntime的差距是否小于sysctl_sched_wakeup_granularity
         * 即如果second能抢占left */
        if (second && wakeup_preempt_entity(second, left) < 1)
            se = second;
    }

    /*
     * Prefer last buddy, try to return the CPU to a preempted task.
     *
     * 
     */
    if (cfs_rq->last && wakeup_preempt_entity(cfs_rq->last, left) < 1)
        se = cfs_rq->last;

    /*
     * Someone really wants this to run. If it's not unfair, run it.
     */
    if (cfs_rq->next && wakeup_preempt_entity(cfs_rq->next, left) < 1)
        se = cfs_rq->next;

    /* 用过一次任何一个next或者last
     * 都需要清除掉这个指针
     * 以免影响到下次pick next sched_entity  */
    clear_buddies(cfs_rq, se);

    return se;
}
```

### 2.3.2 从left，second和curr进程中选择最优的进程

pick\_next\_entity则从CFS的红黑树中摘取一个最优的进程，这个进程往往在红黑树的最左端，即vruntime最小， 但是也有例外，但是不外乎这几个可能。

调度实体 | 描述
---|:---
`left = __pick_first_entity(cfs_rq)` | 红黑树的最左节点，这个节点拥有当前队列中vruntime最小的特性，即应该优先被调度。
`second = __pick_first_entity(left)` | 红黑树的次左节点，为什么这个节点也可能呢，因为内核支持skip跳过某个进程的抢占权力的，</br>如果left被标记为skip（由cfs_rq->skip域指定），那么可能就需要找到次优的那个进程。
curr结点 | curr节点的vruntime可能比left和second更小，但是由于它正在运行，</br>因此它不在红黑树中（进程抢占物理机的时候对应节点同时会从红黑树中删除），但是如果其vruntime足够小，</br>意味着CFS调度器应该尽可能的补偿curr进程，让它再次被调度。

其中`__pick_first_entity`会返回cfs\_rq红黑树中的最左节点rb\_leftmost所属的调度实体信息，该函数定义在kernel/sched/fair.c。
而`__pick_next_entity(se)`函数则返回se在红黑树中中序遍历的下一个节点信息，该函数定义在kernel/sched/fair.c，获取下一个节点的工作可以通过内核红黑树的标准操作rb\_next完成。

### 2.3.3 cfs_rq的last和next指针域

在pick\_next\_entity的最后，要把红黑树最左下角的进程和另外两个进程（即next和last）做比较， next是抢占失败的进程，而last则是抢占成功后被抢占的进程，这三个进程到底哪一个是最优的next进程呢？

Linux CFS实现的判决条件是：
1. 尽可能满足需要刚被唤醒的进程抢占其它进程的需求
2. 尽可能减少以上这种抢占带来的缓存刷新的影响

cfs\_rq的last和next指针，last表示最后一个执行wakeup的sched\_entity，next表示最后一个被wakeup的sched\_entity。它们在进程wakeup的时候会赋值，在pick新sched\_entity的时候，会优先选择这些last或者next指针的sched\_entity，有利于提高缓存的命中率。

因此我们优选出来的进程必须同last和next指针域进行对比，其实就是检查就绪队列中的最优进程， 即红黑树中最左节点last是否可以抢占last和next指针域， 检查是否可以抢占是通过wakeup\_preempt\_entity函数来完成的。

### 2.3.4 wakeup_preempt_entity检查是否可以被抢占

```c
/*
 * Should 'se' preempt 'curr'.
 *
 *         |s1
 *    |s2
 *   |s3
 *     g
 *      |<--->|c
 *
 *  w(c, s1) = -1
 *  w(c, s2) =  0
 *  w(c, s3) =  1
 *
 */
static int
wakeup_preempt_entity(struct sched_entity *curr, struct sched_entity *se)
{
    /*  vdiff为curr和se vruntime的差值*/
    s64 gran, vdiff = curr->vruntime - se->vruntime;

    /*  cfs_rq的vruntime是单调递增的，也就是一个基准
     *  各个进程的vruntime追赶竞争cfsq的vruntime
     *  如果curr的vruntime比较小, 说明curr更加需要补偿, 
     *  即se无法抢占curr */
    if (vdiff <= 0)
        return -1;

    /*  计算curr的最小抢占期限粒度   */
    gran = wakeup_gran(curr, se);
    /*  当差值大于这个最小粒度的时候才抢占，这可以避免频繁抢占  */
    if (vdiff > gran)
        return 1;

    return 0;
}

static unsigned long
wakeup_gran(struct sched_entity *curr, struct sched_entity *se)
{
    /*  NICE_0_LOAD的基准最小运行期限  */
    unsigned long gran = sysctl_sched_wakeup_granularity;

    /*
     * Since its curr running now, convert the gran from real-time
     * to virtual-time in his units.
     *
     * By using 'se' instead of 'curr' we penalize light tasks, so
     * they get preempted easier. That is, if 'se' < 'curr' then
     * the resulting gran will be larger, therefore penalizing the
     * lighter, if otoh 'se' > 'curr' then the resulting gran will
     * be smaller, again penalizing the lighter task.
     *
     * This is especially important for buddies when the leftmost
     * task is higher priority than the buddy.
     *
     * 计算进程运行的期限，即抢占的粒度
     */
    return calc_delta_fair(gran, se);
}
```
到底能不能选择last和next两个进程，则是wakeup\_preempt\_entity函数来决定的，看下面的图解即可：

![image](./images/0x01.png)

- 如果S3是left，curr是next或者last，left的vruntime值小于curr和next，函数wakeup\_preempt\_entity肯定返回1，那么就说明next和last指针的vruntime和left差距过大，这个时候没有必要选择这个last或者next指针，而是应该优先补偿left。
- 如果next或者last是S2、S1，那么vruntime和left差距并不大，并没有超过sysctl\_sched\_wakeup\_granularity ，那么这个next或者last就可以被优先选择，而代替了left。

而清除last和next这两个指针的时机有这么几个：
- sched\_tick的时候，如果一个进程的运行时间超过理论时间（这个时间是根据load和cfs\_rq的load，平均分割sysctl\_sched\_latency的时间），那么如果next或者last指针指向这个正在运行的进程，需要清除这个指针，使得pick sched\_entity不会因为next或者last指针再次选择到这个sched_entity。
- 当一个sched\_entity调度实体dequeue出运行队列，那么如果有next或者last指针指向这个sched_entity，那么需要删除这个next或者last指针。
- 刚才说的那种case，如果next，last指针在pick的时候被使用了一次，那么这次用完了指针，需要清除相应的指针，避免使用过的next，last指针影响到下次pick。
- 当进程yield操作的时候，进程主动放弃了调度机会，那么如果next，last指针指向了这个sched_entity，那么需要清除相应指针。

## 2.4 set_next_entity

现在我们已经通过pick\_next\_task\_fair选择了进程，但是还需要完成一些工作，才能将其标记为运行进程， 这是通过set\_next\_entity来处理的。该函数定义在kernel/sched/fair.c。

当前执行进程（我们选择出来的进程马上要抢占处理器开始执行）不应该再保存在就绪队列上， 因此set\_next\_entity()函数会调用\_\_dequeue\_entity(cfs\_rq, se)把选中的下一个进程移出红黑树。 如果当前进程是最左节点，\_\_dequeue\_entity会将leftmost指针设置到次左进程。
```c
  /* 'current' is not kept within the tree. */
    if (se->on_rq)  /*  如果se尚在rq队列上  */
    {
        /*  ......  */
        
        /*  将se从cfs_rq的红黑树中删除  */
        __dequeue_entity(cfs_rq, se);
        
		/*  ......  */
    }
```
尽管该进程不再包含在红黑树中，但是进程和就绪队列之间的关联并没有丢失，因为curr标记了当前进程`cfs_rq->curr = se;`。
```c
    cfs_rq->curr = se;
```
然后接下来是一些统计信息的处理，如果内核开启了调度统计CONFIG_SCHEDSTATS标识，则会完成调度统计的计算和更新。
```c
#ifdef CONFIG_SCHEDSTATS
    /*
     * Track our maximum slice length, if the CPU's load is at
     * least twice that of our own weight (i.e. dont track it
     * when there are only lesser-weight tasks around):
     */
    if (schedstat_enabled() && rq_of(cfs_rq)->load.weight >= 2*se->load.weight) {
        se->statistics.slice_max = max(se->statistics.slice_max,
            se->sum_exec_runtime - se->prev_sum_exec_runtime);
    }
#endif
```
在set\_next\_entity的最后，将选择出的调度实体se的sum\_exec\_runtime保存在了prev\_sum\_exec\_runtime中， 因为该调度实体指向的进程，马上将抢占处理器成为当前活动进程，在CPU上花费的实际时间将记入sum\_exec\_runtime，因此内核会在prev\_sum\_exec\_runtime保存此前的设置。要注意进程中的sum\_exec\_runtime没有重置， 因此差值sum\_exec\_runtime - prev\_sum\_runtime确实标识了在CPU上执行花费的实际时间。

最后我们附上set\_next\_entity函数的完整注释信息：
```c
static void
set_next_entity(struct cfs_rq *cfs_rq, struct sched_entity *se)
{
    /* 'current' is not kept within the tree. */
    if (se->on_rq)  /*  如果se尚在rq队列上  */
    {
        /*
         * Any task has to be enqueued before it get to execute on
         * a CPU. So account for the time it spent waiting on the
         * runqueue.
         */
        if (schedstat_enabled())
            update_stats_wait_end(cfs_rq, se);
        /*  将se从cfs_rq的红黑树中删除  */
        __dequeue_entity(cfs_rq, se);
        update_load_avg(se, 1);
    }
    /*  新sched_entity中的exec_start字段为当前clock_task  */
    update_stats_curr_start(cfs_rq, se);
    /*  将se设置为curr进程  */
    cfs_rq->curr = se;
#ifdef CONFIG_SCHEDSTATS
    /*
     * Track our maximum slice length, if the CPU's load is at
     * least twice that of our own weight (i.e. dont track it
     * when there are only lesser-weight tasks around):
     */
    if (schedstat_enabled() && rq_of(cfs_rq)->load.weight >= 2*se->load.weight) {
        se->statistics.slice_max = max(se->statistics.slice_max,
            se->sum_exec_runtime - se->prev_sum_exec_runtime);
    }
#endif
    /*  更新task上一次投入运行的从时间  */
    se->prev_sum_exec_runtime = se->sum_exec_runtime;
}
```

# 3. idle进程的调度

```c
    /*  如果nr_running计数器为0,
     *  当前队列上没有可运行进程,
     *  则需要调度idle进程  */
    if (!cfs_rq->nr_running)
        goto idle;
```
如果系统中当前运行队列上没有可调度的进程，那么会调到idle标签去调度idle进程。

idle标签如下所示：
```c
idle:
    /*
     * This is OK, because current is on_cpu, which avoids it being picked
     * for load-balance and preemption/IRQs are still disabled avoiding
     * further scheduler activity on it and we're being very careful to
     * re-start the picking loop.
     */
    lockdep_unpin_lock(&rq->lock);
    new_tasks = idle_balance(rq);
    lockdep_pin_lock(&rq->lock);
    /*
     * Because idle_balance() releases (and re-acquires) rq->lock, it is
     * possible for any higher priority task to appear. In that case we
     * must re-start the pick_next_entity() loop.
     */
    if (new_tasks < 0)
        return RETRY_TASK;

    if (new_tasks > 0)
        goto again;

    return NULL;
```
其关键就是调用idle_balance进行任务的迁移。

每个cpu都有自己的运行队列，如果当前cpu上运行的任务都已经dequeue出运行队列，而且idle\_balance也没有移动到当前运行队列的任务，那么schedule函数中，按照`stop > idle > rt > cfs > idle`这三种调度方式顺序，寻找各自的运行任务，那么如果rt和cfs都未找到运行任务，那么最后会调用idle schedule的idle进程，作为schedule函数调度的下一个任务。

如果某个cpu空闲，而其他CPU不空闲，即当前CPU运行队列为NULL，而其他CPU运行队列有进程等待调度的时候， 则内核会对CPU尝试负载平衡，CPU负载均衡有两种方式：`pull`和`push`，即空闲CPU从其他忙的CPU队列中pull拉一个进程复制到当前空闲CPU上，或者忙的CPU队列将一个进程push推送到空闲的CPU队列中。

idle_balance其实就是pull的工作。

# 4. 组调度策略的支持

组调度的情形下，调度实体之间存在明显的层次关系，因此在跟新子调度实体的时候，需要更新父调度实体的信息， 同时我们为了保证同一组内的进程不能长时间占用处理机，必须补偿其他组内的进程，保证公平性。

```c
#ifdef CONFIG_FAIR_GROUP_SCHED
    /*  如果nr_running计数器为0, 即当前队列上没有可运行进程,
     *  则需要调度idle进程 */
    if (!cfs_rq->nr_running)
        goto idle;
    /*  如果当前运行进程prev不是被fair调度的普通非实时进程  */
    if (prev->sched_class != &fair_sched_class)
        goto simple;

    /*
     * Because of the set_next_buddy() in dequeue_task_fair() it is rather
     * likely that a next task is from the same cgroup as the current.
     *
     * Therefore attempt to avoid putting and setting the entire cgroup
     * hierarchy, only change the part that actually changes.
     */

    do {
        struct sched_entity *curr = cfs_rq->curr;

        /*
         * Since we got here without doing put_prev_entity() we also
         * have to consider cfs_rq->curr. If it is still a runnable
         * entity, update_curr() will update its vruntime, otherwise
         * forget we've ever seen it.
         */
        if (curr)
        {
            /*  如果当前进程curr在队列上, 
             *  则需要更新起统计量和虚拟运行时间
             *  否则设置curr为空  */
            if (curr->on_rq)
                update_curr(cfs_rq);
            else
                curr = NULL;

            /*
             * This call to check_cfs_rq_runtime() will do the
             * throttle and dequeue its entity in the parent(s).
             * Therefore the 'simple' nr_running test will indeed
             * be correct.
             */
            if (unlikely(check_cfs_rq_runtime(cfs_rq)))
                goto simple;
        }
        /*  选择一个最优的调度实体  */
        se = pick_next_entity(cfs_rq, curr);
        cfs_rq = group_cfs_rq(se);
    } while (cfs_rq);  /*  如果被调度的进程仍属于当前组，那么选取下一个可能被调度的任务，以保证组间调度的公平性  */
    /*  获取调度实体se的进程实体信息  */
    p = task_of(se);

    /*
     * Since we haven't yet done put_prev_entity and if the selected task
     * is a different task than we started out with, try and touch the
     * least amount of cfs_rqs.
     */
    if (prev != p)
    {
        struct sched_entity *pse = &prev->se;

        while (!(cfs_rq = is_same_group(se, pse)))
        {
            int se_depth = se->depth;
            int pse_depth = pse->depth;

            if (se_depth <= pse_depth)
            {
                put_prev_entity(cfs_rq_of(pse), pse);
                pse = parent_entity(pse);
            }
            if (se_depth >= pse_depth)
            {
                set_next_entity(cfs_rq_of(se), se);
                se = parent_entity(se);
            }
        }

        put_prev_entity(cfs_rq, pse);
        set_next_entity(cfs_rq, se);
    }

    if (hrtick_enabled(rq))
        hrtick_start_fair(rq, p);

    return p;
```

# 5. 与主调度器schedule进行通信

我们在之前讲解主调度器的时候就提到过，主调度器函数schedule会调用\_\_schedule来完成抢占， 而主调度器的主要功能就是选择一个新的进程来抢占到当前的处理器。
因此其中必然不能缺少pick\_next\_task工作。

\_\_schedule调用全局的pick\_next\_task函数选择一个最优的进程，内核代码参见kernel/sched/core.c。
```c
static void __sched notrace __schedule(bool preempt)
{
	/*  ......  */
	
    next = pick_next_task(rq);
    
	/*  ......  */
}
```
全局的pick\_next\_task函数会从按照优先级遍历所有调度器类的pick_next_task函数，去查找最优的那个进程， 当然因为大多数情况下，系统中全是CFS调度的非实时进程，因而linux内核也有一些优化的策略。

其执行流程如下：
- 如果当前cpu上所有的进程都是cfs调度的普通非实时进程，则直接用cfs调度，如果无程序可调度则调度idle进程。
- 否则从优先级最高的调度器类sched\_class\_highest（目前是stop\_sched\_class）开始依次遍历所有调度器类的pick\_next\_task函数，选择最优的那个进程执行。

# 6. 总结

pick\_next\_task\_fair用于完全公平调度器在CFS的运行队列中优选出一个最优的进程， 为了适应组调度策略和基本的策略，pick\_next\_task_fair使用的不同的代码标签：
- simple标签：是CFS中最基础的pick_next操作
- idle标签：是使得在没有进程被调度时, 调度idle进程
- again标签：是用于循环的进行pick_next操作
- CONFIG\_FAIR\_GROUP\_SCHED宏：是指定了组调度情况下的pick\_next操作, 如果不支持组调度, 则pick\_next\_task_fair将直接从simple开始执行

pick\_next\_task\_fair的基本流程如下：

流程 | 描述
---|:---
如果!cfs\_rq->nr\_running则goto idle;	| 如果nr_running计数器为0，当前队列上没有可运行进程，则需要调度idle进程。
put\_prev\_task(rq, prev); | 将当前进程放入运行队列的合适位置，每次当进程被调度后都会使用set\_next\_entity从红黑树中移除，因此被抢占时需要重新加如红黑树中等待被调度。
se = pick\_next\_entity(cfs\_rq, NULL); | 选出下一个可执行调度实体。
set\_next\_entity(cfs\_rq, se); | set\_next\_entity会调用\_\_dequeue\_entity把选中的进程从红黑树移除，并更新红黑树。

其中最关键的pick\_next\_entity函数选择出下一个最渴望被公平调度器调度的进程，函数的执行流程其实很简单：
1. 先从最左节点left和当前节点curr中选择出最渴望被调度（即vruntime最小）的那个调度实体。
2. 判断第一步优选出的调度实体se是不是cfs\_rq中被跳过调度的那个进程skip，如果是则可能需要继续优选红黑树次左节点。
    - 如果`se == curr == skip`则需要跳过curr选择最左的那个调度实体`second = left = __pick_first_entity(cfs_rq);`。
    - 否则`se == left == skip`，则从次优的调度实体second和curr中选择最优的那个进程。
3. 检查left是否可以抢占last和next调度实体，此项有助于提高缓存的命中率。
    - cfs\_rq的last和next指针，last表示最后一个执行wakeup的sched\_entity，next表示最后一个被wakeup的sched\_entity，在pick新sched\_entity的时候，会优先选择这些last或者next指针的sched\_entity，有利于提高缓存的命中率。

于是我们会发现，下一个将要被调度的调度实体或者进程，总是下列几个调度实体之一：

调度实体 | 描述
---|:---
`left = __pick_first_entity(cfs_rq)` | 红黑树的最左节点，这个节点拥有当前队列中vruntime最小的特性，即应该优先被调度。
`second = __pick_first_entity(left)` | 红黑树的次左节点，为什么这个节点也可能呢，因为内核支持skip跳过某个进程的抢占权力的，</br>如果left被标记为skip（由cfs_rq->skip域指定），那么可能就需要找到次优的那个进程。
cfs\_rq的curr结点 | curr节点的vruntime可能比left和second更小，但是由于它正在运行，</br>因此它不在红黑树中（进程抢占物理机的时候对应节点同时会从红黑树中删除），但是如果其vruntime足够小，</br>意味着cfs调度器应该尽可能的补偿curr进程，让它再次被调度，同样这种优化也有助于提高缓存的命中率。
cfs\_rq的last或者next | last表示最后一个执行wakeup的sched\_entity，next表示最后一个被wakeup的sched\_entity，</br>在pick新sched\_entity的时候，会优先选择这些last或者next指针的sched_entity，有利于提高缓存的命中率。

即红黑树中的最左结点left和次左结点second（检查两个节点是因为cfs\_rq的skip指针域标识了内核需要跳过不调度的实体信息，如果left被跳过，则需要检查second）。

以及cfs\_rq的调度实体curr，last和next，curr是当前正在运行的进程，它虽然已经运行,但是可能仍然很饥饿，那么我们应该继续补偿它，而last表示最后一个执行wakeup的sched\_entity，
next表示最后一个被wakeup的sched\_entity，刚被唤醒的进程可能更希望得到CPU，因此在pick新sched\_entity的时候，会优先选择这些last或者next指针的sched_entity，有利于提高缓存的命中率。