# Bcache简介

Bcache是Linux内核块设备层cache，支持多块HDD使用同一块SSD作为缓存盘。它让SSD作为HDD的缓存成为了可能。由于SSD价格昂贵，存储空间小，而HDD价格低廉，存储空间大，因此采用SSD作为缓存，HDD作为数据存储盘，既解决了SSD容量太小，又解决了HDD运行速度太慢的问题。

## Bcache缓存策略

Bcache支持三种缓存策略，分别是：`writeback`、`writethrough`、`writearoud`，默认使用writethrough，缓存策略可动态修改。

- writeback 回写策略：回写策略默认是关闭的，如果开启此策略，则所有的数据将先写入缓存盘，然后等待系统将数据回写入后端数据盘中。
- writethrough 写通策略：默认的就是写通策略，此模式下，数据将会同时写入缓存盘和后端数据盘。
- writearoud ：选择此策略，数据将直接写入后端磁盘。


## bcache-tools的安装与使用

```
[root@jerrydai opt]# git clone https://github.com/g2p/bcache-tools.git
Cloning into 'bcache-tools'...
remote: Enumerating objects: 498, done.
remote: Total 498 (delta 0), reused 0 (delta 0), pack-reused 498
Receiving objects: 100% (498/498), 106.14 KiB | 44.00 KiB/s, done.
Resolving deltas: 100% (257/257), done.

[root@jerrydai bcache-tools]# make
cc -O2 -Wall -g `pkg-config --cflags uuid blkid`   -c -o bcache.o bcache.c
cc -O2 -Wall -g `pkg-config --cflags uuid blkid`    make-bcache.c bcache.o  `pkg-config --libs uuid blkid` -o make-bcache
cc -O2 -Wall -g `pkg-config --cflags uuid blkid`    probe-bcache.c  `pkg-config --libs uuid blkid` -o probe-bcache
cc -O2 -Wall -g -std=gnu99    bcache-super-show.c bcache.o  `pkg-config --libs uuid` -o bcache-super-show
cc -O2 -Wall -g   -c -o bcache-register.o bcache-register.c
cc   bcache-register.o   -o bcache-register

[root@jerrydai bcache-tools]# make install
install -m0755 make-bcache bcache-super-show    /usr/sbin/
install -m0755 probe-bcache bcache-register             /lib/udev/
install -m0644 69-bcache.rules  /lib/udev/rules.d/
install -m0644 -- *.8 /usr/share/man/man8/
install -D -m0755 initramfs/hook        /usr/share/initramfs-tools/hooks/bcache
install -D -m0755 initcpio/install      /usr/lib/initcpio/install/bcache
install -D -m0755 dracut/module-setup.sh /lib/dracut/modules.d/90bcache/module-setup.sh

[root@jerrydai bcache-tools]# modprobe bcache
[root@jerrydai bcache-tools]# modinfo bcache
filename:       /lib/modules/4.19.105-phytium-centos/kernel/drivers/md/bcache/bcache.ko
license:        GPL
author:         Kent Overstreet <koverstreet@google.com>
author:         Kent Overstreet <kent.overstreet@gmail.com>
license:        GPL
srcversion:     28D5FBB5FFFAE0A55272E72
depends:        crc64
intree:         Y
name:           bcache
vermagic:       4.19.105-phytium-centos SMP mod_unload modversions aarch64
```

## bcache-tools的使用

1. build Cache Disk
```
[root@jerrydai ~]# make-bcache --bdev /dev/sdb
UUID:                   bf7819b5-f72b-4774-ba0c-317e382b8a51
Set UUID:               0578e0f6-c576-4f6a-977e-d437faefaf19
version:                1
block_size:             1
data_offset:            16
```

2. Build Data Disk
```
[root@jerrydai ~]# make-bcache --cache /dev/nvme1n1
Device /dev/nvme1n1 already has a non-bcache superblock, remove it using wipefs and wipefs -a
[root@jerrydai ~]# wipefs -a /dev/nvme1n1
/dev/nvme1n1: 4 bytes were erased at offset 0x00000000 (xfs): 58 46 53 42
[root@jerrydai ~]# make-bcache --cache /dev/nvme1n1
UUID:                   eb88738d-1a6f-4fc0-bf75-11cda1fa7e70
Set UUID:               a434e109-00b6-484a-89fb-9731d04baa4f
version:                0
nbuckets:               1907739
block_size:             1
bucket_size:            1024
nr_in_set:              1
nr_this_dev:            0
first_bucket:           1

[root@jerrydai ~]# bcache-super-show /dev/nvme1n1
sb.magic                ok
sb.first_sector         8 [match]
sb.csum                 4A01306B20615C22 [match]
sb.version              0 [cache device]

dev.label               (empty)
dev.uuid                eb88738d-1a6f-4fc0-bf75-11cda1fa7e70
dev.sectors_per_block   1
dev.sectors_per_bucket  1024
dev.cache.first_sector  1024
dev.cache.cache_sectors 1953523712
dev.cache.total_sectors 1953524736
dev.cache.ordered       no
dev.cache.discard       no
dev.cache.pos           0
dev.cache.replacement   0 [lru]

cset.uuid               a434e109-00b6-484a-89fb-9731d04baa4f

[root@jerrydai ~]# echo "a434e109-00b6-484a-89fb-9731d04baa4f" > /sys/block/bcache0/bcache/attach
```
3. check
```
[root@jerrydai ~]# lsblk /dev/nvme1n1 /dev/sdb
NAME      MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
nvme1n1   259:0    0 931.5G  0 disk
`-bcache0 252:0    0 894.3G  0 disk
sdb         8:16   0 894.3G  0 disk
`-bcache0 252:0    0 894.3G  0 disk

[root@jerrydai ~]# mkfs.xfs /dev/bcache0
meta-data=/dev/bcache0           isize=512    agcount=4, agsize=58605781 blks
         =                       sectsz=512   attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=234423124, imaxpct=25
         =                       sunit=0      swidth=0 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=114464, version=2
         =                       sectsz=512   sunit=0 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
[root@jerrydai ~]#

[root@jerrydai ~]# mount /dev/bcache0 /mnt/
```
4. tuning
```
[root@jerrydai ~]# cat /sys/block/bcache0/bcache/cache_mode
[writethrough] writeback writearound none
[root@jerrydai ~]# echo "writeback" > /sys/block/bcache0/bcache/cache_mode

[root@jerrydai ~]# cat /sys/block/bcache0/bcache/sequential_cutoff
4.0M

[root@jerrydai ~]# cat /sys/fs/bcache/a434e109-00b6-484a-89fb-9731d04baa4f/congested_read_threshold_us
2000
[root@jerrydai ~]# echo 0 > /sys/fs/bcache/a434e109-00b6-484a-89fb-9731d04baa4f/congested_read_threshold_us
[root@jerrydai ~]# cat /sys/fs/bcache/a434e109-00b6-484a-89fb-9731d04baa4f/congested_write_threshold_us
20000
[root@jerrydai ~]# echo 0 > /sys/fs/bcache/a434e109-00b6-484a-89fb-9731d04baa4f/congested_write_threshold_us

```

5. delete bcache disk
```
[root@jerrydai ~]# echo "a434e109-00b6-484a-89fb-9731d04baa4f" > /sys/block/bcache0/bcache/detach
[root@jerrydai ~]# echo 1 > /sys/fs/bcache/a434e109-00b6-484a-89fb-9731d04baa4f/unregister
[root@jerrydai ~]# echo 1 > /sys/block/bcache0/bcache/stop
```