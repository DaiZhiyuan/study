***Linux进程上下文切换过程context_switch详解***

- 1 进程上下文
    - 1.1 进程上下文的概念
    - 1.2 上下文切换
- 2 context_switch进程上下文切换
    - 2.1 context_switch注释
    - 2.2 prepare_arch_switch切换前的准备工作
    - 2.3 next是内核线程时的处理（惰性TLB）
    - 2.4 switch_mm切换进程虚拟地址空间
    - 2.5 prev是内核线程时的处理
    - 2.6 switch_to完成进程切换
        - 2.6.1 switch_to函数
        - 2.6.2 为什么switch_to需要3个参数
        - 2.6.3 switch_to函数注释
        - 2.7 barrier屏障同步
        - 2.8 finish_task_switch完成清理工作

# 1. 进程上下文

## 1.1 进程上下文的概念

操作系统管理很多进程的执行。有些进程是来自各个程序、系统和应用程序的独立进程，而某些进程来自被分解为很多进程的应用和程序。当一个进程从内核中移出，另一个进程称为活动的，这些进程之间发生了上下文切换。

操作系统必须记录重启进程和启动新进程使之活动所需要的所有信息。这些信息被称为上下文，它描述了进程现有的状态，进程上下文时可执行程序代码是进程的重要组成部分，实际上是进程执行活动全过程的静态描述，可以看做是用户进程传递给内核的这些参数以及内核要保存的那一整套的变量、寄存器值和当时的运行环境等。

进程的上下文信息包括：指向可执行文件的指针、栈、内存（数据段和堆）、进程状态、优先级、程序I/O的状态、授予权限、调度信息、审计信息、有关的信息（文件描述符读/写指针）、有关事件和信号的信息、寄存器组（栈指针，指令计数器）等等，诸如此类。

处理器总处于以下三种状态之一：
1. 内核态，运行于进程上下文
2. 内核态，运行于中断上下文
3. 用户态，运行于用户空间

用户空间的应用程序，通过系统调用进入内核空间。这时候用户空间的进程要传递很多变量、参数的值给内核，内核态运行的时候也要保存用户进程的一些寄存器值、变量等。所谓的“进程上下文”。

硬件通过触发信号，导致内核调用中断处理程序，进入内核空间。这个过程中，硬件的一些变量和参数也要传递给内核，内核通过这些参数进行中断处理。所谓的“中断上下文”，其实也可以看作就是硬件传递过来的这些参数和内核需要保存的一些其他环境（主要是当前被打断执行的进程环境）。

> Linux完全注释中的一段话：
>
> 当一个进程在执行时，CPU的所有寄存器中的值、进程的状态、退栈中的内容被称为该进程的上下文。当内核需要切换到另一个进程时，它需要保存当前进程的所有状态，即保存当前进程的上下文，以便再次执行该进程时，必须能够恢复切换时的状态执行下去。在Linux中，当前进程上下文均保存在进程的任务数据结构中。在发生中断时，内核就在被中断进程的上下文中，在内核态下执行中断服务例程。但同时会保留所有需要用的资源，以便中断服务结束能恢复被中断的进程执行。

## 1.2 上下文切换

进程被抢占CPU时候，操作系统保存其上下文信息，同时将新的活动进程的上下文信息加载近来。这个过程就是上下文切换。而当一个被抢占的进程再次成为活动的，它可以恢复自己的上下文继续从抢占的位置开始执行。

上下文切换有时也被称做进程切换或任务切换，是指CPU从一个进程或线程切换到另一个进程或线程。

稍微详细描述一下，上下文切换可以认为是内核在CPU上对进程进行以下的活动：
1. 挂起一个进程，将这个进程在CPU中的状态（上下文）存储在内存中的某处
2. 在内存中检索下一个进程的上下文并将其CPU的寄存器中恢复
3. 跳转到程序计数器所指向的位置（即跳转到进程被中断时的代码行），以恢复该进程

因此上下文是指：某一时间点CPU寄存器和程序计数器的内容，广义上海包括内存中进程的虚拟地址映射信息。

上下文切换只能发生在内核态中，上下文切换通常是CPU密集型的。也就是说，它需要相当可观的处理器事件，在每秒几十上百次的切换中，每次切换都需要纳秒量级的时间。所以，上下文切换对系统来说意味着消耗大量的CPU时间，事实上，可能是操作系统中时间消耗最大的操作。

Linux相比其他操作系统有很多的优点，其中有一项就是，其上下文切换和模式切换的时间消耗非常少。

# 2. context_switch进程上下文切换

Linux中进程调度时，内核在选择新进程之后进行抢占时，通过context\_switch完成进程上下文切换。

> 进程调度与抢占的区别：进程调度不一定发生抢占，但是抢占时一定发生了调度。
>
> 在进程发生调度时，只有当前内核发生当前进程因为主动或者被动需要放弃CPU时，内核才会选择一个与当前进程不同的进程来抢占CPU。

context\_switch其实是一个分配器，它会调用所需的特定体系结构的方法：
- 调用switch\_mm()，把虚拟内存从一个进程映射切换到新进程中。
    - switch\_mm更换通过task\_struct->mm描述的内存管理上下文，该工作的细节取决于处理器，主要包括：加载页表、刷新TLB（部分或者全部），向内存管理单元（MMU）提供新的信息。
- 调用switch\_to()，从上一个进程的处理器状态切换到新进程的处理器状态。这包括：保存、恢复栈信息和寄存器信息。
    - switch\_to切换处理器寄存器的内容和内核栈（虚拟地址空间的用户部分已经通过switch\_mm变更，其中也包括了用户态的栈，因此switch\_to不需要变更用户栈，只需变更内核栈），这段代码严重依赖于体系结构，且代码通常都是用汇编语言编写。

context\_switch函数建立next进程的地址空间。进程描述符的active\_mm字段指向进程所使用的内存描述符，而mm字段指向进程所拥有的内存描述符。对于一般的进程，这两个字段由相同的地址，但是，内核线程没有它自己的地址空间，而且它的mm字段总是被设置为NULL。active\_mm成员被初始化为前一个运行进程的mm值，如果当内核线程被调度之前运行的内核线程也是另外一个内核线程的时候，那么其mm和active\_mm都是NULL。

context\_switch()函数保证：如果next是一个内核线程，它使用prev所使用的地址空间。

由于不同体系结构下地址映射的机制有所区别，寄存器等信息也是依赖于体系结构的，因此switch\_mm和switch\_to两个函数均是体系结构相关的。

## 2.1 context_switch注释

context\_switch定义在kernel/sched/core.c，如下所示：
```c
/*
 * context_switch - switch to the new MM and the new thread's register state.
 */
static __always_inline struct rq *
context_switch(struct rq *rq, struct task_struct *prev,
           struct task_struct *next)
{
    struct mm_struct *mm, *oldmm;

    /*  完成进程切换的准备工作  */
    prepare_task_switch(rq, prev, next);

    mm = next->mm;
    oldmm = prev->active_mm;
    /*
     * For paravirt, this is coupled with an exit in switch_to to
     * combine the page table reload and the switch backend into
     * one hypercall.
     */
    arch_start_context_switch(prev);

    /*  如果next是内核线程，则线程使用prev所使用的地址空间
     *  schedule( )函数把该线程设置为懒惰TLB模式
     *  内核线程并不拥有自己的页表集(task_struct->mm = NULL)
     *  它使用一个普通进程的页表集
     *  不过，没有必要使一个用户态线性地址对应的TLB表项无效
     *  因为内核线程不访问用户态地址空间。
    */
    if (!mm)        /*  内核线程无虚拟地址空间, mm = NULL*/
    {
        /*  内核线程的active_mm为上一个进程的mm
         *  注意此时如果prev也是内核线程,
         *  则oldmm为NULL, 即next->active_mm也为NULL  */
        next->active_mm = oldmm;
        /*  增加mm的引用计数  */
        atomic_inc(&oldmm->mm_count);
        /*  通知底层体系结构不需要切换虚拟地址空间的用户部分
         *  这种加速上下文切换的技术称为惰性TBL  */
        enter_lazy_tlb(oldmm, next);
    }
    else  /* 不是内核线程, 则需要切切换虚拟地址空间 */
        switch_mm(oldmm, mm, next);

    /*  如果prev是内核线程或正在退出的进程
     *  就重新设置prev->active_mm
     *  然后把指向prev内存描述符的指针保存到运行队列的prev_mm字段中
     */
    if (!prev->mm)
    {
        /*  将prev的active_mm赋值和为空  */
        prev->active_mm = NULL;
        /*  更新运行队列的prev_mm成员  */
        rq->prev_mm = oldmm;
    }
    /*
     * Since the runqueue lock will be released by the next
     * task (which is an invalid locking op but in the case
     * of the scheduler it's an obvious special-case), so we
     * do an early lockdep release here:
     */
    lockdep_unpin_lock(&rq->lock);
    spin_release(&rq->lock.dep_map, 1, _THIS_IP_);

    /* Here we just switch the register state and the stack. 
     * 切换进程的执行环境, 包括堆栈和寄存器
     * 同时返回上一个执行的程序
     * 相当于prev = witch_to(prev, next)  */
    switch_to(prev, next, prev);
    
    /*  switch_to之后的代码只有在
     *  当前进程再次被选择运行(恢复执行)时才会运行
     *  而此时当前进程恢复执行时的上一个进程可能跟参数传入时的prev不同
     *  甚至可能是系统中任意一个随机的进程
     *  因此switch_to通过第三个参数将此进程返回
     */
    

    /*  路障同步, 一般用编译器指令实现
     *  确保了switch_to和finish_task_switch的执行顺序
     *  不会因为任何可能的优化而改变  */
    barrier();  

    /*  进程切换之后的处理工作  */
    return finish_task_switch(prev);
}
```

## 2.2 prepare_arch_switch切换前的准备工作

在进程切换之前，首先执行调用每个体系结构都必须定义的prepare\_task\_switch hook函数，这使得内核执行特定于体系结构的代码，为切换做事前准备。

```c
    struct mm_struct *mm, *oldmm;
    
    /*  完成进程切换的准备工作  */
    prepare_task_switch(rq, prev, next);	
```
prepare\_task\_switch函数定义在kernel/sched/core.c，如下所示：
```c
/**
 * prepare_task_switch - prepare to switch tasks
 * @rq: the runqueue preparing to switch
 * @prev: the current task that is being switched out
 * @next: the task we are going to switch to.
 *
 * This is called with the rq lock held and interrupts off. It must
 * be paired with a subsequent finish_task_switch after the context
 * switch.
 *
 * prepare_task_switch sets up locking and calls architecture specific
 * hooks.
 */
static inline void
prepare_task_switch(struct rq *rq, struct task_struct *prev,
            struct task_struct *next)
{
    sched_info_switch(rq, prev, next);
    perf_event_task_sched_out(prev, next);
    fire_sched_out_preempt_notifiers(prev, next);
    prepare_lock_switch(rq, next);
    prepare_arch_switch(next);
}
```

## 2.3 next是内核线程时的处理（惰性TLB）

由于用户空间进程的寄存器内容在陷入内核态时保存在内核栈中，在上下文切换期间无需显式操作。
而因为每个进程首先都是从内核态开始执行（在调度期间控制权传递给新进程），在返回用户空间时，会使用内核栈上保存的值自动回复寄存器数据。

> 注意：内核线程没有自身的用户空间上下文，其task\_struct->mm为NULL，从当前进程“借来”的地址空间记录在active\_mm中。

```c
/*  如果next是内核线程，则线程使用prev所使用的地址空间
 *  schedule( )函数把该线程设置为懒惰TLB模式
 *  内核线程并不拥有自己的页表集(task_struct->mm = NULL)
 *  它使用一个普通进程的页表集
 *  不过，没有必要使一个用户态线性地址对应的TLB表项无效
 *  因为内核线程不访问用户态地址空间。
*/

/*  内核线程无虚拟地址空间, mm = NULL*/
if (!mm)        
{
    /*  内核线程的active_mm为上一个进程的mm
     *  注意此时如果prev也是内核线程,
     *  则oldmm为NULL, 即next->active_mm也为NULL  */
    next->active_mm = oldmm;
    
    /*  增加mm的引用计数  */
    atomic_inc(&oldmm->mm_count);
    
    /*  通知底层体系结构不需要切换虚拟地址空间的用户部分
     *  这种加速上下文切换的技术称为惰性TLB  */
    enter_lazy_tlb(oldmm, next);
}
else            
    /*  不是内核线程, 则需要切切换虚拟地址空间  */
    switch_mm(oldmm, mm, next);
```
其中，enter\_lazy\_tlb通知低层体系结构不需要切换虚拟地址空间的用户空间部分，这种加速上下文切换的计数称之为惰性TLB。

## 2.4 switch_mm切换进程虚拟地址空间

switch\_mm主要完成了进程prev到next虚拟地址空间的映射，由于内核虚拟地址空间是不许切换的，因此切换的主要是用户态虚拟地址空间。

在x86架构下的switch\_mm函数，其定义在arch/x86/include/asm/mmu\_context.h中：
```c
static inline void switch_mm(struct mm_struct *prev, struct mm_struct *next,
                 struct task_struct *tsk)
{
    unsigned cpu = smp_processor_id();


    /*  确保prev和next不是同一进程  */
    if (likely(prev != next))
    {
#ifdef CONFIG_SMP
        /*  刷新cpu地址转换后备缓冲器TLB  */
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        this_cpu_write(cpu_tlbstate.active_mm, next);
#endif
        /*  设置当前进程的mm->cpu_vm_mask表示其占用cpu  */
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
         * 将新进程的pgd页目录表填写到cpu的cr3寄存器中
         */
        load_cr3(next->pgd);

        trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);

        /* Stop flush ipis for the previous mm 
         * 除prev的cpu_vm_mask，表示prev放弃使用cpu  */
        cpumask_clear_cpu(cpu, mm_cpumask(prev));

        /* Load per-mm CR4 state
         */
        load_mm_cr4(next);

#ifdef CONFIG_MODIFY_LDT_SYSCALL
        /*
         * Load the LDT, if the LDT is different.
         *
         * It's possible that prev->context.ldt doesn't match
         * the LDT register.  This can happen if leave_mm(prev)
         * was called and then modify_ldt changed
         * prev->context.ldt but suppressed an IPI to this CPU.
         * In this case, prev->context.ldt != NULL, because we
         * never set context.ldt to NULL while the mm still
         * exists.  That means that next->context.ldt !=
         * prev->context.ldt, because mms never share an LDT.
         *
         * 
         */
        if (unlikely(prev->context.ldt != next->context.ldt))
            load_mm_ldt(next);
#endif
    }
#ifdef CONFIG_SMP
    else
    {
        this_cpu_write(cpu_tlbstate.state, TLBSTATE_OK);
        BUG_ON(this_cpu_read(cpu_tlbstate.active_mm) != next);

        if (!cpumask_test_cpu(cpu, mm_cpumask(next)))
        {
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
            trace_tlb_flush(TLB_FLUSH_ON_TASK_SWITCH, TLB_FLUSH_ALL);
            load_mm_cr4(next);
            load_mm_ldt(next);
        }
    }
#endif
}
```

## 2.5 prev是内核线程时的处理

如果前一个进程prev是内核线程（即prev->mm为NULL），则其active\_mm指针必须重置为NULL，已断开与其之前借用的地址空间的联系，而当prev重新被调度的时候，此时它成为next，会在前面next是内核线程时的处理处重新用`next->active\_mm = oldmm; `赋值。

```c
/*  如果prev是内核线程或正在退出的进程
 *  就重新设置prev->active_mm
 *  然后把指向prev内存描述符的指针保存到运行队列的prev_mm字段中
 */
if (!prev->mm)
{
    /*  将prev的active_mm赋值和为空  */
    prev->active_mm = NULL;
    /*  更新运行队列的prev_mm成员  */
    rq->prev_mm = oldmm;
}
```

## 2.6 switch_to完成进程切换

### 2.6.1 switch_to函数

最后用switch\_to完成进程的切换，该函数切换了寄存器状态和栈，新进程在该调用后开始执行，而switch\_to之后的代码只有在当前进程下一次被选择运行时才会执行。

执行环境的切换时在switch\_to()中完成的，switch\_to完成最终的进程切换，它保存原进程的所有寄存器信息，恢复新进程的所有寄存器信息，并执行新的进程。

该函数往往通过宏来显示，其原型声明如下：
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
2. 硬件上下文，设置eip寄存器的值，并jmp到\_\_switch\_to函数
3. 堆栈切换，即ebp的切换，ebp是栈底指针，它确定了当前进程属于哪个进程

\_\_switch\_to函数：
```c
__notrace_funcgraph struct task_struct *
__switch_to(struct task_struct *prev_p, struct task_struct *next_p)
{
    struct thread_struct *prev = &prev_p->thread,
                 *next = &next_p->thread;
    int cpu = smp_processor_id();
    struct tss_struct *tss = &per_cpu(init_tss, cpu);
    fpu_switch_t fpu;

    /* never put a printk in __switch_to... printk() calls wake_up*() indirectly */

    fpu = switch_fpu_prepare(prev_p, next_p, cpu);

    /*
     * Reload esp0.
     */
    load_sp0(tss, next);

    /*
     * Save away %gs. No need to save %fs, as it was saved on the
     * stack on entry.  No need to save %es and %ds, as those are
     * always kernel segments while inside the kernel.  Doing this
     * before setting the new TLS descriptors avoids the situation
     * where we temporarily have non-reloadable segments in %fs
     * and %gs.  This could be an issue if the NMI handler ever
     * used %fs or %gs (it does not today), or if the kernel is
     * running inside of a hypervisor layer.
     */
    lazy_save_gs(prev->gs);
    
    /*
     * Load the per-thread Thread-Local Storage descriptor.
     */
    load_TLS(next, cpu);

    /*
     * Restore IOPL if needed.  In normal use, the flags restore
     * in the switch assembly will handle this.  But if the kernel
     * is running virtualized at a non-zero CPL, the popf will
     * not restore flags, so it must be done in a separate step.
     */
    if (get_kernel_rpl() && unlikely(prev->iopl != next->iopl))
        set_iopl_mask(next->iopl);

    /*
     * Now maybe handle debug registers and/or IO bitmaps
     */
    if (unlikely(task_thread_info(prev_p)->flags & _TIF_WORK_CTXSW_PREV ||
             task_thread_info(next_p)->flags & _TIF_WORK_CTXSW_NEXT))
        __switch_to_xtra(prev_p, next_p, tss);

    /*
     * Leave lazy mode, flushing any hypercalls made here.
     * This must be done before restoring TLS segments so
     * the GDT and LDT are properly updated, and must be
     * done before math_state_restore, so the TS bit is up
     * to date.
     */
    arch_end_context_switch(next_p);

    /*
     * Restore %gs if needed (which is common)
     */
    if (prev->gs | next->gs)
        lazy_load_gs(next->gs);

    switch_fpu_finish(next_p, fpu);

    this_cpu_write(current_task, next_p);

    /* Load the Intel cache allocation PQR MSR. */
    intel_rdt_sched_in();

    return prev_p;
}
```

### 2.6.2 为什么switch_to需要3个参数

调度过程可能选择一个新的进程，而清理工作则是针对此前的活动进程，请注意，这不是发起上下文切换的那个进程，而是系统中随机的某个其他进程，内核必须办法使得进程能够与context\_switch例程通信，这就可以通过switch\_to宏实现。因此switch\_to函数通过3个参数提供2个变量。

在新进程被选中时，低层的进程切换例程必须将此前执行的进程提供给context\_switch，由于控制流会回到该函数的中间，这无法用普通函数的返回值来做到，因此提供了3个参数的宏。

我们考虑这样一个例子，假定多个进程A、B、C在系统上运行。在某个时间点，内核决定从进程A切换到进程B，此时`prev=A，next=B`，即执行了`switch_to(A, B)`，而后当被抢占的进程A再次被选择执行的时候，系统可能进行了多次进程切换/抢占（至少会经历一次即再次从B到A），假设A再次被选择执行时，当前活动进程是C，即此时`prev=C，next =A`。

在每个switch\_to被调用的时候，prev和next指针位于各个进程的内核栈中，prev指向了当前运行的进程，而next指向了将要运行的下一个进程，那么为了执行从prev到next的切换，switch\_to使用前两个参数prev和next就够了。

在进程A被选中再次执行的时候，会出现一个问题，此时控制权将回到A，switch\_to函数返回，内核开始执行switch\_to之后的点，此时内核栈准备的恢复到切换之前的状态，即进程A上次被切换出去的状态，`prev=A，next=B`。此时，内核无法知道实际上在进程A之前运行的是进程C。

因此，在新进程被选中执行时，内核恢复到进程被切换出去的点继续执行，此时内核只知道谁之前将新进程抢占了（上面例子就是知道了B抢占的是A），但是却不知道新进程再次执行时抢占了谁（A的再次执行时抢占了C，但是因为是状态恢复到原有的点，所以不会知道A这里抢占的是C），因此低层的进程切换机制必须将此前执行的进程（即新进程抢占的哪个进程）提供给context\_switch。由于控制流会回到函数的中间。因此无法通过普通函数的返回值来完成。因此使用了一个3个参数，但是逻辑效果是相同的，仿佛是swich\_to是带有两个参数的函数，而且反悔了一个指向当前运行进程的指针。

> switch_to(prev, next, last) 即 prev = last = switch_to(prev, next)

其中返回的prev值并不是做参数的prev值，而是prev被再次调度的时候抢占掉的哪个进程last（进程C）。

在上个例子中，进程A提供给switch\_to的参数是`prev=A，next=B`，然后控制权从A交给了B，但是恢复执行的时候是通过`prev=C, next=A`完成了再次调度，而后内核恢复了进程A被切换之前的内核栈信息，即`prev=A，next=B`。内核为了通知调度机制A抢占了C的处理器，就通过last参数传递回来，`prev = last = C`。

内核实现该行为特性依赖于低层的体系结构，但内核显然可以通过考虑两个进程的内核栈来重建所需要的信息。

### 2.6.3 switch_to函数注释

switch\_mm()进行用户空间的切换，更确切地说，是切换地址转换表（PGD），由于PGD包括：内核虚拟地址空间和用户虚拟地址空间的映射。Linux内核把进程的整个虚拟地址空间分成两部分，一部分是内核虚拟地址空间，另一部分是用户虚拟地址空间。各个进程的虚拟地址空间各不相同，但是却共用了同样的内核地址空间，这样在进程切换的时候，就置需要切换虚拟地址空间的用户空间部分。

每个进程都有其自身的页目录表pgd。

进程本身尚未切换，而存储管理机制的页目录指针cr3却已经切换了，这样不会造成问题吗？
不会的，因为这个时候CPU是内核空间运行，而所有进程的页目录表中与系统空间对应的页目录项都指向相同的页表，所以，不管切换到那一个进程的页目录表都是一样的，受影响的只是用户空间，内核空间的映射则永远不变。

我们分析一下x86 32位下switch\_to函数，其定义在arch/x86/include/asm/switch\_to.h中。
```c
/*
 * Saving eflags is important. It switches not only IOPL between tasks,
 * it also protects other tasks from NT leaking through sysenter etc.
 */
#define switch_to(prev, next, last)                                     \
do {                                                                    \
        /*                                                              \
         * Context-switching clobbers all registers, so we clobber      \
         * them explicitly, via unused output variables.                \
         * (EAX and EBP is not listed because EBP is saved/restored     \
         * explicitly for wchan access and EAX is the return value of   \
         * __switch_to())                                               \
         */                                                             \
        unsigned long ebx, ecx, edx, esi, edi;                          \
                                                                        \
        asm volatile("pushfl\n\t" /* save flags 保存就的ebp、和flags寄存器到旧进程的内核栈中*/   \
                     "pushl %%ebp\n\t"          /* save    EBP   */     \
                     "movl %%esp,%[prev_sp]\n\t"        /* save ESP  将旧进程esp保存到thread_info结构中 */ \
                     "movl %[next_sp],%%esp\n\t"        /* restore ESP 用新进程esp填写esp寄存器，此时内核栈已切换  */ \
                     "movl $1f,%[prev_ip]\n\t"  /* save EIP 将该进程恢复执行时的下条地址保存到旧进程的thread中*/     \
                     "pushl %[next_ip]\n\t"     /* restore EIP 将新进程的ip值压入到新进程的内核栈中 */     \
                     __switch_canary                                    \
                     "jmp __switch_to\n"        /* regparm call  */     \
                     "1:\t"                                             \
                     "popl %%ebp\n\t"           /* restore EBP 该进程执行，恢复ebp寄存器*/     \
                     "popfl\n"                  /* restore flags  恢复flags寄存器*/     \
                                                                        \
                     /* output parameters */                            \
                     : [prev_sp] "=m" (prev->thread.sp),                \
                       [prev_ip] "=m" (prev->thread.ip),                \
                       "=a" (last),                                     \
                                                                        \
                       /* clobbered output registers: */                \
                       "=b" (ebx), "=c" (ecx), "=d" (edx),              \
                       "=S" (esi), "=D" (edi)                           \
                                                                        \
                       __switch_canary_oparam                           \
                                                                        \
                       /* input parameters: */                          \
                     : [next_sp]  "m" (next->thread.sp),                \
                       [next_ip]  "m" (next->thread.ip),                \
                                                                        \
                       /* regparm parameters for __switch_to(): */      \
                       [prev]     "a" (prev),                           \
                       [next]     "d" (next)                            \
                                                                        \
                       __switch_canary_iparam                           \
                                                                        \
                     : /* reloaded segment registers */                 \
                        "memory");                                      \
} while (0)
```

还函数流程：
- 先对eflags寄存器和ebp压入就进程内核栈（原进程的内核栈），并将确定就进程恢复执行的下一跳地址，将旧进程eip，esp保存到task\_struct->thread\_info中，这样旧进程保存完毕；
- 然后用新进程的task\_sturct->thread\_info->esp恢复新进程的内核堆栈，用thread\_info->eip恢复新进程地址执行。

> 关键点：eflgas、ebp保存到内核栈；内核栈esp地址、eip地址保存到thread\_info中，task\_struct在生命周期中始终是全局的，所以肯定能根据该结构恢复期所有的执行场景来。

## 2.7 barrier屏障同步

switch\_to完成了进程的切换，新进程在该调用后开始执行，而switch\_to之后的代码只有在当前进程下一次被选择运行时才会执行（重新调度到以后才会执行）。

```c
/*  switch_to之后的代码只有在
 *  当前进程再次被选择运行(恢复执行)时才会运行
 *  而此时当前进程恢复执行时的上一个进程可能跟参数传入时的prev不同
 *  甚至可能是系统中任意一个随机的进程
 *  因此switch_to通过第三个参数将此进程返回
*/

/*  路障同步, 一般用编译器指令实现
 *  确保了switch_to和finish_task_switch的执行顺序
 *  不会因为任何可能的优化而改变  */
barrier();

/*  进程切换之后的处理工作  */
return finish_task_switch(prev);
```
而为了程序编译后指令的执行顺序不会因为编译器的优化而改变，因此内核提供了屏障同步barrier来保证程序的执行顺序。

barrier往往通过编译器指令来实现，内核中多出都使用了barrier，形式如下：
```c
/* Copied from linux/compiler-gcc.h since we can't include it directly 
 * 采用内敛汇编实现
 *  __asm__用于指示编译器在此插入汇编语句
 *  __volatile__用于告诉编译器，严禁将此处的汇编语句与其它的语句重组合优化。
 *  即：原原本本按原来的样子处理这这里的汇编。
 *  memory强制gcc编译器假设RAM所有内存单元均被汇编指令修改，这样cpu中的registers和cache中已缓存的内存单元中的数据将作废。cpu将不得不在需要的时候重新读取内存中的数据。这就阻止了cpu又将registers，cache中的数据用于去优化指令，而避免去访问内存。
 *  "":::表示这是个空指令。barrier()不用在此插入一条串行化汇编指令。在后文将讨论什么叫串行化指令。
*/
#define barrier() __asm__ __volatile__("": : :"memory")
```

## 2.8 finish_task_switch完成清理工作

finish\_task\_switch完成一些清理工作，使得能够正确的释放锁，但是我们不会详细讨论这些。
它会向各个体系结构提供了另一个hook函数，满足各个体系结构上下文切换过程的可能性，当然这只在少数体系结构上需要。

前面我们了解switch\_to函数的3个参数时讲到：
- 进程A切换到进程B，而当A再次被选择执行，C再次切换到A，此时A执行，但是系统为了告知调度器A进程再次执行前的进程是C，通过switch\_to的last参数返回的prev指向进程C，在进程A调度时候需要把进程A的信息清除掉。
- 由于从进程C切换到进程A时，进程A内核栈保存的实际上是进程A切换出去时的状态信息，即`prev = A，next = B`，但是在A执行时，其位于context\_switch上下文中，该函数的last参数返回的prev应该是切换到进程A的进程C。进程A负责对进程C信息进程切换后处理，比如，如果切换到进程A后，进程A发现进程C已经处于TASK\_DEAD状态，则进程A将释放进程C的task\_struct结构。

函数定义在kernel/sched/core.c中，如下所示：
```c
/**
 * finish_task_switch - clean up after a task-switch
 * @prev: the thread we just switched away from.
 *
 * finish_task_switch must be called after the context switch, paired
 * with a prepare_task_switch call before the context switch.
 * finish_task_switch will reconcile locking set up by prepare_task_switch,
 * and do any other architecture-specific cleanup actions.
 *
 * Note that we may have delayed dropping an mm in context_switch(). If
 * so, we finish that here outside of the runqueue lock. (Doing it
 * with the lock held can cause deadlocks; see schedule() for
 * details.)
 *
 * The context switch have flipped the stack from under us and restored the
 * local variables which were saved when this task called schedule() in the
 * past. prev == current is still correct but we need to recalculate this_rq
 * because prev may have moved to another CPU.
 */
static struct rq *finish_task_switch(struct task_struct *prev)
        __releases(rq->lock)
{
        struct rq *rq = this_rq();
        struct mm_struct *mm = rq->prev_mm;
        long prev_state;

        /*
         * The previous task will have left us with a preempt_count of 2
         * because it left us after:
         *
         *      schedule()
         *        preempt_disable();                    // 1
         *        __schedule()
         *          raw_spin_lock_irq(&rq->lock)        // 2
         *
         * Also, see FORK_PREEMPT_COUNT.
         */
        if (WARN_ONCE(preempt_count() != 2*PREEMPT_DISABLE_OFFSET,
                      "corrupted preempt_count: %s/%d/0x%x\n",
                      current->comm, current->pid, preempt_count()))
                preempt_count_set(FORK_PREEMPT_COUNT);

        rq->prev_mm = NULL;

        /*
         * A task struct has one reference for the use as "current".
         * If a task dies, then it sets TASK_DEAD in tsk->state and calls
         * schedule one last time. The schedule call will never return, and
         * the scheduled task must drop that reference.
         *
         * We must observe prev->state before clearing prev->on_cpu (in
         * finish_lock_switch), otherwise a concurrent wakeup can get prev
         * running on another CPU and we could rave with its RUNNING -> DEAD
         * transition, resulting in a double drop.
         */
        prev_state = prev->state;
        vtime_task_switch(prev);
        perf_event_task_sched_in(prev, current);
        finish_lock_switch(rq, prev);
        finish_arch_post_lock_switch();

        fire_sched_in_preempt_notifiers(current);
        if (mm)
                mmdrop(mm);
        if (unlikely(prev_state == TASK_DEAD))  /*  如果上一个进程已经终止，释放其task_struct 结构  */
        {
                if (prev->sched_class->task_dead)
                        prev->sched_class->task_dead(prev);

                /*
                 * Remove function-return probe instances associated with this
                 * task and put them back on the free list.
                 */
                kprobe_flush_task(prev);
                put_task_struct(prev);
        }

        tick_nohz_task_switch();
        return rq;
}
```