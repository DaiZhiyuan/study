- 1 Redis
    - 1.1 安装
    - 1.2 配置
    - 1.3 添加防火墙策略
- 2 命令行接口redis-cli
    - 2.1 数据库操作
    - 2.2 字典
    - 2.3 列表
    - 2.4 散列表
    - 2.5 集合
- 3 API接口
    - 3.1 Python
    - 3.2 PHP
    - 3.3 Node.js
- 4 主从复制
    - 4.1 在主节点上更改配置
    - 4.2 配置从节点
    - 4.3 在主/从节点添加防火墙策略
    - 4.4 在从节点上重新编译Selinux策略
    - 4.5 在从节点上确认master_link_status状态
- 5 高可用
    - 5.1 在所有节点上添加Selinux策略
    - 5.2 配置哨兵服务
    - 5.3 测试故障转移
- 6 基准测试

# 1. Redis

Redis是一个开源的使用ANSI C语言编写、支持网络、可基于内存亦可持久化的日志型、Key-Value数据库，并提供多种语言的API。从2010年3月15日起，Redis的开发工作由VMware主持。从2013年5月开始，Redis的开发由Pivotal赞助。

## 1.1 安装

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install redis
```

## 1.2 配置

```Shell
[root@lab ~]# vi /etc/redis.conf

# line 45: listening port
port 6379

# line 64: listening interface
# localhost only by default
# if you'd like to connect from other Hosts,
# change to the own IP address or to 0.0.0.0
bind 127.0.0.1

# line 118: number of Databases
# database ID is is assgined from 0 to (setting value - 1)
databases 16

# line 142: save caching Datase on Disk
# the default settings below means like follows
# after 900 sec if at least 1 key changed
# after 300 sec if at least 10 keys changed
# after 60 sec if at least 10000 keys changed
# if you'd like to disable this function, comment out all lines fo "save ***" or specify [save ""]
save 900 1
save 300 10
save 60 10000

# line 392: authorization password
requirepass password

# line 501: alternative persistence mode ("yes" means enabled)
# if enabled, Redis loses high performance but get more safety
appendonly no

# line 531: if enabled "appendonly yes" when wirting data on Disk
# no means do not fsync by Redis (just let the OS flush the data)
# appendfsync always
appendfsync everysec
# appendfsync no

[root@lab ~]# systemctl start redis 
[root@lab ~]# systemctl enable redis 
```

## 1.3 添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-port=6379/tcp --permanent 
success
[root@lab ~]# firewall-cmd --reload 
success
```

# 2. 命令行接口redis-cli

## 2.1 数据库操作

**1. 连接操作**
```Shell
# connect to local server
# password is the one you set on redis.conf
[root@lab ~]# redis-cli -a password
# exit from connection
127.0.0.1:6379> quit 

# possible to authenticate after connecting to Server
[root@lab ~]# redis-cli 
127.0.0.1:6379> auth password 
OK

# connect to a Database with specifying Database-ID explicitly
# if Database-ID is not specified, connect to Database-ID "0"
[root@lab ~]# redis-cli -a password -n 1 
127.0.0.1:6379[1]>

# change to Database-ID "2"
127.0.0.1:6379[1]> select 2 
OK
127.0.0.1:6379[2]>

# for connecting to Redis on another Host, specify "-h [hostname]"
[root@lab ~]# redis-cli -h node01.centos.org -a password 
10.0.0.51:6379>
```
**2. 控制操作**

```Shell
[root@lab ~]# redis-cli -a password
# refer to statics
127.0.0.1:6379> info 
# Server
redis_version:2.8.19
redis_git_sha1:00000000
redis_git_dirty:0
redis_build_id:c0359e7aa3798aa2
redis_mode:standalone
os:Linux 3.10.0-327.22.2.el7.x86_64 x86_64
arch_bits:64
.....
.....

# show connected clients
127.0.0.1:6379> client list 
id=3 addr=127.0.0.1:44474 fd=5 name= age=447 idle=0 flags=N db=0 sub=0 psub=0 
    multi=-1 qbuf=0 qbuf-free=32768 obl=0 oll=0 omem=0 events=r cmd=client
id=4 addr=10.0.0.31:43668 fd=6 name= age=10 idle=10 flags=N db=0 sub=0 psub=0 
    multi=-1 qbuf=0 qbuf-free=0 obl=0 oll=0 omem=0 events=r cmd=auth

# kill connection of a client
127.0.0.1:6379> client kill 10.0.0.31:43668 
OK

# dump all requests after the command below
127.0.0.1:6379> monitor 
OK
.....
.....

# save data on disk with foreground
127.0.0.1:6379> save 
OK

# save data on disk with background
127.0.0.1:6379> bgsave 
Background saving started

# get UNIX time stamp of the last save to disk
127.0.0.1:6379> lastsave 
(integer) 1469078407

# save data on disk and shutdown Redis
127.0.0.1:6379> shutdown 
not connected> quit 

[root@lab ~]# ps aux | grep [r]edis
```

## 2.2 字典

**1. 基本操作**

```Shell
[root@lab ~]# redis-cli -a password
# set Value of a Key
127.0.0.1:6379> set key01 value01 
OK

# get Value of a Key
127.0.0.1:6379> get key01 
"value01"

# delete a Key
127.0.0.1:6379> del key01 
(integer) 1

# determine if a Key exists or not (1 means true)
127.0.0.1:6379> exists key01 
(integer) 1

# set Value of a Key only when the Key does not exist yet
# integer 0 means Value is not set because the Key already exists
127.0.0.1:6379> setnx key01 value02 
(integer) 0

# set Value of a Key with expiration date (60 means Value will expire after 60 second)
127.0.0.1:6379> setex key01 60 value01 
OK

# set expiration date to existing Key
127.0.0.1:6379> expire key02 30 
(integer) 1

# add Values to a Key
127.0.0.1:6379> append key01 value02 
(integer) 15

# get substring of Value of a Key : [Key] [Start] [End]
127.0.0.1:6379> substr key01 0 3 
"valu"

127.0.0.1:6379> set key02 1 
OK

# increment integer Value of a Key
127.0.0.1:6379> incr key02 
(integer) 2

# increment integer Value of a Key by specified value
127.0.0.1:6379> incrby key02 100 
(integer) 102

# decrement integer Value of a Key
127.0.0.1:6379> decr key02 
(integer) 101

# decrement integer Value of a Key by specified value
127.0.0.1:6379> decrby key02 51 
(integer) 50

# set values of some Keys
127.0.0.1:6379> mset key01 value01 key02 value02 key03 value03 
OK

# get some Values of Keys
127.0.0.1:6379> mget key01 key02 key03 
1) "value01"
2) "value02"
3) "value03"

# rename existing Key
127.0.0.1:6379> rename key01 key02 
OK
127.0.0.1:6379> mget key01 key02 
1) (nil)
2) "value01"

# rename existing Key but if renamed Key already exists, command is not run
127.0.0.1:6379> renamenx key02 key03 
(integer) 0
127.0.0.1:6379> mget key02 key03 
1) "value01"
2) "value03"

# get number of Keys on current Database
127.0.0.1:6379> dbsize 
(integer) 4

# move a key to another Database
127.0.0.1:6379> move key03 1 
(integer) 1
127.0.0.1:6379> select 1 
OK
127.0.0.1:6379[1]> get key03 
"value03"

# delete all Keys on current Database
127.0.0.1:6379> flushdb 
OK

# delete all Keys on all Database
127.0.0.1:6379> flushall 
OK
127.0.0.1:6379> quit 

# process with reading data from stdout
[root@lab ~]# echo 'test_words' | redis-cli -a password -x set key01 
OK
[root@lab ~]# redis-cli -a password get key01 
"test_words\n"
```

**2. CAS操作**

redis的高级事务CAS(乐观锁)

乐观锁介绍：
watch指令在redis事物中提供了CAS的行为。为了检测被watch的keys在是否有多个clients同时改变引起冲突，这些keys将会被监控。如果至少有一个被监控的key在执行exec命令前被修改，整个事物将会回滚，不执行任何动作，从而保证原子性操作，并且执行exec会得到null的回复。

乐观锁工作机制：
watch 命令会监视给定的每一个key，当exec时如果监视的任一个key自从调用watch后发生过变化，则整个事务会回滚，不执行任何动作。注意watch的key是对整个连接有效的，事务也一样。如果连接断开，监视和事务都会被自动清除。当然exec，discard，unwatch命令，及客户端连接关闭都会清除连接中的所有监视。还有，如果watch一个不稳定(有生命周期)的key并且此key自然过期，exec仍然会执行事务队列的指令。

```Shell
# watch a key
127.0.0.1:6379> watch key01 
OK

127.0.0.1:6379> get key01 
"value01"
127.0.0.1:6379> multi 
OK
127.0.0.1:6379> set key01 value02 
QUEUED

127.0.0.1:6379> exec 
1) OK
```

## 2.3 列表

```
[root@lab ~]# redis-cli -a password
# preppend Value to a List (possible to set multiple values with space)
127.0.0.1:6379> lpush list01 value01 
(integer) 1

# append Value to a List (possible to set multiple values with space)
127.0.0.1:6379> rpush list01 value02 
(integer) 2

# get length of a List
127.0.0.1:6379> llen list01 
(integer) 2

# get specified element of a List
127.0.0.1:6379> lindex list01 0 
"value01"

# get specified range of elements
127.0.0.1:6379> lrange list01 0 1 
1) "value01"
2) "value02"

# change specified element to specified value
127.0.0.1:6379> lset list01 1 value03
OK
127.0.0.1:6379> lindex list01 1 
"value03"

# get the head element and remove it
127.0.0.1:6379> lpop list01 
"value01"

# get the last element and remove it
127.0.0.1:6379> rpop list01 
"value03"

# trim specified range of a List
127.0.0.1:6379> ltrim list01 1 3 
OK

127.0.0.1:6379> lrange list02 0 7 
1) "value01"
2) "test"
3) "value02"
4) "value03"
5) "value04"
6) "test"
7) "value05"
8) "test"

# remove specified number of elements of a List
127.0.0.1:6379> lrem list02 2 test 
(integer) 2
127.0.0.1:6379> lrange list02 0 7 
1) "value01"
2) "value02"
3) "value03"
4) "value04"
5) "value05"
6) "test"
```

## 2.4 散列表

```Shell
[root@lab ~]# redis-cli -a password
# set Value of a Hash field
127.0.0.1:6379> hset hash01 field01 value01 
(integer) 1

# get Value of a Hash field
127.0.0.1:6379> hget hash01 field01 
"value01"

# set some Values of some Hash fields
127.0.0.1:6379> hmset hash01 field02 value02 field03 value03 
OK

# get some Values of some Hash fields
127.0.0.1:6379> hmget hash01 field01 field02 field03 
1) "value01"
2) "value02"
3) "value03"

# get all fields of a Hash table
127.0.0.1:6379> hkeys hash01 
1) "field01"
2) "field02"
3) "field03"

# get all values of fields of a Hash table
127.0.0.1:6379> hvals hash01 
1) "value01"
2) "value02"
3) "value03"

# get all values and fields of a Hash table
127.0.0.1:6379> hgetall hash01 
1) "field01"
2) "value01"
3) "field02"
4) "value02"
5) "field03"
6) "value03"

# increment integer Value of a Hash field
127.0.0.1:6379> hincrby hash01 field04 100 
(integer) 101

# determine if a hash field exists or not
127.0.0.1:6379> hexists hash01 field01 
(integer) 1

# get number of fields of a Hash
127.0.0.1:6379> hlen hash01 
(integer) 4

# remove specified field of a Hash
127.0.0.1:6379> hdel hash01 field04 
(integer) 1
```

## 2.5 集合

```Shell
[root@lab ~]# redis-cli -a password
# add member to a Set (possible to multiple members with space)
127.0.0.1:6379> sadd set01 member01 
(integer) 1

# get number of members of a Set
127.0.0.1:6379> scard set01 
(integer) 1

# remove specified member of a Set
127.0.0.1:6379> srem set01 member03 
(integer) 1

# determine if specified member exists or not
127.0.0.1:6379> sismember set01 member01 
(integer) 1

# get all members of a Set
127.0.0.1:6379> smembers set01 
1) "member03"
2) "member02"
3) "member01"
127.0.0.1:6379> smembers set02 
1) "member02"
2) "member05"
3) "member04"
127.0.0.1:6379> smembers set03 
1) "member06"
2) "member02"
3) "member01"

# get members of the set resulting from the intersection of all the given sets
127.0.0.1:6379> sinter set01 set02 set03 
1) "member02"

# equal to SINTER above, but instead of showing the result, it is stored to destination Set(first argument)
127.0.0.1:6379> sinterstore set04 set01 set02 set03 
(integer) 1
127.0.0.1:6379> smembers set04 
1) "member02"

# get members of the set resulting from the difference between the first set and all the successive sets
127.0.0.1:6379> sdiff set01 set02 set03 
1) "member03"

# equal to SDIFF above, but instead of showing the result, it is stored to destination Set(first argument)
127.0.0.1:6379> sdiffstore set05 set01 set02 set03 
(integer) 1

127.0.0.1:6379> smembers set05 
1) "member03"

# get members of the set resulting from the union of all the given sets
127.0.0.1:6379> sunion set01 set02 set03 
(integer) 1

# equal to SUNION above, but instead of showing the result, it is stored to destination Set(first argument)
127.0.0.1:6379> sunionstore set06 set01 set02 set03 
(integer) 6
127.0.0.1:6379> smembers set06 
1) "member06"
2) "member03"
3) "member04"
4) "member02"
5) "member01"
6) "member05"

# move member from a Set(first arg) to a Set(2nd arg)
127.0.0.1:6379> smove set01 set02 member03 
(integer) 1
Matched Co
```

# 3. API接口

## 3.1 Python

**1. 安装**

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install python-redis
```

**2. 例子**

```Python
[JerryDai@lab ~]$ vi use_redis.py
 #!/usr/bin/env python

import redis

client = redis.StrictRedis(host='127.0.0.1', port=6379, db=0, password='password')

# set and get Key
client.set("key01", "value01")
print "key01.value :", client.get("key01")

# append and get Key
client.append("key01", ",value02")
print "key01.value :", client.get("key01")

client.set("key02", 1)

# increment
client.incr("key02", 100)
print "key02.value :", client.get("key02")

# decrement
client.decr("key02", 51)
print "key02.value :", client.get("key02")

# list
client.lpush("list01", "value01", "value02", "value03")
print "list01.value :", client.lrange("list01", "0", "2")

# hash
client.hmset("hash01", {"key01": "value01", "key02": "value02", "key03": "value03"})
print "hash01.value :", client.hmget("hash01", ["key01", "key02", "key03"])

# set
client.sadd("set01", "member01", "member02", "member03")
print "set01.value :", client.smembers("set01")

# run
[JerryDai@lab ~]$ python use_redis.py 
key01.value : value01
key01.value : value01,value02
key02.value : 101
key02.value : 50
list01.value : ['value03', 'value02', 'value01']
hash01.value : ['value01', 'value02', 'value03']
set01.value : set(['member02', 'member03', 'member01'])
```

## 3.2 PHP

**1. 安装**

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install php-pecl-redis
```

**2. 例子**

```php
[JerryDai@lab ~]$ vi use_redis.php
 <?php
$redis = new Redis();
$redis->connect("127.0.0.1",6379);
$redis->auth("password");

// set and get Key
$redis->set('key01', 'value01');
print 'key01.value : ' . $redis->get('key01') . "\n";

// append and get Key
$redis->append('key01', ',value02');
print 'key01.value : ' . $redis->get('key01') . "\n";

$redis->set('key02', 1);
print 'key02.value : ' . $redis->get('key02') . "\n";

// increment
$redis->incr('key02', 100);
print 'key02.value : ' . $redis->get('key02') . "\n";

// decrement
$redis->decr('key02', 51);
print 'key02.value : ' . $redis->get('key02') . "\n";

// list
$redis->lPush('list01', 'value01');
$redis->rPush('list01', 'value02');
print 'list01.value : ';
print_r ($redis->lRange('list01', 0, -1));

// hash
$redis->hSet('hash01', 'key01', 'value01');
$redis->hSet('hash01', 'key02', 'value02');
print 'hash01.value : ';
print_r ($redis->hGetAll('hash01'));

// set
$redis->sAdd('set01', 'member01');
$redis->sAdd('set01', 'member02');
print 'set01.value : ';
print_r ($redis->sMembers('set01'));
?>

# run
[JerryDai@lab ~]$ php use_redis.php 
key01.value : value01
key01.value : value01,value02
key02.value : 1
key02.value : 101
key02.value : 50
list01.value : Array
(
    [0] => value01
    [1] => value02
)
hash01.value : Array
(
    [key01] => value01
    [key02] => value02
)
set01.value : Array
(
    [0] => member01
    [1] => member02
)
```

## 3.3 Node.js

**1. 安装**

```Shell
[JerryDai@lab ~]$ npm install redis 
```

**2. 例子**

```JavaScript
[JerryDai@lab ~]$ vi use_redis.js
 var redis = require('redis');
var client = new redis.createClient();

client.auth('password');

// set and get Key
client.set('key01', 'value01');
client.get('key01', function (err, val) {
    console.log("key01.value :", val);
});

// append and get Key
client.append('key01', ',value02');
client.get('key01', function (err, val) {
    console.log("key01.value :", val);
});

client.set('key02', 1);
client.get('key02', function (err, val) {
    console.log("key02.value :", val);
});

// increment
client.incrby('key02', 100);
client.get('key02', function (err, val) {
    console.log("key02.value :", val);
});

// decrement
client.decrby('key02', 51);
client.get('key02', function (err, val) {
    console.log("key02.value :", val);
});

// list
client.rpush('list01', 'value01');
client.rpush('list01', 'value02');
client.lrange('list01', 0, -1, function (err, val) {
    console.log("list01.value :", val);
});

// hash
client.hset("hash01", "key01", "value01");
client.hset("hash01", "key02", "value02");
client.hgetall('hash01', function (err, val) {
    console.log("hash01.value :", val);
});

// set
client.sadd("set01", "member01");
client.sadd("set01", "member02");
client.smembers('set01', function (err, val) {
    console.log("set01.value :", val);
});

# run
[JerryDai@lab ~]$ node use_redis.js 
key01.value : value01
key01.value : value01,value02
key02.value : 1
key02.value : 101
key02.value : 50
list01.value : [ 'value01', 'value02' ]
hash01.value : { key01: 'value01', key02: 'value02' }
set01.value : [ 'member01', 'member02' ]
```

# 4. 主从复制

## 4.1 在主节点上更改配置

```Shell
[root@lab ~]# vi /etc/redis.conf

# line 64: change to own IP or 0.0.0.0
bind 0.0.0.0

# line 377: add follows if you need
# min-slaves-to-write : if number of slave Hosts are online, Master Host accepts write requests
# min-slaves-max-lag : decision time(sec) for online if Slave Hosts return answer within specified time
min-slaves-to-write 2
min-slaves-max-lag 10

[root@lab ~]# systemctl restart redis
```

## 4.2 配置从节点

```Shell
[root@node01 ~]# vi /etc/redis.conf

# line 64: change to own IP or 0.0.0.0
bind 0.0.0.0

# line 206: add Master server's IP and port
slaveof 10.0.0.30 6379

# line 213: add connection password set on Master Host
masterauth password

# line 241: verify parameter (set Slave Hosts read-only)
slave-read-only yes

[root@node01 ~]# systemctl restart redis
```

## 4.3 在主/从节点添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-port=6379/tcp --permanent 
success
[root@lab ~]# firewall-cmd --reload 
success
```

## 4.4 在从节点上重新编译Selinux策略

```Shell

[root@node01 ~]# vi redis_repl.te
# create new
module redis_repl 1.0;

require {
        type redis_port_t;
        type redis_t;
        class tcp_socket name_connect;
}

#============= redis_t ==============
allow redis_t redis_port_t:tcp_socket name_connect;

[root@node01 ~]# checkmodule -m -M -o redis_repl.mod redis_repl.te 
checkmodule: loading policy configuration from redis_repl.te
checkmodule: policy configuration loaded
checkmodule: writing binary representation (version 17) to redis_repl.mod

[root@node01 ~]# semodule_package --outfile redis_repl.pp --module redis_repl.mod 
[root@node01 ~]# semodule -i redis_repl.pp 
```

## 4.5 在从节点上确认master_link_status状态

```Shell
[root@node01 ~]# redis-cli info Replication 
# Replication
role:slave
master_host:10.0.0.30
master_port:6379
master_link_status:up
master_last_io_seconds_ago:1
master_sync_in_progress:0
slave_repl_offset:384
slave_priority:100
slave_read_only:1
connected_slaves:0
master_repl_offset:0
repl_backlog_active:0
repl_backlog_size:1048576
repl_backlog_first_byte_offset:0
repl_backlog_histlen:0

# try to get a Key normally
[root@node01 ~]# redis-cli get key_on_master 
"value_on_master"
```

# 5. 高可用

网络拓扑：

```
                                  |
+----------------------+          |          +----------------------+
|  [ Redis Sentinel ]  |10.0.0.40 | 10.0.0.30|   [ Redis Master ]   |
|    mon.centos.org    +----------+----------+    lab.centos.org    |
|                      |          |          |                      |
+----------------------+          |          +----------------------+
                                  |
+----------------------+          |          +----------------------+
|  [ Redis Slave#1 ]   |10.0.0.51 | 10.0.0.52|   [ Redis Slave#2 ]  |
|   node01.centos.org  +----------+----------+  node02.centos.org   |
|                      |                     |                      |
+----------------------+                     +----------------------+
```

## 5.1 在所有节点上添加Selinux策略

```Shell
[root@lab ~]# vi redis_ha.te

# create new
module redis_ha 1.0;

require {
        type etc_t;
        type redis_t;
        class file write;
}

#============= redis_t ==============
allow redis_t etc_t:file write;

[root@lab ~]# checkmodule -m -M -o redis_ha.mod redis_ha.te 
checkmodule: loading policy configuration from redis_ha.te
checkmodule: policy configuration loaded
checkmodule: writing binary representation (version 17) to redis_ha.mod

[root@lab ~]# semodule_package --outfile redis_ha.pp --module redis_ha.mod 
[root@lab ~]# semodule -i redis_ha.pp 
```

## 5.2 配置哨兵服务

**1. 安装配置**

```Shell
# install from EPEL
[root@mon ~]# yum --enablerepo=epel -y install redis

[root@mon ~]# vi /etc/redis-sentinel.conf

# line 53: change [sentinel monitor (any name) (Master's IP) (Master's Port) (Quorum)]
# Quorum ⇒ run failover when the specified number of Sentinel servers look Master is down
sentinel monitor mymaster 10.0.0.30 6379 1

# line 73: authentication password for Master
sentinel auth-pass mymaster password

# line 82: the term Sentinel server looks Master is down (30 sec by default below)
sentinel down-after-milliseconds mymaster 30000

# line 90 number of Slaves to be changed when running failover
sentinel parallel-syncs mymaster 1

[root@mon ~]# systemctl start redis-sentinel 
[root@mon ~]# systemctl enable redis-sentinel 
```

**2. 添加Selinux策略**

```
[root@mon ~]# semanage port -a -t redis_port_t -p tcp 26379 
[root@mon ~]# vi redis_sentinel.te
# create new
module redis_sentinel 1.0;

require {
        type redis_port_t;
        type etc_t;
        type redis_t;
        class tcp_socket name_connect;
        class file write;
}

#============= redis_t ==============
allow redis_t redis_port_t:tcp_socket name_connect;
allow redis_t etc_t:file write;

[root@mon ~]# checkmodule -m -M -o redis_sentinel.mod redis_sentinel.te 
checkmodule: loading policy configuration from redis_sentinel.te
checkmodule: policy configuration loaded
checkmodule: writing binary representation (version 17) to redis_sentinel.mod
[root@mon ~]# semodule_package --outfile redis_sentinel.pp --module redis_sentinel.mod 
[root@mon ~]# semodule -i redis_sentinel.pp 
```

## 5.3 测试故障转移

```Shell
[root@mon ~]# redis-cli -p 26379 
# show Master Node for "mymaster"
127.0.0.1:26379> sentinel get-master-addr-by-name mymaster 
1) "10.0.0.30"
2) "6379"

# show details of Master Node for "mymaster"
127.0.0.1:26379> sentinel master mymaster 
 1) "name"
 2) "mymaster"
 3) "ip"
 4) "10.0.0.30"
 5) "port"
 6) "6379"
.....
.....

# show Slave Nodes for "mymaster"
127.0.0.1:26379> sentinel slaves mymaster 
1)  1) "name"
    2) "10.0.0.52:6379"
    3) "ip"
    4) "10.0.0.52"
    5) "port"
    6) "6379"
.....
.....

2)  1) "name"
    2) "10.0.0.51:6379"
    3) "ip"
    4) "10.0.0.51"
    5) "port"
    6) "6379"
.....
.....
```

# 6. 基准测试

```Shell
[root@mon ~]# redis-benchmark -h 10.0.0.30 -p 6379 
====== PING_INLINE ======
  100000 requests completed in 1.26 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

98.30% <= 1 milliseconds
99.90% <= 2 milliseconds
99.94% <= 3 milliseconds
99.95% <= 4 milliseconds
100.00% <= 4 milliseconds
79491.26 requests per second

====== PING_BULK ======
  100000 requests completed in 1.28 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

99.99% <= 1 milliseconds
100.00% <= 1 milliseconds
77942.32 requests per second

====== SET ======
  100000 requests completed in 1.29 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 0 milliseconds
77579.52 requests per second

====== GET ======
  100000 requests completed in 1.28 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 0 milliseconds
78186.08 requests per second

====== INCR ======
  100000 requests completed in 1.29 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 1 milliseconds
77519.38 requests per second

====== LPUSH ======
  100000 requests completed in 1.27 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 0 milliseconds
78678.20 requests per second

====== LPOP ======
  100000 requests completed in 1.27 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 0 milliseconds
78492.93 requests per second

====== SADD ======
  100000 requests completed in 1.28 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 0 milliseconds
78064.01 requests per second

====== SPOP ======
  100000 requests completed in 1.28 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 0 milliseconds
78003.12 requests per second

====== LPUSH (needed to benchmark LRANGE) ======
  100000 requests completed in 1.32 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

99.67% <= 1 milliseconds
99.97% <= 2 milliseconds
100.00% <= 2 milliseconds
75987.84 requests per second

====== LRANGE_100 (first 100 elements) ======
  100000 requests completed in 1.32 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

100.00% <= 1 milliseconds
75930.14 requests per second

====== LRANGE_300 (first 300 elements) ======
  100000 requests completed in 1.32 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

99.96% <= 1 milliseconds
100.00% <= 1 milliseconds
75987.84 requests per second

====== LRANGE_500 (first 450 elements) ======
  100000 requests completed in 1.26 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

99.97% <= 1 milliseconds
100.00% <= 1 milliseconds
79113.92 requests per second

====== LRANGE_600 (first 600 elements) ======
  100000 requests completed in 1.22 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

99.97% <= 2 milliseconds
100.00% <= 2 milliseconds
81900.09 requests per second

====== MSET (10 keys) ======
  100000 requests completed in 1.23 seconds
  50 parallel clients
  3 bytes payload
  keep alive: 1

99.80% <= 1 milliseconds
99.95% <= 2 milliseconds
100.00% <= 2 milliseconds
81433.22 requests per second
```