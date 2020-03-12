<!-- TOC -->

- [1. TPC概述](#1-tpc概述)
- [2. 测试环境](#2-测试环境)
- [3. 数据库部署](#3-数据库部署)
    - [3.1 配置Extra Packages for Enterprise Linux源](#31-配置extra-packages-for-enterprise-linux源)
    - [3.2 安装部署PostgreSQL](#32-安装部署postgresql)
    - [3.3 查看数据库基本情况](#33-查看数据库基本情况)
- [4. TPC-C基准测试](#4-tpc-c基准测试)
    - [4.1 下载测试工具](#41-下载测试工具)
    - [4.2 安装编译依赖](#42-安装编译依赖)
    - [4.3 编译benchmarksql](#43-编译benchmarksql)
    - [4.4 创建测试配置文件](#44-创建测试配置文件)
    - [4.5 构建测试数据](#45-构建测试数据)
    - [4.6 运行基准测试](#46-运行基准测试)
    - [4.7 将测试结果生成规范的测试报告](#47-将测试结果生成规范的测试报告)
- [5. TPC-H基准测试](#5-tpc-h基准测试)
    - [5.1 下载测试工具](#51-下载测试工具)
    - [5.2 修改编译参数](#52-修改编译参数)
    - [5.3 编译](#53-编译)
    - [5.4 生成测试数据100G为例](#54-生成测试数据100g为例)
    - [5.5 转换数据为导入数据库做准备](#55-转换数据为导入数据库做准备)
    - [5.6  准备各种第三方建库建表的相关脚本](#56--准备各种第三方建库建表的相关脚本)
    - [5.7 创建表结构并导入数据](#57-创建表结构并导入数据)
    - [5.8 建立索引](#58-建立索引)
    - [5.9 生成测试需要的SQL查询语言](#59-生成测试需要的sql查询语言)
    - [5.10 执行基准测试等待结果](#510-执行基准测试等待结果)

<!-- /TOC -->

# 1. TPC概述

事务处理性能测试委员会TPC（Transaction process performance Council）是一个专门负责制定计算机事务处理能力测试标准并监督其执行的组织，其总部位于美国。该组织对全世界开放，但迄今为止，绝大多数会员都是美、日、西欧的大公司。TPC的成员主要是计算机软硬件厂家，而非计算机用户，它的功能是制定商务应用基准程序（Benchmark）的标准规范、性能和价格度量，并管理测试结果的发布。 比较不同系统的性能优劣需要公共的衡量标准，它不仅可以帮助用户做出购买决定，而且也可以帮助厂商进行产品推广和市场宣传。

TPC解决了计算机软件系统性能评价标准的纷争与混乱,为用户和厂商提供了双方的利益。TPC的标准还可以裁剪,以更恰当地表示具体的系统。许多政府部门和公司都购买TPC标准作为其特定计算机系统性能评价体系的基础。 TPC的出版物是开放的，可以通过网络获取（<http://www.tpc.org>）。
TPC不给出基准程序的代码，而只给出基准程序的标准规范（Standard Specification）。任何厂家或其它测试者都可以根据规范，最优地构造出自己的系统（测试平台和测试程序）。为保证测试结果的客观性，被测试者（通常是厂家）必须提交给TPC一套完整的报告（Full Disclosure Report），包括被测系统的详细配置、分类价格和包含五年维护费用在内的总价格。该报告必须由TPC授权的审核员核实（TPC本身并不做审计）。现在全球只有几个审核员，全部在美国。

TPC测试标准分类：
- 联机在线事务处理系统（OLTP）测试标准：TPC-C
- 决策支持/大数据（DSS/OLAP）测试标准：TPC-H

# 2. 测试环境

Key | Value
---|---
硬件环境 | Greatwall QingTian DF720
操作系统 | CentOS 7.5 aarch64
数据库 | PostgreSQL 9.2
基准测试工具 | benchmarksql-5.0 tpch-2.18.0_rc2.zip

# 3. 数据库部署

## 3.1 配置Extra Packages for Enterprise Linux源

```bash
[root@bogon ~]# cat /etc/yum.repos.d/epel.repo
[epel]
name=Extra Packages for Enterprise Linux 7 - $basearch
baseurl=http://mirrors.aliyun.com/epel/7/aarch64/
failovermethod=priority
enabled=1
gpgcheck=0
gpgkey=file:///etc/pki/rpm-gpg/RPM-GPG-KEY-EPEL-7
```

## 3.2 安装部署PostgreSQL

```
[root@bogon ~]# yum install postgresql-server

[root@bogon ~]# postgresql-setup initdb
Initializing database ... OK

[root@bogon ~]# systemctl start postgresql.service
[root@bogon ~]# systemctl status postgresql.service
● postgresql.service - PostgreSQL database server
   Loaded: loaded (/usr/lib/systemd/system/postgresql.service; disabled; vendor preset: disabled)
   Active: active (running) since 二 2020-03-03 16:04:31 CST; 44s ago
  Process: 61839 ExecStart=/usr/bin/pg_ctl start -D ${PGDATA} -s -o -p ${PGPORT} -w -t 300 (code=exited, status=0/SUCCESS)
  Process: 61831 ExecStartPre=/usr/bin/postgresql-check-db-dir ${PGDATA} (code=exited, status=0/SUCCESS)
    Tasks: 7
   CGroup: /system.slice/postgresql.service
           ├─61841 /usr/bin/postgres -D /var/lib/pgsql/data -p 5432
           ├─61842 postgres: logger process
           ├─61844 postgres: checkpointer process
           ├─61845 postgres: writer process
           ├─61846 postgres: wal writer process
           ├─61847 postgres: autovacuum launcher process
           └─61848 postgres: stats collector process

3月 03 16:04:30 bogon systemd[1]: Starting PostgreSQL database server...
3月 03 16:04:31 bogon systemd[1]: Started PostgreSQL database server.

[root@bogon data]# systemctl enable postgresql.service
Created symlink from /etc/systemd/system/multi-user.target.wants/postgresql.service to /usr/lib/systemd/system/postgresql.service.
```

## 3.3 查看数据库基本情况

```
[root@bogon data]# su - postgres
-bash-4.2$

postgres=# \conninfo
以用户 "postgres" 的身份，通过套接字"/var/run/postgresql"在端口"5432"连接到数据库 "postgres"

postgres=# SELECT version();
 PostgreSQL 9.2.24 on aarch64-redhat-linux-gnu, compiled by gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-39), 64-bit

postgres=# \l
                                     资料库列表
   名称    |  拥有者  | 字元编码 |  校对规则   |    Ctype    |       存取权限
-----------+----------+----------+-------------+-------------+-----------------------
 postgres  | postgres | UTF8     | zh_CN.UTF-8 | zh_CN.UTF-8 |
 template0 | postgres | UTF8     | zh_CN.UTF-8 | zh_CN.UTF-8 | =c/postgres          +
           |          |          |             |             | postgres=CTc/postgres
 template1 | postgres | UTF8     | zh_CN.UTF-8 | zh_CN.UTF-8 | =c/postgres          +
           |          |          |             |             | postgres=CTc/postgres
(3 行记录)

postgres=# \du
                        角色列表
 角色名称 |               属性                | 成员属于
----------+-----------------------------------+----------
 postgres | 超级用户, 建立角色, 建立 DB, 复制 | {}

postgres=# \q
```

# 4. TPC-C基准测试

## 4.1 下载测试工具

- 下载地址：https://sourceforge.net/projects/benchmarksql/files/latest/download
- 测试工具：Benchmarksql-5.0.zip

## 4.2 安装编译依赖

```
[root@bogon benchmarksql-5.0]# yum install postgresql-jdbc ant R
```

## 4.3 编译benchmarksql

```
[root@bogon benchmarksql-5.0]# ant
Buildfile: /opt/benchmarksql-5.0/build.xml
init:
    [mkdir] Created dir: /opt/benchmarksql-5.0/build
compile:
    [javac] Compiling 11 source files to /opt/benchmarksql-5.0/build
dist:
    [mkdir] Created dir: /opt/benchmarksql-5.0/dist      
    [jar] Building jar: /opt/benchmarksql-5.0/dist/BenchmarkSQL-5.0.jar
    
    BUILD SUCCESSFULTotal time: 2 seconds
```

## 4.4 创建测试配置文件

postgresql.properties
```
-bash-4.2$ cat postgresql.properties
db=postgres
driver=org.postgresql.Driver
conn=jdbc:postgresql://127.0.0.1:5432/postgres
user=postgres
password=abc123

warehouses=100
loadWorkers=100

terminals=20
//To run specified transactions per terminal- runMins must equal zero
runTxnsPerTerminal=0
//To run for specified minutes- runTxnsPerTerminal must equal zero
runMins=10
//Number of total transactions per minute
limitTxnsPerMin=0

//Set to true to run in 4.x compatible mode. Set to false to use the
//entire configured database evenly.
terminalWarehouseFixed=true

//The following five values must add up to 100
//The default percentages of 45, 43, 4, 4 & 4 match the TPC-C spec
newOrderWeight=45
paymentWeight=43
orderStatusWeight=4
deliveryWeight=4
stockLevelWeight=4

// Directory name to create for collecting detailed result data.
// Comment this out to suppress.
resultDirectory=result_%tY-%tm-%td_%tH%tM%tS
osCollectorScript=./misc/os_collector_linux.py
osCollectorInterval=1
osCollectorDevices=blk_sda
```

## 4.5 构建测试数据

```
-bash-4.2$ ./runDatabaseBuild.sh postgresql.properties
# ------------------------------------------------------------
# Loading SQL file ./sql.common/tableCreates.sql
# ------------------------------------------------------------
```

## 4.6 运行基准测试

```
-bash-4.2$ ./runBenchmark.sh postgresql.properties
11:20:17,636 [main] INFO   jTPCC : Term-00,
11:20:17,641 [main] INFO   jTPCC : Term-00, +-------------------------------------------------------------+
11:20:17,641 [main] INFO   jTPCC : Term-00,      BenchmarkSQL v5.0
11:20:17,641 [main] INFO   jTPCC : Term-00, +-------------------------------------------------------------+
11:20:17,642 [main] INFO   jTPCC : Term-00,  (c) 2003, Raul Barbosa
11:20:17,642 [main] INFO   jTPCC : Term-00,  (c) 2004-2016, Denis Lussier
11:20:17,645 [main] INFO   jTPCC : Term-00,  (c) 2016, Jan Wieck
11:20:17,646 [main] INFO   jTPCC : Term-00, +-------------------------------------------------------------+
11:20:17,646 [main] INFO   jTPCC : Term-00,
11:20:17,646 [main] INFO   jTPCC : Term-00, db=postgres
11:20:17,646 [main] INFO   jTPCC : Term-00, driver=org.postgresql.Driver
11:20:17,646 [main] INFO   jTPCC : Term-00, conn=jdbc:postgresql://127.0.0.1:5432/postgres
11:20:17,646 [main] INFO   jTPCC : Term-00, user=postgres
11:20:17,647 [main] INFO   jTPCC : Term-00,
11:20:17,647 [main] INFO   jTPCC : Term-00, warehouses=10
11:20:17,647 [main] INFO   jTPCC : Term-00, terminals=10
11:20:17,650 [main] INFO   jTPCC : Term-00, runTxnsPerTerminal=10
11:20:17,650 [main] INFO   jTPCC : Term-00, limitTxnsPerMin=300
11:20:17,650 [main] INFO   jTPCC : Term-00, terminalWarehouseFixed=true
11:20:17,650 [main] INFO   jTPCC : Term-00,
11:20:17,650 [main] INFO   jTPCC : Term-00, newOrderWeight=45
11:20:17,651 [main] INFO   jTPCC : Term-00, paymentWeight=43
11:20:17,651 [main] INFO   jTPCC : Term-00, orderStatusWeight=4
11:20:17,651 [main] INFO   jTPCC : Term-00, deliveryWeight=4
11:20:17,651 [main] INFO   jTPCC : Term-00, stockLevelWeight=4
11:20:17,651 [main] INFO   jTPCC : Term-00,
11:20:17,651 [main] INFO   jTPCC : Term-00, resultDirectory=result_%tY-%tm-%td_%tH%tM%tS
11:20:17,651 [main] INFO   jTPCC : Term-00, osCollectorScript=./misc/os_collector_linux.py
11:20:17,652 [main] INFO   jTPCC : Term-00,
11:20:17,672 [main] INFO   jTPCC : Term-00, copied postgresql.properties to result_2020-03-06_112017/run.properties
11:20:17,673 [main] INFO   jTPCC : Term-00, created result_2020-03-06_112017/data/runInfo.csv for runID 5
11:20:17,673 [main] INFO   jTPCC : Term-00, writing per transaction results to result_2020-03-06_112017/data/result.csv
11:20:17,675 [main] INFO   jTPCC : Term-00, osCollectorScript=./misc/os_collector_linux.py
11:20:17,675 [main] INFO   jTPCC : Term-00, osCollectorInterval=1
11:20:17,675 [main] INFO   jTPCC : Term-00, osCollectorSSHAddr=null
11:20:17,676 [main] INFO   jTPCC : Term-00, osCollectorDevices=blk_sda
11:20:17,796 [main] INFO   jTPCC : Term-00,
11:20:17,934 [main] INFO   jTPCC : Term-00, C value for C_LAST during load: 220
11:20:17,935 [main] INFO   jTPCC : Term-00, C value for C_LAST this run:    144
11:20:17,935 [main] INFO   jTPCC : Term-00,                                                                                                                                                                            Term-00, Running Av11:20:38,030 [Thread-10] INFO   jTPCC : Term-00,   Memory Usage: 206MB / 1963MB
11:20:38,031 [Thread-10] INFO   jTPCC : Term-00,
11:20:38,031 [Thread-10] INFO   jTPCC : Term-00, Measured tpmC (NewOrders) = 140.92
11:20:38,031 [Thread-10] INFO   jTPCC : Term-00, Measured tpmTOTAL = 302.84
11:20:38,031 [Thread-10] INFO   jTPCC : Term-00, Session Start     = 2020-03-06 11:20:18
11:20:38,032 [Thread-10] INFO   jTPCC : Term-00, Session End       = 2020-03-06 11:20:38
11:20:38,032 [Thread-10] INFO   jTPCC : Term-00, Transaction Count = 100
```

> TpmC测试结果：140.02。

## 4.7 将测试结果生成规范的测试报告

```
-bash-4.2$ ./generateReport.sh result_2020-03-06_112017/
Generating result_2020-03-06_112017//tpm_nopm.png ... OK
Generating result_2020-03-06_112017//latency.png ... OK
Generating result_2020-03-06_112017//cpu_utilization.png ... OK
Generating result_2020-03-06_112017//dirty_buffers.png ... OK
Generating result_2020-03-06_112017//blk_sda_iops.png ... OK
Generating result_2020-03-06_112017//blk_sda_kbps.png ... OK
Generating result_2020-03-06_112017//report.html ... OK
```
> 可以使用firefox查看标准格式报告。


# 5. TPC-H基准测试

## 5.1 下载测试工具

```
[root@bogon dbgen]# wget http://www.tpc.org/tpch-2.18.0_rc2.zip && unzip tpch-2.18.0_rc2.zip
```

## 5.2 修改编译参数

```
vim makefile.suite
CC      = GCC
DATABASE= ORACLE
MACHINE = LINUX
WORKLOAD = TPCH
```

## 5.3 编译

```
[root@bogon tpch-dbgen]# make -f makefile.suite
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o build.o build.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o driver.o driver.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o bm_utils.o bm_utils.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o rnd.o rnd.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o print.o print.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o load_stub.o load_stub.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o bcd2.o bcd2.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o speed_seed.o speed_seed.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o text.o text.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o permute.o permute.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o rng64.o rng64.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64  -O -o dbgen build.o driver.o bm_utils.o rnd.o print.o load_stub.o bcd2.o speed_seed.o text.o permute.o rng64.o -lm
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o qgen.o qgen.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o varsub.o varsub.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DSQLSERVER -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64  -O -o qgen build.o bm_utils.o qgen.o rnd.o varsub.o text.o bcd2.o permute.o speed_seed.o rng64.o -lm
```

## 5.4 生成测试数据100G为例

```
[root@bogon tpch-dbgen]# ./dbgen -vf -s 100
TPC-H Population Generator (Version 2.18.0)
Copyright Transaction Processing Performance Council 1994 - 2010
Generating data for suppliers table/
Preloading text ... 100%
done.
Generating data for customers tabledone.
Generating data for orders/lineitem tablesdone.
Generating data for part/partsupplier tablesdone.
Generating data for nation tabledone.
Generating data for region tabledone.

[root@bogon tpch-dbgen]# du -sh *.tbl
2.3G    customer.tbl
75G     lineitem.tbl
4.0K    nation.tbl
17G     orders.tbl
12G     partsupp.tbl
2.3G    part.tbl
4.0K    region.tbl
137M    supplier.tbl
```

可以并行生成分片文件，如下shell脚本参考（10个分片文件并行生成1000G数据）：
```
for((i=1;i<=10;i++));
do
   ./dbgen -s 1000 -S $i -C 10 -f &
done 
```

合并脚本参考：
```
[root@bogon tpch-dbgen]# cat merge.sh
split=10
filename=customer

> $filename
for (( i=1; i<=$split; i++ )); do
        cat $filename.tbl."$i" >> $filename
done
```

## 5.5 转换数据为导入数据库做准备

处理生成的 tbl 文件，tbl文件每行最后会多1个'|'，可以用sed命令将每行后面的'|'去掉，shell脚本参考：
```
[root@bogon tpch-dbgen]# for i in `ls *.tbl`; do sed 's/|$//' $i > ${i/tbl/csv}; echo $i; done;
[root@bogon tpch-dbgen]# du -sh *csv
2.3G    customer.csv
74G     lineitem.csv
4.0K    nation.csv
17G     orders.csv
2.3G    part.csv
12G     partsupp.csv
4.0K    region.csv
136M    supplier.csv
```

## 5.6  准备各种第三方建库建表的相关脚本

```
[root@bogon tpch-dbgen]# git clone https://github.com/tvondra/pg_tpch.git
```

## 5.7 创建表结构并导入数据

```
[root@bogon tpch-dbgen]# su - postgres
上一次登录：二 3月  3 16:13:28 CST 2020pts/0 上
-bash-4.2$ createdb tpch
-bash-4.2$ psql tpch < /tmp/tpch/tpch-2.18.0_rc2/tpch-dbgen/pg_tpch/dss/tpch-create.sql
CREATE TABLE
CREATE TABLE
CREATE TABLE
CREATE TABLE
CREATE TABLE
CREATE TABLE
CREATE TABLE
CREATE TABLE

-bash-4.2$ cat import_database_from_csv.sh
#!/bin/bash

dir="/tmp/tpch/tpch-2.18.0_rc2/tpch-dbgen"
opts="tpch"

psql $opts -c "COPY part FROM '$dir/part.csv'
                               WITH (FORMAT csv, DELIMITER '|')"

psql $opts -c "COPY region FROM '$dir/region.csv'
                               WITH (FORMAT csv, DELIMITER '|')"

psql $opts -c "COPY nation FROM '$dir/nation.csv'
                               WITH (FORMAT csv, DELIMITER '|')"

psql $opts -c "COPY supplier FROM '$dir/supplier.csv'
                               WITH (FORMAT csv, DELIMITER '|')"

psql $opts -c "COPY customer FROM '$dir/customer.csv'
                                WITH (FORMAT csv, DELIMITER '|')"

psql $opts -c "COPY partsupp FROM '$dir/partsupp.csv'
                               WITH (FORMAT csv, DELIMITER '|')"

psql $opts -c "COPY orders FROM '$dir/orders.csv'
                               WITH (FORMAT csv, DELIMITER '|')"

psql $opts -c "COPY lineitem FROM '$dir/lineitem.csv'
                               WITH (FORMAT csv, DELIMITER '|')"

-bash-4.2$ ./import_database_from_csv.sh
COPY 20000000
COPY 5
COPY 25
COPY 1000000
COPY 15000000
COPY 80000000
COPY 150000000
COPY 600037902
```

## 5.8 建立索引

```
# Primary Key
-bash-4.2$  psql tpch < /tmp/tpch/tpch-2.18.0_rc2/tpch-dbgen/pg_tpch/dss/tpch-pkeys.sql
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE

# foreign kyes
-bash-4.2$ psql tpch < /tmp/tpch/tpch-2.18.0_rc2/tpch-dbgen/pg_tpch/dss/tpch-alter.sql
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE
ALTER TABLE

# create indexs
-bash-4.2$ psql tpch < /tmp/tpch/tpch-2.18.0_rc2/tpch-dbgen/pg_tpch/dss/tpch-index.sql
CREATE INDEX
CREATE INDEX
CREATE INDEX
CREATE INDEX
CREATE INDEX
CREATE INDEX
CREATE INDEX
CREATE INDEX
CREATE INDEX
```

## 5.9 生成测试需要的SQL查询语言

```
[root@bogon tpch-dbgen]# cat make_workload.sh
#!/bin/bash

export DSS_QUERY="../tpch-dbgen/pg_tpch/dss/templates/"

mkdir -pv workload

for nr in `seq 1 22`
do
    ./qgen $nr > ./workload/workload-$nr.sql
done

[root@bogon tpch-dbgen]# ./make_workload.sh

[root@bogon tpch-dbgen]# cp workload -a /var/lib/pgsql/
```

## 5.10 执行基准测试等待结果

```
[root@bogon tpch-dbgen]# su - postgres
-bash-4.2$ cat run_benchmark.sh
#!/bin/bash

mkdir -pv result

for nr in `seq 1 22`
do
  /usr/bin/time -f "total=%e" -o ./result/result-$nr.log \
                psql tpch < ./workload/workload-$nr.sql > /dev/null 2>&1 &
done;

for p in `jobs -p`
do
   wait $p;
done;

-bash-4.2$ ./run_benchmark.sh
-bash-4.2$ ls
result-10.log
result-11.log
result-12.log
result-13.log
result-14.log
result-15.log
result-16.log
result-17.log
result-18.log
result-19.log
result-1.log
result-20.log
result-21.log
result-22.log
result-2.log
result-3.log
result-4.log
result-5.log
result-6.log
result-7.log
result-8.log
result-9.log

-bash-4.2$ cat *
total=1156.21
total=35.56
total=690.46
total=807.87
total=317.16
total=530.67
total=1021.99
total=5647.87
total=1698.81
total=589.84
total=4519.09
total=4352.13
total=2066.66
total=174.72
total=183.49
total=875.77
total=339.85
total=2004.31
total=317.02
total=1060.62
total=2405.71
total=4849.67
```

case | elapsed time | comment
---|---|---
Q1 | 4519.09 | 价格统计报告查询
Q2 | 183.49 | 最小代价供货商查询
Q3 | 875.77 | 运送优先级查询
Q4 | 339.85 | 订单优先级查询
Q5 | 2004.31 | 某地区供货商为公司带来的收入查询
Q6 | 317.02 | 预测收入变化查询
Q7 | 1060.62 | 货运盈利情况查询
Q8 | 2405.71 | 国家市场份额查询
Q9 | 4849.67 | 产品类型利润估量查询
Q10 | 1156.21 | 货运存在问题的查询
Q11 | 35.56 | 库存价值查询
Q12 | 690.46 | 货运模式和订单优先级查询
Q13 | 807.87 | 消费者订单数量查询
Q14 | 317.16 | 促销效果查询
Q15 | 530.67 | 头等供货商查询
Q16 | 1021.99 | 零件/供货商关系查询
Q17 | 5647.87 | 小订单收入查询
Q18 | 1698.81 | 大订单顾客查询
Q19 | 589.84 | 折扣收入查询
Q20 | 4352.13 | 供货商竞争力查询
Q21 | 2066.66 | 不能按时交货供货商查询
Q22 | 174.72 | 全球销售机会查询