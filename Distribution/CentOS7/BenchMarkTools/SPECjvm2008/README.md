# SPECjvm2008

1. prepare
```
#> yum install java-1.8.0-openjdk-devel
#> export JAVA_HOME="/usr/lib/jvm/java-1.8.0-openjdk-1.8.0.181-aarch64"
```

> adoptopenjdk-8-hotspot

2. run
```
[root@jerrydai SPECjvm2008]# ./run-specjvm.sh

SPECjvm2008 Base
  Properties file:   none
  Benchmarks:        startup.helloworld startup.compiler.compiler startup.compiler.sunflow startup.compress startup.crypto.aes startup.crypto.rsa startup.crypto.signverify startup.mpegaudio startup.scimark.fft startup.scimark.lu startup.scimark.monte_carlo startup.scimark.sor startup.scimark.sparse startup.serial startup.sunflow startup.xml.transform startup.xml.validation compiler.compiler compiler.sunflow compress crypto.aes crypto.rsa crypto.signverify derby mpegaudio scimark.fft.large scimark.lu.large scimark.sor.large scimark.sparse.large scimark.fft.small scimark.lu.small scimark.sor.small scimark.sparse.small scimark.monte_carlo serial sunflow xml.transform xml.validation

  Kit signature and checksum is validated.
  This can take several minutes.
  Use argument '-ikv' to skip this.
  ....................................passed.

--- --- --- --- --- --- --- --- ---

  Benchmark:   check
  Run mode:    static run
  Test type:   functional
  Threads:     1
  Iterations:  1
  Run length:  1 operation

Iteration 1 (1 operation) begins: Fri Jul 17 12:49:00 CST 2020
Iteration 1 (1 operation) ends:   Fri Jul 17 12:49:00 CST 2020
Iteration 1 (1 operation) result: PASSED

Valid run!


--- --- --- --- --- --- --- --- ---

  Benchmark:   startup.helloworld
  Run mode:    static run
  Test type:   single
  Threads:     1
  Iterations:  1
  Run length:  1 operation

Iteration 1 (1 operation) begins: Fri Jul 17 12:49:00 CST 2020
Iteration 1 (1 operation) ends:   Fri Jul 17 12:49:01 CST 2020
Iteration 1 (1 operation) result: 141.84 ops/m

Valid run!
Score on startup.helloworld: 141.84 ops/m


--- --- --- --- --- --- --- --- ---

  Benchmark:   startup.compiler.compiler
  Run mode:    static run
  Test type:   single
  Threads:     1
  Iterations:  1
  Run length:  1 operation

Iteration 1 (1 operation) begins: Fri Jul 17 12:49:01 CST 2020
Iteration 1 (1 operation) ends:   Fri Jul 17 12:49:06 CST 2020
Iteration 1 (1 operation) result: 10.05 ops/m

Valid run!
Score on startup.compiler.compiler: 10.05 ops/m


--- --- --- --- --- --- --- --- ---

  Benchmark:   startup.compiler.sunflow
  Run mode:    static run
  Test type:   single
  Threads:     1
  Iterations:  1
  Run length:  1 operation

Iteration 1 (1 operation) begins: Fri Jul 17 12:49:06 CST 2020
Iteration 1 (1 operation) ends:   Fri Jul 17 12:49:12 CST 2020
Iteration 1 (1 operation) result: 10.18 ops/m

Valid run!
Score on startup.compiler.sunflow: 10.18 ops/m


--- --- --- --- --- --- --- --- ---

  Benchmark:   startup.compress
  Run mode:    static run
  Test type:   single
  Threads:     1
  Iterations:  1
  Run length:  1 operation

Iteration 1 (1 operation) begins: Fri Jul 17 12:49:12 CST 2020
Iteration 1 (1 operation) ends:   Fri Jul 17 12:49:15 CST 2020
Iteration 1 (1 operation) result: 25.50 ops/m

Valid run!
Score on startup.compress: 25.50 ops/m
--- --- --- --- --- --- --- --- ---

    ......

--- --- --- --- --- --- --- --- ---

  Benchmark:   serial
  Run mode:    timed run
  Test type:   multi
  Threads:     64
  Warmup:      120s
  Iterations:  1
  Run length:  240s

Warmup (120s) begins: Fri Jul 17 11:47:18 CST 2020
Warmup (120s) ends:   Fri Jul 17 11:49:31 CST 2020
Warmup (120s) result: 464.91 ops/m

Iteration 1 (240s) begins: Fri Jul 17 11:49:31 CST 2020
Iteration 1 (240s) ends:   Fri Jul 17 11:53:44 CST 2020
Iteration 1 (240s) result: 482.60 ops/m

Valid run!
Score on serial: 482.60 ops/m


--- --- --- --- --- --- --- --- ---

  Benchmark:   sunflow
  Run mode:    timed run
  Test type:   multi
  Threads:     32
  Warmup:      120s
  Iterations:  1
  Run length:  240s

Warmup (120s) begins: Fri Jul 17 11:53:45 CST 2020
Warmup (120s) ends:   Fri Jul 17 11:55:54 CST 2020
Warmup (120s) result: 390.45 ops/m

Iteration 1 (240s) begins: Fri Jul 17 11:55:54 CST 2020
Iteration 1 (240s) ends:   Fri Jul 17 12:00:05 CST 2020
Iteration 1 (240s) result: 391.43 ops/m

Valid run!
Score on sunflow: 391.43 ops/m


--- --- --- --- --- --- --- --- ---

  Benchmark:   xml.transform
  Run mode:    timed run
  Test type:   multi
  Threads:     64
  Warmup:      120s
  Iterations:  1
  Run length:  240s

Warmup (120s) begins: Fri Jul 17 12:00:33 CST 2020
Warmup (120s) ends:   Fri Jul 17 12:02:40 CST 2020
Warmup (120s) result: 955.91 ops/m

Iteration 1 (240s) begins: Fri Jul 17 12:02:40 CST 2020
Iteration 1 (240s) ends:   Fri Jul 17 12:06:47 CST 2020
Iteration 1 (240s) result: 975.20 ops/m

Valid run!
Score on xml.transform: 975.20 ops/m


--- --- --- --- --- --- --- --- ---

  Benchmark:   xml.validation
  Run mode:    timed run
  Test type:   multi
  Threads:     64
  Warmup:      120s
  Iterations:  1
  Run length:  240s

Warmup (120s) begins: Fri Jul 17 12:06:47 CST 2020
Warmup (120s) ends:   Fri Jul 17 12:08:51 CST 2020
Warmup (120s) result: 1880.52 ops/m

Iteration 1 (240s) begins: Fri Jul 17 12:08:51 CST 2020
Iteration 1 (240s) ends:   Fri Jul 17 12:12:54 CST 2020
Iteration 1 (240s) result: 2049.82 ops/m

Valid run!
Score on xml.validation: 2049.82 ops/m

Results are stored in:
/opt/SPECjvm2008/results/SPECjvm2008.001/SPECjvm2008.001.raw
Generating reports in:
/opt/SPECjvm2008/results/SPECjvm2008.001

Composite result: 373.89 SPECjvm2008 Base ops/m
firefox results/SPECjvm2008.001/SPECjvm2008.001.html
```
3. result

Benchmark | ops/m
---|---
compiler|452.13
compress|190.25
crypto|2161.92
derby|862.67
mpegaudio|843.54
scimark.large|106.53
scimark.small|248.87
serial|482.6
startup|20.83
sunflow|391.43
xml|1413.85

> Composite result: 373.89

