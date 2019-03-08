***Linux进程启动过程分析do_execve(可执行程序的加载和运行)***

- 1 execve API接口
    - 1.1 execve系统调用
    - 1.2 exec函数族
- 2 ELF文件格式及可执行程序
    - 2.1 ELF可执行文件格式
    - 2.2 struct linux_binprm结构描述一个可执行程序
    - 2.3 struct linux_binfmt结构描述一个可执行文件的结构
- 3 execv加载可执行程序的过程
    - 3.1 execve的入口函数sys_execve
    - 3.2 do_execve函数
    - 3.3 程序的加载do_execve_common和do_execveat_common
    - 3.4 search_binary_handler识别二进制程序
    - 3.6 load_binnary()加载可执行程序

# 1. execve API接口

## 1.1 execve系统调用

我们前面提到了，fork，vfork等复制出来的进程是父进程的一个副本，那么我们想加载新程序，可以通过execve系统调用来加载和启动新的程序。

```c
int execve(const char *filename, char *const argv[],
                  char *const envp[]);
```

> x86架构下，其实还实现了一个新的exec的系统调用叫做execveat（since Linux 3.19）。
>
> 详情参见：《syscall,x86: Add execveat() system call》

## 1.2 exec函数族


exec函数一共有7个，execve是系统调用，其他6个（execl，execlp，execle，execv，execvp，execvpe）都是调用execve的glibc库函数。

```c
#include <unistd.h>

extern char **environ;

int execl(const char *path, const char *arg, ...);
int execlp(const char *file, const char *arg, ...);
int execle(const char *path, const char *arg, ..., char * const envp[]);
int execv(const char *path, char *const argv[]);
int execvp(const char *file, char *const argv[]);
int execvpe(const char *file, char *const argv[], char *const envp[]);
```

这些函数，它们的函数名都是在exec后面加上一到三个字符后缀，不同的字符后缀代表不同的含义。
- -l：即list，新进程的命令行参数以字符指针列表(const char *arge, ...) 的形式传入，列表以空指针结束。
- -p：即path，若第一个参数中不包含“/”，则将其视为文件名，并根据PATH环境变量搜索该文件。
- -e：即environment，新进程的环境变量以字符指针数组 (cahr *const envp[]) 的形式传入，数组以空指针结束，不指定环境变量则从调用进程复制。
- -v：即vector，新进程的命令行参数以字符指针数组(char *const argv[]) 的形式传入，数组以空指针结束。

# 2. ELF文件格式及可执行程序

## 2.1 ELF可执行文件格式

Linux下标准的可执行文件格式是ELF。

ELF（Executable and Linking Format）是一种对象文件格式，用于定义不同类型对象文件（Object files）中都放了什么东西、以及都以什么样的格式去放这些东西。
它自最早在System V系统上出现后，被Unix世界所广泛接受，作为缺省二进制文件格式来使用。

但是Linux也支持其他不同的可执行程序格式，各个可执行程序的执行方式不尽相同，因此Linux内核每种被注册的可执行程序格式都用linux\_binfmt来存储，其中记录了可执行程序的加载和执行函数。

同时我们需要一种方法来保存可执行程序的信息，比如可执行文件的路径，运行的参数和环境变量等信息，即linux\_binprm结构（prm即Parameter）。

## 2.2 struct linux_binprm结构描述一个可执行程序

linux\_binprm是定义在include/linux/binfmts.h中，用来保存要执行的文件的信息：可执行程序的路径、参数、环境变量等信息。

```c
/*
 * This structure is used to hold the arguments that are used when loading binaries.
 */
struct linux_binprm {
    char buf[BINPRM_BUF_SIZE];  // 保存可执行文件的前128字节
#ifdef CONFIG_MMU
    struct vm_area_struct *vma;
    unsigned long vma_pages;
#else
# define MAX_ARG_PAGES  32
    struct page *page[MAX_ARG_PAGES];
#endif
    struct mm_struct *mm;
    unsigned long p; /* current top of mem，当前内存页最高地址 */
    unsigned int
        cred_prepared:1,/* true if creds already prepared (multiple
                 * preps happen for interpreters) */
        cap_effective:1;/* true if has elevated effective capabilities,
                 * false if not; except for init which inherits
                 * its parent's caps anyway */
#ifdef __alpha__
    unsigned int taso:1;
#endif
    unsigned int recursion_depth;
    struct file * file; // 要执行的文件
    struct cred *cred;  /* new credentials */
    int unsafe;     /* how unsafe this exec is (mask of LSM_UNSAFE_*) */
    unsigned int per_clear; /* bits to clear in current->personality */
    int argc, envc; // 命令行参数和环境变量数目
    // 要执行文件的名称
    const char * filename;  /* Name of binary as seen by procps */
    // 要执行文件的真实名称，通常和filename相同
    const char * interp;    /* Name of the binary really executed. Most
                   of the time same as filename, but could be
                   different for binfmt_{misc,script} */
    unsigned interp_flags;
    unsigned interp_data;
    unsigned long loader, exec;
    char tcomm[TASK_COMM_LEN];
};
```

## 2.3 struct linux_binfmt结构描述一个可执行文件的结构

Linux支持其他不同格式的可执行程序，在这种方式下，Linux能运行其他操作系统所编译的程序，如MS-DOS程序、BSD Unix的COFF可执行格式，因此Linux内核用struct linux\_binfmt来描述各种可执行程序。

Linux内核对所支持的每种可执行程序类型都有个struct linux\_binfmt的数据结构，定义如下：

```c
/*
 * This structure defines the functions that are used to load the binary formats that
 * linux accepts.
 */
struct linux_binfmt {
    struct list_head lh;
    struct module *module;
    int (*load_binary)(struct linux_binprm *);
    int (*load_shlib)(struct file *);
    int (*core_dump)(struct coredump_params *cprm);
    unsigned long min_coredump; /* minimal dump size */
};
```
其提供了3个方法来加载和执行可执行程序
1. load_binary：通过读存放在可执行文件中的信息为当前进程建立一个新的执行环境
2. load_shlib：用于动态的把一个共享库捆绑到一个已经在运行的进程，这是由uselib()系统调用激活的
3. core_dump：在名为core的文件中，存放当前进程的执行上下文，这个文件通常是在进程接收到一个缺省操作为dump的信号时被创建的，其格式取决于被执行程序的可执行类型

```man
NAME
       uselib - load shared library

SYNOPSIS
       #include <unistd.h>

       int uselib(const char *library);
```

所有的linux\_binfmt对象都处于一个链表中，第一个元素的地址存放在formats变量中，可以通过register\_binfmt()和unregister\_binfmt()函数在链表中插入和删除元素，在系统启动时期，为每个编译进内核的可执行格式都执行register\_fmt()函数。当实现了一个新的可执行格式的模块正被装载时，也执行这个函数。当模块被卸载时，执行unregster\_binfmt()函数。

```c
static LIST_HEAD(formats);
static DEFINE_RWLOCK(binfmt_lock);

void __register_binfmt(struct linux_binfmt * fmt, int insert)
{
    BUG_ON(!fmt);
    write_lock(&binfmt_lock);
    insert ? list_add(&fmt->lh, &formats) :
         list_add_tail(&fmt->lh, &formats);
    write_unlock(&binfmt_lock);
}

EXPORT_SYMBOL(__register_binfmt);

void unregister_binfmt(struct linux_binfmt * fmt)
{
    write_lock(&binfmt_lock);
    list_del(&fmt->lh);
    write_unlock(&binfmt_lock);
}

EXPORT_SYMBOL(unregister_binfmt);

/* Registration of default binfmt handlers */
static inline void register_binfmt(struct linux_binfmt *fmt)
{
    __register_binfmt(fmt, 0);
}

/* Same as above, but adds a new binfmt at the top of the list */
static inline void insert_binfmt(struct linux_binfmt *fmt)
{
    __register_binfmt(fmt, 1);
}
```

> 当我们执行一个可执行程序的时候，内核会list_for_each_entry遍历所有注册的linux_binfmt对象，对其调用load_binrary方法来尝试加载，直到加载成功为止。

# 3. execv加载可执行程序的过程

内核中实际执行execv()或execve()系统调用都是调用do\_execve()，这个函数的执行流程为：
1. 打开目标映像文件
2. 从目标映像文件中读取128个字节，用于填充ELF文件头部
3. 然后调用search\_binary\_handler函数，它会搜索我们上面提到的Linux支持的可执行文件链表，让各种可执行程序的处理程序前来认领和处理。如果类型匹配，则调用load\_binary函数指针指向的处理函数来处理目标映像文件。

> 在ELF文件格式中，处理函数是load_elf_binary函数，下面主要就是分析load_elf_binary函数的执行过程。

```
    sys_execv() 
        -> do_execve()
            -> do_execveat_common()
                -> search_binary_handler()
                    -> load_elf_binary()
```

## 3.1 execve的入口函数sys_execve

系统调用号，arch/x86/include/generated/uapi/asm/unistd_64.h
```c
#define __NR_execve 59
```

入口函数声明，include/linux/syscalls.h
```c
asmlinkage long sys_execve(const char __user *filename,
        const char __user *const __user *argv,
        const char __user *const __user *envp);
```
系统调用实现，fs/exec.c
```c
SYSCALL_DEFINE3(execve,
        const char __user *, filename,
        const char __user *const __user *, argv,
        const char __user *const __user *, envp)
{
    return do_execve(getname(filename), argv, envp);
}
```

通过参数传递了寄存器集合和可执行文件的名称（filename），而且还传递了指向程序的参数argv和环境变量envp的指针。

参数 | 说明
---|:---
filename | 可执行程序的名称
argv | 程序的参数
envp | 环境变量

指向程序参数argv和环境变量envp两个数组的指针以及数组中所有的指针都位于虚拟地址空间的用户空间部分。
因此，内核在访问用户空间内存时，需要多加小心，而\_\_user注释测将允许静态代码检查工具检测时所有相关事宜都处理得当。

## 3.2 do_execve函数

```c
int do_execve(struct filename *filename,
    const char __user *const __user *__argv,
    const char __user *const __user *__envp)
{
    struct user_arg_ptr argv = { .ptr.native = __argv };
    struct user_arg_ptr envp = { .ptr.native = __envp };
    return do_execve_common(filename, argv, envp);
}
```
早期的代码do\_execve就直接完成了自己的所有工作，后来do\_execve会调用更加低层的do\_execve\_common函数，后来x86架构下引入了新的系统调用execveat，为了使代码更加通用，do\_execveat\_common代替了原来的do\_execve\_common函数。

早期的do\_execve流程如下，基本无差别，可以作为参考：

![image](./images/0x01.png)

## 3.3 程序的加载do_execve_common和do_execveat_common

> 早期Linux 2.4 中直接由do\_execve实现程序的加载和运行</br>Linux 3.18 引入execveat之前，do\_execve调用do\_execve\_common来完成程序的加载和运行</br>Linux 3.19 引入execveat之后，do\_execve调用do\_execveat\_common来完成程序的加载和运行

在Linux中提供了一系列的函数，这些函数都能用可执行文件锁描述的上下文代替进程的上下文。这样的函数名以前缀exec开始。所有exec函数都是调用了execve()系统调用。

sys\_execve接受3个参数：
1. 可执行文件的路径
2. 命令行参数字符串
3. 环境变量字符串

```c
/*
 * sys_execve() executes a new program.
 */
static int do_execve_common(struct filename *filename,
                struct user_arg_ptr argv,
                struct user_arg_ptr envp)
{
    // 这个结构非常重要，下文列出了这个结构体以便查询各个成员变量的意义
    struct linux_binprm *bprm;
    struct file *file;
    struct files_struct *displaced;
    bool clear_in_exec;
    int retval;
    const struct cred *cred = current_cred();

    if (IS_ERR(filename))
        return PTR_ERR(filename);

    /*
     * We move the actual failure in case of RLIMIT_NPROC excess from
     * set*uid() to execve() because too many poorly written programs
     * don't check setuid() return code.  Here we additionally recheck
     * whether NPROC limit is still exceeded.
     */
    if ((current->flags & PF_NPROC_EXCEEDED) &&
        atomic_read(&cred->user->processes) > rlimit(RLIMIT_NPROC)) {
        retval = -EAGAIN;
        goto out_ret;
    }

    /* We're below the limit (still or again), so we don't want to make
     * further execve() calls fail. */
    current->flags &= ~PF_NPROC_EXCEEDED;
    
    // 1. 调用unshare_files()为进程复制一份文件表
    retval = unshare_files(&displaced);
    if (retval)
        goto out_ret;

    retval = -ENOMEM;
    // 2. 调用kzalloc()在堆上分配一份struct linux_binprm结构体
    bprm = kzalloc(sizeof(*bprm), GFP_KERNEL);
    if (!bprm)
        goto out_files;

    retval = prepare_bprm_creds(bprm);
    if (retval)
        goto out_free;

    retval = check_unsafe_exec(bprm);
    if (retval < 0)
        goto out_free;
    clear_in_exec = retval;
    current->in_execve = 1;

    // 3. 调用open_exit()查找并打开二进制文件
    file = do_open_exec(filename);
    retval = PTR_ERR(file);
    if (IS_ERR(file))
        goto out_unmark;

    // 4. 调用sched_exit()找到最小负载的CPU，用来执行该二进制文件
    sched_exec();

    // 5. 根据获取的信息，填充struct linux_binprm结构体中的file、filename、interp成员
    bprm->file = file;
    bprm->filename = bprm->interp = filename->name;
    
    // 6. 调用bprm_mm_init()创建进程的内存地址空间
    //    并调用init_new_context()检查当前进程是否使用自定义的LDT描述符表
    //    如果是，那么分配和准备一个新的LDT
    retval = bprm_mm_init(bprm);
    if (retval)
        goto out_file;

    // 7. 填充struct linux_binprm结构体中的命令行参数argv, 环境变量envp
    bprm->argc = count(argv, MAX_ARG_STRINGS);
    if ((retval = bprm->argc) < 0)
        goto out;

    bprm->envc = count(envp, MAX_ARG_STRINGS);
    if ((retval = bprm->envc) < 0)
        goto out;

    // 8. 调用prepare_binprm()检查该二进制文件的可执行权限
    //    最后，kernel_read()读取二进制文件首128字节，用于识别二进制文件格式
    retval = prepare_binprm(bprm);
    if (retval < 0)
        goto out;

    // 9. 调用copy_strings_kernel从内核空间获取二进制文件的路径名称
    retval = copy_strings_kernel(1, &bprm->filename, bprm);
    if (retval < 0)
        goto out;

    bprm->exec = bprm->p;
    // 10. 调用copy_string从用户空间拷贝环境变量
    retval = copy_strings(bprm->envc, envp, bprm);
    if (retval < 0)
        goto out;
    // 11. 调用copy_string从用户空间拷贝命令行参数
    retval = copy_strings(bprm->argc, argv, bprm);
    if (retval < 0)
        goto out;

    // 至此，二进制文件已经被打开，struct linux_binprm结构体页记录了重要信息
    // 下面需要识别该二进制文件的格式并最终运行该文件
    retval = search_binary_handler(bprm);
    if (retval < 0)
        goto out;

    /* execve succeeded */
    current->fs->in_exec = 0;
    current->in_execve = 0;
    acct_update_integrals(current);
    task_numa_free(current);
    free_bprm(bprm);
    putname(filename);
    if (displaced)
        put_files_struct(displaced);
    return retval;

out:
    if (bprm->mm) {
        acct_arg_size(bprm, 0);
        mmput(bprm->mm);
    }

out_file:
    if (bprm->file) {
        allow_write_access(bprm->file);
        fput(bprm->file);
    }

out_unmark:
    if (clear_in_exec)
        current->fs->in_exec = 0;
    current->in_execve = 0;

out_free:
    free_bprm(bprm);

out_files:
    if (displaced)
        reset_files_struct(displaced);
out_ret:
    putname(filename);
    return retval;
}
```

## 3.4 search_binary_handler识别二进制程序

每种格式的二进制文件对应一个struct linux\_binprm结构体，每种可执行的程序类型都对应一个数据结构struct linux\_binfmt，load_binary成员负责识别该二进制文件的格式。

内核使用链表组织这些struct linux_binfmt结构体，链表头是formats。

该函数对linux\_binprm的formats链表进行扫描，并尝试每个load_binary函数，如果成功加载了文件的执行格式，对formats的扫描终止。

这里需要说明的是，这里的fmt变量的类型是struct linux\_binfmt *,但是这一个类型与之前在do\_execve_common()中的bprm是不一样的。

```c
/*
 * cycle the list of binary formats handler, until one recognizes the image
 */
int search_binary_handler(struct linux_binprm *bprm)
{
    unsigned int depth = bprm->recursion_depth;
    int try,retval;
    struct linux_binfmt *fmt;
    pid_t old_pid, old_vpid;

    /* This allows 4 levels of binfmt rewrites before failing hard. */
    if (depth > 5)
        return -ELOOP;

    retval = security_bprm_check(bprm);
    if (retval)
        return retval;

    /* Need to fetch pid before load_binary changes it */
    old_pid = current->pid;
    rcu_read_lock();
    old_vpid = task_pid_nr_ns(current, task_active_pid_ns(current->parent));
    rcu_read_unlock();

    retval = -ENOENT;
    for (try=0; try<2; try++) {
        read_lock(&binfmt_lock);
        // 遍历formats链表
        list_for_each_entry(fmt, &formats, lh) {
            int (*fn)(struct linux_binprm *) = fmt->load_binary;
            if (!fn)
                continue;
            if (!try_module_get(fmt->module))
                continue;
            read_unlock(&binfmt_lock);
            bprm->recursion_depth = depth + 1;
            // 每个尝试load_binary()函数
            retval = fn(bprm);
            bprm->recursion_depth = depth;
            if (retval >= 0) {
                audit_bprm(bprm);
                if (depth == 0) {
                    trace_sched_process_exec(current, old_pid, bprm);
                    ptrace_event(PTRACE_EVENT_EXEC, old_vpid);
                }
                put_binfmt(fmt);
                allow_write_access(bprm->file);
                if (bprm->file)
                    fput(bprm->file);
                bprm->file = NULL;
                current->did_exec = 1;
                proc_exec_connector(current);
                return retval;
            }
        }
        read_unlock(&binfmt_lock);
#ifdef CONFIG_MODULES
        if (retval != -ENOEXEC || bprm->mm == NULL) {
            break;
        } else {
#define printable(c) (((c)=='\t') || ((c)=='\n') || (0x20<=(c) && (c)<=0x7e))
            if (printable(bprm->buf[0]) &&
                printable(bprm->buf[1]) &&
                printable(bprm->buf[2]) &&
                printable(bprm->buf[3]))
                break; /* -ENOEXEC */
            if (try)
                break; /* -ENOEXEC */
            request_module("binfmt-%04x", *(unsigned short *)(&bprm->buf[2]));
        }
#else
        break;
#endif
    }
    return retval;
}
```

## 3.6 load_binnary()加载可执行程序

我们前面提到了，Linux内核支持多种可执行程序格式。每种格式都被注册为一个linux\_binfmt结构，其中存储了对应可执行程序加载函数等。


格式 | linux\_binfmt定义 | load\_binary | load\_shlib | core\_dump
---|---|---|---|---
a.out | aout\_format | load\_aout\_binary | load\_aout\_library | aout\_core\_dump
flat | style executables | flat\_format | load\_flat\_binary | load\_flat\_shared\_library | flat\_core\_dump
script脚本 | script\_format | load_script | none | none
misc\_format | misc\_format | load\_misc\_binary | none | none
em86 | em86\_format | load\_format | none | none
elf\_fdpic | elf\_fdpic\_format | load\_elf\_fdpic\_binary | none | elf\_fdpic\_core\_dump
elf | elf\_format | load\_elf\_binary | load\_elf\_binary | elf\_core\_dump


a.out：这是为了支持原来风格的Linux二进制文件。它的存在主要是为了满足一些系统的向后兼容需求，但是基本上a.out已经光荣退役了。

flat：二进制的Flat文件通常被称为BFLT，它是基于原始的a.out格式的一种相对简单的轻量级可执行格式。BFLT文件是嵌入式Linux的默认文件格式。

scripts：对于shell脚本、Perl脚本等提供支持。宽松一点地说，所有前面两个字符#!的可执行文件，都归由这个二进制处理程序进行处理。

misc：这是最明智地使用二进制处理程序的方法，这个处理程序通过内嵌的特征数字或者文件名后缀可以识别出各种二进制格式，不过最优秀的特性是它可以在运行期配置，而不是只能在编译期配置。因此，只要遁循其规则，就可以快速的增加对新二进制文件的支持，而不用重新编译内核，也无须重新启动机器。Linux源程序文件的注释建议最终使用它取代EM86二进制处理程序。

em86：允许在Alpha机器上运行Intel的Linux二进制文件，仿佛它们就是Alpha的本地二进制文件。

fdpic elf：可执行和可链接格式最初是由Unix System实验室开发出来的，现在已经成为文件格式的标准。相对于BFLT格式而言，ELF标准更强大并且更灵活。但是，它更重量级一些，需要更多的磁盘空间以及有一定的运行时开销。

elf：目前是Linux默认的二进制文件格式。该格式在可执行文件和共享库中都广泛使用。最新Linux发行版一般只会预装ELF二进制文件解释器，但是特殊情况下决定加载a.out二进制文件，那么系统也通过模块的方式，对它提供支持。虽然ELF被作为惯用的Linux本地格式，但也和其他格式一样使用同一个加载处理程序。