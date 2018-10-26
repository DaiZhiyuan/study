- 1 Pen
    - 1.1 安装
    - 1.2 配置
    - 1.3 配置后端httpd日志系统
- 2 SSL设置
    - 2.1 创建数字证书
    - 2.2 更新配置
- 3 统计信息
- 4 负载均衡MariaDB
    - 4.1 实验环境
    - 4.2 配置

# 1. Pen

Pen：是轻量级负载均衡，它是基于TCP协议的，支持HTTP、SMTP、FTP、LDAP协议等。
 	
实验环境：
```
(1) www.centos.org      [10.0.0.30]   - Frontend Pen Server
(2) node01.centos.org   [10.0.0.51]   - Backend Web Server#1
(3) node02.centos.org   [10.0.0.52]   - Backend Web Server#2
(4) node03.centos.org   [10.0.0.53]   - Backend Web Server#3
```

## 1.1 安装

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install pen
```

## 1.2 配置

```Shell
[root@lab ~]# vi /etc/pen.conf

# create new
# log file
LOGFILE=/var/log/pen.log

# statics report file
WEBFILE=/var/www/pen/webstats.html

# max connections
MAX_CONNECTIONS=256

# send X-Forwarded-For header
XFORWARDEDFOR=true

# Round-Robin mode
ROUNDROBIN=true

# listening port
PORT=80

# number of backends
BACKEND=3

# define backend servers
SERVER1=10.0.0.51:80
SERVER2=10.0.0.52:80
SERVER2=10.0.0.53:80

# create an init script
[root@lab ~]# vi /etc/rc.d/init.d/pend
 #!/bin/bash

# pend: Start/Stop Pend
# chkconfig: - 90 10
# description: Pen is a light weight simple load balancer.
# pidfile: /var/run/pen.pid

. /etc/rc.d/init.d/functions
. /etc/pen.conf

LOCKFILE="/var/lock/subsys/pen"
PID=/var/run/pen.pid
PROG=/usr/bin/pen
PROGNAME=Pend

RETVAL=0
start() {
    SERVER=`grep "^SERVER" /etc/pen.conf | cut -d= -f2`
    [ $XFORWARDEDFOR = "true" ] && SERVER="-H $SERVER"
    [ $ROUNDROBIN = "true" ] && SERVER="-r $SERVER"
    [ $SSLCERTS ] && SERVER="-E $SSLCERTS $SERVER"

    echo -n $"Starting $PROGNAME: "
    daemon $PROG $PORT -w $WEBFILE -x $MAX_CONNECTIONS -p $PID -l $LOGFILE -S $BACKEND $SERVER
    RETVAL=$?
    echo
    [ $RETVAL -eq 0 ] && touch $LOCKFILE
    return $RETVAL
}
stop() {
    echo -n $"Stopping $PROGNAME: "
    killproc $PROG
    RETVAL=$?
    echo
    [ $RETVAL -eq 0 ] && rm -f $PID $LOCKFILE
    return $RETVAL
}
case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    status)
        status -p "$PID" -l $PROG $PROGNAME
        ;;
    restart)
        stop
        start
        ;;
    *)
        echo $"Usage: $0 {start|stop|status|restart}"
        exit 1
esac
exit $?

[root@lab ~]# chmod 755 /etc/rc.d/init.d/pend

# create a Systemd setting file
[root@lab ~]# vi /usr/lib/systemd/system/pen.service
 [Unit]
Description=Pend service
After=network.target

[Service]
Type=oneshot
RemainAfterExit=yes
ExecStart=/etc/rc.d/init.d/pend start
ExecStop=/etc/rc.d/init.d/pend stop

[Install]
WantedBy=multi-user.target

[root@lab ~]# systemctl start pen 
[root@lab ~]# systemctl enable pen 
```

## 1.3 配置后端httpd日志系统

```Shell
[root@www ~]# vi /etc/httpd/conf/httpd.conf

# line 196: change
LogFormat "\"%{X-Forwarded-For}i\" %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined

[root@www ~]# systemctl restart httpd 
```

# 2. SSL设置

## 2.1 创建数字证书

```Shell
[root@lab ~]# cd /etc/pki/tls/certs 

[root@lab certs]# openssl req -x509 -nodes -newkey rsa:2048 -keyout /etc/pki/tls/certs/pen.pem -out /etc/pki/tls/certs/pen.pem -days 365 
Generating a 2048 bit RSA private key
......++++++
.......++++++
writing new private key to '/etc/pki/tls/certs/pen.pem'
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
Organizational Unit Name (eg, section) []:IT Solution   # department
Common Name (eg, YOUR name) []:www.centos.org   # server's FQDN
Email Address []:admin@centos.org# admin email

[root@lab certs]# chmod 600 pen.pem 
```

## 2.2 更新配置

```Shell
[root@lab ~]# vi /etc/pen.conf

# change port and specify certificates
PORT=443
SSLCERTS=/etc/pki/tls/certs/pen.pem

[root@lab ~]# systemctl restart pen 
```

# 3. 统计信息

```Shell
[root@lab ~]# cp /usr/share/doc/pen-*/penstats /var/www/pen 

[root@lab ~]# vi /var/www/pen/penstats

# line 4,5: change
PIDFILE=/var/run/pen.pid
WEBFILE=/var/www/pen/webstats.html

[root@lab ~]# vi /etc/httpd/conf.d/pen.conf
# change like follows
Alias /pen/ /var/www/pen/
<Directory /var/www/pen/>
    DirectoryIndex webstats.html
    
    #Options ExecCGI
    <IfModule mod_authz_core.c>
        # Apache 2.4
        # add access permission
        Require local
        Require ip 10.0.0.0/24
    </IfModule>
    
[root@lab ~]# systemctl restart httpd 

[root@lab ~]# chmod 755 /var/www/pen/penstats 

# generate statics
[root@lab ~]# /var/www/pen/penstats > /dev/null 

# add to Cron
[root@lab ~]# echo '*/5 * * * * /var/www/pen/penstats > /dev/null' > /etc/cron.d/pend 
```

> 访问 http://(Pen server's hostname or IP address):(httpd listening port)/pen/ 查看统计信息

# 4. 负载均衡MariaDB

## 4.1 实验环境

```Shell
(1) [10.0.0.30] - Frontend Pen Server
(2) [10.0.0.51] - Backend MariaDB#1
(3) [10.0.0.52] - Backend MariaDB#2
```
## 4.2 配置

```Shell
[root@lab ~]# vi /etc/pen.conf

# create new
LOGFILE=/var/log/pen.log
WEBFILE=/var/www/pen/webstats.html
MAX_CONNECTIONS=256
ROUNDROBIN=true

# listening port
PORT=3306

# number of backends
BACKEND=2

# define backend servers
SERVER1=10.0.0.51:3306
SERVER2=10.0.0.52:3306

[root@lab ~]# systemctl restart pen 
```