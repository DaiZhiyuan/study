***Linux进程调度之stop调度器类与stop_machine机制***

- 1 概述
- 2 stop调度类
- 3 stop_machine机制
    - 3.1 cpu_stop_work
    - 3.2 stop_one_cpu
    - 3.3 stop_machine
    - 3.4 stop_machine机制的应用

# 1. 概述

stop\_sched\_class调度类的进程是系统中优先级最高优先级的进程，其次才是dl\_sched\_class，再次是rt\_sched\_class。

stop\_sched\_class用于停止CPU，一般在SMP系统上使用，用以实现SMP load-balance和CPU Hotplug，这类有最高的调度优先级，如果你的系统没有定义CONFIG\_SMP，你可以试着将此类移除。

stop调度器类实现了Unix的stop\_machine机制（根据Unix风格，也可能是等效的其他特性）准备拼接新代码。

stop\_machine是一个通过信号：在SMP的情况下相当于暂时停止其他的CPU运行，它让一个CPU继续运行，而让所有其他CPU空闲，在单CPU的情况下这个东西就相当于关中断。

我的理解是：如果SMP架构CPU共享的东西需要修改，无法借助OS的lock，关中断等策略来实现这一功能，则需要stop\_machine。

# 2. stop调度类

stop调度器类是优先级最高的调度器类，定义在kernel/sched/stop\_task.c中：
```c
/*
 * Simple, special scheduling class for the per-CPU stop tasks:
 */
const struct sched_class stop_sched_class = {
    .next           = &dl_sched_class,

    .enqueue_task       = enqueue_task_stop,
    .dequeue_task       = dequeue_task_stop,
    .yield_task         = yield_task_stop,

    .check_preempt_curr     = check_preempt_curr_stop,

    .pick_next_task     = pick_next_task_stop,
    .put_prev_task      = put_prev_task_stop,

#ifdef CONFIG_SMP
    .select_task_rq     = select_task_rq_stop,
    .set_cpus_allowed       = set_cpus_allowed_common,
#endif

    .set_curr_task      = set_curr_task_stop,
    .task_tick          = task_tick_stop,

    .get_rr_interval    = get_rr_interval_stop,

    .prio_changed       = prio_changed_stop,
    .switched_to        = switched_to_stop,
    .update_curr        = update_curr_stop,
};
```
内核提供了sched\_set\_stop\_task函数用来将某个进程stop的调度器类设置为stop\_sched\_class，该函数定义在kernel/sched/core.c中：
```c
void sched_set_stop_task(int cpu, struct task_struct *stop)
{
    struct sched_param param = { .sched_priority = MAX_RT_PRIO - 1 };
    
    /*  获取到cpu上之前的stop进程  */
    struct task_struct *old_stop = cpu_rq(cpu)->stop;			

    if (stop)
    {
        /*
         * Make it appear like a SCHED_FIFO task, its something
         * userspace knows about and won't get confused about.
         *
         * Also, it will make PI more or less work without too
         * much confusion -- but then, stop work should not
         * rely on PI working anyway.
         */
        /*  使用SCHED_FIFO策略设置stop进程的调度信息  */
        sched_setscheduler_nocheck(stop, SCHED_FIFO, &param);  
        
        /*  设置stop进程的调度器类为stop_sched_class  */
        stop->sched_class = &stop_sched_class;		
    }
    
    /*  设置cpu的运行队列的stop进程为设置好的struct task_struct *stop */
    cpu_rq(cpu)->stop = stop;	
    
    /*  如果cpu的运行队列上之前有stop进程  */
    if (old_stop)						   
    {
        /*
         * Reset it back to a normal scheduling class so that
         * it can die in pieces.
         */
        /*  恢复cpu运行队列上之前的stop进程的调度器类为rt_sched_class  */
        old_stop->sched_class = &rt_sched_class;	
    }
}
```
sched\_set\_stop\_task把stop进程绑定为编号为CPU的处理器上的stop进程，进程的调度策略设置为SCHED\_FIFO，但是所属进程的调度类设置为stop\_sched\_class，这样当恢复进程的调度类时，值需要将进程的调度类设置为rt\_sched\_class即可。

# 3. stop_machine机制

内核中很少有地方使用了stop\_sched\_class，因为这个调度器类并不像dl\_sched\_class、rt\_sched\_class和fair\_sched\_class一样直接调度进程。相反，它用于完成stop\_machine机制，有关stop\_machine机制的实现都在include/linux/stop\_machine.h和kernel/stop\_machine.c中。

## 3.1 cpu_stop_work

struct cpu\_stop\_work是用完成stop\_machine工作的任务实体信息，它在SMP和非SMP结构下由不同的定义，参见include/linux/stop\_machine.h。

```c
#ifdef CONFIG_SMP

struct cpu_stop_work {
        struct list_head        list;      /* cpu_stopper->works */
        cpu_stop_fn_t           fn;		   /*  stop进程的工作函数  */
        void                    *arg;	   /*  stop进程工作函数的参数信息  */
        struct cpu_stop_done    *done;	   /*  额外辅助的完成情况, 包括返回值等信息 */ 
};


#else   /* CONFIG_SMP */

#include <linux/workqueue.h>

struct cpu_stop_work {
        struct work_struct      work;
        cpu_stop_fn_t           fn;
        void                    *arg;
};

#endif
```
在SMP系统中，migration用来执行任务迁移的一组进程，其comm字段为migration/%u，后面标识绑定的CPU编号。
```c
static struct smp_hotplug_thread cpu_stop_threads = {
    .store          = &cpu_stopper.thread,
    .thread_should_run  = cpu_stop_should_run,
    .thread_fn      = cpu_stopper_thread,
    .thread_comm        = "migration/%u",
    .create         = cpu_stop_create,
    .setup          = cpu_stop_unpark,
    .park           = cpu_stop_park,
    .pre_unpark     = cpu_stop_unpark,
    .selfparking        = true,
};

```
如下所示，我们可以显示出migration/0（当前系统的7号进程），即第0个CPU上任务迁移内核线程，该线程的调度策略是SCHED\_FIFO，但是所属的调度类为stop\_sched\_class。与我们之前讲解sched\_set\_stop\_task看到的内容一致。
```shell
[root@centos7 ~]# ps aux | grep migration
root         7  0.0  0.0      0     0 ?        S    Mar01   0:01 [migration/0]
root        12  0.0  0.0      0     0 ?        S    Mar01   0:01 [migration/1]
root        17  0.0  0.0      0     0 ?        S    Mar01   0:06 [migration/2]
root        22  0.0  0.0      0     0 ?        S    Mar01   0:06 [migration/3]

PID = 7，COMMAND = migration/0
flags = 0x4208040
Readl Time Process
rt_priority = 99
noarmal_prio = 0
prio =0 
POLICY = Read-Time FIFO SCHEDULER TASK
cpu_allow = 0x1
```

## 3.2 stop_one_cpu

在非SMP系统中，使用stop\_one\_cpu等一组函数来停止一个CPU的工作，其实质相当于关中断，定义在include/linux/stop\_machine.h中。

函数 | 描述
---|:---
stop\_one\_cpu | 停止CPU工作，关闭中断，并执行fn(arg)函数
stop\_one\_cpu\_nowait\_workfn | 开始一个任务完成fn(arg)的工作，而该函数无需等待fn工作的完成
stop\_one\_cpu\_nowait | 关闭中断，并执行fn(arg)函数，但不等待其完成
stop\_cpus | 同stop\_one\_cpu
try\_stop\_cpus | 同stop\_cpus

下面我们列出了，stop\_one\_cpu函数的实现：
```c
static inline int stop_one_cpu(unsigned int cpu, cpu_stop_fn_t fn, void *arg)
{
    int ret = -ENOENT;
    preempt_disable();
    if (cpu == smp_processor_id())
        ret = fn(arg);
    preempt_enable();
    return ret;
}
```
在SMP系统中，则实现了如下函数，声明在include/linux/stop\_machine.h，定义在kernel/stop\_machine.c中：
```c
int stop_one_cpu(unsigned int cpu, cpu_stop_fn_t fn, void *arg);
int stop_two_cpus(unsigned int cpu1, unsigned int cpu2, cpu_stop_fn_t fn, void *arg);
bool stop_one_cpu_nowait(unsigned int cpu, cpu_stop_fn_t fn, void *arg,
                         struct cpu_stop_work *work_buf);
int stop_cpus(const struct cpumask *cpumask, cpu_stop_fn_t fn, void *arg);
int try_stop_cpus(const struct cpumask *cpumask, cpu_stop_fn_t fn, void *arg);
```
下面是stop\_one\_cpu函数的SMP版本实现：
```c
int stop_one_cpu(unsigned int cpu, cpu_stop_fn_t fn, void *arg)
{
    struct cpu_stop_done done;
    struct cpu_stop_work work = { .fn = fn, .arg = arg, .done = &done };

    cpu_stop_init_done(&done, 1);
    cpu_stop_queue_work(cpu, &work);
    wait_for_completion(&done.completion);
    return done.executed ? done.ret : -ENOENT;
}
```

## 3.3 stop_machine

```c
#if defined(CONFIG_SMP) || defined(CONFIG_HOTPLUG_CPU)

int stop_machine(cpu_stop_fn_t fn, void *data, const struct cpumask *cpus);
int stop_machine_from_inactive_cpu(cpu_stop_fn_t fn, void *data,
                                   const struct cpumask *cpus);
#else   /* CONFIG_SMP || CONFIG_HOTPLUG_CPU */

static inline int stop_machine(cpu_stop_fn_t fn, void *data,
                                 const struct cpumask *cpus)
{
        unsigned long flags;
        int ret;
        local_irq_save(flags);
        ret = fn(data);
        local_irq_restore(flags);
        return ret;
}

static inline int stop_machine_from_inactive_cpu(cpu_stop_fn_t fn, void *data,
                                                 const struct cpumask *cpus)
{
        return stop_machine(fn, data, cpus);
}

#endif
```

## 3.4 stop_machine机制的应用

一般来说，内核会在如下情况下使用stop\_machine技术：

应用 | 描述
---|---
module install and remove | 增加删除模块，在不需要重启内核的情况下，加载和删除模块
cpu hotplug | CPU的热插拔，用以执行任务迁移的工作，cpu\_stop\_threads，</br>该任务由CPU绑定的migration内核线程来完成
memory hotplug | Memory的热插拔
ftrace | 内核trace，debug功能
hwlat\_detector | 检测系统硬件引入的latency，debug功能
kernel hotpatch | Ksplice可以在不到一秒时间里动态地应用内核补丁, 不需要重新引导
