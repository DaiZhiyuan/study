# UnixBench

**UnixBench** is the original BYTE UNIX benchmark suite, updated and revised by many people over the years.

The purpose of UnixBench is to provide a basic indicator of the performance of a Unix-like system; hence, multiple
tests are used to test various aspects of the system's performance. These test results are then compared to the
scores from a baseline system to produce an index value, which is generally easier to handle than the raw scores.
The entire set of index values is then combined to make an overall index for the system.

Some very simple graphics tests are included to measure the 2D and 3D graphics performance of the system.

Multi-CPU systems are handled. If your system has multiple CPUs, the default behaviour is to run the selected tests
twice -- once with one copy of each test program running at a time, and once with N copies, where N is the number of
CPUs. This is designed to allow you to assess:

* the performance of your system when running a single task
* the performance of your system when running multiple tasks
* the gain from your system's implementation of parallel processing

Do be aware that this is a system benchmark, not a CPU, RAM or disk benchmark. The results will depend not only on
your hardware, but on your operating system, libraries, and even compiler.

## Run Benchmark

```
   #    #  #    #  #  #    #          #####   ######  #    #   ####   #    #
   #    #  ##   #  #   #  #           #    #  #       ##   #  #    #  #    #
   #    #  # #  #  #    ##            #####   #####   # #  #  #       ######
   #    #  #  # #  #    ##            #    #  #       #  # #  #       #    #
   #    #  #   ##  #   #  #           #    #  #       #   ##  #    #  #    #
    ####   #    #  #  #    #          #####   ######  #    #   ####   #    #

   Version 5.1.3                      Based on the Byte Magazine Unix Benchmark

   Multi-CPU version                  Version 5 revisions by Ian Smith,
                                      Sunnyvale, CA, USA
   January 13, 2011                   johantheghost at yahoo period com

------------------------------------------------------------------------------
   Use directories for:
      * File I/O tests (named fs***) = /opt/byte-unixbench-master/UnixBench/tmp
      * Results                      = /opt/byte-unixbench-master/UnixBench/results
------------------------------------------------------------------------------

Benchmark Run: Thu Jul 16 2020 12:29:17 - 12:38:18
64 CPUs in system; running 1 parallel copy of tests

Dhrystone 2 using register variables       23896440.7 lps   (10.0 s, 2 samples)
Double-Precision Whetstone                     2943.6 MWIPS (9.7 s, 2 samples)
Execl Throughput                               3296.2 lps   (29.1 s, 1 samples)
File Copy 1024 bufsize 2000 maxblocks        473505.0 KBps  (30.0 s, 1 samples)
File Copy 256 bufsize 500 maxblocks          158601.0 KBps  (30.0 s, 1 samples)
File Copy 4096 bufsize 8000 maxblocks       1256658.0 KBps  (30.0 s, 1 samples)
Pipe Throughput                              932898.0 lps   (10.0 s, 2 samples)
Pipe-based Context Switching                 122353.7 lps   (10.0 s, 2 samples)
Process Creation                               5453.5 lps   (30.0 s, 1 samples)
Shell Scripts (1 concurrent)                   4372.6 lpm   (60.0 s, 1 samples)
Shell Scripts (8 concurrent)                   2201.4 lpm   (60.0 s, 1 samples)
System Call Overhead                         946836.7 lps   (10.0 s, 2 samples)

System Benchmarks Index Values               BASELINE       RESULT    INDEX
Dhrystone 2 using register variables         116700.0   23896440.7   2047.7
Double-Precision Whetstone                       55.0       2943.6    535.2
Execl Throughput                                 43.0       3296.2    766.6
File Copy 1024 bufsize 2000 maxblocks          3960.0     473505.0   1195.7
File Copy 256 bufsize 500 maxblocks            1655.0     158601.0    958.3
File Copy 4096 bufsize 8000 maxblocks          5800.0    1256658.0   2166.7
Pipe Throughput                               12440.0     932898.0    749.9
Pipe-based Context Switching                   4000.0     122353.7    305.9
Process Creation                                126.0       5453.5    432.8
Shell Scripts (1 concurrent)                     42.4       4372.6   1031.3
Shell Scripts (8 concurrent)                      6.0       2201.4   3669.1
System Call Overhead                          15000.0     946836.7    631.2
                                                                   ========
System Benchmarks Index Score                                         943.0

------------------------------------------------------------------------
Benchmark Run: Thu Jul 16 2020 12:38:18 - 12:47:24
64 CPUs in system; running 64 parallel copies of tests

Dhrystone 2 using register variables     1480780966.3 lps   (10.0 s, 2 samples)
Double-Precision Whetstone                   186924.3 MWIPS (9.9 s, 2 samples)
Execl Throughput                              38006.2 lps   (29.5 s, 1 samples)
File Copy 1024 bufsize 2000 maxblocks        474703.0 KBps  (30.0 s, 1 samples)
File Copy 256 bufsize 500 maxblocks          132980.0 KBps  (30.0 s, 1 samples)
File Copy 4096 bufsize 8000 maxblocks       1621722.0 KBps  (30.0 s, 1 samples)
Pipe Throughput                            58740530.6 lps   (10.0 s, 2 samples)
Pipe-based Context Switching                6209573.1 lps   (10.0 s, 2 samples)
Process Creation                              35923.5 lps   (30.0 s, 1 samples)
Shell Scripts (1 concurrent)                  72972.4 lpm   (60.0 s, 1 samples)
Shell Scripts (8 concurrent)                   9004.9 lpm   (60.1 s, 1 samples)
System Call Overhead                        2610300.3 lps   (10.0 s, 2 samples)

System Benchmarks Index Values               BASELINE       RESULT    INDEX
Dhrystone 2 using register variables         116700.0 1480780966.3 126887.8
Double-Precision Whetstone                       55.0     186924.3  33986.2
Execl Throughput                                 43.0      38006.2   8838.6
File Copy 1024 bufsize 2000 maxblocks          3960.0     474703.0   1198.7
File Copy 256 bufsize 500 maxblocks            1655.0     132980.0    803.5
File Copy 4096 bufsize 8000 maxblocks          5800.0    1621722.0   2796.1
Pipe Throughput                               12440.0   58740530.6  47219.1
Pipe-based Context Switching                   4000.0    6209573.1  15523.9
Process Creation                                126.0      35923.5   2851.1
Shell Scripts (1 concurrent)                     42.4      72972.4  17210.5
Shell Scripts (8 concurrent)                      6.0       9004.9  15008.2
System Call Overhead                          15000.0    2610300.3   1740.2
                                                                   ========
System Benchmarks Index Score                                        8229.1
------------------------------------------------------------------------------
```