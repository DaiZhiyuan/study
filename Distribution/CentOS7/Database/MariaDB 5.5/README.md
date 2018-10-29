- 1 MariaDB
    - 1.1 安装
    - 1.2 初始化
    - 1.3 添加防火墙策略
- 2 安装phpMyAdmin
- 3 主从复制
    - 3.1 在主节点上创建一个用于主从复制的用户
    - 3.2 在从节点上安装并初始化数据库
    - 3.3 在所有节点上添加防火墙策略
    - 3.4 在从节点上配置
    - 3.5 在从节点上配置主从复制

# 1. MariaDB

MariaDB 5.5是CentOS发行版默认版本。

## 1.1 安装

```Shell
[root@www ~]# yum -y install mariadb-server

[root@www ~]# vi /etc/my.cnf

# add follows within [mysqld] section
[mysqld]
character-set-server=utf8

[root@www ~]# systemctl start mariadb 
[root@www ~]# systemctl enable mariadb 
ln -s '/usr/lib/systemd/system/mariadb.service' '/etc/systemd/system/multi-user.target.wants/mariadb.service'
```

## 1.2 初始化

```Shell
[root@www ~]# mysql_secure_installation 

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
Your MariaDB connection id is 3
Server version: 5.5.37-MariaDB MariaDB Server

Copyright (c) 2000, 2014, Oracle, Monty Program Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# show user list
MariaDB [(none)]> select user,host,password from mysql.user; 
+------+-----------+-------------------------------------------+
| user | host      | password                                  |
+------+-----------+-------------------------------------------+
| root | localhost | *xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx |
| root | 127.0.0.1 | *xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx |
| root | ::1       | *xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx |
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
3 rows in set (0.00 sec)

MariaDB [(none)]> exit
Bye
```

## 1.3 添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=mysql --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

# 2. 安装phpMyAdmin

```Shell
# install from EPEL
[root@www ~]# yum --enablerepo=epel -y install phpMyAdmin php-mysql php-mcrypt

[root@www ~]# vi /etc/httpd/conf.d/phpMyAdmin.conf

# line 15: IP address you permit to access
Require ip 127.0.0.1 10.0.0.0/24

# line 32: IP address you permit to access
Require ip 127.0.0.1 10.0.0.0/24

[root@www ~]# systemctl restart httpd 
```

> 使用访问http://(your hostname or IP address)/phpmyadmin/

# 3. 主从复制

## 3.1 在主节点上创建一个用于主从复制的用户

```Shell
[root@www ~]# vi /etc/my.cnf

[mysqld]
# add follows in [mysqld] section : get binary logs
log-bin=mysql-bin

# define uniq server ID
server-id=101

[root@www ~]# systemctl restart mariadb 

[root@www ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 2
Server version: 5.5.41-MariaDB-log MariaDB Server

Copyright (c) 2000, 2014, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

# create user (set any password for 'password' section)
MariaDB [(none)]> grant replication slave on *.* to replica@'%' identified by 'password'; 
Query OK, 0 rows affected (0.00 sec)
MariaDB [(none)]> flush privileges; 
Query OK, 0 rows affected (0.00 sec)
MariaDB [(none)]> exit
Bye
```

## 3.2 在从节点上安装并初始化数据库

> 参考1章

## 3.3 在所有节点上添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=mysql --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

## 3.4 在从节点上配置

```Shell

[root@node01 ~]# vi /etc/my.cnf

[mysqld]
# add follows in [mysqld] section : get binary logs
log-bin=mysql-bin

# define server ID (different one from Master Host)
server-id=102

# read only
read_only=1

# define own hostname
report-host=node01.centos.org

[root@node01 ~]# systemctl restart mariadb 
```

## 3.5 在从节点上配置主从复制

```Shell
# import dump from Master Host
[root@node01 ~]# mysql -u root -p < /tmp/mysql_dump.sql 
Enter password:

[root@node01 ~]# mysql -u root -p 
Enter password:
Welcome to the MariaDB monitor.  Commands end with ; or \g.
Your MariaDB connection id is 3
Server version: 5.5.41-MariaDB-log MariaDB Server

Copyright (c) 2000, 2014, Oracle, MariaDB Corporation Ab and others.

Type 'help;' or '\h' for help. Type '\c' to clear the current input statement.

MariaDB [(none)]> change master to 
    -> master_host='10.0.0.31',     # Master Hosts's IP
    -> master_user='replica',     # replication ID
    -> master_password='password',     # replication ID's password
    -> master_log_file='mysql-bin.000001',     # File value confirmed on Master
    -> master_log_pos=465;     # Position value confirmed on Master
Query OK, 0 rows affected (0.58 sec)

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
          Read_Master_Log_Pos: 536
               Relay_Log_File: mariadb-relay-bin.000002
                Relay_Log_Pos: 600
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
          Exec_Master_Log_Pos: 536
              Relay_Log_Space: 896
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
1 row in set (0.00 sec)
```