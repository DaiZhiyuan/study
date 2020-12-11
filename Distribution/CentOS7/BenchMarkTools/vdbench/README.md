
# 1. VDBENCH

Vdbench is a command line utility specifically created to help engineers and customers generate disk I/O workloads to be used for validating storage performance and storage data integrity. 

- Raw I/O workload
  - Host Definition (HD)
  - Replay Group (RG)
  - Storage Definition (SD)
  - Workload Definition (WD)
  - Run Definition (RD)
- File system Workload
  - Host Definition (HD)
  - File System Definition (FSD)
  - File system Workload Definition (FWD)
  - Run Definition (RD)

## 1.1 Raw I/O workload example

Ceph RDB benchmark workload:
```cmcc_rdb_workload
sd=sd1,lun=/dev/nvme1n1p1,openflags=o_direct,thread=64
wd=wd1,sd=sd1,xfersize=8k,rdpct=70,seekpct=100
rd=run1,wd=wd1,iorate=max,elapsed=604800,maxdata=300G,interval=5,warmup=30
```

benchmark on intel p4510 2T (raw device):
```
Dec 11, 2020  interval        i/o   MB/sec   bytes   read     resp     read    write     resp     resp queue  cpu%  cpu%
                             rate  1024**2     i/o    pct     time     resp     resp      max   stddev depth sys+u   sys
16:35:20.063        31  312322.60  2440.02    8192  70.06    0.198    0.253    0.069    6.746    0.262  61.7  35.9   6.6
16:35:25.061        32  311582.60  2434.24    8192  70.01    0.198    0.252    0.072   19.997    0.254  61.8  35.6   6.2
16:35:30.061        33  174228.00  1361.16    8192  69.99    0.120    0.152    0.046   21.196    0.189  20.9  34.5   4.8
16:35:35.061        34  275761.00  2154.38    8192  69.93    0.184    0.235    0.067   11.605    0.249  50.9  35.3   6.6
16:35:40.061        35  312312.20  2439.94    8192  70.06    0.197    0.252    0.069   19.005    0.259  61.7  36.0   6.6
16:35:45.067        36  311911.20  2436.81    8192  69.97    0.199    0.253    0.074   10.799    0.260  62.2  35.6   6.5
16:35:45.075 Reached maxdata=300g. rd=run1 shutting down after next interval.
16:35:46.075        37   61272.40   478.69    8192  70.02    0.193    0.245    0.070    2.571    0.236  11.8  36.1   7.2
16:35:46.092  avg_7-37  256348.49  2002.72    8192  70.00    0.177    0.224    0.068   30.083    0.241  45.3  35.8   6.1
```
> IOPS: 256.3K

## 1.2 File system workload exmaple

Ceph File System benchmark workload:
- dirs: 100; files: 20,000; bytes: 3.815t (4,194,304,000,000)
```cmcc_cephfs_workload
fsd=fsd1,anchor=/mnt/ceph_fs,depth=1,width=100,files=200,size=200m,shared=yes
fwd=fwd1,fsd=fsd1,rdpct=60,xfersize=1m,fileio=random,fileselect=random,threads=64
rd=rd1,fwd=fwd1,fwdrate=max,format=restart,elapsed=600,interval=1
```

benchmark xfs on intel p4510 2T:
- dirs: 50; files: 5,000; bytes: 976.563g (1,048,576,000,000)
```
Dec 11, 2020 .Interval. .ReqstdOps... ...cpu%...  read ....read..... ....write.... ..mb/sec... mb/sec .xfer.. ...mkdir.... ...rmdir.... ...create... ....open.... ...close.... ...delete...
                          rate   resp total  sys   pct   rate   resp   rate   resp  read write  total    size  rate   resp  rate   resp  rate   resp  rate   resp  rate   resp  rate   resp
16:18:58.063        590 3334.0 19.110  28.8 3.81  59.8 1995.0  6.600 1339.0 37.750  1995  1339 3334.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  14.0  0.049  14.0  0.018   0.0  0.000
16:18:59.063        591 3485.0 18.440  28.9 3.40  59.5 2074.0  5.489 1411.0 37.477  2074  1411 3485.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  15.0  0.040  15.0  0.013   0.0  0.000
16:19:00.063        592 3460.0 18.404  29.1 3.25  60.0 2077.0  6.152 1383.0 36.803  2077  1383 3460.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  21.0  0.044  21.0  0.013   0.0  0.000
16:19:01.063        593 3221.0 19.843  28.0 3.10  61.3 1975.0  6.551 1246.0 40.911  1975  1246 3221.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  19.0  0.048  19.0  0.016   0.0  0.000
16:19:02.063        594 3222.0 19.765  29.4 3.66  58.8 1895.0  7.083 1327.0 37.877  1895  1327 3222.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  15.0  0.041  15.0  0.012   0.0  0.000
16:19:03.063        595 3524.0 18.253  31.3 5.63  60.4 2128.0  7.827 1396.0 34.146  2128  1396 3524.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  11.0  0.047  11.0  0.017   0.0  0.000
16:19:04.063        596 3285.0 19.554  28.8 2.68  59.8 1965.0  6.547 1320.0 38.916  1965  1320 3285.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  24.0  0.041  24.0  0.017   0.0  0.000
16:19:05.063        597 3344.0 19.002  29.7 3.81  60.9 2036.0  7.982 1308.0 36.154  2036  1308 3344.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  16.0  0.045  16.0  0.020   0.0  0.000
16:19:06.063        598 3445.0 18.550  29.8 3.68  60.0 2066.0  6.476 1379.0 36.638  2066  1379 3445.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  13.0  0.050  13.0  0.015   0.0  0.000
16:19:07.063        599 3273.0 19.550  30.2 4.12  59.4 1945.0  7.347 1328.0 37.424  1945  1328 3273.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  13.0  0.042  13.0  0.013   0.0  0.000
16:19:08.063        600 3297.0 19.393  29.7 3.42  59.3 1956.0  7.464 1341.0 36.794  1956  1341 3297.0 1048576   0.0  0.000   0.0  0.000   0.0  0.000  24.0  0.050  24.0  0.014   0.0  0.000
16:19:08.082    avg_600 3492.2 18.300  29.6 3.52  60.0 2096.2  7.228 1396.0 34.925  2096  1395 3492.2 1048578   0.0  0.000   0.0  0.000   0.0  0.000  17.5  0.045  17.4  0.013   0.0  0.000
```
> result: 3492.2 mb/s