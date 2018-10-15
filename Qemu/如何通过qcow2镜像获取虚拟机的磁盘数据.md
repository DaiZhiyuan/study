***如何通过qcow2镜像获取虚拟机的磁盘数据*** 

[TOC]

# 1. 背景
当虚拟机出现崩溃，客户并不急于恢复虚拟机或者虚拟机已经无法恢复时，可以通过挂载虚拟机镜像，可以获取虚拟机镜像内部的数据，这样用户的数据就可以抢救出来了。

# 2. 挂载方法

假设虚拟机磁盘镜像路径为$PATH，镜像格式为$FORMAT, 挂载的目标目录为$mnt。
按照如下顺序执行命令：

1. 插入nbd模块
```
$> modprobe nbd max_part=16                       
```

2. 查看nbd模块是否加载
```
$> lsmod | grep nbd
```

3. 将磁盘镜像挂载到/dev/nbd0设备
```
$> qemu-nbd -c /dev/nbd0 $PATH -f $FORMAT
```

4. 查看镜像的分区情况
```
$> fdisk -l /dev/nbd0
Disk /dev/nbd0: 107.4 GB, 107374182400 bytes
255 heads, 63 sectors/track, 13054 cylinders, total 209715200 sectors
Units = sectors of 1 * 512 = 512 bytes
Sector size (logical/physical): 512 bytes / 512 bytes
I/O size (minimum/optimal): 512 bytes / 512 bytes
Disk identifier: 0x0008104f

    Device Boot     Start        End      Blocks   Id  System
/dev/nbd0p1  *          63      208844      104391  83  Linux
/dev/nbd0p2          208845    41929649    20860402+  8e  Linux LVM
/dev/nbd0p3        41929650  167766794    62918572+  83  Linux
```

# 2.1 非LVM分区挂载

可以看到其中第二个分区使用了lvm，其他两个分区是非lvm分区，如果需要挂载第三个分区，也就是非lvm分区，可以执行下列命令：
```
$> qemu-nbd -c /dev/nbd1 $PATH -f $FORMAT -P 3
$> mount /dev/nbd1 $mnt
```

# 2.2 LVM分区挂载

如果需要挂载第二个分区，就是lvm分区，可以执行下列命令：
```
$> qemu-nbd -c /dev/nbd2 $PATH -f $FORMAT -P 2
$> pvscan
```

pvscan可以看到物理卷的信息：
```
PV /dev/sda5    VG hty-compute2-vg  lvm2 [465.52 GiB / 0    free]
PV /dev/loop2  VG cinder-volumes    lvm2 [30.00 GiB / 30.00 GiB free]
PV /dev/nbd1    VG VolGroup00       lvm2 [19.88 GiB / 0    free]
Total: 3 [515.39 GiB] / in use: 3 [515.39 GiB] / in no VG: 0 [0  ]
```
lvm分区都被映射到/dev/mapper/下：
```
$> ls -1 /dev/mapper/
control
hty--compute2--vg-root
hty--compute2--vg-swap_1
VolGroup00-LogVol00
VolGroup00-LogVol01
```
以上在VolGroup00组中，有两个lv，分别尝试挂载之后，发现其中一个是swap分区，无法挂载，在本文的例子中，VolGroup00-LogVol01是swap分区，VolGroup00-LogVol00是逻辑分区，因此挂载VolGroup00-LogVol00.
```
$> mount /dev/mapper/VolGroup00-LogVol00 $mnt
```
