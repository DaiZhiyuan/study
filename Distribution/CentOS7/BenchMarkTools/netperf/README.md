# netperf —— a network performance benchmark

Netperf is a benchmark that can be used to measure various aspects of networking performance.  
Currently, its focus is on bulk data transfer and request/response performance using either TCP or UDP, 
and the Berkeley Sockets interface. 

In addition, tests for DLPI, and Unix Domain Sockets, tests for IPv6 may be conditionally compiled-in.

## build&install
```
#> wget https://github.com/HewlettPackard/netperf/archive/netperf-2.7.0.tar.gz
#> tar xvf netperf-2.7.0.tar.gz
#> ./confiugre && make && make install
```

## netserver
```
#> netserver -4
Starting netserver with host 'IN(6)ADDR_ANY' port '12865' and family AF_INET
```

## run benchmark
```
#> netperf -H 127.0.0.1 -t TCP_STREAM -l 30 -f G
MIGRATED TCP STREAM TEST from 0.0.0.0 (0.0.0.0) port 0 AF_INET to 127.0.0.1 () port 0 AF_INET
Recv   Send    Send
Socket Socket  Message  Elapsed
Size   Size    Size     Time     Throughput
bytes  bytes   bytes    secs.    GBytes/sec

131072  16384  16384    30.00       1.84

#> netperf -H 127.0.0.1 -t UDP_STREAM -l 30 -f G
MIGRATED UDP STREAM TEST from 0.0.0.0 (0.0.0.0) port 0 AF_INET to 127.0.0.1 () port 0 AF_INET

Socket  Message  Elapsed      Messages
Size    Size     Time         Okay Errors   Throughput
bytes   bytes    secs            #      #   GBytes/sec

212992   65507   30.00     1753804      0       3.57
212992           30.00     1273777              2.59
```