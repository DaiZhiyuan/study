
# SuperPI

## single core

```
[root@jerrydai superpi_linux]# taskset -c 0 ./pi_css5 $((2<<20))
Calculation of PI using FFT and AGM, ver. LG1.1.2-MP1.5.2a.memsave
initializing...
nfft= 524288
radix= 10000
error_margin= 0.0114669
calculating 2097152 digits of PI...
AGM iteration
precision= 48: 0.65 sec
precision= 80: 0.66 sec
precision= 176: 0.65 sec
precision= 352: 0.65 sec
precision= 688: 0.66 sec
precision= 1392: 0.66 sec
precision= 2784: 0.72 sec
precision= 5584: 0.66 sec
precision= 11168: 0.66 sec
precision= 22336: 0.67 sec
precision= 44688: 0.66 sec
precision= 89408: 0.67 sec
precision= 178816: 0.66 sec
precision= 357648: 0.68 sec
precision= 715312: 0.66 sec
precision= 1430640: 0.66 sec
precision= 2861280: 0.67 sec
writing pi2097152.txt...
13.11 sec. (real time)
```

## numa node

```
[root@jerrydai superpi_linux]# numactl --cpunodebind 0 --localalloc ./pi_css5 $((2<<20))
Calculation of PI using FFT and AGM, ver. LG1.1.2-MP1.5.2a.memsave
initializing...
nfft= 524288
radix= 10000
error_margin= 0.01234
calculating 2097152 digits of PI...
AGM iteration
precision= 48: 0.60 sec
precision= 80: 0.60 sec
precision= 176: 0.60 sec
precision= 352: 0.61 sec
precision= 688: 0.60 sec
precision= 1392: 0.60 sec
precision= 2784: 0.60 sec
precision= 5584: 0.61 sec
precision= 11168: 0.60 sec
precision= 22336: 0.60 sec
precision= 44688: 0.61 sec
precision= 89408: 0.60 sec
precision= 178816: 0.60 sec
precision= 357648: 0.61 sec
precision= 715312: 0.60 sec
precision= 1430640: 0.61 sec
precision= 2861280: 0.60 sec
writing pi2097152.txt...
11.92 sec. (real time)
```

## system wide

```
[root@jerrydai superpi_linux]# ./pi_css5 $((2<<20))
Calculation of PI using FFT and AGM, ver. LG1.1.2-MP1.5.2a.memsave
initializing...
nfft= 524288
radix= 10000
error_margin= 0.0114669
calculating 2097152 digits of PI...
AGM iteration
precision= 48: 0.68 sec
precision= 80: 0.67 sec
precision= 176: 0.67 sec
precision= 352: 0.66 sec
precision= 688: 0.66 sec
precision= 1392: 0.67 sec
precision= 2784: 0.66 sec
precision= 5584: 0.66 sec
precision= 11168: 0.67 sec
precision= 22336: 0.66 sec
precision= 44688: 0.66 sec
precision= 89408: 0.67 sec
precision= 178816: 0.66 sec
precision= 357648: 0.66 sec
precision= 715312: 0.66 sec
precision= 1430640: 0.67 sec
precision= 2861280: 0.66 sec
writing pi2097152.txt...
13.09 sec. (real time)
```

> -march=armv8-a+crc -mtune=cortex-a57 --param l1-cache-line-size=64 --param l1-cache-size=32 --param=l2-cache-size=163844