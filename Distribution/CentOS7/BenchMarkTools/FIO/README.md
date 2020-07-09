
# 1. FIO —— flexible I/O tester

test procedures:
```
[root@jerrydai ~]# cat general_disk_test.fio
[global]
bs=8k
ioengine=libaio
iodepth=32
size=10g
direct=1
stonewall
runtime=60
directory=/path/to/directory
filename=general_disk_test.file

[seq-read]
rw=read

[rand-read]
rw=randread

[seq-write]
rw=write

[rand-write]
rw=randwrite

[root@jerrydai ~]# fio -v
fio-3.7

[root@jerrydai ~]# fio general_disk_test.fio
```

## 1.1 RAID Stripe Intel SSDSC2KB960G8 900GB x4

` storcli64 /c0 add vd r0 drives=252:1-4 pdcache=off WT ra Cached Strip=64`

```
-------------------------------------------------------------
DG/VD TYPE  State Access Consist Cache Cac sCC     Size Name
-------------------------------------------------------------
0/0   RAID0 Optl  RW     Yes     RWTC  -   ON  3.491 TB
-------------------------------------------------------------

---------------------------------------------------------------------------------
EID:Slt DID State DG       Size Intf Med SED PI SeSz Model               Sp Type
---------------------------------------------------------------------------------
252:1    19 Onln   0 893.750 GB SATA SSD N   N  512B INTEL SSDSC2KB960G8 U  -
252:2    17 Onln   0 893.750 GB SATA SSD N   N  512B INTEL SSDSC2KB960G8 U  -
252:3    16 Onln   0 893.750 GB SATA SSD N   N  512B INTEL SSDSC2KB960G8 U  -
252:4    23 Onln   0 893.750 GB SATA SSD N   N  512B INTEL SSDSC2KB960G8 U  -
---------------------------------------------------------------------------------
```

Case | IOPS | Throughput | Latency
---|---|---|---
seq-read| 64.8k | 507MiB/s | 500 usec
rand-read | 39.7k | 311MiB/s | 1000 usec
seq-write | 69.5k | 543MiB/s | 500 usec
rand-write | 65.6k | 513MiB/s | 650 usec

> Block Size is 8K.

## 1.2 NVME Samsung 970EVO plus 1TB

Case | IOPS | Throughput | Latency
---|---|---|---
seq-read| 105k | 824MiB/s | 500 usec
rand-read | 97.0k | 758MiB/s | 500 usec
seq-write | 106k | 825MiB/s | 500 usec
rand-write | 99.7k |779MiB/s | 500 usec

## 1.3 LVM Stripe Samsung 970EVO plus 1TB x2

```
[root@jerrydai ~]# pvcreate /dev/nvme0n1 /dev/nvme1n1
  Physical volume "/dev/nvme0n1" successfully created.
  Physical volume "/dev/nvme1n1" successfully created.
  
[root@jerrydai ~]# vgcreate vg_database /dev/nvme0n1 /dev/nvme1n1
  Volume group "vg_database" successfully created

[root@jerrydai ~]# lvcreate -i 2 -I 64 -l 100%Free -n lv_database /dev/vg_database
  Logical volume "lv_database" created.
[root@jerrydai ~]# lvs
  LV          VG          Attr       LSize  Pool Origin Data%  Meta%  Move Log Cpy%Sync Convert
  lv_database vg_database -wi-a----- <1.82t

[root@jerrydai ~]# mkfs.xfs /dev/vg_database/lv_database
meta-data=/dev/vg_database/lv_database isize=512    agcount=32, agsize=15261872 blks
         =                       sectsz=512   attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=488379904, imaxpct=5
         =                       sunit=16     swidth=32 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=238480, version=2
         =                       sectsz=512   sunit=16 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
```

Case | IOPS | Throughput | Latency
---|---|---|---
seq-read| 83.1k | 649MiB/s | 500 usec
rand-read| 86.2k | 674MiB/s | 500 usec
seq-write| 92.7k | 724MiB/s | 500 usec
rand-write | 88.0k | 688MiB/s | 500 usec

> Block Size is 8K.


## 1.4 MultiDevice Samsung 970EVO plus 1TB x2

```
[root@jerrydai ~]# mdadm --create --verbose /dev/md0 --level=0 --raid-devices=2 /dev/nvme0n1 /dev/nvme1n1
mdadm: chunk size defaults to 512K
mdadm: Defaulting to version 1.2 metadata
mdadm: array /dev/md0 started.

[root@jerrydai ~]# cat /proc/mdstat
Personalities : [raid0]
md0 : active raid0 nvme1n1[1] nvme0n1[0]
      1953260544 blocks super 1.2 512k chunks

unused devices: <none>

[root@jerrydai ~]# mkfs.xfs /dev/md0
meta-data=/dev/md0               isize=512    agcount=32, agsize=15259776 blks
         =                       sectsz=512   attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=488312832, imaxpct=5
         =                       sunit=128    swidth=256 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=238440, version=2
         =                       sectsz=512   sunit=8 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
```

Case | IOPS | Throughput | Latency
---|---|---|---
seq-read| 95.0k | 742MiB/s | 500 usec
rand-read| 94.7k | 740MiB/s | 500 usec
seq-write| 98.0k | 773MiB/s | 500 usec
rand-write | 93.0k |734MiB/s | 500 usec

> Block Size is 8K.

# 2. ioping —— simple disk I/O latency monitoring tool

`ioping -warmup 10 -size 8k -direct -read-write -count 50 /directory`

- NVME Single ：min/avg/max = `67.8 us` / `7.16 ms` / `7.88 ms`
- NVME MultiDevice: min/avg/max = `2.63 ms / 10.2 ms / 14.0 ms` 
- NVME Stripe ：min/avg/max = `6.83 ms` / `10.2 ms` / `14.0 ms`
- RAID Stripe ：min/avg/max = `99.0 us` / `256.8 us` / `3.74 ms`

