***Linux下2号进程的kthreadd***

- 1 概述
- 2 kthreadd进程的创建
- 3 kthreadd进程的时间循环
- 4 create_kthread完成内核线程的创建
- 5 新创建的内核线程kthread函数
- 6 总结

# 1. 概述

内核初始化rest\_init函数中，由进程0（swapper进程）创建了两个process：
- init进程（pid=1，ppid=0）
- kthreadd（pid=2，ppid=0）

所有其他的内核线程的ppid都是2，也就是说它们都是由kthreadd这个thread创建的。

所有的内核线程在大部分时间都处于阻塞状态（TASK\_INTERRUPTIBLE）只有在系统满足进程需要的某些资源的情况下才会运行。

它的任务就是管理和调度其他内核线程kernel\_thread，会循环执行一个kthreadd的函数，该函数的作用就是运行kthread\_create\_list全局链表中维护的kthread，当我们调用kernel\_thread创建的内核线程都会加入到此链表中，因此所有的内核线程都是直接或间接的以kthreadd为父进程。

# 2. kthreadd进程的创建

在rest_init函数中创建2号进程的代码如下：
```c
    ......
    
    pid = kernel_thread(kthreadd, NULL, CLONE_FS | CLONE_FILES);
    rcu_read_lock();
    kthreadd_task = find_task_by_pid_ns(pid, &init_pid_ns);
    rcu_read_unlock();
    complete(&kthreadd_done);

    ......
```

# 3. kthreadd进程的时间循环

```c
int kthreadd(void *unused)
{
    struct task_struct *tsk = current;

    /* Setup a clean context for our children to inherit. */
    set_task_comm(tsk, "kthreadd");
    ignore_signals(tsk);
    set_cpus_allowed_ptr(tsk, cpu_all_mask);	   // 允许kthreadd在任意CPU上运行
    set_mems_allowed(node_states[N_MEMORY]);

    current->flags |= PF_NOFREEZE;

    for (;;) {
     		/* 首先将线程状态设置为TASK_INTERRUPTIBLE, 如果当前
            没有要创建的线程则主动放弃CPU完成调度.此进程变为阻塞态*/
            set_current_state(TASK_INTERRUPTIBLE);
            if (list_empty(&kthread_create_list))  // 没有需要创建的内核线程
                    schedule();					   // 什么也不做, 执行一次调度, 让出CPU

             /* 运行到此表示kthreadd线程被唤醒(就是我们当前)
            设置进程运行状态为 TASK_RUNNING */
            __set_current_state(TASK_RUNNING);

            spin_lock(&kthread_create_lock);					//  加锁访问kthread_create_list链表
            while (!list_empty(&kthread_create_list)) {
                    struct kthread_create_info *create;

					/*  从链表中取得 kthread_create_info 结构的地址，在上文中已经完成插入操作(将
                    kthread_create_info 结构中的 list 成员加到链表中，此时根据成员 list 的偏移
                    获得 create)  */
                    create = list_entry(kthread_create_list.next,
                                        struct kthread_create_info, list);

                    /* 完成穿件后将其从链表中删除 */
                    list_del_init(&create->list);

                    
                    spin_unlock(&kthread_create_lock);	
                    /* 完成真正线程的创建 */
                    create_kthread(create);

                    spin_lock(&kthread_create_lock);
            }
            spin_unlock(&kthread_create_lock);
    }

    return 0;
}
```
kthreadd的核心是for和while循环体。

在for循环中，如果发现kthread\_create\_list是一空链表，则调用schedule调度函数，因为此前已经将该进程的状态设置为TASK\_INTERRUPTIBLE，所以schedule的调用将会使当前进程进入睡眠（会将进程从CPU运行队列中移除，可以通过显式地唤醒即调用wakeup\_process()或者需要处理的信号来唤醒它）。

如果kthread\_create\_list不为空，则进入while循环，在该循环体中会遍历该thread\_create\_list列表，对于该列表上的每一个entry，都会得到对应的类型为struct kthread\_create\_info的节点的指针create。

然后函数在kthread\_create\_list中删除create对应的entry，接下来以create指针为参数调用create\_kthread(create)。


# 4. create_kthread完成内核线程的创建

```
static void create_kthread(struct kthread_create_info *create)
{
    int pid;

#ifdef CONFIG_NUMA
    current->pref_node_fork = create->node;
#endif
    /* We want our own signal handler (we take no signals by default). 
       其实就是调用首先构造一个假的上下文执行环境，最后调用 do_fork()
       返回进程 id, 创建后的线程执行 kthread 函数
    */
    pid = kernel_thread(kthread, create, CLONE_FS | CLONE_FILES | SIGCHLD);
    if (pid < 0) {
            /* If user was SIGKILLed, I release the structure. */
            struct completion *done = xchg(&create->done, NULL);

            if (!done) {
                    kfree(create);
                    return;
            }
            create->result = ERR_PTR(pid);
            complete(done);
    }
}
```
在create\_kthread()函数中，会调用kernel\_thread来生成一个新的进程，该进程的内核函数为kthread，调用参数为：
```c
    pid = kernel_thread(kthread, create, CLONE_FS | CLONE_FILES | SIGCHLD);c
```
我们可以看到，创建的内核线程执行的时间kthread。

此时回到kthreadd thread，它在完成了进程的创建后继续循环，检查kthread\_create\_list链表，如果为空，则threadd内核线程切换到睡眠。

那么我们现在回想我们的操作，我们在内核中通过kernel\_create或者其他方式创建一个内核线程，然后kthreadd线程被唤醒，来执行内核线程创建的真正工作，于是这里有三个线程：
1. kthreadd已经光荣的完成使命（接收执行真正的创建工作），睡眠。
2. 唤醒kthreadd的线程由于新创建的线程还没有创建完毕而继续睡眠（在kthread\_create函数中）。
3. 新创建的线程已经正在运行kthread，但是由于还有其他工作没有做所以还没有最终创建完成。

# 5. 新创建的内核线程kthread函数

```c
static int kthread(void *_create)
{
    /* Copy data: it's on kthread's stack 
     create 指向 kthread_create_info 中的 kthread_create_info */
    struct kthread_create_info *create = _create;
    
     /* 新的线程创建完毕后执行的函数 */
    int (*threadfn)(void *data) = create->threadfn;
    /* 新的线程执行的参数  */
    void *data = create->data;
    struct completion *done;
    struct kthread self;
    int ret;

    self.flags = 0;
    self.data = data;
    init_completion(&self.exited);
    init_completion(&self.parked);
    current->vfork_done = &self.exited;

    /* If user was SIGKILLed, I release the structure. */
    done = xchg(&create->done, NULL);
    if (!done) {
            kfree(create);
            do_exit(-EINTR);
    }
    /* OK, tell user we're spawned, wait for stop or wakeup
     设置运行状态为 TASK_UNINTERRUPTIBLE  */
    __set_current_state(TASK_UNINTERRUPTIBLE);

     /*  current 表示当前新创建的 thread 的 task_struct 结构  */
    create->result = current;
    complete(done);
    /*  至此线程创建完毕 ,  执行任务切换，让出 CPU  */
    schedule();

    ret = -EINTR;

    if (!test_bit(KTHREAD_SHOULD_STOP, &self.flags)) {
            __kthread_parkme(&self);
            ret = threadfn(data);
    }
    /* we can't just return, we must preserve "self" on stack */
    do_exit(ret);
}
```
线程创建完毕：
- 创建新thread的进程（原进程）恢复运行kthread\_create()并且返回新线程的任务描述符
- 新创建的线程由于执行了schedule()调度，此时并没有执行。
- 知道我们使用wake\_up\_process(p)唤醒新创建的线程。

线程被唤醒后，会接着执行threadfn(data):
```c
     ret = -EINTR;

    if (!test_bit(KTHREAD_SHOULD_STOP, &self.flags)) {
            __kthread_parkme(&self);
            ret = threadfn(data);
    }
    /* we can't just return, we must preserve "self" on stack */
    do_exit(ret);
```

# 6. 总结

kthreadd进程由idle通过kernel\_thread创建，并始终运行在内核空间，负责所有内核线程的调度和管理，它的任务就是管理和调度其他内核线程kernel\_thread，会循环执行一个kthreadd函数，该函数的作用就是运行kthread\_create\_list全局链表中维护的kthread，当我们调用kernel\_thread创建的内核线程会被加入到此链表中，因此所有的内核线程都是直接或间接的以kthreadd为父进程。

我们在内核中通过kernel\_creae或者其他方式创建一个内核线程，然后threadd内核线程被唤醒，来执行内核线程创建的真正工作，新的线程将执行kthread函数，完成创建工作，创建完毕后让出CPU，因此新的内核线程不会立刻运行，需要手工wake up，被唤醒后将执行自己的真正工作函数。
- 任何一个内核线程入口都是kthread()
- 通过kthread\_create()创建的内核线程不会立刻执行，需要手工wake up。
- 通过kthread\_create()创建的内核线程有可能不会执行对应线程函数threadfn而直接退出。