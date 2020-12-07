# 1. Bcache

Bcache is a Linux kernel block layer cache. It allows one or more fast disk drives such as flash-based solid state drives (SSDs) to act as a cache for one or more slower hard disk drives.

feature:
- A single cache device can be used to cache an arbitrary number of backing devices, and backing devices can be attached and detached at runtime, while mounted and in use (they run in passthrough mode when they don't have a cache).
- Recovers from unclean shutdown - writes are not completed until the cache is consistent with respect to the backing device (Internally, bcache doesn't distinguish between clean and unclean shutdown).
- Barriers/cache flushes are handled correctly.
- Writethrough, writeback and writearound.
- Detects and bypasses sequential IO (with a configurable threshold, and can be disabled).
- Throttles traffic to the SSD if it becomes congested, detected by latency to the SSD exceeding a configurable threshold (useful if you've got one SSD for many disks).
Readahead on cache miss (disabled by default).
- Highly efficient writeback implementation; dirty data is always written out in sorted order, and if writeback_percent is enabled background writeback is smoothly throttled with a PD controller to keep around that percentage of the cache dirty.
- Very high performance b+ tree - bcache is capable of around 1M iops on random reads, if your hardware is fast enough.
- Stable - in production use now.


## 1.1 build & install bcache tools 
```
[root@jerrydai ~]# git clone https://github.com/g2p/bcache-tools.git
[root@jerrydai ~]# cd bcache-tools && make && make install & modprobe bcache
```

## 1.2 usage bcache exmaple

1. Build cache device
```
[root@jerrydai ~]# make-bcache --cache /dev/sdc
UUID:                   d41ec2ad-5fe5-4e16-b239-3204e1639e7f
Set UUID:               67db6adb-02b7-4e6c-a59f-d4e23dba8fd8
version:                0
nbuckets:               953880
block_size:             1
bucket_size:            1024
nr_in_set:              1
nr_this_dev:            0
first_bucket:           1

[root@jerrydai ~]# bcache-super-show /dev/sdc
sb.magic                ok
sb.first_sector         8 [match]
sb.csum                 C44073FAE84433D5 [match]
sb.version              3 [cache device]

dev.label               (empty)
dev.uuid                d41ec2ad-5fe5-4e16-b239-3204e1639e7f
dev.sectors_per_block   1
dev.sectors_per_bucket  1024
dev.cache.first_sector  1024
dev.cache.cache_sectors 976772096
dev.cache.total_sectors 976773120
dev.cache.ordered       yes
dev.cache.discard       no
dev.cache.pos           0
dev.cache.replacement   0 [lru]

cset.uuid               67db6adb-02b7-4e6c-a59f-d4e23dba8fd8
```

2. Build backing device

```
[root@jerrydai ~]# make-bcache --bdev /dev/sdb
UUID:                   07760740-f3a2-4381-b74c-51d138c6e29e
Set UUID:               d5cdaca8-4526-41ba-9fbe-524e929cfaec
version:                1
block_size:             1
data_offset:            16

[root@jerrydai ~]# bcache-super-show /dev/sdb
sb.magic                ok
sb.first_sector         8 [match]
sb.csum                 97A66824EAC5CFF7 [match]
sb.version              1 [backing device]

dev.label               (empty)
dev.uuid                07760740-f3a2-4381-b74c-51d138c6e29e
dev.sectors_per_block   1
dev.sectors_per_bucket  1024
dev.data.first_sector   16
dev.data.cache_mode     0 [writethrough]
dev.data.cache_state    0 [detached]

cset.uuid               d5cdaca8-4526-41ba-9fbe-524e929cfaec
```

3. Attach Cache disk
```
[root@jerrydai ~]# echo "67db6adb-02b7-4e6c-a59f-d4e23dba8fd8" > /sys/block/bcache0/bcache/attach
[root@jerrydai ~]# lsblk
NAME      MAJ:MIN RM   SIZE RO TYPE MOUNTPOINT
sdb         8:16   0   3.7T  0 disk
└─bcache0 251:0    0   3.7T  0 disk
sr0        11:0    1  1024M  0 rom
sdc         8:32   0 465.8G  0 disk
└─bcache0 251:0    0   3.7T  0 disk
sda         8:0    0 465.8G  0 disk
├─sda4      8:4    0   100G  0 part /
├─sda2      8:2    0     1G  0 part /boot
├─sda3      8:3    0     4G  0 part [SWAP]
└─sda1      8:1    0   200M  0 part /boot/efi

[root@jerrydai ~]# mkfs.xfs /dev/bcache0
meta-data=/dev/bcache0           isize=512    agcount=4, agsize=244154368 blks
         =                       sectsz=512   attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=976617470, imaxpct=5
         =                       sunit=0      swidth=0 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=476863, version=2
         =                       sectsz=512   sunit=0 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
```

4. optimize bcache
```

# set cache mode
[root@jerrydai ~]# cat /sys/block/bcache0/bcache/cache_mode
[writethrough] writeback writearound none
[root@jerrydai ~]# echo "writeback" > /sys/block/bcache0/bcache/cache_mode

# cache sequential I/O
[root@jerrydai ~]# echo 0 > /sys/block/bcache0/bcache/sequential_cutoff

# turnoff congestion management
[root@jerrydai ~]# echo 0 > /sys/fs/bcache/67db6adb-02b7-4e6c-a59f-d4e23dba8fd8/congested_write_threshold_us
[root@jerrydai ~]# echo 0 > /sys/fs/bcache/67db6adb-02b7-4e6c-a59f-d4e23dba8fd8/congested_read_threshold_us
```

5. stop bcahe device
```
[root@jerrydai ~]# echo 67db6adb-02b7-4e6c-a59f-d4e23dba8fd8 > /sys/block/bcache0/bcache/detach
[root@jerrydai ~]# echo 1 > /sys/fs/bcache/67db6adb-02b7-4e6c-a59f-d4e23dba8fd8/unregister
[root@jerrydai ~]# echo 1 > /sys/block/bcache0/bcache/stop
```

## 1.3 use fio to meeasure disk performance

- HDD: HGST HUS726T4TALE6L4 4TB 
- SSD: Samsung SSD 860 EVO 500GB 

workload:
```
# rand read
fio --filename=/dev/sdX --name=randread --ioengine=libaio --iodepth=1 --rw=randread --bs=4k --direct=1 --size=1G --numjobs=64 --runtime=240 --group_reporting

# rand write 
fio --filename=/dev/sdX --name=randwrite --ioengine=libaio --iodepth=1 --rw=randwrite --bs=4k --direct=1 --size=1G --numjobs=64 --runtime=240 --group_reporting
```

### 1.3.1 HDD disk

- rand read: IOPS = 3.4K, bandwidth = 14.2MB/s, latency 50 ~ 100 msec.
- rand write: IOPS = 1.0K, bandwidth = 4.2MB/s, latency 100 msec.

### 1.3.2 bcache disk

- normal:
    - rand read: IOPS = 57.8k, bandwidth = 237MB/s, latency 500 usec ~ 2 msec.
    - rand write: IOPS = 1.0K, bandwidth = 4.2MB/s, latency 100 msec.

- optimize:
    - rand read: IPS = 75.8K, bandwidth = 310MB/s, latency 500 usec ~ 2 msec.
    - rand write: IOPS = 29.0k, bandwidth = 119MB/s, latency 250 usec.


### 1.3.3 bar graph

![avatar](./Images/bcache.png)

