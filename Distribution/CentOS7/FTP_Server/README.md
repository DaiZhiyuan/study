- 1 Vsftpd
    - 1.1 安装配置Vsftpd
    - 1.2 添加防火墙策略
    - 1.3 变更SELinux策略
- 2 CentOS下客户端
    - 2.1 安装使用lftp
- 3 基于SSL/TLS的Vsftpd服务
    - 3.1 创建自签证书
    - 3.2 配置Vsftpd
    - 3.3 添加防火墙策略
    - 3.4 配置使用lftp客户端

# 1. Vsftpd

vsftpd 是“very secure FTP daemon”的缩写，安全性是它的一个最大的特点。vsftpd 是一个UNIX类操作系统上运行的服务器的名字，它可以运行在诸如 Linux、BSD、Solaris、 HP-UNIX等系统上面，是一个完全免费的、开放源代码的ftp服务器软件，支持很多其他的FTP服务器所不支持的特征。

比如：非常高的安全性需求、带宽限制、良好的可伸缩性、可创建虚拟用户、支持IPv6、速率高等。

## 1.1 安装配置Vsftpd

```Shell
[root@www ~]# yum -y install vsftpd

[root@www ~]# vi /etc/vsftpd/vsftpd.conf
# line 12: no anonymous
anonymous_enable=NO

# line 82,83: uncomment ( allow ascii mode )
ascii_upload_enable=YES
ascii_download_enable=YES

# line 100, 101: uncomment ( enable chroot )
chroot_local_user=YES
chroot_list_enable=YES

# line 103: uncomment ( specify chroot list )
chroot_list_file=/etc/vsftpd/chroot_list

# line 109: uncomment
ls_recurse_enable=YES

# line 114: change ( if use IPv4 )
listen=YES

# line 123: change ( turn to OFF if it's not need )
listen_ipv6=NO

# add follows to the end
# specify root directory ( if don't specify, users' home directory become FTP home directory)
local_root=public_html

# use localtime
use_localtime=YES

# turn off for seccomp filter ( if you cannot login, add this line )
seccomp_sandbox=NO

[root@www ~]# vi /etc/vsftpd/chroot_list
# add users you allow to move over their home directory
JerryDai
[root@www ~]# systemctl start vsftpd 
[root@www ~]# systemctl enable vsftpd 
```

## 1.2 添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=ftp --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

## 1.3 变更SELinux策略

```Shell
[root@www ~]# setsebool -P ftpd_full_access on 
```

# 2. CentOS下客户端

## 2.1 安装使用lftp

```Shell
[root@lab ~]# yum -y install lftp

# lftp [option] [hostname]
[redhat@lab ~]$ lftp -u JerryDai www.centos.org
Password:     # password of the user
lftp JerryDai@www.centos.org:~>

# show current directory on FTP server
lftp JerryDai@www.centos.org:~> pwd
ftp://JerryDai@www.centos.org

# show current directory on local server
lftp JerryDai@www.centos.org:~> !pwd
/home/redhat

# show files in current directory on FTP server
lftp JerryDai@www.centos.org:~> ls
drwxr-xr-x    2 1000     1000           23 Jul 19 01:33 public_html
-rw-r--r--    1 1000     1000          399 Jul 20 16:32 test.py

# show files in current directory on local server
lftp JerryDai@www.centos.org:~> !ls -l
total 12
-rw-rw-r-- 1 redhat redhat 10 Jul 20 14:30 redhat.txt
-rw-rw-r-- 1 redhat redhat 10 Jul 20 14:59 test2.txt
-rw-rw-r-- 1 redhat redhat 10 Jul 20 14:59 test.txt

# change directory
lftp JerryDai@www.centos.org:~> cd public_html
lftp JerryDai@www.centos.org:~/public_html> pwd
ftp://JerryDai@www.centos.org/%2Fhome/JerryDai/public_html

# upload a file to FTP server
# "-a" means ascii mode ( default is binary mode )
lftp JerryDai@www.centos.org:~> put -a redhat.txt
22 bytes transferred
Total 2 files transferred
lftp JerryDai@www.centos.org:~> ls
drwxr-xr-x    2 1000     1000           23 Jul 19 01:33 public_html
-rw-r--r--    1 1000     1000           10 Jul 20 17:01 redhat.txt
-rw-r--r--    1 1000     1000          399 Jul 20 16:32 test.py
-rw-r--r--    1 1000     1000           10 Jul 20 17:01 test.txt

# upload some files to FTP server
lftp JerryDai@www.centos.org:~> mput -a test.txt test2.txt
22 bytes transferred
Total 2 files transferred
lftp JerryDai@www.centos.org:~> ls
drwxr-xr-x    2 1000     1000           23 Jul 19 01:33 public_html
-rw-r--r--    1 1000     1000          399 Jul 20 16:32 test.py
-rw-r--r--    1 1000     1000           10 Jul 20 17:06 test.txt
-rw-r--r--    1 1000     1000           10 Jul 20 17:06 test2.txt

# download a file from FTP server
# "-a" means ascii mode ( default is binary mode )
lftp JerryDai@www.centos.org:~> get -a test.py
416 bytes transferred

# download some files from FTP server
lftp JerryDai@www.centos.org:~> mget -a test.txt test2.txt
20 bytes transferred
Total 2 files transferred

# create a directory in current directory on FTP Server
lftp JerryDai@www.centos.org:~> mkdir testdir
mkdir ok, `testdir' created
lftp JerryDai@www.centos.org:~> ls
drwxr-xr-x    2 1000     1000           23 Jul 19 01:33 public_html
-rw-r--r--    1 1000     1000          399 Jul 20 16:32 test.py
-rw-r--r--    1 1000     1000           10 Jul 20 17:06 test.txt
-rw-r--r--    1 1000     1000           10 Jul 20 17:06 test2.txt
drwxr-xr-x    2 1000     1000            6 Jul 20 17:16 testdir
226 Directory send OK.

# delete a direcroty in current directory on FTP Server
lftp JerryDai@www.centos.org:~> rmdir testdir
rmdir ok, `testdir' removed
lftp JerryDai@www.centos.org:~> ls
drwxr-xr-x    2 1000     1000           23 Jul 19 01:33 public_html
-rw-r--r--    1 1000     1000          399 Jul 20 16:32 test.py
-rw-r--r--    1 1000     1000           10 Jul 20 17:06 test.txt
-rw-r--r--    1 1000     1000           10 Jul 20 17:06 test2.txt

# delete a file in current directory on FTP Server
lftp JerryDai@www.centos.org:~> rm test2.txt
rm ok, `test2.txt' removed
lftp JerryDai@www.centos.org:~> ls
drwxr-xr-x    2 1000     1000           23 Jul 19 01:33 public_html
-rw-r--r--    1 1000     1000          399 Jul 20 16:32 test.py
-rw-r--r--    1 1000     1000           10 Jul 20 17:06 test.txt

# delete some files in current directory on FTP Server
lftp JerryDai@www.centos.org:~> mrm redhat.txt test.txt
rm ok, 2 files removed
lftp JerryDai@www.centos.org:~> ls
drwxr-xr-x    2 1000     1000           23 Jul 19 01:33 public_html

# execute commands with "![command]"
lftp JerryDai@www.centos.org:~> !cat /etc/passwd
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
...
...
redhat:x:1001:1001::/home/redhat:/bin/bash

# exit
lftp JerryDai@www.centos.org:~> quit
221 Goodbye.
```

# 3. 基于SSL/TLS的Vsftpd服务

## 3.1 创建自签证书

```Shell
[root@www ~]# cd /etc/pki/tls/certs 
[root@www certs]# openssl req -x509 -nodes -newkey rsa:2048 -keyout vsftpd.pem -out vsftpd.pem -days 365 
Generating a 2048 bit RSA private key
......++++++
.......++++++
writing new private key to '/etc/pki/tls/certs/vsftpd.pem'
-----
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:CN     # country
State or Province Name (full name) [Some-State]:Beijing     # state
Locality Name (eg, city) []:Beijing     # city
Organization Name (eg, company) [Internet Widgits Pty Ltd]:CentOS     # company
Organizational Unit Name (eg, section) []:CentOS Org     # department
Common Name (eg, YOUR name) []:www.centos.org     # server's FQDN
Email Address []:admin@centos.org     # admin email
[root@www certs]# chmod 400 vsftpd.pem 
```

## 3.2 配置Vsftpd

```Shell
[root@www ~]# vi /etc/vsftpd/vsftpd.conf

# add to the end
# fix PASV ports
pasv_enable=YES
pasv_min_port=21000
pasv_max_port=21010

# enable TLS
rsa_cert_file=/etc/pki/tls/certs/vsftpd.pem
ssl_enable=YES
ssl_ciphers=HIGH
ssl_tlsv1=YES
ssl_sslv2=NO
ssl_sslv3=NO
force_local_data_ssl=YES
force_local_logins_ssl=YES

[root@www ~]# systemctl restart vsftpd 
```

## 3.3 添加防火墙策略

```Shell
[root@dlp ~]# firewall-cmd --add-port=21000-21010/tcp --permanent 
success
[root@dlp ~]# firewall-cmd --reload 
success
```

## 3.4 配置使用lftp客户端

```Shell
[redhat@lab ~]$ vi ~/.lftprc

# create new
set ftp:ssl-auth TLS
set ftp:ssl-force true
set ftp:ssl-protect-list yes
set ftp:ssl-protect-data yes
set ftp:ssl-protect-fxp yes
set ssl:verify-certificate no

[redhat@dlp ~]$ lftp -u JerryDai www.centos.org 
Password:
lftp cent@www.centos.org:~>
```

