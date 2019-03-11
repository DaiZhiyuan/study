***Linux进程退出详解(do_exit)***

- 1 Linux进程的退出
    - 1.1 linux下进程的退出方式
        - 1.1.1 正常退出
        - 1.1.2 异常退出
    - 1.2 _exit，exit和_Exit的区别与联系
- 2 进程退出的系统调用
    - 2.1 _exit系统调用和exit_group系统调用
    - 2.2 系统调用声明
    - 2.3 系统调用号
    - 2.4 系统调用实现
- 3 do_group_exist流程
- 4 do_exit流程
    - 4.1 触发task_exit_notifier通知链实例的处理函数
    - 4.2 检查进程的blk_plug是否为空
    - 4.3 Oops消息
    - 4.4 设置进程可以使用的虚拟地址的上限（用户空间）
    - 4.5 检查进程设置进程PF_EXITING
    - 4.6 内存屏障
    - 4.7 获取current->mm->rss_stat.count[member]计数
    - 4.8 同步进程的mm的rss_stat
    - 4.9 清除定时器
    - 4.10 收集进程会计信息
    - 4.11 审计
    - 4.12 设置退出码及退出状态
    - 4.13 释放进程占用的资源
    - 4.14 调度其他进程

# 1. Linux进程的退出

## 1.1 linux下进程的退出方式

### 1.1.1 正常退出

正常退出有3种情况：
1. 从main函数返回
2. 调用exit库函数
3. 调用\_exit系统调用或\_Exit库函数

### 1.1.2 异常退出

异常退出有2中情况：
1. 调用abort库函数
2. 有收到特定信号而终止，例如：SIGTERM、SIGKILL。

## 1.2 _exit，exit和_Exit的区别与联系

- \_exit：是Linux系统调用：关闭所有文件描述符，然后退出进程。
- \_Exit：是C语言的库函数，从C99引入，等价于\_exit，即可以认为它直接调用\_exit。
- exit：是C语言库函数，它最终调用\_exit系统调用。在此之前，先刷新标准输出的缓冲，调用atexit注册的函数等，在C语言的main函数中，调用return就等价于调用exit。

基本来说，\_Exit或\_exit两者建议使用\_Exit，它是为fork之后的子进程准备的特殊API：</br>
由fork()函数创建的子进程分支里，正常情况下使用函数exit()是不正确的，这是因为使用它会导致stdio、stdout的缓冲区被清空两次，而且临时文件可能被意外删除。
因为在fork之后，exec之前，很多资源还是共享的（如某些文件描述符），如果使用exit会关闭这些资源，导致某些非预期的副作用（如删除临时文件等）。

# 2. 进程退出的系统调用

## 2.1 _exit系统调用和exit_group系统调用

进程退出由\_exit系统调用来完成，这使得内核有机会将该进程所使用的资源释放会系统中。
进程终止时，一般调用exit()库函数来释放进程所拥有的资源。

\_exit系统调用的入口点时sys\_exit()函数，需要一个错误码作为参数，以便退出进程。其定义是体系结构无关的，参见kernel/exit.c。

而我们用户空间的多线程程序，对应内核中就有多个进程，这些进程共享虚拟地址空间和资源。
它们有各自的进程ID（pid），但是它们的进程组id（tpid）是相同的，都等于组长（领头进程）的pid。

> 在Linux内核中对线程并没有做特殊的处理，还是由task\_struct来管理。所以从内核的角度看，用户态的线程本质上还是一个进程。对于一个进程（用户态角度）中不同的线程其tgid是相同的，但是pid各不相同。主线程即group\_leader（主线程会创建其他所有的子线程）。如果是单线程进程（用户态角度），它的pid等于tgid。

为什么还需要exit\_group？

我们如果了解Linux的线程实现机制的话，会知道所有的线程是属于一个线程组的，同时即使不是线程，Linux也允许多个进程组成进程组、多个进程组组成一个会话。因此我们本质上了解到不管是多线程还是进程组，其本质都是多个进程组成一个集合，那么我们的应用程序在退出的时候，自然希望一次性退出组内所有的进程。因此，exit\_group系统调用就诞生了！

exit\_group函数会杀死属于当前进程所在线程组的所有线程。它接收进程终止代码作为参数，进程终止代码可能是系统调用exit\_group（正常结束）指定的一个值，也可能是内核提供的一个错误码（异常结束）。

因此：
- C语言的库函数exit使用系统调用exit\_group来终止整个线程组，内核的入口函数为sys\_exit。
- 库函数pthread\_exit使用系统调用\_exit终止某一个线程，内核的入口函数为sys\_exit\_group。

## 2.2 系统调用声明

include/linux/syscall.h
```c
asmlinkage long sys_exit(int error_code);
asmlinkage long sys_exit_group(int error_code);
asmlinkage long sys_wait4(pid_t pid, int __user *stat_addr,
                int options, struct rusage __user *ru);
asmlinkage long sys_waitid(int which, pid_t pid,
               struct siginfo __user *infop,
               int options, struct rusage __user *ru);
asmlinkage long sys_waitpid(pid_t pid, int __user *stat_addr, int options);
```

## 2.3 系统调用号

系统调用号是一个体系结构相关的定义，但是多数体系结构定义如下，在include/uapi/asm-generic/unistd.h中：
```c
/* kernel/exit.c */
#define __NR_exit 93
__SYSCALL(__NR_exit, sys_exit)
#define __NR_exit_group 94
__SYSCALL(__NR_exit_group, sys_exit_group)
#define __NR_waitid 95
__SC_COMP(__NR_waitid, sys_waitid, compat_sys_waitid)
```

## 2.4 系统调用实现

```c
SYSCALL_DEFINE1(exit, int, error_code)
{
    do_exit((error_code&0xff)<<8);
}

/*
 * this kills every thread in the thread group. Note that any externally
 * wait4()-ing process will get the correct exit code - even if this
 * thread is not the thread group leader.
 */
SYSCALL_DEFINE1(exit_group, int, error_code)
{
    do_group_exit((error_code & 0xff) << 8);
    /* NOTREACHED */
    return 0;
}
```

# 3. do_group_exist流程

```c
/*
 * Take down every thread in the group.  This is called by fatal signals
 * as well as by sys_exit_group (below).
 */
void
do_group_exit(int exit_code)
{
    struct signal_struct *sig = current->signal;

    BUG_ON(exit_code & 0x80); /* core dumps don't get here */

    if (signal_group_exit(sig))
        exit_code = sig->group_exit_code;
    else if (!thread_group_empty(current)) {
        struct sighand_struct *const sighand = current->sighand;
        spin_lock_irq(&sighand->siglock);
        if (signal_group_exit(sig))
            /* Another thread got here before we took the lock.  */
            exit_code = sig->group_exit_code;
        else {
            sig->group_exit_code = exit_code;
            sig->flags = SIGNAL_GROUP_EXIT;
            zap_other_threads(current);
        }
        spin_unlock_irq(&sighand->siglock);
    }

    do_exit(exit_code);
    /* NOTREACHED */
}
```
do\_group\_exit()函数杀死属于current线程组的所有进程。它接受进程终止代码作为参数，进程终止代码可能是系统调用exit\_group()指定的一个值，也可能是内核提供的一个错误代码。

该函数执行下述操作：
1. 检查退出进程的SIGNAL\_GROUP\_EXIT标志是否不为0，如果不为0，说明内核已经开始为线程组执行退出的过程。这种情况下，就把存在current->signal->group\_exit\_code的值当作退出码，然后跳转到第4步。
2. 否则，设置进程的SIGNAL\_GROUP\_EXIT标志，并把终止代码放到current->signal->group\_exit\_code字段。
3. 调用zap\_other\_threads()函数杀死current线程组的其他进程。为了完成这个步骤，函数扫描当前线程所在线程组的链表，向表中所有除了current的进程发送SIGKILL信号，所有收到SIGKILL信号的进程都将执行do\_exit()函数，从而被杀死。

遍历线程所在线程组的所有线程函数while\_each\_thread(p, t) ：
```c
static inline struct task_struct *next_thread(const struct task_struct *p)
{
    return list_entry_rcu(p->thread_group.next,
                  struct task_struct, thread_group);
}

#define while_each_thread(g, t) \
    while ((t = next_thread(t)) != g)
```
同一个进程组的可以扫描与current->pids[PIDTYPE\_PGID]（这是进程组组长的pid结构体）对应的PIDTYPE\_PGID类型的散列表（因为是进程组组长，所以其真实的pid结构体中tasks[PIDTYPE\_PGID]是这个散列表的表头）中的每个PID链表。

4. 调用do\_exit()函数，把进程的终止代码传递给它。do\_exit()不再返回。

# 4. do_exit流程

进程终止所要完成的任务都是由do\_exit函数来处理，该函数定义在kernel/exit.c中：
```c
void do_exit(long code)
{
    struct task_struct *tsk = current;
    int group_dead;

    profile_task_exit(tsk);

    WARN_ON(blk_needs_flush_plug(tsk));

    if (unlikely(in_interrupt()))
        panic("Aiee, killing interrupt handler!");
    if (unlikely(!tsk->pid))
        panic("Attempted to kill the idle task!");

    /*
     * If do_exit is called because this processes oopsed, it's possible
     * that get_fs() was left as KERNEL_DS, so reset it to USER_DS before
     * continuing. Amongst other possible reasons, this is to prevent
     * mm_release()->clear_child_tid() from writing to a user-controlled
     * kernel address.
     */
    set_fs(USER_DS);

    ptrace_event(PTRACE_EVENT_EXIT, code);

    validate_creds_for_do_exit(tsk);

    /*
     * We're taking recursive faults here in do_exit. Safest is to just
     * leave this task alone and wait for reboot.
     */
    if (unlikely(tsk->flags & PF_EXITING)) {
        printk(KERN_ALERT
            "Fixing recursive fault but reboot is needed!\n");
        /*
         * We can do this unlocked here. The futex code uses
         * this flag just to verify whether the pi state
         * cleanup has been done or not. In the worst case it
         * loops once more. We pretend that the cleanup was
         * done as there is no way to return. Either the
         * OWNER_DIED bit is set by now or we push the blocked
         * task into the wait for ever nirwana as well.
         */
        tsk->flags |= PF_EXITPIDONE;
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule();
    }

    exit_signals(tsk);  /* sets PF_EXITING */
    /*
     * tsk->flags are checked in the futex code to protect against
     * an exiting task cleaning up the robust pi futexes.
     */
    smp_mb();
    raw_spin_unlock_wait(&tsk->pi_lock);

    if (unlikely(in_atomic()))
        printk(KERN_INFO "note: %s[%d] exited with preempt_count %d\n",
                current->comm, task_pid_nr(current),
                preempt_count());

    acct_update_integrals(tsk);
    /*
     * tsk->flags are checked in the futex code to protect against
     * an exiting task cleaning up the robust pi futexes.
     */
    smp_mb();
    raw_spin_unlock_wait(&tsk->pi_lock);

    if (unlikely(in_atomic()))
        printk(KERN_INFO "note: %s[%d] exited with preempt_count %d\n",
                current->comm, task_pid_nr(current),
                preempt_count());

    acct_update_integrals(tsk);
    /* sync mm's RSS info before statistics gathering */
    if (tsk->mm)
        sync_mm_rss(tsk->mm);
    group_dead = atomic_dec_and_test(&tsk->signal->live);
    if (group_dead) {
        hrtimer_cancel(&tsk->signal->real_timer);
        exit_itimers(tsk->signal);
        if (tsk->mm)
            setmax_mm_hiwater_rss(&tsk->signal->maxrss, tsk->mm);
    }
    acct_collect(code, group_dead);
    if (group_dead)
        tty_audit_exit();
    audit_free(tsk);

    tsk->exit_code = code;
    taskstats_exit(tsk, group_dead);

    exit_mm(tsk);

    if (group_dead)
        acct_process();
    trace_sched_process_exit(tsk);

    exit_sem(tsk);
    exit_shm(tsk);
    exit_files(tsk);
    exit_fs(tsk);
    if (group_dead)
        disassociate_ctty(1);
    exit_task_namespaces(tsk);
    exit_task_work(tsk);
    check_stack_usage();
    exit_thread();

    /*
     * Flush inherited counters to the parent - before the parent
     * gets woken up by child-exit notifications.
     *
     * because of cgroup mode, must be called before cgroup_exit()
     */
    perf_event_exit_task(tsk);

    sched_autogroup_exit_task(tsk);
    cgroup_exit(tsk, 1);

    module_put(task_thread_info(tsk)->exec_domain->module);

    proc_exit_connector(tsk);
    /*
     * FIXME: do that only when needed, using sched_exit tracepoint
     */
    ptrace_put_breakpoints(tsk);

    exit_notify(tsk, group_dead);
#ifdef CONFIG_NUMA
    task_lock(tsk);
    mpol_put(tsk->mempolicy);
    tsk->mempolicy = NULL;
    task_unlock(tsk);
#endif
#ifdef CONFIG_FUTEX
    if (unlikely(current->pi_state_cache))
        kfree(current->pi_state_cache);
#endif
    /*
     * Make sure we are holding no locks:
     */
    debug_check_no_locks_held();
    /*
     * We can do this unlocked here. The futex code uses this flag
     * just to verify whether the pi state cleanup has been done
     * or not. In the worst case it loops once more.
     */
    tsk->flags |= PF_EXITPIDONE;

    if (tsk->io_context)
        exit_io_context(tsk);

    if (tsk->splice_pipe)
        free_pipe_info(tsk->splice_pipe);

    if (tsk->task_frag.page)
        put_page(tsk->task_frag.page);

    validate_creds_for_do_exit(tsk);

    preempt_disable();
    if (tsk->nr_dirtied)
        __this_cpu_add(dirty_throttle_leaks, tsk->nr_dirtied);
    exit_rcu();

    /*
     * The setting of TASK_RUNNING by try_to_wake_up() may be delayed
     * when the following two conditions become true.
     *   - There is race condition of mmap_sem (It is acquired by
     *     exit_mm()), and
     *   - SMI occurs before setting TASK_RUNINNG.
     *     (or hypervisor of virtual machine switches to other guest)
     *  As a result, we may become TASK_RUNNING after becoming TASK_DEAD
     *
     * To avoid it, we have to wait for releasing tsk->pi_lock which
     * is held by try_to_wake_up()
     */
    smp_mb();
    raw_spin_unlock_wait(&tsk->pi_lock);

    /* causes final put_task_struct in finish_task_switch(). */
    tsk->state = TASK_DEAD;
    tsk->flags |= PF_NOFREEZE;  /* tell freezer to ignore us */
    schedule();
    BUG();
    /* Avoid "noreturn function does return".  */
    for (;;)
        cpu_relax();    /* For when BUG is null */
}
```

## 4.1 触发task_exit_notifier通知链实例的处理函数

```c
    profile_task_exit(tsk);
```
该函数会定义在触发kernel/profile.c
```c
void profile_task_exit(struct task_struct *task)
{
    blocking_notifier_call_chain(&task_exit_notifier, 0, task);
}
```
会触发task\_exit\_notifier通知，从而触发对应的处理函数。其中task\_exit\_notifier被定义如下：
```c
static BLOCKING_NOTIFIER_HEAD(task_exit_notifier);

#define BLOCKING_NOTIFIER_HEAD(name)                \
    struct blocking_notifier_head name =            \
        BLOCKING_NOTIFIER_INIT(name)

#define BLOCKING_NOTIFIER_INIT(name) {              \
        .rwsem = __RWSEM_INITIALIZER((name).rwsem), \
        .head = NULL }

#define __RWSEM_INITIALIZER(name)               \
    { __RWSEM_INIT_COUNT(name),             \
      .wait_list = SLIST_HEAD_INIT((name).wait_list),   \
      .wait_lock = __RAW_SPIN_LOCK_UNLOCKED(name.wait_lock) \
      __RWSEM_OPT_INIT(name)                \
      __RWSEM_DEP_MAP_INIT(name) }
```

## 4.2 检查进程的blk_plug是否为空

保证task\_struct中的plug字段是空的，或者pulg字段指向的队列是空的。plug字段的意义是stack plugging。
```c
WARN_ON(blk_needs_flush_plug(tsk));
```
其中blk\_needs\_flush\_plug函数定义在include/linux/blkdev.h中：
```c
static inline bool blk_needs_flush_plug(struct task_struct *tsk)
{
    struct blk_plug *plug = tsk->plug;

    return plug &&
        (!list_empty(&plug->list) ||
         !list_empty(&plug->mq_list) ||
         !list_empty(&plug->cb_list));
}
```

## 4.3 Oops消息

中断上下文不能执行do\_exit()函数，也不能终止PID为0的进程。
```c
    if (unlikely(in_interrupt()))
        panic("Aiee, killing interrupt handler!");
    if (unlikely(!tsk->pid))
        panic("Attempted to kill the idle task!");
```

## 4.4 设置进程可以使用的虚拟地址的上限（用户空间）

```c
/*
     * If do_exit is called because this processes oopsed, it's possible
     * that get_fs() was left as KERNEL_DS, so reset it to USER_DS before
     * continuing. Amongst other possible reasons, this is to prevent
     * mm_release()->clear_child_tid() from writing to a user-controlled
     * kernel address.
     */
    set_fs(USER_DS);
```
这是一个体系结构相关的代码，定义在arch/x86/include/asm/uaccess.h：
```c
#define set_fs(x)   (current_thread_info()->addr_limit = (x))
```

## 4.5 检查进程设置进程PF_EXITING

首先是检查PF\EXITING标识，此标识标识进程正在推出。

如果此标识已被设置，则进一步设置PF\_EXITPIDONE标识，并将进程的状态设置为不可中断状态TASK\_UNINTERRUPTIBLE，并进行一次进程调度。

```c
 /*
     * We're taking recursive faults here in do_exit. Safest is to just
     * leave this task alone and wait for reboot.
     */
    // current->flags的PF_EXITING标识标识进程正在被删除
    if (unlikely(tsk->flags & PF_EXITING)) {
        printk(KERN_ALERT
            "Fixing recursive fault but reboot is needed!\n");
        /*
         * We can do this unlocked here. The futex code uses
         * this flag just to verify whether the pi state
         * cleanup has been done or not. In the worst case it
         * loops once more. We pretend that the cleanup was
         * done as there is no way to return. Either the
         * OWNER_DIED bit is set by now or we push the blocked
         * task into the wait for ever nirwana as well.
         */
        // 设置进程标识为PF_EXITPIDONE
        tsk->flags |= PF_EXITPIDONE;
        // 设置进程状态为不可中断的等待状态
        set_current_state(TASK_UNINTERRUPTIBLE);
        // 调度其他进程
        schedule();
    }
```
如果此标识未被设置，则通过exit\_signals来设置：
``` c    
        // 设置PF_EXITING标志
        exit_signals(tsk);  /* sets PF_EXITING */
```

## 4.6 内存屏障

```c
    /*
     * tsk->flags are checked in the futex code to protect against
     * an exiting task cleaning up the robust pi futexes.
     */
    // 内存瓶装，用于确保在它之后的操作开始执行前，它之前的操作已经完成
    smp_mb();
    // 一直等待，直到获得current->pi_lock自旋锁
    raw_spin_unlock_wait(&tsk->pi_lock);
```

## 4.7 获取current->mm->rss_stat.count[member]计数

```c
    // 更新进程的运行时间，获取current->mm->rss\_stat.count[member]计数
    acct_update_integrals(tsk);
```
其定义在kernel/tsacct.c：
```c
/**
 * acct_update_integrals - update mm integral fields in task_struct
 * @tsk: task_struct for accounting
 */
void acct_update_integrals(struct task_struct *tsk)
{
    cputime_t utime, stime;
    unsigned long flags;

    local_irq_save(flags);
    task_cputime(tsk, &utime, &stime);
    __acct_update_integrals(tsk, utime, stime);
    local_irq_restore(flags);
}
```
其中task\_cputime获取了进程的cpu时间，即utime，stime。

\_\_acct\_update\_integrals定义如下：
```c
static void __acct_update_integrals(struct task_struct *tsk,
                    cputime_t utime, cputime_t stime)
{
    cputime_t time, dtime;
    u64 delta;

    if (!likely(tsk->mm))
        return;

    time = stime + utime;
    dtime = time - tsk->acct_timexpd;
    /* Avoid division: cputime_t is often in nanoseconds already. */
    delta = cputime_to_nsecs(dtime);

    if (delta < TICK_NSEC)
        return;

    tsk->acct_timexpd = time;
    /*
     * Divide by 1024 to avoid overflow, and to avoid division.
     * The final unit reported to userspace is Mbyte-usecs,
     * the rest of the math is done in xacct_add_tsk.
     */
    tsk->acct_rss_mem1 += delta * get_mm_rss(tsk->mm) >> 10;
    tsk->acct_vm_mem1 += delta * tsk->mm->total_vm >> 10;
}
```

## 4.8 同步进程的mm的rss_stat

```c
    /* sync mm's RSS info before statistics gathering */
        if (tsk->mm)
            sync_mm_rss(tsk->mm);
```

## 4.9 清除定时器

```c
    group_dead = atomic_dec_and_test(&tsk->signal->live);
    if (group_dead) {
        hrtimer_cancel(&tsk->signal->real_timer);
        exit_itimers(tsk->signal);
        if (tsk->mm)
            setmax_mm_hiwater_rss(&tsk->signal->maxrss, tsk->mm);
    }
```

## 4.10 收集进程会计信息

```c
    acct_collect(code, group_dead);
```

## 4.11 审计

```c
    if (group_dead)
        tty_audit_exit();
    audit_free(tsk);
```
## 4.12 设置退出码及退出状态

```c
    tsk->exit_code = code;
    taskstats_exit(tsk, group_dead);
```

## 4.13 释放进程占用的资源

- **释放线性区描述符和页表**
```c
    // 释放存储空间
    // 放弃进程占用的mm，如果没有其他今晨更实用该mm，则释放它。
    exit_mm(tsk);
```

- **输出进程会计信息**
```c
    if (group_dead)
        acct_process();
    trace_sched_process_exit(tsk);
```

- **释放用户空间的信号量**
```c
    exit_sem(tsk);
```
遍历current->sysvsem.undo\_list链表，并清除进程所涉及的每个IPC信号量的操作痕迹。

- **释放共享内存**
```c
    exit_shm(tsk);
```

- **释放文件对象相关资源**
```c
    // 释放已经打开的文件
    exit_files(tsk);
```

- **释放struct fs_struct结构体**

```c
    // 释放用于表示工作目录等结构
    exit_shm(tsk); 
```

- **脱离控制终端**
```c
    if (group_dead)
        disassociate_ctty(1);
```

- **释放命名空间**
```c
    exit_task_namespaces(tsk);
    exit_task_work(tsk);
```

- **检查由多少未使用的进程内核栈**
```c
    check_stack_usage();
```

- **释放task_struct中的thread_struct结构**
```c
    exit_thread();
```
触发thread\_notify\_head链表中所有通知链实例的处理函数，用于处理struct thread\_info结构体。

- **Performance Event功能相关资源的释放**
```c
    perf_event_exit_task(tsk);
```

- **释放Cgroup**
```c
    sched_autogroup_exit_task(tsk);
    cgroup_exit(tsk, 1);
```

- **注销断点**
```c
    ptrace_put_breakpoints(tsk);
```

- **进程事件连接器（通过它来报告进程fork、exec、exit以及进程用户ID与组ID的变化）**
```c
    proc_exit_connector(tsk);
```

- **更新所有子进程的父进程**
```c
    exit_notify(tsk, group_dead);
```

- **用于NUMA，当引用计数为0时，释放struct mempolicy结构体所占用的内存**
```c
#ifdef CONFIG_NUMA
    task_lock(tsk);
    mpol_put(tsk->mempolicy);
    tsk->mempolicy = NULL;
    task_unlock(tsk);
#endif
```
- **释放struct futex_pi_state结构体所占用的内存**
```c
#ifdef CONFIG_FUTEX
    if (unlikely(current->pi_state_cache))
        kfree(current->pi_state_cache);
#endif
```

- **释放struct io_context结构体所占用的内存**
```c
    if (tsk->io_context)
        exit_io_context(tsk);
```

- **释放与进程描述符splice_pipe字段相关的资源**
```c
    if (tsk->splice_pipe)
        free_pipe_info(tsk->splice_pipe);

    if (tsk->task_frag.page)
        put_page(tsk->task_frag.page);
```

## 4.14 调度其他进程

```c
    /* causes final put_task_struct in finish_task_switch(). */
    tsk->state = TASK_DEAD;
    tsk->flags |= PF_NOFREEZE;  /* tell freezer to ignore us */
    
    // 重新调度，因为该进程已经被设置成了僵死状态，因此永远都不会再把它调度回来运行了
    // 也就实现了do_exit不会有返回的目标
    schedule();
```
在设置了进程状态为TASK\_DEAD后，进程进入僵死状态，进程已经无法被再次调度， 因为对应用程序或者用户空间来说此进程已经死了，但是尽管进程已经不能再被调度，但系统还是保留了它的进程描述符。这样做是为了让系统有办法在进程终止后仍能获得它的信息。

在父进程获得已终止子进程的信息后，子进程的task\_struct结构体才被释放（包括此进程的内核栈）。
