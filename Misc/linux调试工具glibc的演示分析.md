
# 1. MALLOC_CHECK_

GNU的标准库(glibc)可以通过内置的调试特性对动态内存进行调试,它就是MALLOC\_CHECK\_环境变量，它在默认情况下是不设定的，在老的版本默认这个值为0，新的版本默认值为2，但有一个矛盾，如果设定为空，它将会打印出长长的跟踪信息，这比设为2更详细。

MALLOC\_CHECK_有三种设定，即：
- 0 ----- 关闭所有检查。
- 1 ----- 当有错误被探测到时，在标准错误输出(stderr)上打印错误信息。
- 2 ----- 当有错误被探测到时，不显示错误信息，直接进行中断。

我们用下面的小程序做一下测试，源程序如下：

```c
#include <stdio.h>
#include <stdlib.h>

int main (int argc,char *argv[])
{
    int i;
    char* p = (char *)malloc(10);
    char* q = p;

    for (i = 0;i < 10;i++) {
        p[i] = 'z';
    }

    free(p);
    free(q);

    return 0;
}
```

运行效果：
```
[root@lab task]# ./malloc
*** Error in `./malloc': double free or corruption (fasttop): 0x000000000216b010 ***
======= Backtrace: =========
/lib64/libc.so.6(+0x81499)[0x7f2ec8b49499]
./malloc[0x4005bd]
/lib64/libc.so.6(__libc_start_main+0xf5)[0x7f2ec8aea445]
./malloc[0x400499]
======= Memory map: ========
00400000-00401000 r-xp 00000000 fd:00 100790854                          /root/task/malloc
00600000-00601000 r--p 00000000 fd:00 100790854                          /root/task/malloc
00601000-00602000 rw-p 00001000 fd:00 100790854                          /root/task/malloc
0216b000-0218c000 rw-p 00000000 00:00 0                                  [heap]
7f2ec4000000-7f2ec4021000 rw-p 00000000 00:00 0
7f2ec4021000-7f2ec8000000 ---p 00000000 00:00 0
7f2ec88b2000-7f2ec88c7000 r-xp 00000000 fd:00 36032517                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2ec88c7000-7f2ec8ac6000 ---p 00015000 fd:00 36032517                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2ec8ac6000-7f2ec8ac7000 r--p 00014000 fd:00 36032517                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2ec8ac7000-7f2ec8ac8000 rw-p 00015000 fd:00 36032517                   /usr/lib64/libgcc_s-4.8.5-20150702.so.1
7f2ec8ac8000-7f2ec8c8b000 r-xp 00000000 fd:00 36027789                   /usr/lib64/libc-2.17.so
7f2ec8c8b000-7f2ec8e8a000 ---p 001c3000 fd:00 36027789                   /usr/lib64/libc-2.17.so
7f2ec8e8a000-7f2ec8e8e000 r--p 001c2000 fd:00 36027789                   /usr/lib64/libc-2.17.so
7f2ec8e8e000-7f2ec8e90000 rw-p 001c6000 fd:00 36027789                   /usr/lib64/libc-2.17.so
7f2ec8e90000-7f2ec8e95000 rw-p 00000000 00:00 0
7f2ec8e95000-7f2ec8eb7000 r-xp 00000000 fd:00 36027782                   /usr/lib64/ld-2.17.so
7f2ec9099000-7f2ec909c000 rw-p 00000000 00:00 0
7f2ec90b4000-7f2ec90b6000 rw-p 00000000 00:00 0
7f2ec90b6000-7f2ec90b7000 r--p 00021000 fd:00 36027782                   /usr/lib64/ld-2.17.so
7f2ec90b7000-7f2ec90b8000 rw-p 00022000 fd:00 36027782                   /usr/lib64/ld-2.17.so
7f2ec90b8000-7f2ec90b9000 rw-p 00000000 00:00 0
7ffe0abfc000-7ffe0ac1d000 rw-p 00000000 00:00 0                          [stack]
7ffe0aca8000-7ffe0acaa000 r-xp 00000000 00:00 0                          [vdso]
ffffffffff600000-ffffffffff601000 r-xp 00000000 00:00 0                  [vsyscall]
Aborted (core dumped)
```

切换MALLOC\_CHECK级别：
```
[root@lab task]# export MALLOC_CHECK_=1
```
再次运行效果：
```
[root@lab task]# ./malloc
*** Error in `./malloc': free(): invalid pointer: 0x0000000000e44010 ***
```

# 2. 用mtrace查找内存泄露

mtrace是由glibc提供的一个工具，在Redhat中将它打包在glibc-utils包中。

mtrace的主要作用是查找内存泄露，为了应用mtrace程序，必须在代码中使用glibc提供的函数mtrace和muntrace。
另外，必须设置一个文件的名字给环境变量MALLOC_TRACE，因为glibc利用它为mtrace程序存储数据。
当执行完代码后，数据将会存在这个确认的文件中，每执行一次程序，这个文件的内容都会被重写。

我们用下面的代码进行测试，如下：
```c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <mcheck.h>

int main (int argc,char *argv[])
{
    setenv("MALLOC_TRACE","output",1);
    mtrace();

    int i;
    char* p = (char *)malloc(10);
    char* q = p;

    for (i = 0;i < 10;i++) {
        p[i] = 'z';
    }

    return 0;
}
```

程序编译执行后，会在当前目录生成output文件。

```shell
= Start
@ ./mtrace:[0x4005df] + 0x7785d0 0xa
@ /lib64/libc.so.6:(clearenv+0x7a)[0x7f98b04a996a] - 0x778010
@ /lib64/libc.so.6:(tdestroy+0x5d)[0x7f98b056b1fd] - 0x778120
@ /lib64/libc.so.6:[0x7f98b05df9dc] - 0x778140
```

用mtrace查找内存泄露，它告诉我们memory not freed
```
[root@lab task]# mtrace output
- 0x0000000000778010 Free 3 was never alloc'd 0x7f98b04a996a
- 0x0000000000778120 Free 4 was never alloc'd 0x7f98b056b1fd
- 0x0000000000778140 Free 5 was never alloc'd 0x7f98b05df9dc

Memory not freed:
-----------------
           Address     Size     Caller
0x00000000007785d0      0xa  at 0x4005df
```