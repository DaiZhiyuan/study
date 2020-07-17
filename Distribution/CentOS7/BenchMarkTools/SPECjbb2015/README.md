## SPECjbb2015

1. run
```
[root@jerrydai SPECjbb2015]# ./run_composite.sh
Run 1: 20-07-17_131029
Launching SPECjbb2015 in Composite mode...

Start Composite JVM
Composite JVM PID = 43471

SPECjbb2015 is running...
Please monitor ./20-07-17_131029/controller.out for progress

Composite JVM has stopped
SPECjbb2015 has finished
```

2. composite.out
```
SPECjbb2015 Java Business Benchmark
 (c) Standard Performance Evaluation Corporation, 2015

Preparing to launch All-in-one SPECjbb2015.

Reading property file: /opt/SPECjbb2015/20-07-17_131029/./config/specjbb2015.props

     0s: System.out.println: Using patched version parser in org/glassfish/grizzly/utils/JdkVersion.java
System.out.println: Using patched version parser in org/glassfish/grizzly/utils/JdkVersion.java
System.out.println: Using patched version parser in org/glassfish/grizzly/utils/JdkVersion.java
Enumerating plugins...
     0s:    Connectivity:
     0s:             HTTP_Grizzly: Grizzly HTTP server, JDK HTTP client
     0s:              NIO_Grizzly: Grizzly NIO server, Grizzly NIO client
     0s:               HTTP_Jetty: Jetty HTTP server, JDK HTTP client
     0s:    Snapshot:
     0s:                 InMemory: Stores snapshots in heap memory
     0s:    Data Writers:
     0s:                     Demo: Send all frame to listener
     0s:                   InFile: Writing Data into file
     0s:                   Silent: Drop all frames
     0s:
     0s: Validating kit integrity...
     0s: Kit validation had passed.
     0s:
     0s: Tests are skipped.
     0s:
     0s: Ladies and gentlemen, this is your Controller speaking.
     2s: We are delighted in having you for benchmark run today.
     2s:
     2s: Binary log file is ./specjbb2015-C-20200717-00001.data.gz
     2s:
     2s: Performing handshake with all agents ...
     2s:
     2s:  Agent Group1.TxInjector.CompositeTxInjector has attached to Controller
     2s:  Agent Group1.Backend.CompositeBackend has attached to Controller
     2s:
     2s: All agents have connected.
     2s:
     2s: Attached agents info:

Group "Group1"
    TxInjectors:
        CompositeTxInjector, includes { Driver } @ [local in-Java link]
    Backends:
        CompositeBackend, includes { SM(2),SP(2) } @ [local in-Java link]

     2s: Initializing... (init) OK
     8s:
     8s: Check whether properties are compliant
     8s: (validation=PASSED) Property settings are COMPLIANT.
     8s:
     8s: Searching for high-bound for max-jOPS possible for this system
     8s: Searching from base IR = 100
     8s:       rampup IR =      0 [reconfig (term) (init) ] ... (rIR:aIR:PR = 0:0:0) (tPR = 0) [OK]
    16s:       rampup IR =     10 ... (rIR:aIR:PR = 10:11:11) (tPR = 763) [OK]
    19s:       rampup IR =     20 ... (rIR:aIR:PR = 20:20:20) (tPR = 1592) [OK]
    22s:       rampup IR =     30 .... (rIR:aIR:PR = 30:27:27) (tPR = 2112) [OK]
    26s:       rampup IR =     40 .... (rIR:aIR:PR = 40:38:38) (tPR = 2952) [OK]
    30s:       rampup IR =     50 .... (rIR:aIR:PR = 50:53:53) (tPR = 3546) [OK]
    34s:       rampup IR =     60 .... (rIR:aIR:PR = 60:61:60) (tPR = 3499) [OK]
    42s:       rampup IR =     70 .... (rIR:aIR:PR = 70:71:69) (tPR = 3857) [OK]
    46s:       rampup IR =     80 .... (rIR:aIR:PR = 80:81:79) (tPR = 3990) [OK]
    50s:       rampup IR =     90 .... (rIR:aIR:PR = 90:93:90) (tPR = 4178) [OK]
    55s:       rampup IR =    100 .... (rIR:aIR:PR = 100:100:97) (tPR = 4311) [OK]
    59s:    snapshot, IR =    100 .... (rIR:aIR:PR = 100:100:98) (tPR = 2874) [OK]
    
    ......

  8565s: Ending RT curve measurement because max-jOPS reached (too many failed points in a row)
  8565s: max-jOPS is presumably 18489
  8565s:
  8565s: Settling (term) (init) ... OK
  8578s:
  8578s: Running for validation
  8578s: (load =  0%) ...|.......... (rIR:aIR:PR = 0:0:0) (tPR = 0) [OK]
  8591s: (load = 10%) ....|.......... (rIR:aIR:PR = 1849:1853:1853) (tPR = 27503) [OK]
  8605s: (load = 20%) ....|.......... (rIR:aIR:PR = 3698:3702:3702) (tPR = 54502) [OK]
  8619s: (load = 30%) ....|.......... (rIR:aIR:PR = 5547:5546:5546) (tPR = 82825) [OK]
  8633s: (load = 40%) ....|.......... (rIR:aIR:PR = 7396:7416:7414) (tPR = 109550) [OK]
  8647s: (load = 50%) ........|.......... (rIR:aIR:PR = 9245:9353:9353) (tPR = 134783) [OK]
  8665s: (load = 60%) ....|.......... (rIR:aIR:PR = 11093:11190:11202) (tPR = 164185) [OK]
  8679s: (load = 70%) .....|.......... (rIR:aIR:PR = 12942:12781:12781) (tPR = 185378) [OK]
  8694s: (load = 80%) .....|.......... (rIR:aIR:PR = 14791:15175:15190) (tPR = 226345) [OK]
  8709s: (load = 90%) ......|.......... (rIR:aIR:PR = 16640:16932:16754) (tPR = 245328) [OK]
  8726s: (load = 100%) .?...........................|.......... (rIR:aIR:PR = 18489:18532:17720) (tPR = 266876) [OK]
  8767s: Validating... (quiescence........) (stop TxI) (stop BE) (saving... 257711 Kb) (validation=PASSED)(term) (init) ... ...completed!
  8802s: Restarting (term) (init) ... OK
  8813s:
  8813s:
  8813s: Profiling....|.............................. (rIR:aIR:PR = 1848:1849:1849) (tPR = 27492) [OK]
  8847s:  OK
  8847s: (term... (rIR:aIR:PR = 0:0:0) (tPR = 0) [OK]
  8850s: ) Tests are skipped.
  8850s:
  8850s: Generating level 0 report from ./specjbb2015-C-20200717-00001.data.gz

SPECjbb2015 Java Business Benchmark
 (c) Standard Performance Evaluation Corporation, 2015

Preparing to launch SPECjbb2015 reporter.

Reading property file: /opt/SPECjbb2015/20-07-17_131029/./config/specjbb2015.props
Report directory is result/specjbb2015-C-20200717-00001/report-00001
  8850s: Building report...

      3316 msec: Pre-reading source
         0 msec: Validation
         1 msec: Printing JbbProperties
         0 msec: Controller time verification
         3 msec: Build report with HW/SW parameters
        12 msec: Dump run logs
        62 msec: Parsing attributes
         4 msec: Parsing agent names
       142 msec: Building throughput - response time curve
RUN RESULT: hbIR (max attempted) = 19881, hbIR (settled) = 18803, max-jOPS = 18489, critical-jOPS = 7698
        62 msec: Primary metrics calculation
       213 msec: Render Allowed Failures
      1670 msec: Request Mix accuracy
      1817 msec: Render IR by probes tasks
      1879 msec: Overall RT curves
         7 msec: Render RT
      1812 msec: Max Delay during RT curve building
      2048 msec: Render IR/PR Accuracy
        13 msec: Render template to file result/specjbb2015-C-20200717-00001/report-00001/data/specjbb2015-C-20200717-00001-runProperties.txt
        46 msec: Render template to file result/specjbb2015-C-20200717-00001/report-00001/specjbb2015-C-20200717-00001.html
Report generation finished. Wallclock = 5973 msecs, real = 13107 msecs, parallelism = 2.19x

#> firefox result/specjbb2015-C-20200717-00001/report-00001/specjbb2015-C-20200717-00001.html
```

> SPECjbb2015-Composite max-jOPS：18489 

> SPECjbb2015-Composite critical-jOPS：7698 
