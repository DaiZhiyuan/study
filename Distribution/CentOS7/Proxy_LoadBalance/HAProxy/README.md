- 1 HAProxy
    - 1.1 安装
    - 1.2 配置
    - 1.3 配置Rsyslog日志系统
    - 1.4 配置后端httpd日志系统
- 2 SSL/TLS设置
    - 2.1 创建数字证书
    - 2.2 配置SSL
- 3 统计信息
    - 3.1 基于Web展示统计信息
    - 3.2 基于命令行展示统计信息
- 4 L4负载均衡
    - 4.1 实验环境
    - 4.2 配置

# 1. HAProxy

HAProxy是一个使用C语言编写的自由及开放源代码软件，其提供高可用性、负载均衡，以及基于TCP和HTTP的应用程序代理。

HAProxy特别适用于那些负载特大的web站点，这些站点通常又需要会话保持或七层处理。HAProxy运行在当前的硬件上，完全可以支持数以万计的并发连接。并且它的运行模式使得它可以很简单安全的整合进您当前的架构中， 同时可以保护你的web服务器不被暴露到网络上。

HAProxy实现了一种事件驱动, 单一进程模型，此模型支持非常大的并发连接数。多进程或多线程模型受内存限制、系统调度器限制以及无处不在的锁限制，很少能处理数千并发连接。事件驱动模型因为在有更好的资源和时间管理的用户空间(User-Space)实现所有这些任务，所以没有这些问题。此模型的弊端是，在多核系统上，这些程序通常扩展性较差。这就是为什么他们必须进行优化以 使每个CPU时间片(Cycle)做更多的工作。
 	
实验环境：

```
(1) [10.0.0.30]   - Frontend HAProxy
(2) [10.0.0.31]   - Backend Web Server#1
(3) [10.0.0.32]   - Backend Web Server#2
```

## 1.1 安装

```Shell
[root@lab ~]# yum -y install haproxy
```

## 1.2 配置

```
[root@lab ~]# mv /etc/haproxy/haproxy.cfg /etc/haproxy/haproxy.cfg.org 
[root@lab ~]# vi /etc/haproxy/haproxy.cfg

# create new
 global
      # for logging section
    log         127.0.0.1 local2 info
    chroot      /var/lib/haproxy
    pidfile     /var/run/haproxy.pid
    
      # max per-process number of connections
    maxconn     256
    
      # process' user and group
    user        haproxy
    group       haproxy
    
      # makes the process fork into background
    daemon

defaults

      # running mode
    mode               http
    
      # use global settings
    log                global
    
      # get HTTP request log
    option             httplog
    
      # timeout if backends do not reply
    timeout connect    10s
    
      # timeout on client side
    timeout client     30s
    
      # timeout on server side
    timeout server     30s

# define frontend ( set any name for "http-in" section )
frontend http-in

      # listen 80
    bind *:80
    
      # set default backend
    default_backend    backend_servers
    
      # send X-Forwarded-For header
    option             forwardfor

# define backend
backend backend_servers

      # balance with roundrobin
    balance            roundrobin
    
      # define backend servers
    server             www01 10.0.0.31:80 check
    server             www02 10.0.0.32:80 check
    
[root@lab ~]# systemctl start haproxy 
[root@lab ~]# systemctl enable haproxy 
```

## 1.3 配置Rsyslog日志系统

```Shell
[root@lab ~]# vi /etc/rsyslog.conf

# line 15,16: uncomment, lne 17: add
$ModLoad imudp
$UDPServerRun 514
$AllowedSender UDP, 127.0.0.1

# line 54: change like follows
*.info;mail.none;authpriv.none;cron.none,local2.none   /var/log/messages
 local2.*                                                /var/log/haproxy.log

[root@lab ~]# systemctl restart rsyslog 
```

## 1.4 配置后端httpd日志系统

```Shell
[root@www ~]# vi /etc/httpd/conf/httpd.conf

# line 196: change like follows
LogFormat "\"%{X-Forwarded-For}i\" %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined

[root@www ~]# systemctl restart httpd 
```

# 2. SSL/TLS设置

## 2.1 创建数字证书

```Shell
[root@lab ~]# cd /etc/pki/tls/certs 
[root@lab certs]# openssl req -x509 -nodes -newkey rsa:2048 -keyout /etc/pki/tls/certs/haproxy.pem -out /etc/pki/tls/certs/haproxy.pem -days 365 
Generating a 2048 bit RSA private key
......++++++
.......++++++
writing new private key to '/etc/pki/tls/certs/haproxy.pem'
-----
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:CN# country
State or Province Name (full name) [Some-State]:Beijing   # state
Locality Name (eg, city) []:Beijing# city
Organization Name (eg, company) [Internet Widgits Pty Ltd]:CentOS   # company
Organizational Unit Name (eg, section) []:CentOS Org   # department
Common Name (eg, YOUR name) []:www.centos.org   # server's FQDN
Email Address []:admin@centos.org # admin email address

[root@lab certs]# chmod 600 haproxy.pem 
```

## 2.2 配置SSL
    
```Shell
[root@lab ~]# vi /etc/haproxy/haproxy.cfg

# add in the "global" section
global
      # max per-process number of SSL connections
    maxsslconn     256
    
      # set 2048 bits for Diffie-Hellman key
    tune.ssl.default-dh-param 2048

# add follows in the "frontend" section
frontend  http-in
    bind *:80
    
      # specify port and certs
    bind *:443 ssl crt /etc/pki/tls/certs/haproxy.pem

[root@lab ~]# systemctl restart haproxy 
```

# 3. 统计信息

## 3.1 基于Web展示统计信息

```Shell
[root@lab ~]# vi /etc/haproxy/haproxy.cfg

# add follows in the "frontend" section
frontend  http-in
    bind *:80
    
      # enable statistics reports
    stats enable
    
      # auth info for statistics site
    stats auth admin:adminpassword
    
      # hide version of HAProxy
    stats hide-version
    
      # display HAProxy hostname
    stats show-node
    
      # refresh time
    stats refresh 60s
    
      # statistics reports' URI
    stats uri /haproxy?stats

[root@lab ~]# systemctl restart haproxy 
```

## 3.2 基于命令行展示统计信息

1. 安装

```Shell
[root@lab ~]# yum -y install socat
```

2. 配置

```Shell
[root@lab ~]# vi /etc/haproxy/haproxy.cfg

# add follows in the "global" section
global
      # binds UNIX sockets
    stats socket /var/lib/haproxy/stats

[root@lab ~]# systemctl restart haproxy 
```

3. 显示统计信息

```Shell
# display current stats
[root@lab ~]# echo "show info" | socat /var/lib/haproxy/stats stdio
Name: HAProxy
Version: 1.5.2
Release_date: 2014/07/12
Nbproc: 1
Process_num: 1
Pid: 1953
...
...
Idle_pct: 100
node: www.centos.org
description:

# display stas with CSV
[root@lab ~]# echo "show stat" | socat /var/lib/haproxy/stats stdio
# pxname,svname,qcur,qmax,scur,smax,slim,stot,bin,bout,dreq,dresp,ereq,econ,eresp,.....
http-in,FRONTEND,,,0,1,2000,1,0,187,0,0,1,,,,,OPEN,,,,,,,,,1,2,0,,,,0,0,0,1,,,,0,0,.....
backend_servers,www01,0,0,0,0,,0,0,0,,0,,0,0,0,0,UP,1,1,0,0,0,67,0,,1,3,1,,0,,2,0,,.....
backend_servers,www02,0,0,0,0,,0,0,0,,0,,0,0,0,0,UP,1,1,0,0,0,67,0,,1,3,2,,0,,2,0,,.....
backend_servers,BACKEND,0,0,0,0,200,0,0,0,0,0,,0,0,0,0,UP,2,2,0,,0,67,0,,1,3,0,,0,,.....

# display current session
[root@lab ~]# echo "show sess" | socat /var/lib/haproxy/stats stdio
0x7fbc09349150: proto=tcpv4 src=10.0.0.18:55987 fe=http-in be=<NONE> srv=<none> ts=08 
age=4s calls=8 rq[f=400000h,i=0,an=1ch,rx=26s,wx=,ax=] rp[f=048200h,i=0,an=00h,rx=,wx=,ax=] 
s0=[7,8h,fd=1,ex=] s1=[0,0h,fd=2,ex=] exp=26s
0x7fbc09351d80: proto=unix_stream src=unix:1 fe=GLOBAL be=<NONE> srv=<none> ts=0b age=0s 
calls=1 rq[f=c08200h,i=0,an=00h,rx=10s,wx=,ax=] rp[f=008002h,i=0,an=00h,rx=,wx=,ax=] s0=[7,8h,fd=3,ex=] 
s1=[7,8h,fd=-1,ex=] exp=

# for other operations
# enter the interactive mode
[root@lab ~]# socat readline /var/lib/haproxy/stats 
prompt
# show help
> help
Unknown command. Please enter one of the following commands only :
  clear counters : clear max statistics counters (add 'all' for all counters)
  clear table    : remove an entry from a table
  help           : this message
  prompt         : toggle interactive mode with prompt
  quit           : disconnect
  show info      : report information about the running process
  show pools     : report information about the memory pools usage
  show stat      : report counters for each proxy and server
  show errors    : report last request and response errors for each proxy
  show sess [id] : report the list of current sessions or dump this session
  show table [id]: report table usage stats or dump this table's contents
  get weight     : report a server's current weight
  set weight     : change a server's weight
  set server     : change a server's state or weight
  set table [id] : update or create a table entry's data
  set timeout    : change a timeout setting
  set maxconn    : change a maxconn setting
  set rate-limit : change a rate limiting value
  disable        : put a server or frontend in maintenance mode
  enable         : re-enable a server or frontend which is in maintenance mode
  shutdown       : kill a session or a frontend (eg:to release listening ports)
  show acl [id]  : report avalaible acls or dump an acl's contents
  get acl        : reports the patterns matching a sample for an ACL
  add acl        : add acl entry
  del acl        : delete acl entry
  clear acl <id> : clear the content of this acl
  show map [id]  : report avalaible maps or dump a map's contents
  get map        : reports the keys and values matching a sample for a map
  set map        : modify map entry
  add map        : add map entry
  del map        : delete map entry
  clear map <id> : clear the content of this map
  set ssl <stmt> : set statement for ssl

# exit from interactive mode
> quit
```

# 4. L4负载均衡

OSI是一个开放性的通信系统互连参考模型，他是一个定义得非常好的协议规范。

OSI模型7层结是：
- 7 应用层 
- 6 表示层 
- 5 会话层 
- 4 传输层 
- 3 网络层 
- 2 数据链路层 
- 1 物理层

其中高层（即7、6、5、4层）定义了应用程序的功能，下面3层（即3、2、1层）主要面向通过网络的端到端的数据流。

## 4.1 实验环境

```
(1) [10.0.0.30]   - Frontend HAProxy
(2) [10.0.0.31]   - Backend MariaDB#1
(3) [10.0.0.32]   - Backend MariaDB#2
```

## 4.2 配置

```Shell
[root@lab ~]# vi /etc/haproxy/haproxy.cfg
 global
    log         127.0.0.1 local2 info
    chroot      /var/lib/haproxy
    pidfile     /var/run/haproxy.pid
    maxconn     256
    maxsslconn  256
    user        haproxy
    group       haproxy
    daemon

defaults
      # set "mode tcp" for Layer4
    mode               tcp
    log                global
    timeout connect    10s
    timeout client     30s
    timeout server     30s

# define frontend and backend servers
frontend  mysql-in
    bind *:3306
    default_backend    backend_servers

backend backend_servers
    balance            roundrobin
    server             db01 10.0.0.31:3306 check
    server             db02 10.0.0.32:3306 check

[root@lab ~]# systemctl restart haproxy 
```