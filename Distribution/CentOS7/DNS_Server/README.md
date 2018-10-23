
- 1 安装BIND
    - 1.1 安装BIND
    - 1.2 配置BIND
- 2 配置Zone信息
    - 2.1 内网域名解析
    - 2.2 外网域名解析
    - 2.3 内网地址解析
    - 2.4 外网地址解析
- 3 启动并验证BIND
    - 3.1 启动BIND
    - 3.2 添加防火墙规则
    - 3.3 改变DNS服务器地址
    - 3.4 验证名称解析与地址解析功能
- 4 启用Chroot环境
- 5 设置CNAME记录
- 6 配置主从模式
    - 6.1 配置主DNS服务
    - 6.2 配置从DNS服务


# 1. 安装BIND

## 1.1 安装BIND

```Shell
[root@lab ~]# yum -y install bind bind-utils
```

## 1.2 配置BIND

实验环境：
```
域名：centos.org
公网IP：[172.16.0.80/29]
内网IP：[10.0.0.0/24]
```

对应配置：
```Shell
[root@lab ~]# vi /etc/named.conf
//
// named.conf
//
// Provided by Red Hat bind package to configure the ISC BIND named(8) DNS
// server as a caching only nameserver (as a localhost DNS resolver only).
//
// See /usr/share/doc/bind*/sample/ for example named configuration files.
//

options {
        # change ( listen all )
        listen-on port 53 { any; };
        
        # change if not use IPv6
        listen-on-v6 { none; };
        directory           "/var/named";
        dump-file           "/var/named/data/cache_dump.db";
        statistics-file     "/var/named/data/named_stats.txt";
        memstatistics-file  "/var/named/data/named_mem_stats.txt";
        
        # query range ( set internal server and so on )
        allow-query         { localhost; 10.0.0.0/24; };
        
        # transfer range ( set it if you have secondary DNS )
        allow-transfer      { localhost; 10.0.0.0/24; };

        recursion yes;

        dnssec-enable yes;
        dnssec-validation yes;
        dnssec-lookaside auto;

        /* Path to ISC DLV key */
        bindkeys-file "/etc/named.iscdlv.key";

        managed-keys-directory "/var/named/dynamic";

        pid-file "/run/named/named.pid";
        session-keyfile "/run/named/session.key";
};
logging {
        channel default_debug {
                file "data/named.run";
                severity dynamic;
        };
};

# change all from here
view "internal" {
        match-clients {
                localhost;
                10.0.0.0/24;
        };
        zone "." IN {
                type hint;
                file "named.ca";
        };
        zone "centos.org" IN {
                type master;
                file "centos.org.lan";
                allow-update { none; };
        };
        zone "0.0.10.in-addr.arpa" IN {
                type master;
                file "0.0.10.db";
                allow-update { none; };
        };
include "/etc/named.rfc1912.zones";
include "/etc/named.root.key";
};

view "external" {
        match-clients { any; };
        allow-query { any; };
        recursion no;
        zone "centos.org" IN {
                type master;
                file "centos.org.wan";
                allow-update { none; };
        };
        zone "80.0.16.172.in-addr.arpa" IN {
                type master;
                file "80.0.16.172.db";
                allow-update { none; };
        };
};

# allow-query               ⇒ query range you permit
# allow-transfer            ⇒ the range you permit to transfer zone info
# recursion                 ⇒ allow or not to search recursively
# view "internal" { *** };  ⇒ write for internal definition
# view "external" { *** };  ⇒ write for external definition

# For How to write for reverse resolving, Write network address reversely like below
# 10.0.0.0/24
# network address           ⇒ 10.0.0.0
# range of network          ⇒ 10.0.0.0 - 10.0.0.255
# how to write              ⇒ 0.0.10.in-addr.arpa

# 172.16.0.80/29
# network address           ⇒ 172.16.0.80
# range of network          ⇒ 172.16.0.80 - 172.16.0.87
# how to write              ⇒ 80.0.16.172.in-addr.arpa
```

# 2. 配置Zone信息


## 2.1 内网域名解析

- 内网地址：[10.0.0.0/24]
- 域名：[centos.org]

```Shell
[root@lab ~]# vi /var/named/centos.org.lan
 $TTL 86400
@   IN  SOA     lab.centos.org. root.centos.org. (
        2014071001  ;Serial
        3600        ;Refresh
        1800        ;Retry
        604800      ;Expire
        86400       ;Minimum TTL
)

# define name server
        IN  NS      lan.centos.org.
        
# internal IP address of name server
        IN  A       10.0.0.30
        
# define Mail exchanger
        IN  MX 10   lab.centos.org.

# define IP address and hostname
lab     IN  A       10.0.0.30
```

## 2.2 外网域名解析

- 外网地址：[172.16.0.80/29]
- 域名：[centos.org]

```
[root@lab ~]# vi /var/named/centos.org.wan
 $TTL 86400
@   IN  SOA     lab.centos.org. root.centos.org. (
        2014071001  ;Serial
        3600        ;Refresh
        1800        ;Retry
        604800      ;Expire
        86400       ;Minimum TTL
)

# define name server
        IN  NS      lab.centos.org.
        
# external IP address of name server
        IN  A       172.16.0.82
        
# define Mail exchanger
        IN  MX 10   lab.centos.org.

# define IP address and hostname
dlp     IN  A       172.16.0.82
```

## 2.3 内网地址解析

- 内网地址：[10.0.0.0/24]
- 域名：[centos.org]

```Shell
[root@lab ~]# vi /var/named/0.0.10.db
 $TTL 86400
@   IN  SOA     lab.centos.org. root.centos.org. (
        2014071001  ;Serial
        3600        ;Refresh
        1800        ;Retry
        604800      ;Expire
        86400       ;Minimum TTL
)

# define name server
        IN  NS      lab.centos.org.

# define the range of this domain included
        IN  PTR     centos.org.
        IN  A       255.255.255.0

# define hostname of an IP address
30      IN  PTR     centos.org.
```

## 2.4 外网地址解析

- 外网地址：[172.16.0.80/29]
- 域名：[centos.org]

```Shell
[root@lab ~]# vi /var/named/80.0.16.172.db
 $TTL 86400
@   IN  SOA     lab.centos.org. root.centos.org. (
        2014071001  ;Serial
        3600        ;Refresh
        1800        ;Retry
        604800      ;Expire
        86400       ;Minimum TTL
)

# define name server
        IN  NS      lab.centos.org.

# define the range of this domain included
        IN  PTR     centos.org.
        IN  A       255.255.255.248

# define hostname of an IP address
82      IN  PTR     lab.centos.org.
```

# 3. 启动并验证BIND

## 3.1 启动BIND

```Shell
[root@lab ~]# systemctl start named 
[root@lab ~]# systemctl enable named 
```

## 3.2 添加防火墙规则

DNS服务：使用TCP协议的53号端口，UDP协议的53号端口。

```Shell
[root@lab ~]# firewall-cmd --add-service=dns --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

## 3.3 改变DNS服务器地址

```Shell
[root@lab ~]# nmcli c modify enp0s3 ipv4.dns 10.0.0.30 
[root@lab ~]# nmcli c down enp0s3; nmcli c up enp0s3 
```

## 3.4 验证名称解析与地址解析功能

名称解析：

```Shell
[root@lab ~]# dig lab.centos.org. 

; <<>> DiG 9.9.4-RedHat-9.9.4-14.el7 <<>> lab.centos.org.
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 41735
;; flags: qr aa rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 1, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 4096
;; QUESTION SECTION:
;dlp.srv.world.              IN      A

;; ANSWER SECTION:
lab.centos.org.       86400   IN      A       10.0.0.30

;; AUTHORITY SECTION:
centos.org.           86400   IN      NS      lab.centos.org.

;; Query time: 1 msec
;; SERVER: 10.0.0.30#53(10.0.0.30)
;; WHEN: Thu Jul 10 14:43:35 JST 2014
;; MSG SIZE  rcvd: 75
```

地址解析：

```Shell
[root@dlp ~]# dig -x 10.0.0.30 

; <<>> DiG 9.9.4-RedHat-9.9.4-14.el7 <<>> -x 10.0.0.30
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 14268
;; flags: qr aa rd ra; QUERY: 1, ANSWER: 1, AUTHORITY: 1, ADDITIONAL: 2

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 4096
;; QUESTION SECTION:
;30.0.0.10.in-addr.arpa.                IN      PTR

;; ANSWER SECTION:
30.0.0.10.in-addr.arpa. 86400   IN      PTR     lab.centos.org.

;; AUTHORITY SECTION:
0.0.10.in-addr.arpa.    86400   IN      NS      lab.centos.org.

;; ADDITIONAL SECTION:
lab.centos.org.       86400   IN      A       10.0.0.30

;; Query time: 1 msec
;; SERVER: 10.0.0.30#53(10.0.0.30)
;; WHEN: Thu Jul 10 14:44:39 JST 2014
;; MSG SIZE  rcvd: 111
```

# 4. 启用Chroot环境

配合Chroot环境需要安装bind-chroot软件包，DNS服务的根目录为/var/named/chroot。

```Shell
[root@lab ~]# yum -y install bind-chroot

[root@lab ~]# /usr/libexec/setup-named-chroot.sh /var/named/chroot on 

[root@lab ~]# systemctl stop named 

[root@lab ~]# systemctl disable named 

[root@lab ~]# systemctl start named-chroot 

[root@lab ~]# systemctl enable named-chroot 
ln -s '/usr/lib/systemd/system/named-chroot.service' '/etc/systemd/system/multi-user.target.wants/named-chroot.service'

[root@lab ~]# ll /var/named/chroot/etc 
total 24
-rw-r--r-- 1 root root   331 Jul 10 14:46 localtime
drwxr-x--- 2 root named    6 Jun 10 17:13 named
-rw-r----- 1 root named 2211 Jul 10 14:13 named.conf
-rw-r--r-- 1 root named 2389 Jun 10 17:13 named.iscdlv.key
-rw-r----- 1 root named  931 Jun 21  2007 named.rfc1912.zones
-rw-r--r-- 1 root named  487 Jul 19  2010 named.root.key
drwxr-x--- 3 root named   24 Jul 10 14:46 pki
-rw-r----- 1 root named   77 Jul 10 14:39 rndc.key

[root@lab ~]# ll /var/named/chroot/var/named 
total 28
-rw-r--r-- 1 root  root   358 Jul 10 14:31 0.0.10.db
drwxr-x--- 7 root  named   56 Jul 10 14:46 chroot
drwxrwx--- 2 named named   22 Jul 10 14:39 data
drwxrwx--- 2 named named 4096 Jul 10 14:42 dynamic
-rw-r----- 1 root  named 2076 Jan 28  2013 named.ca
-rw-r----- 1 root  named  152 Dec 15  2009 named.empty
-rw-r----- 1 root  named  152 Jun 21  2007 named.localhost
-rw-r----- 1 root  named  168 Dec 15  2009 named.loopback
-rw-r--r-- 1 root  root   350 Jul 10 14:30 srv.world.lan
drwxrwx--- 2 named named    6 Jun 10 17:13 slaves
```

# 5. 设置CNAME记录

在ZONE文件中设置CNAME

```Shell
[root@lab ~]# vi /var/named/centos.org.lan
$TTL 86400
@   IN  SOA     lab.centos.org. root.centos.org. (
# update serial
        2014071002  ;Serial
        3600        ;Refresh
        1800        ;Retry
        604800      ;Expire
        86400       ;Minimum TTL
)
        IN  NS      lab.centos.org.
        IN  A       10.0.0.30
        IN  MX 10   lab.centos.org.

lab     IN  A       10.0.0.30
# [ aliase IN CNAME server's name ]
ftp     IN  CNAME   lab.centos.org.

[root@lab ~]# systemctl restart named 

[root@lab ~]# dig ftp.centos.org. 

; <<>> DiG 9.9.4-RedHat-9.9.4-14.el7 <<>> ftp.centos.org.
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 64374
;; flags: qr aa rd ra; QUERY: 1, ANSWER: 2, AUTHORITY: 1, ADDITIONAL: 1

;; OPT PSEUDOSECTION:
; EDNS: version: 0, flags:; udp: 4096
;; QUESTION SECTION:
;ftp.centos.org.              IN      A

;; ANSWER SECTION:
ftp.centos.org.       86400   IN      CNAME   lab.centos.org.
lab.centos.org.       86400   IN      A       10.0.0.30

;; AUTHORITY SECTION:
centos.org.           86400   IN      NS      lab.centos.org.

;; Query time: 1 msec
;; SERVER: 10.0.0.30#53(10.0.0.30)
;; WHEN: Thu Jul 10 14:54:56 JST 2014
;; MSG SIZE  rcvd: 93
```

# 6. 配置主从模式

- Master：lab.centos.org 
- Slave：ns.example.host

## 6.1 配置主DNS服务

```Shell
[root@lab ~]# vi /etc/named.conf

# add an IP address of slave DNS server
allow-transfer { localhost; 172.16.0.85; };

[root@lab ~]# vi /var/named/cenots.org.wan
$TTL 86400
@   IN  SOA     lab.centos.org. root.centos.org. (
# update serial
        2014071003  ;Serial
        3600        ;Refresh
        1800        ;Retry
        604800      ;Expire
        86400       ;Minimum TTL
)
        IN  NS      lab.centos.org.
        
# add Slave server
        IN  NS      ns.example.host.
        IN  A       172.16.0.82
        IN  MX 10   lab.centos.org.

lab     IN  A       172.16.0.82

[root@lab ~]# systemctl restart named 
```

## 6.2 配置从DNS服务

```Shell
[root@ns ~]# vi /etc/named.conf

# add lines like follows
     zone "centos.org" IN {
        type slave;
        masters { 172.16.0.82; };
        file "slaves/centos.org.wan";
        notify no;
    };

[root@ns ~]# systemctl restart named 
[root@ns ~]# ls /var/named/slaves 
centos.org.wan     # zone file from Master server has been transfered
```