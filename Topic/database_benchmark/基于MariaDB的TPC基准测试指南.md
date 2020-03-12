<!-- TOC -->

- [1. TPC概述](#1-tpc概述)
- [2. 测试环境](#2-测试环境)
- [3. 数据库部署](#3-数据库部署)
    - [3.1 安装](#31-安装)
    - [3.2 初始化](#32-初始化)
    - [3.3 测试数据库连接](#33-测试数据库连接)
- [4. TPC-C基准测试](#4-tpc-c基准测试)
    - [4.1 编译安装tpcc-mysql](#41-编译安装tpcc-mysql)
    - [4.2 构建表结构](#42-构建表结构)
    - [4.3 建立测试数据](#43-建立测试数据)
    - [4.4 运行基准测试](#44-运行基准测试)
- [5. TPC-H基准测试](#5-tpc-h基准测试)
    - [5.1 编译部署dbgen](#51-编译部署dbgen)
    - [5.2 生成测试数据（10G）](#52-生成测试数据10g)
    - [5.3 初始化库表](#53-初始化库表)
    - [5.4 导入数据](#54-导入数据)
    - [5.5 创建索引与外键约束](#55-创建索引与外键约束)
    - [5.6 生成查询的SQL语句](#56-生成查询的sql语句)
    - [5.7 进行TPC-H测试](#57-进行tpc-h测试)

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
数据库 | MariaDB 5.5
基准测试工具 | tpcc-mysql.git tpch-2.18.0_rc2.zip

# 3. 数据库部署

## 3.1 安装
```
[root@bogon ~]# yum install mariadb-server
[root@bogon ~]# systemctl start mariadb
[root@bogon ~]# systemctl enable mariadb
Created symlink from /etc/systemd/system/multi-user.target.wants/mariadb.service to /usr/lib/systemd/system/mariadb.service.
```
## 3.2 初始化
```
[root@bogon ~]# mysql_secure_installation

NOTE: RUNNING ALL PARTS OF THIS SCRIPT IS RECOMMENDED FOR ALL MariaDB
      SERVERS IN PRODUCTION USE!  PLEASE READ EACH STEP CAREFULLY!

In order to log into MariaDB to secure it, we'll need the current
password for the root user.  If you've just installed MariaDB, and
you haven't set the root password yet, the password will be blank,
so you should just press enter here.

Enter current password for root (enter for none):
OK, successfully used password, moving on...

Setting the root password ensures that nobody can log into the MariaDB
root user without the proper authorisation.

Set root password? [Y/n] Y
New password:
Re-enter new password:
Password updated successfully!
Reloading privilege tables..
 ... Success!


By default, a MariaDB installation has an anonymous user, allowing anyone
to log into MariaDB without having to have a user account created for
them.  This is intended only for testing, and to make the installation
go a bit smoother.  You should remove them before moving into a
production environment.

Remove anonymous users? [Y/n] Y
 ... Success!

Normally, root should only be allowed to connect from 'localhost'.  This
ensures that someone cannot guess at the root password from the network.

Disallow root login remotely? [Y/n] N
 ... skipping.

By default, MariaDB comes with a database named 'test' that anyone can
access.  This is also intended only for testing, and should be removed
before moving into a production environment.

Remove test database and access to it? [Y/n] N
 ... skipping.

Reloading the privilege tables will ensure that all changes made so far
will take effect immediately.

Reload privilege tables now? [Y/n] Y
 ... Success!

Cleaning up...

All done!  If you've completed all of the above steps, your MariaDB
installation should now be secure.

Thanks for using MariaDB!
```
## 3.3 测试数据库连接

```
[root@bogon ~]# mysql -h localhost -u root -p
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 4
Server version: 5.5.64-MariaDB MariaDB Server

Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| test               |
+--------------------+
4 rows in set (0.01 sec)
```

# 4. TPC-C基准测试

## 4.1 编译安装tpcc-mysql

```
[root@bogon ~]# git clone https://github.com/Percona-Lab/tpcc-mysql.git
正克隆到 'tpcc-mysql'...
remote: Enumerating objects: 392, done.
remote: Total 392 (delta 0), reused 0 (delta 0), pack-reused 392
接收对象中: 100% (392/392), 202.81 KiB | 236.00 KiB/s, done.
处理 delta 中: 100% (216/216), done.

[root@bogon ~]# yum install mariadb-devel

[root@bogon tpcc-mysql]# ls
add_fkey_idx.sql  create_table.sql  drop_cons.sql         load.sh    schema2  src
count.sql         Dockerfile        load_multi_schema.sh  README.md  scripts  tpcc_load
[root@bogon tpcc-mysql]# cd src/ ; make
cc -w -O3 -g -I. `mysql_config --include`  -c load.c
cc -w -O3 -g -I. `mysql_config --include`  -c support.c
cc load.o support.o `mysql_config --libs_r` -lrt -o ../tpcc_load
cc -w -O3 -g -I. `mysql_config --include`  -c main.c
cc -w -O3 -g -I. `mysql_config --include`  -c spt_proc.c
cc -w -O3 -g -I. `mysql_config --include`  -c driver.c
cc -w -O3 -g -I. `mysql_config --include`  -c sequence.c
cc -w -O3 -g -I. `mysql_config --include`  -c rthist.c
cc -w -O3 -g -I. `mysql_config --include`  -c sb_percentile.c
cc -w -O3 -g -I. `mysql_config --include`  -c neword.c
cc -w -O3 -g -I. `mysql_config --include`  -c payment.c
cc -w -O3 -g -I. `mysql_config --include`  -c ordstat.c
cc -w -O3 -g -I. `mysql_config --include`  -c delivery.c
cc -w -O3 -g -I. `mysql_config --include`  -c slev.c
cc main.o spt_proc.o driver.o support.o sequence.o rthist.o sb_percentile.o neword.o payment.o ordstat.o delivery.o slev.o `mysql_config --libs_r` -lrt -o ../tpcc_start

[root@bogon tpcc-mysql]# ./tpcc_load -h
*************************************
*** TPCC-mysql Data Loader        ***
*************************************
./tpcc_load: option requires an argument -- 'h'
Usage: tpcc_load -h server_host -P port -d database_name -u mysql_user -p mysql_password -w warehouses -l part -m min_wh -n max_wh
* [part]: 1=ITEMS 2=WAREHOUSE 3=CUSTOMER 4=ORDERS

***************************************
*** ###easy### TPC-C Load Generator ***
***************************************
./tpcc_start: option requires an argument -- 'h'
Usage: tpcc_start -h server_host -P port -d database_name -u mysql_user -p mysql_password -w warehouses -c connections -r warmup_time -l running_time -i report_interval -f report_file -t trx_file
```

## 4.2 构建表结构

```
[root@bogon tpcc-mysql]# mysql -h localhost -P 3306 -u root -pabc123
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 6
Server version: 5.5.64-MariaDB MariaDB Server

Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| test               |
+--------------------+
4 rows in set (0.00 sec)

MariaDB [(none)]> CREATE DATABASE tpcc;
Query OK, 1 row affected (0.00 sec)

MariaDB [(none)]> show databases;
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
| test               |
| tpcc               |
+--------------------+
5 rows in set (0.00 sec)

MariaDB [(none)]> use tpcc;
Database changed

[root@bogon tpcc-mysql]# mysql -h localhost -P 3306 -uroot -pabc123 tpcc < create_table.sql
[root@bogon tpcc-mysql]# mysql -h localhost -P 3306 -uroot -pabc123 tpcc < add_fkey_idx.sql
[root@bogon tpcc-mysql]# mysql -h localhost -P 3306 -uroot -pabc123 tpcc -e "show tables;"
+----------------+
| Tables_in_tpcc |
+----------------+
| customer       |
| district       |
| history        |
| item           |
| new_orders     |
| order_line     |
| orders         |
| stock          |
| warehouse      |
+----------------+
```

## 4.3 建立测试数据

```
[root@bogon tpcc-mysql]# ./tpcc_load -h localhost -P 3306 -d tpcc -uroot -pabc123 -w 10
*************************************
*** TPCC-mysql Data Loader        ***
*************************************
option h with value 'localhost'
option P with value '3306'
option d with value 'tpcc'
option u with value 'root'
option p with value 'abc123'
option w with value '10'
<Parameters>
     [server]: localhost
     [port]: 3306
     [DBname]: tpcc
       [user]: root
       [pass]: abc123
  [warehouse]: 10
TPCC Data Load Started...

...DATA LOADING COMPLETED SUCCESSFULLY.
```

## 4.4 运行基准测试

```
[root@bogon tpcc-mysql]# ./tpcc_start -h 10.10.20.247 -P 3306 -d tpcc -u root -p abc123 -w 10 -c 30 -r 5 -l 10 -i 1
***************************************
*** ###easy### TPC-C Load Generator ***
***************************************
option h with value '10.10.20.247'
option P with value '3306'
option d with value 'tpcc'
option u with value 'root'
option p with value 'abc123'
option w with value '10'
option c with value '30'
option r with value '5'
option l with value '10'
option i with value '1'
<Parameters>
     [server]: 10.10.20.247
     [port]: 3306
     [DBname]: tpcc
       [user]: root
       [pass]: abc123
  [warehouse]: 10
 [connection]: 30
     [rampup]: 5 (sec.)
    [measure]: 10 (sec.)

RAMP-UP TIME.(5 sec.)

MEASURING START.

   1, trx: 274, 95%: 106.060, 99%: 148.348, max_rt: 230.483, 279|201.386, 28|53.579, 26|335.661, 28|257.033
   2, trx: 305, 95%: 96.314, 99%: 124.854, max_rt: 139.945, 303|113.256, 31|30.864, 32|202.235, 32|250.355
   3, trx: 278, 95%: 99.628, 99%: 142.047, max_rt: 189.909, 274|110.123, 27|31.196, 28|265.568, 28|234.393
   4, trx: 264, 95%: 104.766, 99%: 140.988, max_rt: 229.958, 270|160.621, 27|41.568, 24|230.903, 25|242.118
   5, trx: 262, 95%: 99.747, 99%: 145.402, max_rt: 184.047, 263|157.303, 27|46.186, 25|310.120, 27|252.782
   6, trx: 266, 95%: 112.032, 99%: 135.485, max_rt: 138.254, 269|175.965, 26|45.717, 28|256.945, 27|227.014
   7, trx: 273, 95%: 100.016, 99%: 106.952, max_rt: 133.194, 264|113.779, 27|28.934, 28|240.763, 29|243.386
   8, trx: 296, 95%: 114.884, 99%: 153.176, max_rt: 221.732, 299|139.529, 30|26.388, 29|341.738, 27|221.464
   9, trx: 278, 95%: 105.269, 99%: 141.707, max_rt: 175.832, 278|138.306, 28|46.644, 28|355.627, 30|253.843
  10, trx: 268, 95%: 102.901, 99%: 135.526, max_rt: 194.533, 269|233.829, 27|29.493, 28|225.803, 24|243.009

STOPPING THREADS..............................

<Raw Results>
  [0] sc:0 lt:2764  rt:0  fl:0 avg_rt: 80.8 (5)
  [1] sc:939 lt:1829  rt:0  fl:0 avg_rt: 30.3 (5)
  [2] sc:120 lt:158  rt:0  fl:0 avg_rt: 18.6 (5)
  [3] sc:6 lt:270  rt:0  fl:0 avg_rt: 237.8 (80)
  [4] sc:1 lt:276  rt:0  fl:0 avg_rt: 257.2 (20)
 in 10 sec.

<Raw Results2(sum ver.)>
  [0] sc:0  lt:2765  rt:0  fl:0
  [1] sc:939  lt:1831  rt:0  fl:0
  [2] sc:120  lt:158  rt:0  fl:0
  [3] sc:6  lt:270  rt:0  fl:0
  [4] sc:1  lt:276  rt:0  fl:0

<Constraint Check> (all must be [OK])
 [transaction percentage]
        Payment: 43.50% (>=43.0%) [OK]
   Order-Status: 4.37% (>= 4.0%) [OK]
       Delivery: 4.34% (>= 4.0%) [OK]
    Stock-Level: 4.35% (>= 4.0%) [OK]
 [response time (at least 90% passed)]
      New-Order: 0.00%  [NG] *
        Payment: 33.92%  [NG] *
   Order-Status: 43.17%  [NG] *
       Delivery: 2.17%  [NG] *
    Stock-Level: 0.36%  [NG] *

<TpmC>  16584.000 TpmC
```

> TpmC 测试结果为：16584。

# 5. TPC-H基准测试

## 5.1 编译部署dbgen

```
[root@bogon opt]# unzip tpch-2.18.0_rc2.zip && cd tpch-2.18.0_rc2
[root@bogon tpch-2.18.0_rc2]# ls
dbgen  dev-tools  EULA.txt  ref_data

[root@bogon dbgen]# cp makefile.suite makefile
```

修改makefile中的CC、DATABASE、MACHINE、WORKLOAD等定义：
```
################
## CHANGE NAME OF ANSI COMPILER HERE
################
CC      = gcc
# Current values for DATABASE are: INFORMIX, DB2, TDAT (Teradata)
#                                  SQLSERVER, SYBASE, ORACLE, VECTORWISE
# Current values for MACHINE are:  ATT, DOS, HP, IBM, ICL, MVS,
#                                  SGI, SUN, U2200, VMS, LINUX, WIN32
# Current values for WORKLOAD are:  TPCH
DATABASE= MYSQL
MACHINE = LINUX
WORKLOAD = TPCH
```
修改tpcd.h文件，增加几行宏定义：
```
#ifdef MYSQL
#define GEN_QUERY_PLAN ""
#define START_TRAN "START TRANSACTION"
#define END_TRAN "COMMMIT"
#define SET_OUTPUT ""
#define SET_ROWCOUNT "limit %d;\n"
#define SET_DBASE "use %s;\n"
#endif
```

编译：
```
[root@bogon dbgen]# make
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o build.o build.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o driver.o driver.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o bm_utils.o bm_utils.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o rnd.o rnd.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o print.o print.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o load_stub.o load_stub.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o bcd2.o bcd2.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o speed_seed.o speed_seed.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o text.o text.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o permute.o permute.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o rng64.o rng64.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64  -O -o dbgen build.o driver.o bm_utils.o rnd.o print.o load_stub.o bcd2.o speed_seed.o text.o permute.o rng64.o -lm
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o qgen.o qgen.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64    -c -o varsub.o varsub.c
gcc -g -DDBNAME=\"dss\" -DLINUX -DMYSQL -DTPCH -DRNG_TEST -D_FILE_OFFSET_BITS=64  -O -o qgen build.o bm_utils.o qgen.o rnd.o varsub.o text.o bcd2.o permute.o speed_seed.o rng64.o -lm

[root@bogon dbgen]# file dbgen
dbgen: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV), dynamically linked (uses shared libs), for GNU/Linux 3.7.0, BuildID[sha1]=13437e3931d151a84b7483cebdc0d079e46a7557, not stripped
[root@bogon dbgen]# file qgen
qgen: ELF 64-bit LSB executable, ARM aarch64, version 1 (SYSV), dynamically linked (uses shared libs), for GNU/Linux 3.7.0, BuildID[sha1]=b22ee27622a59442c893eee800a31dd4e7aa9336, not stripped
```
- dbgen：数据生成工具
- qgen：SQL生成工具

## 5.2 生成测试数据（10G）

```
[root@bogon dbgen]# ./dbgen -vf -s 10
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

[root@bogon dbgen]# du -sh *.tbl
234M    customer.tbl
7.3G    lineitem.tbl
4.0K    nation.tbl
1.7G    orders.tbl
1.2G    partsupp.tbl
233M    part.tbl
4.0K    region.tbl
14M     supplier.tbl
```

> dbgen参数 -s 的作用是指定生成测试数据的仓库数，建议基准值设定在100以上，在我的测试环境中，一般都设定为1000。

## 5.3 初始化库表

tpch提供的数据库表初始化脚本(dss.ddl)对MySQL数据库无支持，所以需要在dss.ddl中增加几行：
```
DROP DATABASE tpch;
CREATE DATABASE tpch;
USE tpch;
```

修改后的dss.ddl内容如下：
```
[root@bogon dbgen]# cat dss.ddl
-- Sccsid:     @(#)dss.ddl      2.1.8.1

DROP DATABASE tpch;
CREATE DATABASE tpch;
USE tpch;

CREATE TABLE NATION  ( N_NATIONKEY  INTEGER NOT NULL,
                            N_NAME       CHAR(25) NOT NULL,
                            N_REGIONKEY  INTEGER NOT NULL,
                            N_COMMENT    VARCHAR(152));

CREATE TABLE REGION  ( R_REGIONKEY  INTEGER NOT NULL,
                            R_NAME       CHAR(25) NOT NULL,
                            R_COMMENT    VARCHAR(152));

CREATE TABLE PART  ( P_PARTKEY     INTEGER NOT NULL,
                          P_NAME        VARCHAR(55) NOT NULL,
                          P_MFGR        CHAR(25) NOT NULL,
                          P_BRAND       CHAR(10) NOT NULL,
                          P_TYPE        VARCHAR(25) NOT NULL,
                          P_SIZE        INTEGER NOT NULL,
                          P_CONTAINER   CHAR(10) NOT NULL,
                          P_RETAILPRICE DECIMAL(15,2) NOT NULL,
                          P_COMMENT     VARCHAR(23) NOT NULL );

CREATE TABLE SUPPLIER ( S_SUPPKEY     INTEGER NOT NULL,
                             S_NAME        CHAR(25) NOT NULL,
                             S_ADDRESS     VARCHAR(40) NOT NULL,
                             S_NATIONKEY   INTEGER NOT NULL,
                             S_PHONE       CHAR(15) NOT NULL,
                             S_ACCTBAL     DECIMAL(15,2) NOT NULL,
                             S_COMMENT     VARCHAR(101) NOT NULL);

CREATE TABLE PARTSUPP ( PS_PARTKEY     INTEGER NOT NULL,
                             PS_SUPPKEY     INTEGER NOT NULL,
                             PS_AVAILQTY    INTEGER NOT NULL,
                             PS_SUPPLYCOST  DECIMAL(15,2)  NOT NULL,
                             PS_COMMENT     VARCHAR(199) NOT NULL );

CREATE TABLE CUSTOMER ( C_CUSTKEY     INTEGER NOT NULL,
                             C_NAME        VARCHAR(25) NOT NULL,
                             C_ADDRESS     VARCHAR(40) NOT NULL,
                             C_NATIONKEY   INTEGER NOT NULL,
                             C_PHONE       CHAR(15) NOT NULL,
                             C_ACCTBAL     DECIMAL(15,2)   NOT NULL,
                             C_MKTSEGMENT  CHAR(10) NOT NULL,
                             C_COMMENT     VARCHAR(117) NOT NULL);

CREATE TABLE ORDERS  ( O_ORDERKEY       INTEGER NOT NULL,
                           O_CUSTKEY        INTEGER NOT NULL,
                           O_ORDERSTATUS    CHAR(1) NOT NULL,
                           O_TOTALPRICE     DECIMAL(15,2) NOT NULL,
                           O_ORDERDATE      DATE NOT NULL,
                           O_ORDERPRIORITY  CHAR(15) NOT NULL,
                           O_CLERK          CHAR(15) NOT NULL,
                           O_SHIPPRIORITY   INTEGER NOT NULL,
                           O_COMMENT        VARCHAR(79) NOT NULL);

CREATE TABLE LINEITEM ( L_ORDERKEY    INTEGER NOT NULL,
                             L_PARTKEY     INTEGER NOT NULL,
                             L_SUPPKEY     INTEGER NOT NULL,
                             L_LINENUMBER  INTEGER NOT NULL,
                             L_QUANTITY    DECIMAL(15,2) NOT NULL,
                             L_EXTENDEDPRICE  DECIMAL(15,2) NOT NULL,
                             L_DISCOUNT    DECIMAL(15,2) NOT NULL,
                             L_TAX         DECIMAL(15,2) NOT NULL,
                             L_RETURNFLAG  CHAR(1) NOT NULL,
                             L_LINESTATUS  CHAR(1) NOT NULL,
                             L_SHIPDATE    DATE NOT NULL,
                             L_COMMITDATE  DATE NOT NULL,
                             L_RECEIPTDATE DATE NOT NULL,
                             L_SHIPINSTRUCT CHAR(25) NOT NULL,
                             L_SHIPMODE     CHAR(10) NOT NULL,
                             L_COMMENT      VARCHAR(44) NOT NULL);
```

基于dss.ddl脚本创建表结构：
```
[root@bogon dbgen]# mysql -h 10.10.20.247 -P 3306 -uroot -pabc123
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 84
Server version: 5.5.64-MariaDB MariaDB Server

Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> \. ./dss.ddl
Query OK, 8 rows affected (0.01 sec)

Query OK, 1 row affected (0.00 sec)

Database changed
Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.01 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)
```

## 5.4 导入数据

```
mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/customer.tbl' INTO TABLE customer FIELDS TERMINATED BY '|';"

mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/orders.tbl'   INTO TABLE ORDERS   FIELDS TERMINATED BY '|';"

mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/lineitem.tbl' INTO TABLE LINEITEM FIELDS TERMINATED BY '|';"

mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/nation.tbl'   INTO TABLE NATION   FIELDS TERMINATED BY '|';"

mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/partsupp.tbl' INTO TABLE PARTSUPP FIELDS TERMINATED BY '|';"

mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/part.tbl'     INTO TABLE PART     FIELDS TERMINATED BY '|';"

mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/region.tbl'   INTO TABLE REGION   FIELDS TERMINATED BY '|';"

mysql -uroot -pabc123 tpch -e "LOAD DATA INFILE '/opt/tpch-2.18.0_rc2/dbgen/supplier.tbl' INTO TABLE SUPPLIER FIELDS TERMINATED BY '|';"
```

## 5.5 创建索引与外键约束

dss.ri是创建索引、外键脚本，由于对MYSQL无支持需要做多处的修改。

修改第4行：
```
CONNECT TO TPCD;
=>
USE tpch;
```

修改第6~13行，所有的SQL注释符 “--” 后面再加一个空格：
```
-- ALTER TABLE TPCD.REGION DROP PRIMARY KEY;
-- ALTER TABLE TPCD.NATION DROP PRIMARY KEY;
-- ALTER TABLE TPCD.PART DROP PRIMARY KEY;
-- ALTER TABLE TPCD.SUPPLIER DROP PRIMARY KEY;
-- ALTER TABLE TPCD.PARTSUPP DROP PRIMARY KEY;
-- ALTER TABLE TPCD.ORDERS DROP PRIMARY KEY;
-- ALTER TABLE TPCD.LINEITEM DROP PRIMARY KEY;
-- ALTER TABLE TPCD.CUSTOMER DROP PRIMARY KEY;
```

修改第25行：
```
ADD FOREIGN KEY NATION_FK1 (N_REGIONKEY) references TPCD.REGION;
=>
ADD FOREIGN KEY NATION_FK1 (N_REGIONKEY) references TPCD.REGION(R_REGIONKEY);
```

修改第40行：
```
ADD FOREIGN KEY SUPPLIER_FK1 (S_NATIONKEY) references TPCD.NATION;
=>
ADD FOREIGN KEY SUPPLIER_FK1 (S_NATIONKEY) references TPCD.NATION(N_NATIONKEY);
```

修改第55行：
```
ADD FOREIGN KEY CUSTOMER_FK1 (C_NATIONKEY) references TPCD.NATION;
=>
ADD FOREIGN KEY CUSTOMER_FK1 (C_NATIONKEY) references TPCD.NATION(N_NATIONKEY);
```

修改第73行：
```
ADD FOREIGN KEY PARTSUPP_FK1 (PS_SUPPKEY) references TPCD.SUPPLIER;
=>
ADD FOREIGN KEY PARTSUPP_FK1 (PS_SUPPKEY) references TPCD.SUPPLIER(S_SUPPKEY);
```

修改第78行：
```
ADD FOREIGN KEY PARTSUPP_FK2 (PS_PARTKEY) references TPCD.PART;
=>
ADD FOREIGN KEY PARTSUPP_FK2 (PS_PARTKEY) references TPCD.PART(P_PARTKEY);
```

修改第84行：
```
ADD FOREIGN KEY ORDERS_FK1 (O_CUSTKEY) references TPCD.CUSTOMER;
=>
ADD FOREIGN KEY ORDERS_FK1 (O_CUSTKEY) references TPCD.CUSTOMER(C_SUSTKEY);
```

修改第90行：
```
ADD FOREIGN KEY LINEITEM_FK1 (L_ORDERKEY)  references TPCD.ORDERS;
=>
ADD FOREIGN KEY LINEITEM_FK1 (L_ORDERKEY)  references TPCD.ORDERS(O_ORDERKEY);
```

修改第96行：
```
TPCD.PARTSUPP;
=>
TPCD.PARTSUPP(PS_PARTKEY,PS_SUPPKEY);
```

另外，由于tpch生成的表名是大写的，qgen生成的查询语句是小写的，所以需要修改下表名成小写的，因此再增加几行：
```
use tpch;

alter table CUSTOMER rename to customer;
alter table LINEITEM rename to lineitem;
alter table NATION rename to nation;
alter table ORDERS rename to orders;
alter table PART rename to part;
alter table PARTSUPP rename to partsupp;
alter table REGION rename to region;
alter table SUPPLIER rename to supplier;
```

最后，全局替换`TPCD => tpch`数据库名称。

修改后的dss.ri脚本内容如下：
```
[root@bogon dbgen]# cat dss.ri
-- Sccsid:     @(#)dss.ri       2.1.8.1
-- tpch Benchmark Version 8.0

USE tpch;

-- ALTER TABLE tpch.REGION DROP PRIMARY KEY;
-- ALTER TABLE tpch.NATION DROP PRIMARY KEY;
-- ALTER TABLE tpch.PART DROP PRIMARY KEY;
-- ALTER TABLE tpch.SUPPLIER DROP PRIMARY KEY;
-- ALTER TABLE tpch.PARTSUPP DROP PRIMARY KEY;
-- ALTER TABLE tpch.ORDERS DROP PRIMARY KEY;
-- ALTER TABLE tpch.LINEITEM DROP PRIMARY KEY;
-- ALTER TABLE tpch.CUSTOMER DROP PRIMARY KEY;


-- For table REGION
ALTER TABLE tpch.REGION
ADD PRIMARY KEY (R_REGIONKEY);

-- For table NATION
ALTER TABLE tpch.NATION
ADD PRIMARY KEY (N_NATIONKEY);

ALTER TABLE tpch.NATION
ADD FOREIGN KEY NATION_FK1 (N_REGIONKEY) references tpch.REGION(R_REGIONKEY);

COMMIT WORK;

-- For table PART
ALTER TABLE tpch.PART
ADD PRIMARY KEY (P_PARTKEY);

COMMIT WORK;

-- For table SUPPLIER
ALTER TABLE tpch.SUPPLIER
ADD PRIMARY KEY (S_SUPPKEY);

ALTER TABLE tpch.SUPPLIER
ADD FOREIGN KEY SUPPLIER_FK1 (S_NATIONKEY) references tpch.NATION(N_NATIONKEY);

COMMIT WORK;

-- For table PARTSUPP
ALTER TABLE tpch.PARTSUPP
ADD PRIMARY KEY (PS_PARTKEY,PS_SUPPKEY);

COMMIT WORK;

-- For table CUSTOMER
ALTER TABLE tpch.CUSTOMER
ADD PRIMARY KEY (C_CUSTKEY);

ALTER TABLE tpch.CUSTOMER
ADD FOREIGN KEY CUSTOMER_FK1 (C_NATIONKEY) references tpch.NATION(N_NATIONKEY);

COMMIT WORK;

-- For table LINEITEM
ALTER TABLE tpch.LINEITEM
ADD PRIMARY KEY (L_ORDERKEY,L_LINENUMBER);

COMMIT WORK;

-- For table ORDERS
ALTER TABLE tpch.ORDERS
ADD PRIMARY KEY (O_ORDERKEY);

COMMIT WORK;

-- For table PARTSUPP
ALTER TABLE tpch.PARTSUPP
ADD FOREIGN KEY PARTSUPP_FK1 (PS_SUPPKEY) references tpch.SUPPLIER(S_SUPPKEY);

COMMIT WORK;

ALTER TABLE tpch.PARTSUPP
ADD FOREIGN KEY PARTSUPP_FK2 (PS_PARTKEY) references tpch.PART(P_PARTKEY);

COMMIT WORK;

-- For table ORDERS
ALTER TABLE tpch.ORDERS
ADD FOREIGN KEY ORDERS_FK1 (O_CUSTKEY) references tpch.CUSTOMER(C_SUSTKEY);

COMMIT WORK;

-- For table LINEITEM
ALTER TABLE tpch.LINEITEM
ADD FOREIGN KEY LINEITEM_FK1 (L_ORDERKEY)  references tpch.ORDERS(O_ORDERKEY);

COMMIT WORK;

ALTER TABLE tpch.LINEITEM
ADD FOREIGN KEY LINEITEM_FK2 (L_PARTKEY,L_SUPPKEY) references
        tpch.PARTSUPP(PS_PARTKEY,PS_SUPPKEY);

COMMIT WORK;

use tpch;

alter table CUSTOMER rename to customer;
alter table LINEITEM rename to lineitem;
alter table NATION rename to nation;
alter table ORDERS rename to orders;
alter table PART rename to part;
alter table PARTSUPP rename to partsupp;
alter table REGION rename to region;
alter table SUPPLIER rename to supplier;
```

创建索引及外键约束：

```
[root@bogon dbgen]# mysql -u root -pabc123
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 103
Server version: 5.5.64-MariaDB MariaDB Server

Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> use tpch;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
MariaDB [tpch]> \. ./dds.ri
ERROR: Failed to open file './dds.ri', error: 2
MariaDB [tpch]> \. ./dss.ri
Database changed
Query OK, 0 rows affected (0.01 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.01 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 25 rows affected (0.00 sec)
Records: 25  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (24.24 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (1.04 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 100000 rows affected (1.42 sec)
Records: 100000  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (1 min 45.25 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (18.16 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 1500000 rows affected (22.41 sec)
Records: 1500000  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (14 min 0.15 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (2 min 46.47 sec)
Records: 0  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 8000000 rows affected (4 min 56.24 sec)
Records: 8000000  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 8000000 rows affected (4 min 40.76 sec)
Records: 8000000  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 59986052 rows affected (13 min 49.82 sec)
Records: 59986052  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Stage: 1 of 2 'copy to tmp table'   23.7% of stage done
Stage: 1 of 2 'copy to tmp table'   30.9% of stage done
Stage: 1 of 2 'copy to tmp table'   64.7% of stage done
Stage: 1 of 2 'copy to tmp table'   65.5% of stage done
Query OK, 59986052 rows affected (4 hours 36 min 16.39 sec)
Records: 59986052  Duplicates: 0  Warnings: 0

Query OK, 0 rows affected (0.00 sec)

Database changed
Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.01 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

Query OK, 0 rows affected (0.00 sec)

MariaDB [tpch]>
```

## 5.6 生成查询的SQL语句

由于源码包中自带的tpch初始化库表脚本并不能完全适用MySQL，需要修改部分代码。
```
[root@bogon dbgen]# DSS_QUERY=queries/ ./qgen | sed -e 's/\r//' > queries/tpch_queries.sql
```
而后用vim打开tpch_queries.sql脚本，进行下面几次全局替换：
```
:%s/;\nlimit/ limit/g
:%s/limit -1/limit 1/g
```

修改第18行附近：
```
l_shipdate <= date '1998-12-01' - interval '106' day (3)
=>
l_shipdate <= date '1998-12-01' - interval '106' day
```

修改第369行附近：
```
count(o_orderkey)
=>
count(o_orderkey) as c_count
```

修改第376行附近：
```
) as c_orders (c_custkey, c_count)
=>
) as c_orders
```
修改第431行附近：
```
drop view revenue0 limit 1;
=>
drop view revenue0;
```

> 22条SQL查询语句在tpch_queries.sql文件中。

## 5.7 进行TPC-H测试

以Q11查询语句为例：
```
[root@bogon dbgen]# mysql -u root -p
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 110
Server version: 5.5.64-MariaDB MariaDB Server

Copyright (c) 2000, 2018, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.


MariaDB [(none)]> use tpch;
Reading table information for completion of table and column names
You can turn off this feature to get a quicker startup with -A

Database changed
MariaDB [tpch]> select
  ps_partkey having
                sum(ps_supplycost * ps_availqty) > (
                        select
                                sum(ps_supplycost * ps_availqty) * 0.0001000000
                        from
                                partsupp,
     ->         ps_partkey,
    ->         sum(ps_supplycost * ps_availqty) as value
    -> from
    ->         partsupp,
    ->         supplier,
    ->         nation
    -> where
    ->         ps_suppkey = s_suppkey
    ->         and s_nationkey = n_nationkey
    ->         and n_name = 'ARGENTINA'
    -> group by
    ->         ps_partkey having
    ->                 sum(ps_supplycost * ps_availqty) > (
    ->                         select
    ->                                 sum(ps_supplycost * ps_availqty) * 0.0001000000
    ->                         from
    ->                                 partsupp,
    ->                                 supplier,
    ->                                 nation
    ->                         where
    ->                                 ps_suppkey = s_suppkey
    ->                                 and s_nationkey = n_nationkey
    ->                                 and n_name = 'ARGENTINA'
    ->                 )
    -> order by
    ->         value desc limit 1;
Empty set (36.72 sec)
```

> Q11查询耗时：36.72秒。