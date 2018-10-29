- 1 MariaDB
    - 1.1 安装
    - 1.2 加载环境变量
    - 1.3 初始化数据库
    - 1.4 添加防火墙策略
- 2 安装phpMyAdmin
- 3 配置SSL/TLS
    - 3.1 创建数字证书
    - 3.2 复制证书文件并配置Maria
    - 3.3 客户端连接时指定证书
    - 3.4 强制要求用户使用SSL/TLS连接
    - 3.5 复制证书给其他客户端用于连接
- 4 主从复制
    - 4.1 在主节点上创建主从复制用户
    - 4.2 在从节点上配置
    - 4.3 在主节点上dump数据
    - 4.4 在从节点上导入dump数据并更改配置
- 5 高可用
    - 5.1 在所有节点上安装软件包
    - 5.2 配置第一个节点
    - 5.3 配置其他节点
    - 5.4 测试wsrep_local_state_comment状态是否为Synced

# 1. MariaDB 

## 1.1 安装

```Shell
# install from SCLo
[root@www ~]# yum --enablerepo=centos-sclo-rh -y install rh-mariadb102-mariadb-server
```

## 1.2 加载环境变量

Software Collections仓库中的软件包的安装路径为/opt目录，如果直接使用需要手动请加载环境变量。

```Shell
# load environment variables
[root@www ~]# scl enable rh-mariadb102 bash

[root@www ~]# mysql -V 
mysql Ver 15.1 Distrib 10.2.8-MariaDB, for Linux (x86_64) using EditLine wrapper

[root@www ~]# which mysql 
/opt/rh/rh-mariadb102/root/usr/bin/mysql
```

如果想在启动时自动加载环境变量：

```Shell
[root@www ~]# vi /etc/profile.d/rh-mariadb102.sh
# create new
 #!/bin/bash

source /opt/rh/rh-mariadb102/enable
export X_SCLS="`scl enable rh-mariadb102 'echo $X_SCLS'`"
```

## 1.3 初始化数据库

```Shell
[root@www ~]# vi /etc/opt/rh/rh-mariadb102/my.cnf.d/mariadb-server.cnf

# add follows into [mysqld] section
[mysqld]
character-set-server=utf8
[root@www ~]# systemctl start rh-mariadb102-mariadb 
[root@www ~]# systemctl enable rh-mariadb102-mariadb
[root@www ~]# mysql_secure_installation 

NOTE: RUNNING ALL PARTS OF THIS SCRIPT IS RECOMMENDED FOR ALL MariaDB
      SERVERS IN PRODUCTION USE!  PLEASE READ EACH STEP CAREFULLY!

In order to log into MariaDB to secure it, we'll need the current
password for the root user.  If you've just installed MariaDB, and
you haven't set the root password yet, the password will be blank,
so you should just press enter here.

# Enter
Enter current password for root (enter for none):
OK, successfully used password, moving on...

Setting the root password ensures that nobody can log into the MariaDB
root user without the proper authorisation.

# set root password
Set root password? [Y/n] y
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

# remove anonymous users
Remove anonymous users? [Y/n] y
 ... Success!

Normally, root should only be allowed to connect from 'localhost'.  This
ensures that someone cannot guess at the root password from the network.

# disallow root login remotely
Disallow root login remotely? [Y/n] y
 ... Success!

By default, MariaDB comes with a database named 'test' that anyone can
access.  This is also intended only for testing, and should be removed
before moving into a production environment.

# remove test database
Remove test database and access to it? [Y/n] y
 - Dropping test database...
 ... Success!
 - Removing privileges on test database...
 ... Success!

Reloading the privilege tables will ensure that all changes made so far
will take effect immediately.

# reload privilege tables
Reload privilege tables now? [Y/n] y
 ... Success!

Cleaning up...

All done!  If you've completed all of the above steps, your MariaDB
installation should now be secure.

Thanks for using MariaDB!

# connect to MariaDB with root
[root@www ~]# mysql -u root -p 
Enter password:     # MariaDB root password you set
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 16
Server version: 10.2.8-MariaDB MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# show user list
MariaDB [(none)]> select user,host,password from mysql.user; 
+------+-----------+-------------------------------------------+
| user | host      | password                                  |
+------+-----------+-------------------------------------------+
| root | localhost | ***************************************** |
| root | 127.0.0.1 | ***************************************** |
| root | ::1       | ***************************************** |
+------+-----------+-------------------------------------------+
3 rows in set (0.00 sec)

# show database list
MariaDB [(none)]> show databases; 
+--------------------+
| Database           |
+--------------------+
| information_schema |
| mysql              |
| performance_schema |
+--------------------+
3 rows in set (0.01 sec)

MariaDB [(none)]> exit
Bye
```

## 1.4 添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=mysql --permanent 
success

[root@www ~]# firewall-cmd --reload 
success
```

# 2. 安装phpMyAdmin

```Shell
# install from EPEL, Remi
[root@www ~]# yum --enablerepo=remi -y install php70-php-mysql php70-php-mcrypt 
[root@www ~]# yum --enablerepo=epel -y install phpMyAdmin

[root@www ~]# vi /etc/httpd/conf.d/phpMyAdmin.conf
# line 17, 34: IP address you permit to access
Require ip 127.0.0.1 10.0.0.0/24

[root@www ~]# systemctl restart httpd 
```

# 3. 配置SSL/TLS

## 3.1 创建数字证书

```Shell
[root@www ~]# cd /etc/pki/tls/certs 
[root@www certs]# make server.key 
umask 77 ; \
/usr/bin/openssl genrsa -aes128 2048 > server.key
Generating RSA private key, 2048 bit long modulus
...
...
e is 65537 (0x10001)
Enter pass phrase:# set passphrase
Verifying - Enter pass phrase:# confirm
# remove passphrase from private key
[root@www certs]# openssl rsa -in server.key -out server.key 
Enter pass phrase for server.key:# input passphrase
writing RSA key
[root@www certs]# make server.csr 
umask 77 ; \
/usr/bin/openssl req -utf8 -new -key server.key -out server.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [XX]:CN# country
State or Province Name (full name) []:Beijing   # state
Locality Name (eg, city) [Default City]:Beijing# city
Organization Name (eg, company) [Default Company Ltd]:CentOS   # company
Organizational Unit Name (eg, section) []:CentOS Org   # department
Common Name (eg, your name or your server's hostname) []:www.centos.org   # server's FQDN
Email Address []:admin@centos.org# email address
Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:# Enter
An optional company name []:# Enter

[root@www certs]# openssl x509 -in server.csr -out server.crt -req -signkey server.key -days 3650
Signature ok
subject=/C=CN/ST=Beijing/L=Beijing/O=CentOS/OU=CentOS Org/CN=www.centos.org/emailAddress=admin@centos.org
Getting Private key
```

## 3.2 复制证书文件并配置Maria

```Shell
[root@www ~]# cp /etc/pki/tls/certs/server.key \
/etc/pki/tls/certs/server.crt \
/etc/pki/tls/certs/ca-bundle.crt \
/etc/opt/rh/rh-mariadb102/pki/ 

[root@www ~]# chown mysql. /etc/opt/rh/rh-mariadb102/pki/* 

[root@www ~]# vi /etc/opt/rh/rh-mariadb102/my.cnf.d/mariadb-server.cnf
# add into [mysqld] section
[mysqld]
ssl-ca=/etc/opt/rh/rh-mariadb102/pki/ca-bundle.crt
ssl-cert=/etc/opt/rh/rh-mariadb102/pki/server.crt
ssl-key=/etc/opt/rh/rh-mariadb102/pki/server.key
[root@www ~]# systemctl restart rh-mariadb102-mariadb

# verify
[root@www ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 16
Server version: 10.2.8-MariaDB MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# OK if status is like follows
MariaDB [(none)]> show variables like '%ssl%'; 
+---------------------+---------------------------------------------+
| Variable_name       | Value                                       |
+---------------------+---------------------------------------------+
| have_openssl        | YES                                         |
| have_ssl            | YES                                         |
| ssl_ca              | /etc/opt/rh/rh-mariadb102/pki/ca-bundle.crt |
| ssl_capath          |                                             |
| ssl_cert            | /etc/opt/rh/rh-mariadb102/pki/server.crt    |
| ssl_cipher          |                                             |
| ssl_crl             |                                             |
| ssl_crlpath         |                                             |
| ssl_key             | /etc/opt/rh/rh-mariadb102/pki/server.key    |
| version_ssl_library | OpenSSL 1.0.2k-fips  26 Jan 2017            |
+---------------------+---------------------------------------------+
10 rows in set (0.01 sec)
```

## 3.3 客户端连接时指定证书

```Shell
[root@www ~]# mysql -u root -p --ssl-ca=/etc/opt/rh/rh-mariadb102/pki/server.crt 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 16
Server version: 10.2.8-MariaDB MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# show status
MariaDB [(none)]> show status like 'ssl_cipher'; 
+---------------+---------------------------+
| Variable_name | Value                     |
+---------------+---------------------------+
| Ssl_cipher    | DHE-RSA-AES256-GCM-SHA384 |
+---------------+---------------------------+
1 row in set (0.00 sec)

MariaDB [(none)]> exit 
Bye

# for no SSL/TLS connection
[root@www ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 16
Server version: 10.2.8-MariaDB MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# value is empty
MariaDB [(none)]> show status like 'ssl_cipher'; 
+---------------+-------+
| Variable_name | Value |
+---------------+-------+
| Ssl_cipher    |       |
+---------------+-------+
1 row in set (0.01 sec)
```

## 3.4 强制要求用户使用SSL/TLS连接

```Shell
[root@www ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 15
Server version: 10.2.8-MariaDB MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# create a user who is required SSL/TLS
MariaDB [(none)]> create user redhat identified by 'password' require ssl; 
Query OK, 0 rows affected (0.00 sec)

# show status
MariaDB [(none)]> select user,host,ssl_type from mysql.user; 
+--------+-----------+----------+
| user   | host      | ssl_type |
+--------+-----------+----------+
| root   | localhost |          |
| root   | 127.0.0.1 |          |
| root   | ::1       |          |
| redhat | %         | ANY      |
| cent   | %         |          |
+--------+-----------+----------+
5 rows in set (0.00 sec)

# set SSL/TLS required to an existing user
MariaDB [(none)]> grant usage on *.* to 'cent'@'%' require ssl; 
Query OK, 0 rows affected (0.00 sec)

MariaDB [(none)]> select user,host,ssl_type from mysql.user; 
+--------+-----------+----------+
| user   | host      | ssl_type |
+--------+-----------+----------+
| root   | localhost |          |
| root   | 127.0.0.1 |          |
| root   | ::1       |          |
| redhat | %         | ANY      |
| cent   | %         | ANY      |
+--------+-----------+----------+
5 rows in set (0.00 sec)
```

## 3.5 复制证书给其他客户端用于连接

```Shell
[root@node01 ~]# mysql -h www.centos.org -u JerryDai -p --ssl-ca=server.crt 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 15
Server version: 10.2.8-MariaDB MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> show status like 'ssl_cipher'; 
+---------------+---------------------------+
| Variable_name | Value                     |
+---------------+---------------------------+
| Ssl_cipher    | DHE-RSA-AES256-GCM-SHA384 |
+---------------+---------------------------+
1 row in set (0.00 sec)
```

# 4. 主从复制

## 4.1 在主节点上创建主从复制用户

```Shell
[root@www ~]# vi /etc/opt/rh/rh-mariadb102/my.cnf.d/mariadb-server.cnf

[server]
# add follows in [server] section : get binary logs
log-bin=mysql-bin

# define uniq server ID
server-id=101

[root@www ~]# systemctl restart rh-mariadb102-mariadb
[root@www ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 9
Server version: 10.2.8-MariaDB-log MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# create user (set any password for 'password' section)
MariaDB [(none)]> grant replication slave on *.* to replica@'%' identified by 'password'; 
Query OK, 0 rows affected (0.00 sec)

MariaDB [(none)]> flush privileges; 
Query OK, 0 rows affected (0.00 sec)

MariaDB [(none)]> exit
Bye
```

## 4.2 在从节点上配置

```Shell
[root@node01 ~]# vi /etc/opt/rh/rh-mariadb102/my.cnf.d/mariadb-server.cnf

[server]
# add follows in [server] section : get binary logs
log-bin=mysql-bin

# define server ID (different one from Master Host)
server-id=102

# read only
read_only=1

# define own hostname
report-host=node01.centos.org

[root@node01 ~]# systemctl restart rh-mariadb102-mariadb
```

## 4.3 在主节点上dump数据

```Shell
[root@www ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 10
Server version: 10.2.8-MariaDB-log MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# lock all tables
MariaDB [(none)]> flush tables with read lock; 
Query OK, 0 rows affected (0.00 sec)
# show status (remember File, Position value)
MariaDB [(none)]> show master status; 
+------------------+----------+--------------+------------------+
| File             | Position | Binlog_Do_DB | Binlog_Ignore_DB |
+------------------+----------+--------------+------------------+
| mysql-bin.000001 |      640 |              |                  |
+------------------+----------+--------------+------------------+
1 row in set (0.00 sec)

# remain the window above and open the another window and execute dump
[root@www ~]# mysqldump -u root -p --all-databases --lock-all-tables --events > mysql_dump.sql 
Enter password:
# back to the remained window and unlock
MariaDB [(none)]> unlock tables; 
Query OK, 0 rows affected (0.00 sec)
MariaDB [(none)]> exit
Bye

# transfer the dump to Slave Host
[root@www ~]# scp mysql_dump.sql node01.centos.org:/tmp/ 
root@node01.centos.org's password:
mysql_dump.sql 100% 466KB 51.0MB/s 00:00
```

## 4.4 	在从节点上导入dump数据并更改配置

```Shell
# import dump from Master Host
[root@node01 ~]# mysql -u root -p < /tmp/mysql_dump.sql 
Enter password:
[root@node01 ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 11
Server version: 10.2.8-MariaDB-log MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> change master to 
    -> master_host='10.0.0.31',     # Master Hosts's IP
    -> master_user='replica',     # replication ID
    -> master_password='password',     # replication ID's password
    -> master_log_file='mysql-bin.000001',     # File value confirmed on Master
    -> master_log_pos=640;     # Position value confirmed on Master
Query OK, 0 rows affected (0.15 sec)

# start replication
MariaDB [(none)]> start slave; 
Query OK, 0 rows affected (0.00 sec)

# show status
MariaDB [(none)]> show slave status\G 
*************************** 1. row ***************************
               Slave_IO_State: Waiting for master to send event
                  Master_Host: 10.0.0.31
                  Master_User: replica
                  Master_Port: 3306
                Connect_Retry: 60
              Master_Log_File: mysql-bin.000001
          Read_Master_Log_Pos: 640
               Relay_Log_File: mariadb-relay-bin.000002
                Relay_Log_Pos: 555
        Relay_Master_Log_File: mysql-bin.000001
             Slave_IO_Running: Yes
            Slave_SQL_Running: Yes
              Replicate_Do_DB:
          Replicate_Ignore_DB:
           Replicate_Do_Table:
       Replicate_Ignore_Table:
      Replicate_Wild_Do_Table:
  Replicate_Wild_Ignore_Table:
                   Last_Errno: 0
                   Last_Error:
                 Skip_Counter: 0
          Exec_Master_Log_Pos: 640
              Relay_Log_Space: 866
              Until_Condition: None
               Until_Log_File:
                Until_Log_Pos: 0
           Master_SSL_Allowed: No
           Master_SSL_CA_File:
           Master_SSL_CA_Path:
              Master_SSL_Cert:
            Master_SSL_Cipher:
               Master_SSL_Key:
        Seconds_Behind_Master: 0
Master_SSL_Verify_Server_Cert: No
                Last_IO_Errno: 0
                Last_IO_Error:
               Last_SQL_Errno: 0
               Last_SQL_Error:
  Replicate_Ignore_Server_Ids:
             Master_Server_Id: 101
               Master_SSL_Crl:
           Master_SSL_Crlpath:
                   Using_Gtid: No
                  Gtid_IO_Pos:
      Replicate_Do_Domain_Ids:
  Replicate_Ignore_Domain_Ids:
                Parallel_Mode: conservative
                    SQL_Delay: 0
          SQL_Remaining_Delay: NULL
      Slave_SQL_Running_State: Slave has read all relay log; waiting for the slave I/O thread to update it
1 row in set (0.00 sec)
```

# 5. 高可用

## 5.1 在所有节点上安装软件包

```Shell
# install from SCLo
[root@www ~]# yum --enablerepo=centos-sclo-rh -y install rh-mariadb102-mariadb-server-galera

# if Firewalld is running, set like follows
[root@www ~]# firewall-cmd --add-service=mysql --permanent 
[root@www ~]# firewall-cmd --add-port={3306/tcp,4567/tcp,4568/tcp,4444/tcp} --permanent 
[root@www ~]# firewall-cmd --reload 
```

## 5.2 配置第一个节点

```Shell
[root@www ~]# mv /etc/opt/rh/rh-mariadb102/my.cnf.d/galera.cnf /etc/opt/rh/rh-mariadb102/my.cnf.d/galera.cnf.org 
[root@www ~]# vi /etc/opt/rh/rh-mariadb102/my.cnf.d/mariadb-server.cnf

# line 30: uncomment and change like follows
[galera]
# Mandatory settings
wsrep_on=ON
wsrep_provider=/opt/rh/rh-mariadb102/root/usr/lib64/galera/libgalera_smm.so 
wsrep_cluster_address=gcomm:// 

# uncomment like follows
binlog_format=row
default_storage_engine=InnoDB
innodb_autoinc_lock_mode=2
bind-address=0.0.0.0

# add follows
# cluster name
wsrep_cluster_name="MariaDB_Cluster"

# own IP address
wsrep_node_address="10.0.0.31"

# replication provider
wsrep_sst_method=rsync

# start Galera Cluster
[root@www ~]# /opt/rh/rh-mariadb102/root/usr/bin/galera_new_cluster

# run basic initial settings
[root@www ~]# mysql_secure_installation 
```

## 5.3 配置其他节点

```Shell
[root@node01 ~]# mv /etc/opt/rh/rh-mariadb102/my.cnf.d/galera.cnf /etc/opt/rh/rh-mariadb102/my.cnf.d/galera.cnf.org 
[root@node01 ~]# vi /etc/opt/rh/rh-mariadb102/my.cnf.d/mariadb-server.cnf

# line 30: uncomment and change like follows
[galera]
# Mandatory settings
wsrep_on=ON
wsrep_provider=/opt/rh/rh-mariadb102/root/usr/lib64/galera/libgalera_smm.so 

# specify all nodes in cluster
wsrep_cluster_address="gcomm://10.0.0.31,10.0.0.51" 
binlog_format=row
default_storage_engine=InnoDB
innodb_autoinc_lock_mode=2
bind-address=0.0.0.0

# add follows
# cluster name
wsrep_cluster_name="MariaDB_Cluster"

# own IP address
wsrep_node_address="10.0.0.51"

# replication provider
wsrep_sst_method=rsync

[root@node01 ~]# systemctl start rh-mariadb102-mariadb 
```

## 5.4 测试wsrep_local_state_comment状态是否为Synced

```Shell
[root@node01 ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 11
Server version: 10.2.8-MariaDB-log MariaDB Server

Copyright (c) 2000, 2017, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> show status like 'wsrep_%'; 
+------------------------------+------------------------------------------------+
| Variable_name                | Value                                          |
+------------------------------+------------------------------------------------+
| wsrep_apply_oooe             | 0.000000                                       |
| wsrep_apply_oool             | 0.000000                                       |
| wsrep_apply_window           | 0.000000                                       |
| wsrep_causal_reads           | 0                                              |
| wsrep_cert_deps_distance     | 0.000000                                       |
| wsrep_cert_index_size        | 0                                              |
| wsrep_cert_interval          | 0.000000                                       |
| wsrep_cluster_conf_id        | 2                                              |
| wsrep_cluster_size           | 2                                              |
| wsrep_cluster_state_uuid     | 5f73ca9f-be0e-11e7-b57e-66deb0603196           |
| wsrep_cluster_status         | Primary                                        |
| wsrep_commit_oooe            | 0.000000                                       |
| wsrep_commit_oool            | 0.000000                                       |
| wsrep_commit_window          | 0.000000                                       |
| wsrep_connected              | ON                                             |
| wsrep_desync_count           | 0                                              |
| wsrep_evs_delayed            |                                                |
| wsrep_evs_evict_list         |                                                |
| wsrep_evs_repl_latency       | 0.000607146/0.00150996/0.00418607/0.00134739/5 |
| wsrep_evs_state              | OPERATIONAL                                    |
| wsrep_flow_control_paused    | 0.000000                                       |
| wsrep_flow_control_paused_ns | 0                                              |
| wsrep_flow_control_recv      | 0                                              |
| wsrep_flow_control_sent      | 0                                              |
| wsrep_gcomm_uuid             | e1ceedcc-be0e-11e7-bdc4-6332d9eeeb20           |
| wsrep_incoming_addresses     | 10.0.0.31:3306,10.0.0.51:3306                  |
| wsrep_last_committed         | 3                                              |
| wsrep_local_bf_aborts        | 0                                              |
| wsrep_local_cached_downto    | 18446744073709551615                           |
| wsrep_local_cert_failures    | 0                                              |
| wsrep_local_commits          | 0                                              |
| wsrep_local_index            | 1                                              |
| wsrep_local_recv_queue       | 0                                              |
| wsrep_local_recv_queue_avg   | 0.000000                                       |
| wsrep_local_recv_queue_max   | 1                                              |
| wsrep_local_recv_queue_min   | 0                                              |
| wsrep_local_replays          | 0                                              |
| wsrep_local_send_queue       | 0                                              |
| wsrep_local_send_queue_avg   | 0.333333                                       |
| wsrep_local_send_queue_max   | 2                                              |
| wsrep_local_send_queue_min   | 0                                              |
| wsrep_local_state            | 4                                              |
| wsrep_local_state_comment    | Synced                                         |
| wsrep_local_state_uuid       | 5f73ca9f-be0e-11e7-b57e-66deb0603196           |
| wsrep_protocol_version       | 7                                              |
| wsrep_provider_name          | Galera                                         |
| wsrep_provider_vendor        | Codership Oy <info@codership.com>              |
| wsrep_provider_version       | 3.20(rXXXX)                                    |
| wsrep_ready                  | ON                                             |
| wsrep_received               | 3                                              |
| wsrep_received_bytes         | 231                                            |
| wsrep_repl_data_bytes        | 0                                              |
| wsrep_repl_keys              | 0                                              |
| wsrep_repl_keys_bytes        | 0                                              |
| wsrep_repl_other_bytes       | 0                                              |
| wsrep_replicated             | 0                                              |
| wsrep_replicated_bytes       | 0                                              |
| wsrep_thread_count           | 2                                              |
+------------------------------+------------------------------------------------+
58 rows in set (0.01 sec)
```
