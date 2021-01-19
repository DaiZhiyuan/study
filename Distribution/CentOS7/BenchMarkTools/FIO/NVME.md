# NVME磁盘FIO测试

- 处理器：FT2000plus/64core
- 操作系统：CentOS Linux 7 (AltArch)
- 文件系统：xfs
- 测试工具：FIO 3.7
- 测试磁盘：Intel Corporation NVMe Datacenter SSD [3DNAND] SE 2.5" U.2 (P4510)
- 链路速度：Speed 8GT/s, Width x4

```
[root@centos7 ~]# nvme list
Node             SN                   Model                                    Namespace Usage                      Format           FW Rev
---------------- -------------------- ---------------------------------------- --------- -------------------------- ---------------- --------
/dev/nvme0n1     PHLJ908000TJ2P0BGN   INTEL SSDPE2KX020T8                      1           2.00  TB /   2.00  TB      4 KiB +  0 B   VDV10131
/dev/nvme1n1     PHLJ915100MH2P0BGN   INTEL SSDPE2KX020T8                      1           2.00  TB /   2.00  TB      4 KiB +  0 B   VDV10131
```

## 单块NVME测试

### 场景1：100%随机 100%读 4k

> read: IOPS=422k, BW=1650MiB/s (1730MB/s)

```
[root@centos7 ~]# fio -filename=/disk/database.file -direct=1 -iodepth 1 -thread -rw=randread -ioengine=psync -bs=4k -size=10G -numjobs=64 -runtime=180 -group_reporting -name=rand_100read_4k
rand_100read_4k: (g=0): rw=randread, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=psync, iodepth=1
...
fio-3.7
Starting 64 threads
rand_100read_4k: Laying out IO file (1 file / 10240MiB)
Jobs: 64 (f=64): [r(64)][100.0%][r=1544MiB/s,w=0KiB/s][r=395k,w=0 IOPS][eta 00m:00s]
rand_100read_4k: (groupid=0, jobs=64): err= 0: pid=59502: Tue Jan 19 14:42:42 2021
   read: IOPS=422k, BW=1650MiB/s (1730MB/s)(290GiB/180003msec)
    clat (usec): min=34, max=36494, avg=149.20, stdev=76.38
     lat (usec): min=34, max=36494, avg=149.43, stdev=76.39
    clat percentiles (usec):
     |  1.00th=[   75],  5.00th=[   83], 10.00th=[   90], 20.00th=[  100],
     | 30.00th=[  111], 40.00th=[  124], 50.00th=[  135], 60.00th=[  145],
     | 70.00th=[  161], 80.00th=[  186], 90.00th=[  227], 95.00th=[  269],
     | 99.00th=[  379], 99.50th=[  433], 99.90th=[  668], 99.95th=[ 1057],
     | 99.99th=[ 2278]
   bw (  KiB/s): min=21296, max=27824, per=1.56%, avg=26406.04, stdev=713.77, samples=22997
   iops        : min= 5324, max= 6956, avg=6601.49, stdev=178.44, samples=22997
  lat (usec)   : 50=0.01%, 100=19.89%, 250=73.30%, 500=6.56%, 750=0.17%
  lat (usec)   : 1000=0.03%
  lat (msec)   : 2=0.04%, 4=0.02%, 10=0.01%, 20=0.01%, 50=0.01%
  cpu          : usr=2.46%, sys=8.14%, ctx=76058166, majf=0, minf=4433
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=76046821,0,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1

Run status group 0 (all jobs):
   READ: bw=1650MiB/s (1730MB/s), 1650MiB/s-1650MiB/s (1730MB/s-1730MB/s), io=290GiB (311GB), run=180003-180003msec

Disk stats (read/write):
  nvme0n1: ios=75996647/10, merge=0/3, ticks=10535355/1, in_queue=10, util=100.00%
```

### 场景2：100%随机 100%写 4k

> write: IOPS=436k, BW=1702MiB/s (1785MB/s)

```
[root@centos7 ~]# fio -filename=/disk/database.file -direct=1 -iodepth 1 -thread -rw=randwrite -ioengine=psync -bs=4k -size=10G -numjobs=64 -runtime=180 -group_reporting -name=rand_100write_4k
rand_100write_4k: (g=0): rw=randwrite, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=psync, iodepth=1
...
fio-3.7
Starting 64 threads
Jobs: 64 (f=64): [w(64)][100.0%][r=0KiB/s,w=1436MiB/s][r=0,w=368k IOPS][eta 00m:00s]
rand_100write_4k: (groupid=0, jobs=64): err= 0: pid=7873: Tue Jan 19 14:48:47 2021
  write: IOPS=436k, BW=1702MiB/s (1785MB/s)(299GiB/180003msec)
    clat (usec): min=17, max=21808, avg=144.34, stdev=127.08
     lat (usec): min=17, max=21808, avg=144.66, stdev=127.08
    clat percentiles (usec):
     |  1.00th=[   21],  5.00th=[   24], 10.00th=[   26], 20.00th=[   33],
     | 30.00th=[   48], 40.00th=[   88], 50.00th=[  125], 60.00th=[  155],
     | 70.00th=[  180], 80.00th=[  219], 90.00th=[  297], 95.00th=[  383],
     | 99.00th=[  586], 99.50th=[  693], 99.90th=[  938], 99.95th=[ 1045],
     | 99.99th=[ 1352]
   bw (  KiB/s): min=20384, max=34880, per=1.56%, avg=27241.89, stdev=2449.96, samples=22992
   iops        : min= 5096, max= 8720, avg=6810.46, stdev=612.49, samples=22992
  lat (usec)   : 20=0.04%, 50=30.80%, 100=12.09%, 250=42.11%, 500=13.04%
  lat (usec)   : 750=1.58%, 1000=0.27%
  lat (msec)   : 2=0.07%, 4=0.01%, 10=0.01%, 20=0.01%, 50=0.01%
  cpu          : usr=2.64%, sys=13.58%, ctx=78684870, majf=0, minf=9108
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=0,78431779,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1

Run status group 0 (all jobs):
  WRITE: bw=1702MiB/s (1785MB/s), 1702MiB/s-1702MiB/s (1785MB/s-1785MB/s), io=299GiB (321GB), run=180003-180003msec

Disk stats (read/write):
  nvme0n1: ios=0/78397365, merge=0/0, ticks=0/9852401, in_queue=0, util=100.00%
```

### 场景3：100%顺序 100%读 4k

> read: IOPS=427k, BW=1669MiB/s (1750MB/s)

```
[root@centos7 ~]# fio -filename=/disk/database.file -direct=1 -iodepth 1 -thread -rw=read -ioengine=psync -bs=4k -size=10G -numjobs=64 -runtime=180 -group_reporting -name=seq_100read_4k
seq_100read_4k: (g=0): rw=read, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=psync, iodepth=1
...
fio-3.7
Starting 64 threads
Jobs: 64 (f=64): [R(64)][100.0%][r=1692MiB/s,w=0KiB/s][r=433k,w=0 IOPS][eta 00m:00s]
seq_100read_4k: (groupid=0, jobs=64): err= 0: pid=8578: Tue Jan 19 14:59:13 2021
   read: IOPS=427k, BW=1669MiB/s (1750MB/s)(293GiB/180005msec)
    clat (usec): min=31, max=21347, avg=147.96, stdev=107.89
     lat (usec): min=31, max=21347, avg=148.19, stdev=107.89
    clat percentiles (usec):
     |  1.00th=[   73],  5.00th=[   81], 10.00th=[   88], 20.00th=[   97],
     | 30.00th=[  106], 40.00th=[  120], 50.00th=[  131], 60.00th=[  141],
     | 70.00th=[  155], 80.00th=[  180], 90.00th=[  223], 95.00th=[  265],
     | 99.00th=[  388], 99.50th=[  469], 99.90th=[ 1811], 99.95th=[ 2311],
     | 99.99th=[ 2999]
   bw (  KiB/s): min=20728, max=28416, per=1.56%, avg=26698.21, stdev=702.69, samples=22994
   iops        : min= 5182, max= 7104, avg=6674.53, stdev=175.67, samples=22994
  lat (usec)   : 50=0.01%, 100=23.36%, 250=70.32%, 500=5.91%, 750=0.18%
  lat (usec)   : 1000=0.04%
  lat (msec)   : 2=0.11%, 4=0.08%, 10=0.01%, 20=0.01%, 50=0.01%
  cpu          : usr=2.11%, sys=7.71%, ctx=76900550, majf=0, minf=3259
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=76897835,0,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1

Run status group 0 (all jobs):
   READ: bw=1669MiB/s (1750MB/s), 1669MiB/s-1669MiB/s (1750MB/s-1750MB/s), io=293GiB (315GB), run=180005-180005msec

Disk stats (read/write):
  nvme0n1: ios=76850430/6, merge=0/0, ticks=10620596/0, in_queue=0, util=100.00%
```

### 场景4：100%顺序 100%写 4k

> write: IOPS=463k, BW=1809MiB/s (1897MB/s)

```
[root@centos7 ~]# fio -filename=/disk/database.file -direct=1 -iodepth 1 -thread -rw=write -ioengine=psync -bs=4k -size=10G -numjobs=64 -runtime=180 -group_reporting -name=seq_100write_4k
seq_100write_4k: (g=0): rw=write, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=psync, iodepth=1
...
fio-3.7
Starting 64 threads
Jobs: 64 (f=64): [W(64)][100.0%][r=0KiB/s,w=1654MiB/s][r=0,w=423k IOPS][eta 00m:00s]
seq_100write_4k: (groupid=0, jobs=64): err= 0: pid=8863: Tue Jan 19 15:02:41 2021
  write: IOPS=463k, BW=1809MiB/s (1897MB/s)(318GiB/180003msec)
    clat (usec): min=17, max=21965, avg=136.07, stdev=118.19
     lat (usec): min=17, max=21966, avg=136.39, stdev=118.19
    clat percentiles (usec):
     |  1.00th=[   21],  5.00th=[   25], 10.00th=[   29], 20.00th=[   39],
     | 30.00th=[   58], 40.00th=[   81], 50.00th=[  105], 60.00th=[  137],
     | 70.00th=[  167], 80.00th=[  206], 90.00th=[  281], 95.00th=[  359],
     | 99.00th=[  553], 99.50th=[  644], 99.90th=[  914], 99.95th=[ 1057],
     | 99.99th=[ 1500]
   bw (  KiB/s): min=21096, max=36616, per=1.56%, avg=28945.04, stdev=2252.91, samples=22989
   iops        : min= 5274, max= 9154, avg=7236.24, stdev=563.23, samples=22989
  lat (usec)   : 20=0.07%, 50=26.64%, 100=21.39%, 250=38.70%, 500=11.72%
  lat (usec)   : 750=1.23%, 1000=0.19%
  lat (msec)   : 2=0.06%, 4=0.01%, 10=0.01%, 20=0.01%, 50=0.01%
  cpu          : usr=2.52%, sys=11.64%, ctx=83607489, majf=0, minf=3707
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=0,83346258,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1

Run status group 0 (all jobs):
  WRITE: bw=1809MiB/s (1897MB/s), 1809MiB/s-1809MiB/s (1897MB/s-1897MB/s), io=318GiB (341GB), run=180003-180003msec

Disk stats (read/write):
  nvme0n1: ios=0/83302839, merge=0/0, ticks=0/10118868, in_queue=0, util=100.00%
```

### 场景5：100%随机 70%读 30%写 4k

> read: IOPS=256k, BW=1000MiB/s (1048MB/s) & write: IOPS=110k, BW=428MiB/s (449MB/s)

```
[root@centos7 ~]# fio -filename=/disk/database.file -direct=1 -iodepth 1 -thread -rw=randrw -rwmixread=70 -ioengine=psync -bs=4k -size=10G -numjobs=64 -runtime=180 -group_reporting -name=randrw_70read_4k
randrw_70read_4k: (g=0): rw=randrw, bs=(R) 4096B-4096B, (W) 4096B-4096B, (T) 4096B-4096B, ioengine=psync, iodepth=1
...
fio-3.7
Starting 64 threads
Jobs: 64 (f=64): [m(64)][100.0%][r=963MiB/s,w=413MiB/s][r=246k,w=106k IOPS][eta 00m:00s]
randrw_70read_4k: (groupid=0, jobs=64): err= 0: pid=9198: Tue Jan 19 15:06:06 2021
   read: IOPS=256k, BW=1000MiB/s (1048MB/s)(176GiB/180002msec)
    clat (usec): min=33, max=17878, avg=231.76, stdev=224.12
     lat (usec): min=33, max=17878, avg=231.98, stdev=224.12
    clat percentiles (usec):
     |  1.00th=[   72],  5.00th=[   83], 10.00th=[   91], 20.00th=[  109],
     | 30.00th=[  127], 40.00th=[  143], 50.00th=[  165], 60.00th=[  192],
     | 70.00th=[  231], 80.00th=[  285], 90.00th=[  400], 95.00th=[  611],
     | 99.00th=[ 1287], 99.50th=[ 1500], 99.90th=[ 1975], 99.95th=[ 2180],
     | 99.99th=[ 2606]
   bw (  KiB/s): min=13160, max=17856, per=1.56%, avg=15993.49, stdev=614.20, samples=22979
   iops        : min= 3290, max= 4464, avg=3998.35, stdev=153.55, samples=22979
  write: IOPS=110k, BW=428MiB/s (449MB/s)(75.3GiB/180002msec)
    clat (usec): min=16, max=1665, avg=34.90, stdev=18.57
     lat (usec): min=16, max=1665, avg=35.26, stdev=18.57
    clat percentiles (usec):
     |  1.00th=[   21],  5.00th=[   22], 10.00th=[   23], 20.00th=[   25],
     | 30.00th=[   26], 40.00th=[   28], 50.00th=[   30], 60.00th=[   33],
     | 70.00th=[   36], 80.00th=[   41], 90.00th=[   51], 95.00th=[   70],
     | 99.00th=[  116], 99.50th=[  137], 99.90th=[  190], 99.95th=[  208],
     | 99.99th=[  247]
   bw (  KiB/s): min= 5216, max= 8240, per=1.56%, avg=6854.95, stdev=370.82, samples=22979
   iops        : min= 1304, max= 2060, avg=1713.71, stdev=92.72, samples=22979
  lat (usec)   : 20=0.01%, 50=26.87%, 100=13.62%, 250=41.29%, 500=13.55%
  lat (usec)   : 750=2.00%, 1000=1.10%
  lat (msec)   : 2=1.50%, 4=0.06%, 10=0.01%, 20=0.01%
  cpu          : usr=2.17%, sys=6.71%, ctx=66069132, majf=0, minf=6897
  IO depths    : 1=100.0%, 2=0.0%, 4=0.0%, 8=0.0%, 16=0.0%, 32=0.0%, >=64=0.0%
     submit    : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     complete  : 0=0.0%, 4=100.0%, 8=0.0%, 16=0.0%, 32=0.0%, 64=0.0%, >=64=0.0%
     issued rwts: total=46062968,19742678,0,0 short=0,0,0,0 dropped=0,0,0,0
     latency   : target=0, window=0, percentile=100.00%, depth=1

Run status group 0 (all jobs):
   READ: bw=1000MiB/s (1048MB/s), 1000MiB/s-1000MiB/s (1048MB/s-1048MB/s), io=176GiB (189GB), run=180002-180002msec
  WRITE: bw=428MiB/s (449MB/s), 428MiB/s-428MiB/s (449MB/s-449MB/s), io=75.3GiB (80.9GB), run=180002-180002msec

Disk stats (read/write):
  nvme0n1: ios=46035023/19731053, merge=0/0, ticks=10207638/480183, in_queue=0, util=100.00%
```

## 两块NVME测试

### LVM Stripe

```
[root@centos7 ~]# pvcreate /dev/nvme0n1 /dev/nvme1n1
  Physical volume "/dev/nvme0n1" successfully created.
  Physical volume "/dev/nvme1n1" successfully created.

[root@centos7 ~]# vgcreate vg_database /dev/nvme0n1 /dev/nvme1n1
  Volume group "vg_database" successfully created

[root@centos7 ~]# lvcreate -i 2 -I 64 -l 100%Free -n lv_database /dev/vg_database
  Rounding size (953859 extents) down to stripe boundary size (953858 extents)
  Logical volume "lv_database" created.

[root@centos7 ~]# lvs
  LV          VG          Attr       LSize  Pool Origin Data%  Meta%  Move Log Cpy%Sync Convert
  lv_database vg_database -wi-a----- <3.64t

[root@centos7 ~]# mkfs.xfs /dev/vg_database/lv_database
meta-data=/dev/vg_database/lv_database isize=512    agcount=32, agsize=30523312 blks
         =                       sectsz=4096  attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=976745984, imaxpct=5
         =                       sunit=16     swidth=32 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=476926, version=2
         =                       sectsz=4096  sunit=1 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
```

- 场景1：100%随机 100%读 4k：`read: IOPS=343k, BW=1338MiB/s (1403MB/s)`
- 场景2：100%随机 100%写 4k: `write: IOPS=694k, BW=2710MiB/s (2841MB/s)`
- 场景3：100%顺序 100%读 4k: `read: IOPS=314k, BW=1225MiB/s (1285MB/s)`
- 场景4：100%顺序 100%写 4k: `write: IOPS=696k, BW=2719MiB/s (2851MB/s)`
- 场景5：100%随机 70%读 30%写 4k: `read: IOPS=319k, BW=1247MiB/s (1307MB/s) & write: IOPS=137k, BW=534MiB/s (560MB/s)`

### Software RAID

```
[root@centos7 ~]# mdadm --create --verbose /dev/md0 --level=0 --raid-devices=2 /dev/nvme0n1 /dev/nvme1n1
mdadm: chunk size defaults to 512K
mdadm: Defaulting to version 1.2 metadata
mdadm: array /dev/md0 started.
[root@centos7 ~]# cat /proc/mdstat
Personalities : [raid0]
md0 : active raid0 nvme1n1[1] nvme0n1[0]
      3906747392 blocks super 1.2 512k chunks

unused devices: <none>
[root@centos7 ~]# mkfs.xfs /dev/md0
meta-data=/dev/md0               isize=512    agcount=32, agsize=30521472 blks
         =                       sectsz=4096  attr=2, projid32bit=1
         =                       crc=1        finobt=0, sparse=0
data     =                       bsize=4096   blocks=976686848, imaxpct=5
         =                       sunit=128    swidth=256 blks
naming   =version 2              bsize=4096   ascii-ci=0 ftype=1
log      =internal log           bsize=4096   blocks=476897, version=2
         =                       sectsz=4096  sunit=1 blks, lazy-count=1
realtime =none                   extsz=4096   blocks=0, rtextents=0
```

- 场景1：100%随机 100%读 4k：`read: IOPS=311k, BW=1216MiB/s (1275MB/s)`
- 场景2：100%随机 100%写 4k: `write: IOPS=586k, BW=2287MiB/s (2399MB/s)`
- 场景3：100%顺序 100%读 4k: `read: IOPS=310k, BW=1211MiB/s (1270MB/s)`
- 场景4：100%顺序 100%写 4k: `write: IOPS=651k, BW=2544MiB/s (2668MB/s)`
- 场景5：100%随机 70%读 30%写 4k: `read: IOPS=310k, BW=1213MiB/s (1271MB/s) & write: IOPS=133k, BW=520MiB/s (545MB/s)`

## 结果比较


 Intel P4510 | Single Nvme | Lvm Strip | Software RAID
---|---|---|---
rand_read | IOPS=422k, BW=1730MB/s | IOPS=343k, BW=1403MB/s | IOPS=311k, BW=1275MB/s
rand_write | IOPS=436k, BW=1785MB/s | IOPS=694k, BW=2841MB/s | IOPS=586k, BW=2399MB/s
seq_read | IOPS=427k, BW=1750MB/s | IOPS=314k, BW=1285MB/s | IOPS=310k, BW=1270MB/s
seq_write | IOPS=463k, BW=1897MB/s | IOPS=696k, BW=2851MB/s | IOPS=651k, BW=2668MB/s
oltp | read: IOPS=256k, BW=1048MB/s <br/> write: IOPS=110k, BW=449MB/s | read: IOPS=319k, BW=1307MB/s </br> write: IOPS=137k, BW=560MB/s | read: IOPS=310k, BW=1271MB/s <br/> write: IOPS=133k, BW=545MB/s