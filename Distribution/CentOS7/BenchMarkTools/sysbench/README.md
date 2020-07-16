# sysbench

SysBench is a modular, cross-platform and multi-threaded benchmark tool for evaluating OS parameters that are important for a system running a database under intensive load.

1. General options
```
General options:
  --num-threads=N                    number of threads to use [1]
  --max-requests=N                   limit for total number of requests [10000]
  --max-time=N                       limit for total execution time in seconds [0]
  --forced-shutdown=STRING           amount of time to wait after --max-time before forcing shutdown [off]
  --thread-stack-size=SIZE           size of stack per thread [32K]
  --init-rng=[on|off]                initialize random number generator [off]
  --seed-rng=N                       seed for random number generator, ignored when 0 [0]
  --tx-rate=N                        target transaction rate (tps) [0]
  --tx-jitter=N                      target transaction variation, in microseconds [0]
  --intermediate-result-timer=N      intermediate result timer, 0 is off [0]
  --test=STRING                      test to run
  --debug=[on|off]                   print more debugging info [off]
  --validate=[on|off]                perform validation checks where possible [off]
  --help=[on|off]                    print help and exit
  --version=[on|off]                 print version and exit
```

## fileio - File I/O test

1. help
```
[root@jerrydai ~]# sysbench --test=fileio  help
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

fileio options:
  --file-num=N                  number of files to create [128]
  --file-block-size=N           block size to use in all IO operations [16384]
  --file-total-size=SIZE        total size of files to create [2G]
  --file-test-mode=STRING       test mode {seqwr, seqrewr, seqrd, rndrd, rndwr, rndrw}
  --file-io-mode=STRING         file operations mode {sync,async,fastmmap,slowmmap} [sync]
  --file-async-backlog=N        number of asynchronous operatons to queue per thread [128]
  --file-extra-flags=STRING     additional flags to use on opening files {sync,dsync,direct} []
  --file-fsync-freq=N           do fsync() after this number of requests (0 - don't use fsync()) [100]
  --file-fsync-all=[on|off]     do fsync() after each write operation [off]
  --file-fsync-end=[on|off]     do fsync() at the end of test [on]
  --file-fsync-mode=STRING      which method to use for synchronization {fsync, fdatasync} [fsync]
  --file-merged-requests=N      merge at most this number of IO requests if possible (0 - don't merge) [0]
  --file-rw-ratio=N             reads/writes ratio for combined test [1.5]
```
2. run

```
[root@jerrydai nvme]# sysbench --test=fileio --file-block-size=32768 --file-num=512 --file-io-mode=async --file-test-mode=rndwr --file-fsync-freq=100 --file-rw-ratio=2.8 run
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

Running the test with following options:
Number of threads: 1
Random number generator seed is 0 and will be ignored


Extra file open flags: 0
512 files, 4Mb each
2Gb total file size
Block size 32Kb
Number of random requests for random IO: 10000
Read/Write ratio for combined random IO test: 2.80
Periodic FSYNC enabled, calling fsync() each 100 requests.
Calling fsync() at the end of test, Enabled.
Using asynchronous I/O mode
Doing random write test
Threads started!
Done.

Operations performed:  0 Read, 10000 Write, 51200 Other = 61200 Total
Read 0b  Written 312.5Mb  Total transferred 312.5Mb  (94.865Mb/sec)
 3035.67 Requests/sec executed

Test execution summary:
    total time:                          3.2942s
    total number of events:              10000
    total time taken by event execution: 0.1129
    per-request statistics:
         min:                                  0.01ms
         avg:                                  0.01ms
         max:                                  0.09ms
         approx.  95 percentile:               0.01ms

Threads fairness:
    events (avg/stddev):           10000.0000/0.00
    execution time (avg/stddev):   0.1129/0.00
```

## cpu - CPU performance test

1. help
```
[root@jerrydai ~]# sysbench --test=cpu help
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

cpu options:
  --cpu-max-prime=N      upper limit for primes generator [10000]
```

2. run
```
[root@jerrydai ~]# sysbench --test=cpu --cpu-max-prime=20000 run
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

Running the test with following options:
Number of threads: 1
Random number generator seed is 0 and will be ignored


Doing CPU performance benchmark

Threads started!
Done.

Maximum prime number checked in CPU test: 20000


Test execution summary:
    total time:                          11.3142s
    total number of events:              10000
    total time taken by event execution: 11.3130
    per-request statistics:
         min:                                  1.13ms
         avg:                                  1.13ms
         max:                                  1.25ms
         approx.  95 percentile:               1.13ms

Threads fairness:
    events (avg/stddev):           10000.0000/0.00
    execution time (avg/stddev):   11.3130/0.00
```

## memory - Memory functions speed test

1. help
```
[root@jerrydai ~]# sysbench --test=memory help
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

memory options:
  --memory-block-size=SIZE    size of memory block for test [1K]
  --memory-total-size=SIZE    total size of data to transfer [100G]
  --memory-scope=STRING       memory access scope {global,local} [global]
  --memory-hugetlb=[on|off]   allocate memory from HugeTLB pool [off]
  --memory-oper=STRING        type of memory operations {read, write, none} [write]
  --memory-access-mode=STRING memory access mode {seq,rnd} [seq]
```

2. run
```
[root@jerrydai ~]# sysbench --test=memory --memory-block-size=32768 --memory-total-size=10G --memory-scope=global --memory-hugetlb=off --memory-oper=write --memory-access-mode=rnd  run
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

Running the test with following options:
Number of threads: 1
Random number generator seed is 0 and will be ignored


Doing memory operations speed test
Memory block size: 32K

Memory transfer size: 10240M

Memory operations type: write
Memory scope type: global
Threads started!
Done.

Operations performed: 327680 (2389822.56 ops/sec)

10240.00 MB transferred (74681.96 MB/sec)


Test execution summary:
    total time:                          0.1371s
    total number of events:              327680
    total time taken by event execution: 0.1008
    per-request statistics:
         min:                                  0.00ms
         avg:                                  0.00ms
         max:                                  0.02ms
         approx.  95 percentile:               0.00ms

Threads fairness:
    events (avg/stddev):           327680.0000/0.00
    execution time (avg/stddev):   0.1008/0.00
```

## threads - Threads subsystem performance test

1. help
```
[root@jerrydai ~]# sysbench --test=threads help
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

threads options:
  --thread-yields=N      number of yields to do per request [1000]
  --thread-locks=N       number of locks per thread [8]
```

2. run
```
[root@jerrydai ~]# sysbench --test=threads --thread-yields=3000 --thread-locks=64 run
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

Running the test with following options:
Number of threads: 1
Random number generator seed is 0 and will be ignored


Doing thread subsystem performance test
Thread yields per test: 3000 Locks used: 64
Threads started!
Done.


Test execution summary:
    total time:                          16.6641s
    total number of events:              10000
    total time taken by event execution: 16.6630
    per-request statistics:
         min:                                  1.66ms
         avg:                                  1.67ms
         max:                                  1.79ms
         approx.  95 percentile:               1.67ms

Threads fairness:
    events (avg/stddev):           10000.0000/0.00
    execution time (avg/stddev):   16.6630/0.00
```

## mutex - Mutex performance test

## oltp - OLTP test

Benchmark MySQL server Performance with Sysbench.

1. prepare
```
[root@jerrydai ~]# sysbench --test=oltp --oltp-table-size=1000000 --db-driver=mysql --mysql-host=127.0.0.1 --mysql-port=3306 --mysql-db=sysbench_test_database --mysql-user=root --mysql-password=abc123 --mysql-ssl=off prepare
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark
```

2. OLTP test
```
[root@jerrydai ~]# sysbench --test=oltp --oltp-table-size=1000000 --oltp-test-mode=complex --oltp-read-only=off --num-threads=64 --max-time=60 --max-requests=0 --db-driver=mysql --mysql-host=127.0.0.1 --mysql-port=3306 --mysql-db=sysbench_test_database --mysql-user=root --mysql-password=abc123 --mysql-ssl=off run
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

Running the test with following options:
Number of threads: 64
Random number generator seed is 0 and will be ignored


Doing OLTP test.
Running mixed OLTP test
Using Special distribution (12 iterations,  1 pct of values are returned in 75 pct cases)
Using "BEGIN" for starting transactions
Using auto_inc on the id column
Using 1 test tables
Threads started!
Time limit exceeded, exiting...
(last message repeated 63 times)
Done.

OLTP test statistics:
    queries performed:
        read:                            6214614
        write:                           2219489
        other:                           887796
        total:                           9321899
    transactions:                        443895 (7397.52 per sec.)
    deadlocks:                           6      (0.10 per sec.)
    read/write requests:                 8434103 (140554.55 per sec.)
    other operations:                    887796 (14795.14 per sec.)

Test execution summary:
    total time:                          60.0059s
    total number of events:              443895
    total time taken by event execution: 3835.5670
    per-request statistics:
         min:                                  3.14ms
         avg:                                  8.64ms
         max:                                175.71ms
         approx.  95 percentile:              18.90ms

Threads fairness:
    events (avg/stddev):           6935.8594/490.64
    execution time (avg/stddev):   59.9307/0.01
```

3. cleanup
```
[root@jerrydai ~]# sysbench --test=oltp --db-driver=mysql --mysql-host=127.0.0.1 --mysql-port=3306 --mysql-db=sysbench_test_database --mysql-user=root --mysql-password=abc123 --mysql-ssl=off cleanup
sysbench 0.4.12.5:  multi-threaded system evaluation benchmark

Dropping table 'sbtest'...
Done.
```