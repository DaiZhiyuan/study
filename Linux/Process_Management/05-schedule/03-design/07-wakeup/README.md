***Linux唤醒抢占***

- 1 CFS回顾
    - 1.1 CFS调度算法
    - 1.2 CFS的pick_next_fair选择下一个进程
    - 1.3 CFS如何处理周期性调度
    - 1.4 唤醒抢占
- 2 Linux进程的睡眠
- 3 Linux进程的唤醒
    - 3.1 wake_up_process
    - 3.2 try_to_wake_up
    - 3.3 wake_up_new_task
    - 3.4 check_preempt_curr
- 4 无效唤醒
    - 4.1 无效唤醒的概念
    - 4.2 无效唤醒的原因
    - 4.3 避免无效抢占
- 5 Linux内核的例子
    - 5.1 一个最基本的例子
    - 5.2 2号进程的例子——避免无效抢占
    - 5.3 kthread_worker_fn
- 6 总结

# 1. CFS回顾

## 1.1 CFS调度算法

CFS调度算法的思想：理想状态下每个进程都能获得相同的时间片，并且同时运行在CPU上，但是一个CPU同一时刻运行的进程只有一个。也就是说，当一个进程占用CPU时，其他进程就必须等待。
CFS为了实现公平，必须惩罚当前正在运行的进程，以使那些正在等待的进程下次被调度。

## 1.2 CFS的pick_next_fair选择下一个进程

CFS的pick\_next操作pick\_next\_task\_fair函数，它从当前运行队列上找出一个最优的进程来抢占处理器，
一般来说，这个最优进程总是红黑树的最左侧的left结点（其vruntime值最小），当然如果挑出的进程正好是队列上需要被调度过的则skip，则可能需要进一步读取红黑树的次左结点，而同样curr进程可能vruntime与cfs\_rq的min\_vruntime小，因此，它可能更渴望得到处理器，而last和next进程由于刚被唤醒也应该尽可能的补偿。

主调度器schedule在选择最优的进程抢占处理器的时候，通过\_\_schedule调用全局的pick\_next\_task函数，在全局的pick\_next\_task函数中，按照`stop > dl > rt > cfs > idle`的顺序依次从各个调度器类中pick\_next函数，从而选择一个最优的进程。

## 1.3 CFS如何处理周期性调度

周期性调度器的工作由scheduler\_tick函数完成，在schduler\_tick中周期性调度器先找到就绪队列上当前运行的进程curr，然后通过调用curr进程所述调度器类sched\_class的task\_tick函数完成周期性调度的工作。

周期性调度的工作形式上sched\_class调度类的task\_tick函数完成，CFS则对应task\_tick\_fair函数，但实际上工作交给entity\_tick完成。

## 1.4 唤醒抢占

当在try\_to\_wake\_up、wake\_up\_process和wake\_up\_new\_task中唤醒进程时，内核使用全局check\_preempt\_curr看看是否进程可以抢占当前进程可以抢占当前运行的进程。请注意：该过程不涉及核心调度器。

每个调度器类都应该实现一个check\_preempt\_curr函数，在全局check\_preempt\_curr中会调用其进程所属调度器。
check\_preempt\_curr进行抢占检查，对于完全公平调度器CFS处理的进程，则对应由check\_preempt\_wakeup函数执行该策略。

新唤醒的进程不必一定由完全公平调度器处理，如果新进程是一个实时进程，则会立即请求调度，因此实时进程优先级极高，实时进程总是会抢占CFS进程。

# 2. Linux进程的睡眠

在Linux中，仅等待CPU时间的进程称为就绪进程，它们被放置在一个运行队列中，一个就绪进程的状态标志为TASK\_RUNNING。一旦运行中的进程时间片用完，Linux内核的调度器会剥夺这个进程对CPU的控制权，并且从运行队列中选择一个合适的进程投入运行。

当然，一个进程也可以主动释放CPU的控制权。函数schedule()是一个调度函数，它可以被一个进程主动调用，从而调度其他进程占用CPU。一旦这个主动放弃CPU的进程被重新调度占用CPU，那么它将从上次停止的位置开始执行，也就是说它将从调用schedule()的下一行代码处开始执行。

有时候，进程需要等待直到某个特定的时间发生，例如，设备初始化完成、I/O操作完成、定时器到时等。在这种情况下，进程则必须从运行队列移出，加入到一个等待队列中，这个时候进程就进入了睡眠状态。

Linux中的进程睡眠状态有两种：
- 可中断的睡眠状态，其状态标志位TASK\_INTERRUPTIBLE：进程会睡眠直到某个条件变为帧，比如说，产生一个硬件中断、释放进程正在等待的系统资源、传递一个信号都可以是唤醒进程的条件。
- 不可中断的睡眠状态，其状态标志位TASK\_UNINTERRUPTIBLE：它与可中断睡眠状态类似，但是它有一个例外，就是把信号传递到这种睡眠状态的进程，不能改变它的状态，也就是说，它不相应信号的唤醒。不可睡眠状态一般较少用到，在一些特定情况下这种状态是很有用的，比如说，进程必须等待不能被中断，知道某个特定的事件发生。

在现代的Linux操作系统中，进程一般都是调用schedule()的方法进入睡眠状态的，下面的代码演示了，如何让正在运行的进程进入睡眠状态。
```c
    sleeping_task = current;
    set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    some_function();
    /* Rest of the code ... */
```
在第一条语句中，程序存储了一份进程结构指针sleeping\_task，current是一个宏，它指向正在执行的进程结构。
set\_current\_state()将该进程的状态从执行状态TASK\_RUNNING变成睡眠状态TASK\_INTERRUPTIBLE。
- 如果schedule是被一个状态为TASK\_RUNNING的进程调度，那么schedule将调度另外一个进程占用CPU；
- 如果schedule是被一个状态为TASK\_INTERRUPTIBLE或TASK\_UNINTERRUPTIBLE的进程调度，那么还有一个附加的步骤将被执行：当前执行的进程在另外一个进程被调度之前会被从运行队列中移出，这将导致正在运行的哪个进程进入睡眠，因为它已经不在运行队列中了。

# 3. Linux进程的唤醒

当在try\_to\_wake\_up、wake\_up\_process和wake\_up\_new\_task中唤醒进程时，内核使用全局check\_preempt\_curr查看是否进程可抢占当前进程。清注意：该过程不涉及核心调度器。

## 3.1 wake_up_process

我们可以使用wake\_up\_process将刚才哪个进入睡眠的进程唤醒，该函数定义在kernel/sched/core.c：
```c
int wake_up_process(struct task_struct *p)
{
	return try_to_wake_up(p, TASK_NORMAL, 0);
}
```
在调用了wake\_up\_process以后，这个睡眠进程的状态会被设置为TASK\_RUNNING，而且调度器会把它加入到运行队列中去。当然，这个进程只有在下次被调度器调度的时候才能真正地投入运行。

## 3.2 try_to_wake_up

try\_to\_wake\_up函数通过把进程状态设置为TASK\_RUNNING，并把该进程插入本地CPU运行队列rq，来达到唤醒睡眠和停止进程的目的。

例如：调用该函数唤醒等待队列中的进程，或恢复执行等待信号的进程。
```c
static int
try_to_wake_up(struct task_struct *p, unsigned int state, int wake_flags)
```
该函数接收的参数有：
- 被唤醒进程的描述符指针p
- 可被唤醒的进程状态掩码state
- 一个标志wake\_flags，用来禁止被唤醒的进程抢占本地CPU上正在运行的进程

## 3.3 wake_up_new_task

该函数定义在kernel/sched/core.c中：
```c
void wake_up_new_task(struct task_struct *p)
```
之前进入睡眠状态可以通过try\_to\_wake\_up和wake\_up\_process完成唤醒，而我们fork新创建的进程在完成自己的创建工作后，可以通过wake\_up\_new\_task完成唤醒工作。

使用fork创建进程的时候，内核会调用do\_fork函数完成内核的创建，其中在进程的信息创建完毕后，就可以使用wake\_up\_new\_task将进程唤醒并添加到就绪队列中等待调度。

## 3.4 check_preempt_curr

wake\_up\_new\_task中唤醒进程时，内核使用全局check\_preempt\_curr看看是否进程可以抢占当前运行的进程。
```c
check_preempt_curr(rq, p, WF_FORK);
```
函数定义在kernel/sched/core.c中：
```c
void check_preempt_curr(struct rq *rq, struct task_struct *p, int flags)
{
    const struct sched_class *class;

    if (p->sched_class == rq->curr->sched_class)
    {
        rq->curr->sched_class->check_preempt_curr(rq, p, flags);
    }
    else
    {
        for_each_class(class) {
            if (class == rq->curr->sched_class)
                break;
            if (class == p->sched_class) {
                resched_curr(rq);
                break;
            }
        }
    }

    /*
     * A queue event has occurred, and we're going to schedule.  In
     * this case, we can save a useless back to back clock update.
     */
    if (task_on_rq_queued(rq->curr) && test_tsk_need_resched(rq->curr))
        rq_clock_skip_update(rq, true);
}
```

# 4. 无效唤醒

## 4.1 无效唤醒的概念

几乎在所有的情况下，进程都会检查了某些条件之后，发现条件不满足才进入睡眠。可是有的时候进程却会在判定条件为真后开始睡眠，如果这样的话进程就会无限期地休眠下去，这就是所谓的无效唤醒问题。

在操作系统中，当多个进程都试图对共享数据进行某种处理，而最后的结果又取决于进程运行的顺序时，就会发生竞争条件，这是操作系统中一个典型的问题，无效唤醒恰恰就是由于竞争条件导致的。

设想有两个进程A和B，A进程正在处理一个链表，它需要检查这个链表是否为空，如果不为空就对链表里面的数据进行一些操作，同时B进程也在往这个链表添加节点。当这个链表式空的时后，由于无数据可操作，这是进程A就进入睡眠，当进程B向链表里添加了节点之后它就唤醒进程A，其代码如下：

进程A
```c
spin_lock(&list_lock);
if(list_empty(&list_head))
{
	spin_unlock(&list_lock);
	set_current_state(TASK_INTERRUPTIBLE);
	schedule();
	spin_lock(&list_lock);
}
/* Rest of the code ... */
spin_unlock(&list_lock);
}
```
进程B
```c
    spin_lock(&list_lock);
    list_add_tail(&list_head, new_node);
    spin_unlock(&list_lock);
    wake_up_process(A);
```
这里会出现一个问题，假如，当进程A执行到第4行后（spin\_unlock(&list\_lcok);）第5行前（set\_current\_state(TASK\_INTERRUPTIBLE);）的时候，进程B被另外一个处理器调度投入运行。
在这个时间片内，进程B执行完了它所有的指令，因此，它试图唤醒进程A，而此时的进程A还没有进入睡眠，所以唤醒操作无效。

在这之后，进程A继续执行，它会错误地认为这个时候链表仍然是空的，于是将自己状态设置为TASK\_INTERRUPTIBLE然后调度schedule()进入睡眠，由于错误了进程B的唤醒，它将会无限期的睡眠下去，这就是无效唤醒问题，因为即使链表中有数据需要处理，进程A也还是睡眠了。

## 4.2 无效唤醒的原因

如何避免无效唤醒问题呢？

我们发现无效唤醒，主要发生在检查条件之后（链表为空）和进程状态被设置为睡眠状态之前，本来进程B的wake\_up\_process提供了一次将进程A状态置位TASK\_RUNING的机会，可惜这个时候进程A的状态仍然是TASK\_RUNING，所以wake\_up\_process将进程A状态从睡眠态转变为运行态的努力没有起到预期的作用。

## 4.3 避免无效抢占

要解决这个问题，必须使用一种保障机制使得判断链表为空和设置进程状态为睡眠状态成为一个不可分割的步骤才行，也就是必须消除竞争条件产生的根源，这样在这之后出现的wake\_up\_process就可以起到唤醒状态进程的作用了。

找到了原因后，重新设计一下进程A的代码结构，这就可以避免上面例子中的无效唤醒问题了。

进程A
```c
    set_current_state(TASK_INTERRUPTIBLE);
    spin_lock(&list_lock);
    if(list_empty(&list_head))
    {
    	spin_unlock(&list_lock);
    	schedule();
    	spin_lock(&list_lock);
    }
    set_current_state(TASK_RUNNING);
    /* Rest of the code ... */
    spin_unlock(&list_lock);
```
可以看到，这段代码在测试条件（链表为空）就当前执行进程状态设置成TASK\_INTERRUPTIBLE了，而且在链表不为空的情况下由将自己设置为TASK\_RUNNING状态。

这样一来，如果进程B在进程A检查链表为空后调用wake\_up\_process，那么进程A的状态就会自动由原来TASK\_INTERRUPTIBLE变成TASK\_RUNING，此后即使进程又调用了schedule，由于它现在的状态是TASK\_RUNNING，所以仍然不会被从运行队列中移出，因为不会错误的进入睡眠，当然也就避免了无效唤醒问题。

# 5. Linux内核的例子

## 5.1 一个最基本的例子

在Linux操作系统中，内核的稳定性至关重要，为了避免Linux操作系统内核中出现无效唤醒问题，Linux内核在需要进程睡眠的时候，应使用类似如下的操作：
```c
    /* ‘q’是我们希望睡眠的等待队列 */
    DECLARE_WAITQUEUE(wait,current);
    add_wait_queue(q, &wait);
    set_current_state(TASK_INTERRUPTIBLE);
    
    /* 或TASK_INTERRUPTIBLE */
    while(!condition) /* ‘condition’ 是等待的条件*/
    schedule();
    set_current_state(TASK_RUNNING);
    remove_wait_queue(q, &wait);
```
上面的操作，使得进程通过下面的一些列步骤安全地将自己加入到一个等待队列中进行睡眠：
- 首先调用DECLERE\_WAITQEUEUE创建一个等待队列
- 然后调用add\_wait\_queue()把自己加入到等待队列中，并将进程的状态设置为TASK\_INTERRUPTIBLE或者TASK\_UNINTERRUPTIBLE。
- 然后循环检查条件是否为真：如果为真就没有必要睡眠，如果条件为假，就调用schedule。
- 当进程检查的条件满足后，进程将自己设置为TASK\_RUNNING并调用remove\_wait\_queue将自己移除等待队列。

从上面可以看到，Linux的内核代码维护者也是在进程检查条件之前就设置进程的状态为睡眠状态，然后才循环检查条件。如果在进程开始睡眠之前条件就已经达成，那么循环退出并用set\_current\_state将自己的状态设置为就绪，这样同时保证了进程不会存在错误的进入睡眠的倾向，当然也就不会导致出现无效唤醒问题。

内核中有很多地方使用了避免无效唤醒的时候，最普遍的地方是内核线程，因为内核线程的主要功能是辅助内核完成一定的工作，大多数情况下它们处于睡眠态，当内核发现有任务要做的时候，才会唤醒它们。

## 5.2 2号进程的例子——避免无效抢占
下面让我们用Linux内核的实例来看看Linux内核时如何避免无效睡眠的，还记得2号进程吧，它的主要工作就是接手内核线程kthread的创建，其工作流程函数是kthreadd。代码在kernel/kthread.c中。
```c
for (;;) {
    // 避免无效睡眠
    /* 首先将kthreadd线程状态设置为TASK_INTERRUPTIBLE, 如果当前
    没有要创建的线程则主动放弃CPU完成调度.此进程变为阻塞态*/
    set_current_state(TASK_INTERRUPTIBLE);
    if (list_empty(&kthread_create_list))  // 没有需要创建的内核线程
        schedule();                        // 什么也不做, 执行一次调度, 让出CPU
        
    /* 运行到此表示kthreadd线程被唤醒(就是我们当前)
    设置进程运行状态为 TASK_RUNNING */
    __set_current_state(TASK_RUNNING);

    spin_lock(&kthread_create_lock);
	/*  ==do_something start==  */
	while (!list_empty(&kthread_create_list)) {
        struct kthread_create_info *create;

        create = list_entry(kthread_create_list.next,
                    struct kthread_create_info, list);
        list_del_init(&create->list);
        spin_unlock(&kthread_create_lock);

        create_kthread(create);
        /*  ==do_something end == */

        spin_lock(&kthread_create_lock);
    }
    spin_unlock(&kthread_create_lock);
```
## 5.3 kthread_worker_fn

kthread\_worker/kthread\_work是一种内核工作的更好管理方式，可以多个内核线程在同一个worker上工作，共同完成work的工作，有点像线程池的工作方式。

内核提供了kthread\_worker\_fn函数一般作为kthread\_create或者kthread\_run函数的threadfn参数运行，可以将多个内核线程附加到同一个worker上面，即将同一个worker结构传给kthread\_run或者kthread\_create当做threadfn的参数就可以了。

其kthread\_worker\_fn函数作为worker的主函数框架，也包含了避免无效唤醒的代码：
```c
int kthread_worker_fn(void *worker_ptr)
{
	/* ......*/
	set_current_state(TASK_INTERRUPTIBLE);  /* mb paired w/ kthread_stop */

	if (kthread_should_stop()) {
		__set_current_state(TASK_RUNNING);
		spin_lock_irq(&worker->lock);
	worker->task = NULL;
	spin_unlock_irq(&worker->lock);
	return 0;
	}
	/* ......*/
}
```
此外，内核的\_\_kthread\_parkme函数也包含了类似的代码。

# 6. 总结

通过上面的讨论，可以发现在Linux中避免进程的无效唤醒的关键是：
- 在进程检查条件之前就将进程状态置为TASK\_INTERRUPTIBLE或TASK\_UNINTERRUPTIBLE
- 并且如果检查的条件满足的话就应该将其状态重新设置为TASK\_RUNNING

这样无论进程等待的条件是否满足，进程都不会被移除就绪队列而错误地进入睡眠状态，从而避免了无效唤醒问题。
```c
    set_current_state(TASK_INTERRUPTIBLE);
    spin_lock(&list_lock);
    if(list_empty(&list_head))
    {
    	spin_unlock(&list_lock);
    	schedule();
    	spin_lock(&list_lock);
    }
    set_current_state(TASK_RUNNING);
    /* Rest of the code ... */
    spin_unlock(&list_lock);
```