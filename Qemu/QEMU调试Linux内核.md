***QEMU调试Linux内核***

[TOC]

# 1. 简介
排查Linux内核Bug，研究内核机制，除了查看资料阅读源码，还可通过调试器，动态分析内核执行流程。

QEMU模拟器原生支持GDB调试器，这样可以很方便地使用GDB的强大功能对操作系统进行调试，如设置断点；单步执行；查看调用栈、查看寄存器、查看内存、查看变量；修改变量改变执行流程等。

用QEMU与GDB来调试内核和ko，当然我们需要准备如下：

- 带调试信息的内核vmlinux
- 一个压缩的内核vmlinuz或者bzImage
- 一份裁剪过的文件系统initrd

# 2. 测试环境

- CentOS 7.4
- QEMU 2.5.0
- GDB 7.6.1
- linux-4.14.71.tar.gz


# 3. 编译调试内核

```
make menuconfig

    Kernel hacking 
        → Compile-time checks and compiler options
            [*] Compile the kernel with debug info
            [*]   Provide GDB scripts for kernel debugging
            [*]   Reduce debugging information
```

```
$> cat .config| grep CONFIG_DEBUG_INFO_REDUCED
CONFIG_DEBUG_INFO_REDUCED=y
$> cat .config| grep CONFIG_GDB_SCRIPTS
CONFIG_GDB_SCRIPTS=y
```

- 一个压缩的内核vmlinuz或者bzImage
```
[linux-4.14.71]#> file arch/x86/boot/bzImage
arch/x86/boot/bzImage: Linux kernel x86 boot executable bzImage, version 4.14.71 (root@centos7) #1 SMP Thu Oct 11 17:05:14 CST 2018, RO-rootFS, swap_dev 0x7, Normal VGA
```

- 带调试信息的内核vmlinux
```
[linux-4.14.71]#> file vmlinux
vmlinux: ELF 64-bit LSB executable, x86-64, version 1 (SYSV), statically linked, BuildID[sha1]=f3141e126cec198bdd00e7172d5d5e3afa2f059f, not stripped
```

# 4. 构建initramfs根文件系统

Linux系统启动阶段，boot loader加载完内核文件vmlinuz后，内核紧接着需要挂载磁盘根文件系统，但如果此时内核没有相应驱动，无法识别磁盘，就需要先加载驱动，而驱动又位于/lib/modules，得挂载根文件系统才能读取，这就陷入了一个两难境地，系统无法顺利启动。于是有了initramfs根文件系统，其中包含必要的设备驱动和工具，boot loader加载initramfs到内存中，内核会将其挂载到根目录/,然后运行/init脚本，挂载真正的磁盘根文件系统。

这里借助BusyBox构建极简initramfs，提供基本的用户态可执行程序。

编译BusyBox，配置CONFIG_STATIC参数，编译静态版BusyBox，编译好的可执行文件busybox不依赖动态链接库，可以独立运行，方便构建initramfs。


## 4.1 下载源代码
```
$> wget https://busybox.net/downloads/busybox-1.29.3.tar.bz2
$> tar xvf busybox-1.29.3
$> cd busybox-1.29.3
$> make menuconfig
```

## 4.2 配置busybox
```
Settings  ---> 
    [*] Don't use /usr 
    [*] Build static binary (no shared libs) 
```

## 4.3 编译安装

```
$> make -j4 && make install
```

## 4.4 制作initramfs

### 4.4.1 创建目录结构
```
$> cd _install
$> mkdir -p proc sys dev/pts mnt tmp etc/init.d/
```

### 4.4.2 创建设备文件
```
$> cd _install/dev/
$> mknod console c 5 1
$> mknod null c 1 3
```

### 4.4.3 创建fstab

在_install/etc目录新创建一个叫fstab的文件，并写入如下内容。

```
proc /proc proc defaults 0 0
tmpfs /tmp tmpfs defaults 0 0
sysfs /sys sysfs defaults 0 0
tmpfs /dev tmpfs defaults 0 0
debugfs /sys/kernel/debug debugfs defaults 0 0
```

### 4.4.4 创建inittab

在_install/etc目录新创建一个叫inittab 的文件，并写入如下内容。

```
::sysinit:/etc/init.d/rcS
::respawn:-/bin/sh
::askfirst:-/bin/sh
::ctrlaltdel:/bin/umount -a –r
```

### 4.4.5 创建rcS脚本

在_install /etc/init.d/目录下新创建一个叫rcS 的文件，增加执行权限并且写入如下内容：
```
#!/bin/sh
export PATH=/sbin:/bin

mount -a
mount -t devpts devpts /dev/pts
echo /sbin/mdev > /proc/sys/kernel/hotplug
mdev -s
```

### 4.4.6 打包压缩initramfs

```
find . -print0 | cpio --null -ov --format=newc | gzip -9 > ../initramfs.img
```

# 5. 启动QEMU

start_kernel.sh
```
#!/usr/bin/bash
qemu-system-x86_64 -M pc -smp 2 -m 1024 \
    -kernel arch/x86/boot/bzImage \
    -initrd initramfs.img \
    -append "rdinit=/linuxrc loglevel=7" -S -s
```
- -s是-gdb tcp::1234缩写，监听1234端口，在GDB中可以通过target remote localhost:1234连接。

- -S表示 QEMU 虚拟机会冻结 CPU 直到远程的 GDB 输入相应控制命令，所以运行后看不到任何输出。


# 6. 使用GDB远程调试Kernel

> 推荐使用GDB7.4以上的版本，内置python支持gdb扩展功能，如：vmlinux-gdb.py。

start_gdb.sh
```
#!/bin/bash
gdb -ex "set debug auto-load on"             \
    -ex "set auto-load safe-path ."          \
    -ex "file vmlinux"                       \
    -ex "target remote localhost:1234"       \
    -ex "break x86_64_start_kernel"          \
    -ex "continue"                           \
    -ex "disconnect"                         \
    -ex "set architecture i386:x86-64:intel" \
    -ex "target remote localhost:1234"

```



