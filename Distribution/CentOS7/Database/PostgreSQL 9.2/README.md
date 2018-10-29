- 1 PostgreSQL
    - 1.1 安装启动
    - 1.2 添加防火墙策略
    - 1.3 更改管理员密码并添加测试用户
    - 1.4 登陆测试用户测试testdb库
- 2 安装phpPgAdmin
    - 2.1 安装配置
    - 2.2 变更SELinux策略
- 3 主从复制
    - 3.1 配置主节点
    - 3.2 添加防火墙策略
    - 3.3 配置从节点
    - 3.4 在主节点上创建数据库测试

# 1. PostgreSQL

## 1.1 安装启动

```Shell
[root@lab ~]# yum -y install postgresql-server

[root@lab ~]# postgresql-setup initdb 
Initializing database ... OK

[root@lab ~]# vi /var/lib/pgsql/data/postgresql.conf

# line 59: uncomment and change if allow accesses from remote hosts
listen_addresses = '*'

# line 395: uncomment and change if change log format
# the exmaple below is [Date User DB ***] format
log_line_prefix = '%t %u %d '

[root@lab ~]# systemctl start postgresql 
[root@lab ~]# systemctl enable postgresql 
```

## 1.2 添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-service=postgresql --permanent 
success
[root@lab ~]# firewall-cmd --reload 
success
```

## 1.3 更改管理员密码并添加测试用户

```Shell
# set password

[root@lab ~]# su - postgres 
-bash-4.2$ psql -c "alter user postgres with password 'password'" 

ALTER ROLE
# add DB user "jerrydai" as an example
-bash-4.2$ createuser jerrydai

# create a test database (owner is the user above)
-bash-4.2$ createdb testdb -O jerrydai
```

## 1.4 登陆测试用户测试testdb库

```Shell
# show Databases
[jerrydai@lab ~]$ psql -l 
                                  List of databases
   Name    |  Owner   | Encoding |   Collate   |    Ctype    |   Access privileg
es
-----------+----------+----------+-------------+-------------+------------------
-----
 postgres  | postgres | UTF8     | en_US.UTF-8 | en_US.UTF-8 |
 template0 | postgres | UTF8     | en_US.UTF-8 | en_US.UTF-8 | =c/postgres    +
           |          |          |             |             | postgres=CTc/postgres
 template1 | postgres | UTF8     | en_US.UTF-8 | en_US.UTF-8 | =c/postgres    +
           |          |          |             |             | postgres=CTc/postgres
 testdb    | jerrydai | UTF8     | en_US.UTF-8 | en_US.UTF-8 |
(4 rows)

# connect to test DB
[jerrydai@lab ~]$ psql testdb 
psql (9.2.13)
Type "help" for help.
# set password
testdb=# alter user jerrydai with password 'password'; 

ALTER ROLE
# create a test table
testdb=# create table test ( no int,name text ); 
CREATE TABLE

# insert test data
testdb=# insert into test (no,name) values (1,'jerrydai'); 
INSERT 0 1

# show tables
testdb=# select * from test;
 no | name
----+---------
  1 | jerrydai
(1 row)

# delete test table
testdb=# drop table test; 
DROP TABLE

# quit
testdb=# \q

# delete test database
[jerrydai@lab ~]$ dropdb testdb
```

# 2. 安装phpPgAdmin

## 2.1 安装配置

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install phpPgAdmin php-pgsql

[root@lab ~]# vi /etc/phpPgAdmin/config.inc.php

# line 18: add
$conf['servers'][0]['host'] = 'localhost';

# line 93: change to false if you allow to login with priviledged user like postgres, root
$conf['extra_login_security'] = false;

# line 99: change
$conf['owned_only'] = true;

[root@lab ~]# vi /var/lib/pgsql/data/pg_hba.conf

# line 82: change like follows and add access permission
host    all         all         127.0.0.1/32          md5
host    all         all         10.0.0.0/24           md5
host    all         all         ::1/128               md5

[root@lab ~]# vi /etc/httpd/conf.d/phpPgAdmin.conf

# line 11: add access permission
Require local
Require ip 10.0.0.0/24

[root@lab ~]# systemctl restart postgresql httpd 
```

## 2.2 变更SELinux策略

```Shell
[root@lab ~]# setsebool -P httpd_can_network_connect_db on 
```

# 3. 主从复制

## 3.1 配置主节点

```Shell
[root@lab ~]# yum -y install postgresql-server

[root@lab ~]# postgresql-setup initdb 
Initializing database ... OK

[root@lab ~]# vi /var/lib/pgsql/data/postgresql.conf

# line 59: uncomment and change
listen_addresses = '*'

# line 165: uncomment and change
wal_level = hot_standby

# line 168: uncomment and change
# on ⇒ sync
# remote_write ⇒ memory sync
# local ⇒ slave is asynchronous
# off ⇒ asynchronous
synchronous_commit = local

# line 194: uncomment and change (enable archive_mode)
archive_mode = on

# line 196: uncomment and change (command to get archives)
archive_command = 'cp %p /var/lib/pgsql/archive/%f'

# line 212: uncomment and change (slave servers + 1)
max_wal_senders = 2

# line 214: uncomment and change
wal_keep_segments = 10

# line 221: uncomment and change (any name you like)
synchronous_standby_names = 'slave01'

[root@lab ~]# vi /var/lib/pgsql/data/pg_hba.conf
# add to the end
# host replication [replication user] [allowed IP addresses] password
host    replication     replica          127.0.0.1/32            md5
host    replication     replica          10.0.0.30/32            md5
host    replication     replica          10.0.0.51/32            md5

[root@lab ~]# systemctl start postgresql 
[root@lab ~]# systemctl enable postgresql 

# create a user for replication
[root@lab ~]# su - postgres
-bash-4.2$ createuser --replication -P replica 
Enter password for new role:
Enter it again:
```

## 3.2 添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-service=postgresql --permanent 
success
[root@lab ~]# firewall-cmd --reload 
success
```

## 3.3 配置从节点

```Shell
[root@node01 ~]# yum -y install postgresql-server

[root@node01 ~]# su - postgres
# get backup from Master Server
-bash-4.2$ pg_basebackup -h 10.0.0.30 -U replica -D /var/lib/pgsql/data -P --xlog 
Password:     # "replica" user's password

-bash-4.2$ vi /var/lib/pgsql/data/postgresql.conf

# line 230: uncomment and change
hot_standby = on
-bash-4.2$ cp /usr/share/pgsql/recovery.conf.sample /var/lib/pgsql/data/recovery.conf 
-bash-4.2$ vi /var/lib/pgsql/data/recovery.conf

# line 44: uncomment and change (command to get archives)
restore_command = 'scp 10.0.0.30:/var/lib/pgsql/archive/%f %p'

# line 108: uncomment and change
standby_mode = on

# line 115: uncomment and change (connection info to Master Server)
primary_conninfo = 'host=10.0.0.30 port=5432 user=replica password=password application_name=slave01'

-bash-4.2$ exit 
logout
[root@node01 ~]# systemctl start postgresql 
[root@node01 ~]# systemctl enable postgresql 
```

## 3.4 在主节点上创建数据库测试

```Shell
-bash-4.2$ psql -c "select application_name, state, sync_priority, sync_state from pg_stat_replication;" 
 application_name |   state   | sync_priority | sync_state
------------------+-----------+---------------+------------
 slave01          | streaming |             1 | sync
(1 row)
```