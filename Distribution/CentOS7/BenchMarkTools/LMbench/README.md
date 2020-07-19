# LMbench —— Tools for performance analysis

- Suite of simple, portable benchmarks
- Compares different  systems performance
- Free software, covered by the GNU General Public License.


## Feature

- Bandwidth benchmarks
    - Cached file read
    - Memory copy (bcopy)
    - Memory read
    - Memory write
    - Pipe
    - TCP
- Latency benchmarks
    - Context switching.
    - Networking: connection establishment, pipe, TCP, UDP, and RPC hot potato
    - File system creates and deletes.
    - Process creation.
    - Signal handling
    - System call overhead
    - Memory read latency
- Miscellanious
    - Processor clock rate calculation

## Build & Run

```
#> tar xzvf lmbench3.tar.gz
#> cd  lmbench3
#> mkdir ./SCCS && touch ./SCCS/s.ChangeSet
#> make results
#> make see | tee
```

## configureation

```
=====================================================================

                L M B E N C H   C ON F I G U R A T I O N
                ----------------------------------------

You need to configure some parameters to lmbench.  Once you have configured
these parameters, you may do multiple runs by saying

        "make rerun"

in the src subdirectory.

NOTICE: please do not have any other activity on the system if you can
help it.  Things like the second hand on your xclock or X perfmeters
are not so good when benchmarking.  In fact, X is not so good when
benchmarking.
=====================================================================

Hang on, we are calculating your timing granularity.
OK, it looks like you can time stuff down to 5000 usec resolution.

Hang on, we are calculating your timing overhead.
OK, it looks like your gettimeofday() costs 0 usecs.

Hang on, we are calculating your loop overhead.
OK, it looks like your benchmark loop costs 0.00000066 usecs.

=====================================================================

If you are running on an MP machine and you want to try running
multiple copies of lmbench in parallel, you can specify how many here.

Using this option will make the benchmark run 100x slower (sorry).

NOTE:  WARNING! This feature is experimental and many results are
        known to be incorrect or random!

MULTIPLE COPIES [default 1] 64
Options to control job placement
1) Allow scheduler to place jobs
2) Assign each benchmark process with any attendent child processes
   to its own processor
3) Assign each benchmark process with any attendent child processes
   to its own processor, except that it will be as far as possible
   from other processes
4) Assign each benchmark and attendent processes to their own
   processors
5) Assign each benchmark and attendent processes to their own
   processors, except that they will be as far as possible from
   each other and other processes
6) Custom placement: you assign each benchmark process with attendent
   child processes to processors
7) Custom placement: you assign each benchmark and attendent
   processes to processors

Note: some benchmarks, such as bw_pipe, create attendent child
processes for each benchmark process.  For example, bw_pipe
needs a second process to send data down the pipe to be read
by the benchmark process.  If you have three copies of the
benchmark process running, then you actually have six processes;
three attendent child processes sending data down the pipes and
three benchmark processes reading data and doing the measurements.

Job placement selection: 1
=====================================================================

Several benchmarks operate on a range of memory.  This memory should be
sized such that it is at least 4 times as big as the external cache[s]
on your system.   It should be no more than 80% of your physical memory.

The bigger the range, the more accurate the results, but larger sizes
take somewhat longer to run the benchmark.

MB [default 182123]
Checking to see if you have 182123 MB; please wait for a moment...
2048MB OK
6144MB OK
182123MB OK
182123MB OK
182123MB OK
Hang on, we are calculating your cache line size.
OK, it looks like your cache line is 64 bytes.

=====================================================================

lmbench measures a wide variety of system performance, and the full suite
of benchmarks can take a long time on some platforms.  Consequently, we
offer the capability to run only predefined subsets of benchmarks, one
for operating system specific benchmarks and one for hardware specific
benchmarks.  We also offer the option of running only selected benchmarks
which is useful during operating system development.

Please remember that if you intend to publish the results you either need
to do a full run or one of the predefined OS or hardware subsets.

SUBSET (ALL|HARWARE|OS|DEVELOPMENT) [default all] =====================================================================

This benchmark measures, by default, memory latency for a number of
different strides.  That can take a long time and is most useful if you
are trying to figure out your cache line size or if your cache line size
is greater than 128 bytes.

If you are planning on sending in these results, please don't do a fast
run.

Answering yes means that we measure memory latency with a 128 byte stride.

FASTMEM [default no] =================================================

This benchmark measures, by default, file system latency.  That can
take a long time on systems with old style file systems (i.e., UFS,
FFS, etc.).  Linux' ext2fs and Sun's tmpfs are fast enough that this
test is not painful.

If you are planning on sending in these results, please don't do a fast
run.

If you want to skip the file system latency tests, answer "yes" below.

SLOWFS [default no] yes
=====================================================================

Calculating mhz, please wait for a moment...
I think your CPU mhz is

        2200 MHz, 0.4545 nanosec clock

but I am frequently wrong.  If that is the wrong Mhz, type in your
best guess as to your processor speed.  It doesn't have to be exact,
but if you know it is around 800, say 800.

Please note that some processors, such as the P4, have a core which
is double-clocked, so on those processors the reported clock speed
will be roughly double the advertised clock rate.  For example, a
1.8GHz P4 may be reported as a 3592MHz processor.

Processor mhz [default 2200 MHz, 0.4545 nanosec clock]
=====================================================================

We need a place to store a 2032 Mbyte file as well as create and delete a
large number of small files.  We default to /usr/tmp.  If /usr/tmp is a
memory resident file system (i.e., tmpfs), pick a different place.
Please specify a directory that has enough space and is a local file
system.

FSDIR [default /usr/tmp]
=====================================================================

lmbench outputs status information as it runs various benchmarks.
By default this output is sent to /dev/tty, but you may redirect
it to any file you wish (such as /dev/null...).

Status output file [default /dev/tty]
=====================================================================

There is a database of benchmark results that is shipped with new
releases of lmbench.  Your results can be included in the database
if you wish.  The more results the better, especially if they include
remote networking.  If your results are interesting, i.e., for a new
fast box, they may be made available on the lmbench web page, which is

        http://www.bitmover.com/lmbench

Mail results [default yes] no
OK, no results mailed.
=====================================================================

Confguration done, thanks.
```

```
[root@jerrydai lmbench3]# cat ./bin/aarch64-linux-gnu/CONFIG.jerrydai
DISKS=""
DISK_DESC=""
OUTPUT=/dev/tty
ENOUGH=5000
FASTMEM="NO"
FILE=/usr/tmp/XXX
FSDIR=/usr/tmp
INFO=INFO.jerrydai
LINE_SIZE=64
LOOP_O=0.00000066
MAIL=no
TOTAL_MEM=260176
MB=2032
MHZ="2200 MHz, 0.4545 nanosec clock"
MOTHERBOARD=""
NETWORKS=""
OS="aarch64-linux-gnu"
PROCESSORS="64"
REMOTE=""
SLOWFS="YES"
SYNC_MAX="64"
LMBENCH_SCHED="DEFAULT"
TIMING_O=0
RSH=
RCP=
VERSION=lmbench-3alpha4
BENCHMARK_HARDWARE=YES
BENCHMARK_OS=YES
BENCHMARK_SYSCALL=
BENCHMARK_SELECT=
BENCHMARK_PROC=
BENCHMARK_CTX=
BENCHMARK_PAGEFAULT=
BENCHMARK_FILE=
BENCHMARK_MMAP=
BENCHMARK_PIPE=
BENCHMARK_UNIX=
BENCHMARK_UDP=
BENCHMARK_TCP=
BENCHMARK_CONNECT=
BENCHMARK_RPC=
BENCHMARK_HTTP=
BENCHMARK_BCOPY=
BENCHMARK_MEM=
BENCHMARK_OPS=
```

## runBenchmark

```
#> make rerun

Using config in CONFIG.jerrydai
Thu Jul 16 13:16:55 CST 2020
Latency measurements

Thu Jul 16 17:42:52 CST 2020
Local networking

Thu Jul 16 18:41:09 CST 2020
Bandwidth measurements

Fri Jul 17 02:23:50 CST 2020
Calculating context switch overhead

Fri Jul 17 03:28:55 CST 2020
McCalpin's STREAM benchmark

Fri Jul 17 03:39:02 CST 2020
Calculating memory load latency
```

## result

```
#> make see

                 L M B E N C H  3 . 0   S U M M A R Y
                 ------------------------------------
                 (Alpha software, do not distribute)

Basic system parameters
------------------------------------------------------------------------------
Host                 OS Description              Mhz  tlb  cache  mem   scal
                                                     pages line   par   load
                                                           bytes
--------- ------------- ----------------------- ---- ----- ----- ------ ----
jerrydai  Linux 4.19.10       aarch64-linux-gnu 2200          64          64

Processor, Processes - times in microseconds - smaller is better
------------------------------------------------------------------------------
Host                 OS  Mhz null null      open slct sig  sig  fork exec sh
                             call  I/O stat clos TCP  inst hndl proc proc proc
--------- ------------- ---- ---- ---- ---- ---- ---- ---- ---- ---- ---- ----
jerrydai  Linux 4.19.10 2200 0.23 5.93 91.6 199. 4.64 0.36 36.6 1889 3397 5664

Context switching - times in microseconds - smaller is better
-------------------------------------------------------------------------
Host                 OS  2p/0K 2p/16K 2p/64K 8p/16K 8p/64K 16p/16K 16p/64K
                         ctxsw  ctxsw  ctxsw ctxsw  ctxsw   ctxsw   ctxsw
--------- ------------- ------ ------ ------ ------ ------ ------- -------
jerrydai  Linux 4.19.10   24.7   16.0   22.7   19.5   33.7    21.2    51.5

*Local* Communication latencies in microseconds - smaller is better
---------------------------------------------------------------------
Host                 OS 2p/0K  Pipe AF     UDP  RPC/   TCP  RPC/ TCP
                        ctxsw       UNIX         UDP         TCP conn
--------- ------------- ----- ----- ---- ----- ----- ----- ----- ----
jerrydai  Linux 4.19.10  24.7  27.2 42.1       824.5  88.6 1078.

File & VM system latencies in microseconds - smaller is better
-------------------------------------------------------------------------------
Host                 OS   0K File      10K File     Mmap    Prot   Page   100fd
                        Create Delete Create Delete Latency Fault  Fault  selct
--------- ------------- ------ ------ ------ ------ ------- ----- ------- -----
jerrydai  Linux 4.19.10                              252.3K  24.0    12.7 2.611

*Local* Communication bandwidths in MB/s - bigger is better
-----------------------------------------------------------------------------
Host                OS  Pipe AF    TCP  File   Mmap  Bcopy  Bcopy  Mem   Mem
                             UNIX      reread reread (libc) (hand) read write
--------- ------------- ---- ---- ---- ------ ------ ------ ------ ---- -----
jerrydai  Linux 4.19.10 45.K 35.K 18.K  20.2K  22.0K  34.8K  47.0K 114K 126.K

Memory latencies in nanoseconds - smaller is better
    (WARNING - may not be correct, check graphs)
------------------------------------------------------------------------------
Host                 OS   Mhz   L1 $   L2 $    Main mem    Rand mem    Guesses
--------- -------------   ---   ----   ----    --------    --------    -------
jerrydai  Linux 4.19.10  2200 1.8190   10.9   38.0       205.7
```