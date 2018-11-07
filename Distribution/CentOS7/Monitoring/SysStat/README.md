- 1 SysStat
    - 1.1 安装
    - 1.2 配置周期性任务
    - 1.3 配置
    - 1.4 使用
        - 1.4.1 显示今天的统计报告
        - 1.4.2 指定一个文件显示过去的统计信息
        - 1.4.3 显示当前统计信息
        - 1.4.4 SysStat其他统计信息命令

# 1. SysStat

sysstat是一个软件包，包含监测系统性能及效率的一组工具，这些工具对于我们收集系统性能数据，比如CPU使用率、硬盘和网络吞吐数据，这些数据的收集和分析，有利于我们判断系统是否正常运行，是提高系统运行效率、安全运行服务器的得力助手。

## 1.1 安装

```shell
[root@lab ~]# yum -y install sysstat

[root@lab ~]# systemctl start sysstat 
[root@lab ~]# systemctl enable sysstat
```

## 1.2 配置周期性任务

- 使用/usr/lib64/sa/sa1命令，每隔10秒，日志存储到/var/log/sa/sa**。
- 使用/usr/lib64/sa/sa2命令，每天23:53，将统计信息存储到/var/log/sa/sar**。

```
[root@lab ~]# cat /etc/cron.d/sysstat 
# Run system activity accounting tool every 10 minutes
*/10 * * * * root /usr/lib64/sa/sa1 1 1
# 0 * * * * root /usr/lib64/sa/sa1 600 6 &
# Generate a daily summary of process accounting at 23:53
53 23 * * * root /usr/lib64/sa/sa2 -A
```

## 1.3 配置

```shell
[root@lab ~]# vi /etc/sysconfig/sysstat
# sysstat-10.1.5 configuration file.

# How long to keep log files (in days).
# If value is greater than 28, then log files are kept in
# multiple directories, one for each month.
HISTORY=28

# Compress (using gzip or bzip2) sa and sar files older than (in days):
COMPRESSAFTER=31

# Parameters for the system activity data collector (see sadc manual page)
# which are used for the generation of log files.
# *note
SADC_OPTIONS="-S DISK"

# *note : valid options
INT     ⇒  System Interrupts
DISK    ⇒  Block Devices
SNMP    ⇒  SNMP statistics
IPV6    ⇒  IPv6 statistics
POWER   ⇒  Power Management statistics
ALL     ⇒  All of the above
XDISK   ⇒  DISK + Partition statistics
XALL    ⇒  All of the above (ALL + XDISK)
```

## 1.4 使用

Option | Description
---|---
-u | CPU utilization statistics
-r | Memory utilization statistics
-b | I/O and transfer rate statistics
-B | Paging statistics
-d | Activity for each block device
-n [keyword] | Network statistics
-q | Queue length and load averages
-A | Display All

### 1.4.1 显示今天的统计报告

```shell
# CPU
[root@lab ~]# sar -u 
01:10:01 AM     CPU     %user     %nice   %system   %iowait    %steal     %idle
10:20:01 AM     all      0.90      0.00      0.23      1.19      0.03     97.65
...
...
01:30:01 PM     all      0.04      0.00      0.06      0.14      0.04     99.72
Average:        all      3.60      0.00      0.05      0.21      0.01     96.13

# memory
[root@lab ~]# sar -r 
01:10:01 AM kbmemfree kbmemused  %memused kbbuffers  kbcached  kbcommit   %commit
10:20:01 AM   3681144    241544      6.16     10744    138392     83984      1.04
...
...
01:40:01 PM   3663328    259360      6.61     14752    145988     87996      1.09
Average:      3666930    255758      6.52     13204    144710     88273      1.10

# I/O
[root@lab ~]# sar -b 
01:10:01 AM       tps      rtps      wtps   bread/s   bwrtn/s
10:20:01 AM     45.21      9.35     35.87    257.22    409.24
...
...
01:40:01 PM      0.52      0.00      0.52      0.00      5.43
Average:         3.11      0.49      2.62     14.55     29.32

# network
[root@lab ~]# sar -n DEV 
01:10:01 AM     IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s
10:20:01 AM        lo      1.04      1.04      0.09      0.09      0.00      0.00      0.00
10:20:01 AM      eth0      7.48      4.48     10.06      0.30      0.00      0.00      0.00
...
...
01:40:01 PM        lo      0.01      0.01      0.00      0.00      0.00      0.00      0.00
01:40:01 PM      eth0      0.05      0.04      0.00      0.00      0.00      0.00      0.00
Average:           lo      0.08      0.08      0.01      0.01      0.00      0.00      0.00
Average:         eth0      0.54      0.38      0.51      0.03      0.00      0.00      0.00

# load avarage
[root@lab ~]# sar -q 
01:10:01 AM   runq-sz  plist-sz   ldavg-1   ldavg-5  ldavg-15
10:20:01 AM         1        99      0.04      0.07      0.04
...
...
01:30:01 PM         1       104      0.00      0.00      0.00
01:40:01 PM         1       101      0.00      0.00      0.00
Average:            1       102      0.11      0.08      0.06
```

### 1.4.2 指定一个文件显示过去的统计信息

```shell
# display all statisc by a log file
[root@lab ~]# sar -A -f /var/log/sa/sa05 
01:10:01 AM     CPU      %usr     %nice      %sys   %iowait    %steal      %irq     %soft    %guest     %idle
10:20:01 AM     all      0.90      0.00      0.19      1.19      0.03      0.04      0.00      0.00     97.65
10:20:01 AM       0      0.96      0.00      0.19      1.27      0.03      0.04      0.00      0.00     97.51
...
...
01:50:01 PM       125         3         4         0         0         0
02:00:01 PM       125         3         4         0         0         0
Average:          122         3         4         0         0         0

# display the load average with specifying times by a log file
[root@lab ~]# sar -q -s 11:00:00 -e 12:00:00 -f /var/log/sa/sa05 
11:00:01 AM   runq-sz  plist-sz   ldavg-1   ldavg-5  ldavg-15
11:10:01 AM         1       103      0.01      0.03      0.14
11:20:01 AM         1       102      0.01      0.01      0.06
11:30:01 AM         1       102      0.00      0.00      0.01
11:40:01 AM         1       101      0.00      0.00      0.00
11:50:01 AM         1       101      0.01      0.01      0.00
Average:            1       102      0.01      0.01      0.04
```
### 1.4.3 显示当前统计信息

```shell
# display CPU utilization per a second 3 times
[root@lab ~]# sar -u 1 3 
01:51:34 AM     CPU     %user     %nice   %system   %iowait    %steal     %idle
01:51:35 AM     all      0.00      0.00      0.50      0.00      0.00     99.50
01:51:36 AM     all      0.00      0.00      0.00      0.00      0.50     99.50
01:51:37 AM     all      0.00      0.00      0.00      0.00      0.00    100.00
Average:        all      0.00      0.00      0.17      0.00      0.17     99.67

# display I/O and network per 2 seconds 5 times
[root@lab ~]# sar -b -n DEV 2 5 
01:54:11 AM       tps      rtps      wtps   bread/s   bwrtn/s
01:54:13 AM      0.00      0.00      0.00      0.00      0.00

01:54:11 AM     IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s
01:54:13 AM        lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00
01:54:13 AM      eth0      0.00      0.00      0.00      0.00      0.00      0.00      0.00
...
...
01:54:19 AM       tps      rtps      wtps   bread/s   bwrtn/s
01:54:21 AM      0.00      0.00      0.00      0.00      0.00

01:54:19 AM     IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s
01:54:21 AM        lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00
01:54:21 AM      eth0      0.00      0.00      0.00      0.00      0.00      0.00      0.00

Average:          tps      rtps      wtps   bread/s   bwrtn/s
Average:         0.70      0.00      0.70      0.00      4.80

Average:        IFACE   rxpck/s   txpck/s    rxkB/s    txkB/s   rxcmp/s   txcmp/s  rxmcst/s
Average:           lo      0.00      0.00      0.00      0.00      0.00      0.00      0.00
Average:         eth0      0.20      0.00      0.01      0.00      0.00      0.00      0.00
```

### 1.4.4 SysStat其他统计信息命令

```shell
# display I/O with Mega bytes per 2 seconds
[root@lab ~]# iostat -mx -d 2 
Device:         rrqm/s   wrqm/s     r/s     w/s    rMB/s    wMB/s avgrq-sz avgqu-sz   await  svctm  %util
vda               0.11     1.05    0.43    0.63     0.01     0.01    28.44     0.05   51.01   9.29   0.99
dm-0              0.00     0.00    0.48    1.65     0.01     0.01    14.05     0.20   95.25   4.64   0.99
dm-1              0.00     0.00    0.02    0.00     0.00     0.00     8.00     0.00    1.14   0.52   0.00
...
...

# display all CPU utilization per 2 seconds 3 times
[root@lab ~]# mpstat -P ALL 2 3 
02:28:57 PM  CPU    %usr   %nice    %sys %iowait    %irq   %soft  %steal  %guest   %idle
02:28:59 PM  all    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
02:28:59 PM    0    0.00    0.00    0.00    0.00    0.50    0.00    0.00    0.00   99.50
02:28:59 PM    1    0.00    0.00    0.00    0.00    0.00    0.00    0.00    0.00  100.00
...
...

# display memory utilization of a process per a second 3 times
[root@lab ~]# pidstat -r -p 1202 1 3 
02:34:07 PM       PID  minflt/s  majflt/s     VSZ    RSS   %MEM  Command
02:34:08 PM      1202      0.00      0.00  175360   2456   0.06  httpd
02:34:09 PM      1202      0.00      0.00  175360   2456   0.06  httpd
02:34:10 PM      1202      0.00      0.00  175360   2456   0.06  httpd
Average:         1202      0.00      0.00  175360   2456   0.06  httpd
...
...

# display CIFS statics with Mega bytes per a 1 second 3 times
[root@lab ~]# cifsiostat -m 1 3 
Filesystem:                    rB/s         wB/s    rops/s    wops/s         fo/s         fc/s         fd/s
\\10.0.0.100\tmp               0.00        20.93      0.00      0.00         0.00         0.00         0.00
...
...
```