<!-- TOC -->

- [1. System.map符号表解析](#1-systemmap%e7%ac%a6%e5%8f%b7%e8%a1%a8%e8%a7%a3%e6%9e%90)
- [2. 查看符号表的命令](#2-%e6%9f%a5%e7%9c%8b%e7%ac%a6%e5%8f%b7%e8%a1%a8%e7%9a%84%e5%91%bd%e4%bb%a4)

<!-- /TOC -->

# 1. System.map符号表解析

System.map是内核符号表文件,是由“nm vmlinux”命令产生的，位于源码根目录。
所谓“符号”就是函数名，全局变量名等东东，这个表就记录的这些“符号”在内核中的运行地址。

下面是文件的一个片段

```
ffffffff8101bf30 T vcpu_load
ffffffff8101bf90 t kvm_sched_out
ffffffff8101bfc0 T vcpu_put
ffffffff8101bff0 t mark_page_dirty_in_slot
ffffffff8101c010 t cpumask_clear_cpu
ffffffff8101c020 T kvm_vcpu_init
ffffffff8101c140 T kvm_vcpu_uninit
ffffffff8101c180 t vcpu_stat_get
ffffffff8101c240 t vm_stat_get
ffffffff8101c2c0 t kvm_free_memslot
ffffffff8101c320 t kvm_free_memslots
ffffffff8101c390 t kvm_device_ioctl_attr
ffffffff8101c410 t kvm_device_ioctl
ffffffff8101c490 t kvm_reboot
ffffffff8101c4d0 T kvm_dump_memslots
ffffffff8101c920 t kvm_mmu_notifier_change_pte
ffffffff8101c9a0 t kvm_mmu_notifier_test_young
ffffffff8101ca00 t kvm_mmu_notifier_release
```

由上可知，`kvm_dump_memslots()` 的地址就是 `ffffffff8101c4d0`
这对调试内核非常有用。比如内核调用了一个函数指针，想知道它到底指向哪个函数，直接printk这个函数指针的地址，根据地址一查表就ok了。

符号类型 | 名称　| 说明
---|---|---
A|Absolute|符号的值是绝对值，并且在进一步链接过程中不会被改变。
B|BSS|符号在未初始化数据区或区（section）中，即在BSS段中。
C|Common|符号是公共的。公共符号是未初始化的数据。在链接时，多个公共符号可能具有同一名称。如果该符号定义在其他地方，则公共符号被看作是未定义的引用。
D|Data|符号在已初始化数据区中。
G|Global|符号是在小对象已初始化数据区中的符号。某些目标文件的格式允许对小数据对象（例如一个全局整型变量）可进行更有效的访问。
I|Inderect|符号是对另一个符号的间接引用。
N|Debugging|符号是一个调试符号。
R|Read only|符号在一个只读数据区中。
S|Small|符号是小对象未初始化数据区中的符号。
T|Text|符号是代码区中的符号。
U|Undefined|符号是外部的，并且其值为0（未定义）。
-|Stabs|符号是a.out目标文件中的一个stab符号，用于保存调试信息。
?|Unknown|符号的类型未知，或者与具体文件格式有关。

 Linux内核启动后，可以通过`cat /proc/kallsyms`来查看符号表。

# 2. 查看符号表的命令
```
#> nm -n  example.ko
#> ld example.ko --print-map
```