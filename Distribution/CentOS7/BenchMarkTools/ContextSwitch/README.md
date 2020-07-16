# contextswitch

Little micro-benchmarks to assess the performance overhead of context switching.

## build&install

```
#> git clone https://github.com/tsuna/contextswitch.git
#> cd contextswitch
#> make
```

## run benchmark

```
#> ./cpubench.sh
16 physical CPUs, 4 cores/CPU, 1 hardware threads/core = 64 hw threads total
-- No CPU affinity --
10000000 system calls in 3328689041ns (332.9ns/syscall)
2000000 process context switches in 7157743717ns (3578.9ns/ctxsw)
2000000  thread context switches in 7299953553ns (3650.0ns/ctxsw)
2000000  thread context switches in 361200634ns (180.6ns/ctxsw)
-- With CPU affinity --
10000000 system calls in 3269437138ns (326.9ns/syscall)
2000000 process context switches in 8312970425ns (4156.5ns/ctxsw)
2000000  thread context switches in 6599169698ns (3299.6ns/ctxsw)
2000000  thread context switches in 1090190852ns (545.1ns/ctxsw)
-- With CPU affinity to CPU 0 --
10000000 system calls in 3279252305ns (327.9ns/syscall)
2000000 process context switches in 7308203062ns (3654.1ns/ctxsw)
2000000  thread context switches in 7353160165ns (3676.6ns/ctxsw)
2000000  thread context switches in 366700027ns (183.4ns/ctxsw)
```