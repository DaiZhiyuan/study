***Linux CFS调度器之唤醒抢占***

- 1 CFS调度算法
- 2 进程的创建
- 3 处理新进程
    - 3.1 place_entity设置新进程的虚拟运行时间
    - 3.2 sysctl_sched_child_runs_first控制子进程运行时机
    - 3.3 适应迁移的vruntime值

# 1. CFS调度算法

CFS调度算法的思想：理想状态下每个进程都能获得相同的时间片，并且同时运行在CPU上，但实际上一个CPU同一时刻只能运行一个进程。也就是说，当一个进程占用CPU时，其他进程就必须等待。

CFS为了实现公平，必须惩罚当前运行的进程，以使那些正在等待的进程下次被调度。

# 2. 进程的创建

fork、vfork和clone的系统调用入口地址分别是：sys\_fork、sys\_vfork和sys\_clone，而它们的定义是依赖于体系结构的，而它们最终都调用了do\_fork，在do\_fork中通过cope\_process复制进程的信息，调用wake\_up\_new\_task将子进程加入调度器中：
1. dup\_task\_struct中为其分配了新的堆栈
2. 调用了sched\_fork，将其置为TASK\_RUNNING
3. copy\_thread(tls)中将父进程的寄存器上下文复制给子进程，保证了父子进程的堆栈信息时一致的
4. 将ret\_from\_fork的地址设置为eip寄存器的值
5. 为新进程分配并设置新的pid
6. 最终子进程从ret\_from\_fork开始执行

# 3. 处理新进程

CFS相关信息：


信息 | 描述
---|:---
负荷权重</br>load\_weight | CFS进程的负荷权重，与进程的优先级相关，优先级越高的进程，负荷权重越高。
虚拟运行时间</br>vruntime | vruntime是通过进程的实际运行时间和进程的权重weight计算出来的。</br>在CFS调度器中，将进程由优先级这个概念弱化，而是强调进程的权重。</br>一个进程的权重越大，则说明这个进程更需要运行，因此它的vruntime就越小，这样被调度的机会就越大。</br>而，CFS调度器中权重在内核时对用户态进程的优先级nice值，通过prio\_to\_weiht数组进程nice值和权重的转换而计算出来的。

CFS相关操作：

操作 | 函数 | 描述
---|---|:---
进程入队</br>进程出队 | enqueue\_task\_fair</br>dequeue\_task\_fair | 向CFS的就绪队列中添加进程</br>向CFS的就绪队列中删除进程
选择最优进程</br>主调度器 | pick\_next\_task\_fair | 主调度器会按照如下顺序调度sched->\_\_schedule->全局pick\_next\_task。</br></br>全局的pick\_next\_task函数会从按照优先级遍历所有调度器类的pick\_next\_task函数，去寻找最优的那个进程，</br>当然因为大多数情况下，系统中全是CFS调度的非实时进程，因而，Linux内核也有一些优化的策略。</br></br>一般情况下，选择红黑树的最左进程left作为最优进程完成调度，</br>如果选出的进程正好是cfs\_rq->skip需要跳过的那个进程，则可能需要在检查红黑树的次左进程second，</br>同时由于curr进程不再红黑树中，它可能比较饥饿，将选出的进程与curr进程择优选取，</br>同样last进程和next进程由于刚被唤醒，可能比较饥饿，优先调度它们能提供系统缓存的命中率。
周期性调度 | task\_tick\_fair | 周期性调度器的工作由scheduler\_tick函数完成，</br>在scheduler\_tick中周期性调度器通过调用curr进程所属调度器类sched\_class的task\_tick函数完成周期性调度的工作。</br></br>而entity\_tick中则通过check\_preempt\_tick函数检查是否需要抢占当前进程curr，</br>如果发现curr进程已经运行了足够长的时间，其他进程已经开始饥饿。</br>那么我们就需要通过resched\_curr函数来设置重调度标识TIF\_NEED\_RESCHED，</br>此标志会提示系统在合适的时间进行调度。

完全公平调度器如何处理一个新创建的进程，该工作由task\_fork\_fair函数来完成。

```c
/*
 * called on fork with the child task as argument from the parent's context
 *  - child not yet on the tasklist
 *  - preemption disabled
 */
static void task_fork_fair(struct task_struct *p)
{
    struct cfs_rq *cfs_rq;
    struct sched_entity *se = &p->se, *curr;
    int this_cpu = smp_processor_id();
    struct rq *rq = this_rq();
    unsigned long flags;

    raw_spin_lock_irqsave(&rq->lock, flags);

    update_rq_clock(rq);

    cfs_rq = task_cfs_rq(current);
    curr = cfs_rq->curr;

    /*
     * Not only the cpu but also the task_group of the parent might have
     * been changed after parent->se.parent,cfs_rq were copied to
     * child->se.parent,cfs_rq. So call __set_task_cpu() to make those
     * of child point to valid ones.
     */
    rcu_read_lock();
    __set_task_cpu(p, this_cpu);
    rcu_read_unlock();

    update_curr(cfs_rq);

    if (curr)
        se->vruntime = curr->vruntime;
    place_entity(cfs_rq, se, 1);

    if (sysctl_sched_child_runs_first && curr && entity_before(curr, se)) {
        /*
         * Upon rescheduling, sched_class::put_prev_task() will place
         * 'current' within the tree based on its new key value.
         */
        swap(curr->vruntime, se->vruntime);
        resched_curr(rq);
    }

    se->vruntime -= cfs_rq->min_vruntime;

    raw_spin_unlock_irqrestore(&rq->lock, flags);
}
```

## 3.1 place_entity设置新进程的虚拟运行时间

该函数先调用update\_curr进程通常的统计量更新，然后调用此前讨论过的place\_entity设置调度实体se的vruntime。
```c
    /*  更新统计量  */
    update_curr(cfs_rq);

    if (curr)
        se->vruntime = curr->vruntime;
        
    /*  调整调度实体se的虚拟运行时间  */
    place_entity(cfs_rq, se, 1);
```
我们可以看到，此时调用place\_entity时的inital参数设置为1，以便用sched\_vslice\_add计算初始的vruntime，内核以这种方式确定了进程在延迟周期中所占的时间份额，并转换成vruntime。这个是调度器最初向进程欠下的站务。

## 3.2 sysctl_sched_child_runs_first控制子进程运行时机

接下来可以使用参数sysctl\_sched\_child\_runs\_first控制新建子进程是否应该在父进程之前运行。这通常是有益的，特别在子进程随后执行exec系统调用的情况下。该参数的默认设置是0，但可以通过/proc/sys/kernel/sched\_child\_first修改，代码如下所示：
```c
/*  如果设置了sysctl_sched_child_runs_first期望se进程先运行
     *  但是se进行的虚拟运行时间却大于当前进程curr
     *  此时我们需要保证se的entity_key小于curr, 才能保证se先运行
     *  内核此处是通过swap(curr, se)的虚拟运行时间来完成的  */
    if (sysctl_sched_child_runs_first && curr && entity_before(curr, se))
    {
        /*
         * Upon rescheduling, sched_class::put_prev_task() will place
         * 'current' within the tree based on its new key value.
         */
        /*  由于curr的vruntime较小, 为了使se先运行, 交换两者的vruntime  */
        swap(curr->vruntime, se->vruntime);
        /*  设置重调度标识, 通知内核在合适的时间进行进程调度  */
        resched_curr(rq);
    }
```
如果`entity\_before(curr, se)`，则父进程curr的vruntime小于子进程se的vruntime，即在红黑树中父进程curr更靠左，这就意味着父进程将在子进程之前被调度。
这种情况下如果设置了sysctl\_sched\_child\_runs\_first标识，这时候我们必须采取策略保证子进程先运行，可以通过交换curr和se的vruntime值，来保证se进程（子进程）的vruntime小于curr。

## 3.3 适应迁移的vruntime值

在task\_fork\_fair函数的最后，使用了一个小技巧，通过place\_entity计算出的基准rruntime，减去了运行队列的min\_vruntime。
```c
    se->vruntime -= cfs_rq->min_vruntime;
```
我们前面讲解place\_entity的时候说到，新进程的创建和睡眠后苏醒的进程为了保证它们的vruntime与系统中进程的vruntime差距不会太大，会使用place\_entity来设置其vruntime，在place\_entity中重新设置vruntime值，以cfs\_rq - min\_vruntime值为基础，给与一定补偿，但是不能补偿太多。这样由于睡眠进程在唤醒时或者新进程创建完成后会获得vruntime的补偿，所以它在醒来和创建后有能力抢占CPU是大概率时间，这也是CFS调度器的本意，即保证交互式进程的响应速度，因此交互式进程等待用户输入会频繁睡眠。

但是这样子也会有一个问题，我们是以某个cfs就绪队列的min\_vruntime值为基础来设定的，在多CPU的系统上，不同的CPU负载不一样，有的CPU会更忙一些，而每个CPU都有自己的运行队列，每个队列中进程的vruntime也走得有快有慢，比如，我们对比每个运行队列的min\_vruntime值，都会有不同，如果一个进程从min\_vruntime更小的CPU0上迁移到min\_vruntime更大的CPU1上，可能会占便宜了，因此CPU1的运行队列中进程的vruntime普遍较大，迁移过来的进程就会获得更多的CPU时间片。这显然不公平。

同样的问题出现在刚创建的进程上，还没有投入运行，没有加入到某个就绪队列中，它以某个就绪队列的min\_vruntime为基准设置了vruntime，但是进程不一定在当前CPU上运行，即新创建的进程应该是可以被迁移的。

CFS是这样做的：
- 当进程从一个CPU的运行队列中出来（dequeue\_entity）的时候，它的vruntime要减去队列的min\_vruntime值
- 当进程加入到另一个CPU运行队列（enqueue\_entity）时，它的vruntime要加上该队列的min\_vruntime值
- 当进程刚刚创建以某个cfs\_rq的min\_vruntime为基准设置vruntime后，也叫减去队列的min\_vruntime值

这样，进程从一个CPU迁移到另一个CPU之后，vruntime保持相对公平。

减去min\_vruntime的情形如下：
```c
dequeue_entity()：

	if (!(flags & DEQUEUE_SLEEP))
		se->vruntime -= cfs_rq->min_vruntime;

task_fork_fair():

	se->vruntime -= cfs_rq->min_vruntime;

switched_from_fair():
    if (!se->on_rq && p->state != TASK_RUNNING) 
    {
    	/*
         * Fix up our vruntime so that the current sleep doesn't
         * cause 'unlimited' sleep bonus.
         */
		place_entity(cfs_rq, se, 0);
        se->vruntime -= cfs_rq->min_vruntime;
	}
```
加上min\_vruntime的情形如下：
```c
	if (!(flags & ENQUEUE_WAKEUP) || (flags & ENQUEUE_WAKING))
		se->vruntime += cfs_rq->min_vruntime;

attach_task_cfs_rq:
    if (!vruntime_normalized(p))
		se->vruntime += cfs_rq->min_vruntime;
```