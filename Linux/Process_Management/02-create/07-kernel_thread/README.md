***Linux内核线程kernel thread详解***

- 1 内核线程
    - 1.1 为什么需要内核线程
    - 1.2 内核线程概述
    - 1.3 内核线程的进程描述符task_struct
- 2 内核线程的创建
    - 2.1 创建内核线程接口的演变
    - 2.2 2号进程kthread的诞生
        - 2.2.1 WorkQueue机制
        - 2.2.2 2号进程kthreadd
    - 2.3 kernel_thread
    - 2.4 kthread_create
    - 2.5 kthread_run
- 3 内核线程的退出

# 1. 内核线程

## 1.1 为什么需要内核线程

Linux内核可以看做一个服务进程（管理软硬件资源，响应用户进程的种种合理以及不合理的请求）。

内核需要多个执行流并行，为了防止可能的阻塞，支持多线程是必要的。

内核线程就是内核的分身，一个分身可以处理一件特定事情。内核线程的调度由内核负责，一个内核线程处于阻塞状态时不影响其他的内核线程，因为其是调度的基本单位。

这与用户态线程是不一样的，因为内核线程只运行在内核态。因此，它只能使用大于PAGE\_OFFSET（传统的x86_32上是3G）的地址空间。

## 1.2 内核线程概述

内核线程是直接由内核本身启动的进程。内核线程实际上是将内核函数委托给独立的进程，它与内核中其他进程“并行”执行。内核线程通常被称之为内核守护进程。

它们执行下列任务：
- 周期性地将修改的内存页与页来源块设备同步
- 如果内存页很少使用，则写入交换区
- 管理延迟动作，如2号进程接手内核进程的创建
- 实现文件系统的事务日志

内核线程主要有两种类型：
1. 线程启动后一直等待，知道内核请求线程执行某一特定操作。
2. 线程启动后按周期性间隔运行，检测特定资源的使用，在用量超出或低于预置的限制时采取行动。

内核线程由内核自身生成，其特点在于：
1. 它们在内核态执行，而不是用户态。
2. 它们只可以访问虚拟地址空间的内核部分（高于TASK\_SIZE的所有地址），但不能访问用户空间。

```c
/*
 * User space process size: 3GB (default).
 */
#define TASK_SIZE           PAGE_OFFSET
#define TASK_SIZE_MAX       TASK_SIZE
```

## 1.3 内核线程的进程描述符task_struct

task\_struct进程描述符中包含两个和进程地址空间相关的字段：mm、active\_mm。
```c
struct task_struct

{
	......
	
	struct mm_struct *mm;
	struct mm_struct *avtive_mm;
	
	......
};
```

大多数计算机上系统的全部虚拟地址空间分为两部分：
- 提供用户态程序访问的虚拟地址空间。
- 提供内核访问的内核空间。

每当内核执行上下文切换时，虚拟地址空间的用户态部分都会切换，以便当前运行的进程匹配，
而内核空间不会发生切换（内核空间都是一样的）。

对于普通用户进程来说：mm指向虚拟地址空间的用户空间部分，而对于内核线程，mm为NULL。

这为优化提供了一些余地，可遁循所以的惰性TLB处理（Lazy TLB handing）。

active\_mm主要用于优化，由于内核线程不与任何特定的用户层相关，内核并不需要倒换虚拟空间的用户层部分，保留旧设置即可。由于内核线程之前可能是任何用户层进程在执行，故用户空间部分的内容本质上是随机的，内核线程决不能修改其内容，故将mm设置为NULL，同时如果切换出去的是用户进程，内核将原来进程的mm存放在新内核线程的actve\_mm中，因为某些时候内核必须知道用户空间当前包含了什么。

> 为什么没有mm指针的进程称为惰性TLB进程？
>
> 加入内核线程之后运行的进程与之前是同一个，在这个情况下，内核并不需要修改用户空间地址表。地址转换后备缓冲器（即TLB）中的信息仍然有效。只有在内核线程之后，执行的进程是与此前不同的用户层进程时，才需要切换（并对应清除TLB数据）。

内核线程和普通的进程间的区别在于内核线程没有独立的地址空间，mm指针被设置为NULL；
它只在内核空间运行，从来不切换到用户空间去；
并且和普通进程一样，可以被调度，可以被抢占。

# 2. 内核线程的创建

## 2.1 创建内核线程接口的演变

内核线程可以通过两种方式实现：

- 古老的接口kernel\_create和daemonize

将一个函数传递给kernel\_thread创建并初始化一个task，该函数接下来负责帮助内核调用daemonize转换为内核守护进程，daemonize随后完成一系列操作，如该函数释放其父进程的所有资源，不然这些资源会一直锁定到线程结束。阻塞信号的接收，将init用作守护进程的父进程。

- 现代的接口kthread\_create和kthread\_run

创建内核更常用的方法是辅助函数kthread\_create，该函数创建一个新的内核线程。最初线程是停止的，需要使用wake\_up\_process启动它。

使用kthread\_run，与kthread\_create不同的是，其创建新线程后立即唤醒它，其本质就是先用kthread\_create创建一个内核线程，然后通过wake\_up\_process唤醒它。

## 2.2 2号进程kthread的诞生

早期的kernel\_create和daemonize接口：
在早期的内核中，提供了kernel\_create和daemonize接口，但是这种机制操作复杂而且将所有的任务交给内核去完成。但是这种机制低效而且繁琐，将所有的操作赛给内核，我们创建内核线程的初衷本来就是为了分担内核部分工作，减少内核的开销的码？

### 2.2.1 WorkQueue机制

因此在Linux 2.6版本之后，提供了更加方便的接口kthread\_create和kthread\_run，同时将内核线程的创建操作延后，交给一个工作队列workqueue。

Linux中的workqueue机制就是为了简化内核线程的创建。通过kthread\_create并不真正创建内核线程，而是将创建工作create work插入到工作队列helper\_wq，随后调用workqueue的接口就能创建内核线程。
并且可以根据当前CPU的个数来选择创建线程的数量，使得线程处理的事务能够并行化。
workqueue是内核中实现简单而有效的机制，它显然简化了内核daemon的创建，方便了用户的编程。

> 工作队列（workqueue）是另外一种将工作推后执行的形式。工作队列可以把工作推后，交由一个内核线程去执行，也就是说，这个下半部可以在进程上下文中执行。最重要的就是工作队列允许被重新调度甚至睡眠。

### 2.2.2 2号进程kthreadd

但是这种方法依然看起来不够优美，我们为何不把这种创建内核线程的工作交给一个特殊的内核线程来做呢？

于是Linux 2.6.22，引入了kthreadd进程，并随后演变为2号进程，它在系统初始化时同1号进程一起被创建（当然肯定是通过kernel\_thread），参见rest\_init函数，并随后演变为创建内核线程的真正建造师，参见kthreadd函数，它会循环的查询工作链表static LIST\_HEAD(kthread\_create\_list)中是否有需要被创建的内核线程，而我们通过kthread\_create执行的操作时，只是在内核线程任务队列kthread\_create\_list中增加了一个create任务，然后会唤醒kthreadd进程来执行真正的创建工作。

内核线程会出现在系统进程列表中，但是在ps的输出中进程名command由方括号保卫，以便与普通进程区分。

如下所示，我们可以看到系统中，所有内核线程都用[]标识，而且这是父进程的PID都为2，而且2号进程kthreadd的父进程是0号进程。

```shell
[root@centos7]# ps -eo pid,ppid,command
  PID  PPID COMMAND
    1     0 /usr/lib/systemd/systemd --switched-root --system --deserialize 22
    2     0 [kthreadd]
    3     2 [ksoftirqd/0]
    5     2 [kworker/0:0H]
    7     2 [migration/0]
    8     2 [rcu_bh]
    9     2 [rcu_sched]
   10     2 [watchdog/0]
   11     2 [watchdog/1]
   12     2 [migration/1]
   13     2 [ksoftirqd/1]
   15     2 [kworker/1:0H]
   16     2 [watchdog/2]
   17     2 [migration/2]
   18     2 [ksoftirqd/2]
   20     2 [kworker/2:0H]
   21     2 [watchdog/3]
   22     2 [migration/3]
   23     2 [ksoftirqd/3]
   25     2 [kworker/3:0H]
   27     2 [kdevtmpfs]
   28     2 [netns]
   29     2 [khungtaskd]
   30     2 [writeback]
   31     2 [kintegrityd]
   32     2 [bioset]

    ......
```

## 2.3 kernel_thread

早期的内核中，kernel\_thread并不是使用统一的do\_fork这一封装好的接口实现的，它内部调用了更加低层的arch\_kernel\_thread创建了一个线程，但是这种方式创建的线程并不适合运行，因此内核提供了daemonize函数。

daemonize函数主要执行如下操作：
1. 释放其父进程的所有资源，不然这些资源会一直锁定知道线程结束。
2. 阻塞信号的接收
3. 将init用作守护进程的父进程

我们讲了这么多kernel\_thread，但是我们并不提倡使用它，因为这个是非常低层的创建内核的操作接口，使用kernel\_thread在内核中执行大量的操作，虽然创建的代码已经很小了，但是对于追求性能的Linux内核来说还是无法忍受。

因此我们只能说kernel\_thread是一个古老的接口，内核中的有些地方仍然在使用该方法，将一个函数直接传递给内核来创建内核线程。

于是Linux 3.x版本之后，有了更好的实现那就是延后内核的创建工作，将内核线程的创建工作交给一个内核线程来做，即kthreadd 2号进程。
但是在kthreadd还没创建之前，我们只能通过kernel\_thread这种方式去创建，同时kernel\_thread的实现也改为由do\_fork来实现。
```c
/*
 * Create a kernel thread.
 */
pid_t kernel_thread(int (*fn)(void *), void *arg, unsigned long flags)
{
    return do_fork(flags|CLONE_VM|CLONE_UNTRACED, (unsigned long)fn,
        (unsigned long)arg, NULL, NULL);
}
```

## 2.4 kthread_create

```c
struct task_struct *kthread_create_on_node(int (*threadfn)(void *data),
                       void *data,
                       int node,
                       const char namefmt[], ...);

#define kthread_create(threadfn, data, namefmt, arg...) \
    kthread_create_on_node(threadfn, data, -1, namefmt, ##arg)
```
创建内核线程更常用的方法是辅助函数kthread\_create，该函数创建一个新的内核线程。最初线程是终止的，需要使用wake\_up\_process唤醒它。

## 2.5 kthread_run

```c
/**
 * kthread_run - create and wake a thread.
 * @threadfn: the function to run until signal_pending(current).
 * @data: data ptr for @threadfn.
 * @namefmt: printf-style name for the thread.
 *
 * Description: Convenient wrapper for kthread_create() followed by
 * wake_up_process().  Returns the kthread or ERR_PTR(-ENOMEM).
 */
#define kthread_run(threadfn, data, namefmt, ...)              \
({                                     \
    struct task_struct *__k                        \
        = kthread_create(threadfn, data, namefmt, ## __VA_ARGS__); \
    if (!IS_ERR(__k))                          \
        wake_up_process(__k);                      \
    __k;                                   \
})
```
使用kthread\_run，与kthread\_create不同的是，其创建新线程后立即唤醒它，其本质就是先利用kthread\_create创建一个内核线程，然后通过wake\_up\_process唤醒它。

# 3. 内核线程的退出

线程一旦启动起来后，会一直运行。


主动退出，调用do\_exit函数：
```c
    void do_exit(long code);
```

被动退出，其他线程调用kthread\_stop函数：
```c
    int kthread_stop(struct task_struct *k);
```
kthread\_stop函数是通过向线程发送停止信号（KTHREAD\_SHOULD_STOP）。

如果线程函数正在处理一个非常重要的任务，它不会被信号中断的；
当然如果线程函数永远不返回并且不检查信号，它将永远都不会停止。

在执行kthread\_stop的时候，目标线程必须没有退出，否则会Oops。
原因很容易理解，当目标线程退出的时候，其对应的task结构也变的无效，kthread\_stop引用该无效task结构就会出错。

为了避免这种情况，确保线程没有退出，其方法如下代码所示：

```c
thread_func() {

    // 1. do work
    
    // 2. wait to exit
    while (!thread_could_stop()) {
        wait();
    }
}

exit_code() {
    kthread_stop(task);     // send to task
}
```
这种退出机制很温和，一切尽在thread_func()的掌控之中，线程在退出时可以从容地释放资源，而不是莫名其妙地被人“暗杀”。
