# stream

STREAM is the de facto industry standard benchmark
for measuring sustained memory bandwidth.


## 编译

`gcc stream.c -O3 -fopenmp -DSTREAM_ARRAY_SIZE=80000000 -DNTIMES=20 -o stream`

## 测试结果
```
-------------------------------------------------------------
STREAM version $Revision: 5.10 $
-------------------------------------------------------------
Function    Best Rate MB/s  Avg time     Min time     Max time
Copy:           90014.1     0.031581     0.014220     0.050416
Scale:          91318.6     0.028011     0.014017     0.045802
Add:            97299.2     0.032065     0.019733     0.040896
Triad:          98350.8     0.028845     0.019522     0.037835
--------------------------------------------------------------
```