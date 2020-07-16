# iperf3

iperf3 is a tool for performing network throughput measurements.

## TPC measurements 

```
[root@jerrydai ~]# iperf3 --format --interval 1 --bandwidth 10G --client 192.168.1.101
Connecting to host 192.168.1.101, port 5201
[  4] local 192.168.1.101 port 49490 connected to 192.168.1.101 port 5201
[ ID] Interval           Transfer     Bandwidth       Retr  Cwnd
[  4]   0.00-1.00   sec  1015 MBytes  8.51 Gbits/sec    0    639 KBytes
[  4]   1.00-2.00   sec  1.05 GBytes  8.98 Gbits/sec    0    639 KBytes
[  4]   2.00-3.00   sec  1.09 GBytes  9.38 Gbits/sec    0    639 KBytes
[  4]   3.00-4.00   sec  1.10 GBytes  9.42 Gbits/sec    0    639 KBytes
[  4]   4.00-5.00   sec  1.11 GBytes  9.55 Gbits/sec    0    639 KBytes
[  4]   5.00-6.00   sec  1.10 GBytes  9.47 Gbits/sec    0    639 KBytes
[  4]   6.00-7.00   sec  1.10 GBytes  9.49 Gbits/sec    0    639 KBytes
[  4]   7.00-8.00   sec  1.10 GBytes  9.43 Gbits/sec    0    639 KBytes
[  4]   8.00-9.00   sec  1.08 GBytes  9.30 Gbits/sec    0    639 KBytes
[  4]   9.00-10.00  sec  1.11 GBytes  9.56 Gbits/sec    0    639 KBytes
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bandwidth       Retr
[  4]   0.00-10.00  sec  10.8 GBytes  9.31 Gbits/sec    0             sender
[  4]   0.00-10.00  sec  10.8 GBytes  9.31 Gbits/sec                  receiver

iperf Done.
```

## UDP measurements
```
[root@jerrydai ~]# iperf3 --format --interval 1 --bandwidth 10G --udp --client 192.168.1.101
Connecting to host 192.168.1.101, port 5201
[  4] local 192.168.1.101 port 62289 connected to 192.168.1.101 port 5201
[ ID] Interval           Transfer     Bandwidth       Total Datagrams
[  4]   0.00-1.00   sec   812 MBytes  6.81 Gbits/sec  103984
[  4]   1.00-2.00   sec   780 MBytes  6.54 Gbits/sec  99793
[  4]   2.00-3.00   sec   802 MBytes  6.73 Gbits/sec  102635
[  4]   3.00-4.00   sec   796 MBytes  6.68 Gbits/sec  101913
[  4]   4.00-5.00   sec   839 MBytes  7.04 Gbits/sec  107390
[  4]   5.00-6.00   sec   820 MBytes  6.87 Gbits/sec  105008
[  4]   6.00-7.00   sec   792 MBytes  6.65 Gbits/sec  101369
[  4]   7.00-8.00   sec   782 MBytes  6.56 Gbits/sec  100038
[  4]   8.00-9.00   sec   784 MBytes  6.58 Gbits/sec  100416
[  4]   9.00-10.00  sec   810 MBytes  6.79 Gbits/sec  103651
- - - - - - - - - - - - - - - - - - - - - - - - -
[ ID] Interval           Transfer     Bandwidth       Jitter    Lost/Total Datagrams
[  4]   0.00-10.00  sec  7.83 GBytes  6.73 Gbits/sec  0.001 ms  160569/1026197 (16%)
[  4] Sent 1026197 datagrams

iperf Done.
```

## MTR —— a network diagnostic tool

```
[root@jerrydai ~]# mtr --report -c 10000 --interval 0.001 --no-dns 192.168.1.101
Start: Fri Jul 10 10:44:38 2020
HOST: jerrydai                    Loss%   Snt   Last   Avg  Best  Wrst StDev
  1.|-- 192.168.1.101              0.0% 10000    0.0   0.0   0.0   0.1   0.0
```