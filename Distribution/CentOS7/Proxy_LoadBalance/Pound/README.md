- 1 POUND
    - 1.1 安装
    - 1.2 配置
    - 1.3 配置Rsyslog日志系统
    - 1.4 配置后端httpd日志系统
- 2 设置SSL/TLS
    - 2.1 创建数字证书
    - 2.2 配置SSL
- 3 URL重定向
    - 3.1 实验环境
    - 3.2 实验目的
    - 3.3 配置

# 1. POUND 

Pound程序是一个反向代理、负载均衡和支持HTTPS的前端Web服务。

实验环境：

```
(1) [10.0.0.30]   - Frontend Pound
(2) [10.0.0.51]   - Backend Web Server#1
(3) [10.0.0.52]   - Backend Web Server#2
(4) [10.0.0.53]   - Backend Web Server#3
```

## 1.1 安装

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install Pound
```

## 1.2 配置

```Shell
[root@lab ~]# mv /etc/pound.cfg /etc/pound.cfg.org 
[root@lab ~]# vi /etc/pound.cfg

 User "pound"
Group "pound"

# log level (max: 5)
LogLevel 3

# specify LogFacility
LogFacility local1

# interval of heartbeat - seconds
Alive 30

# define frontend
ListenHTTP
    Address 0.0.0.0
    Port 80
End

# define backend
Service
    BackEnd
       # backend server's IP address
        Address  10.0.0.51
        
       # backend server's port
        Port     80
        
       # set priority (value is 1-9, max 9)
        Priority 5
    End

    BackEnd
        Address  10.0.0.52
        Port     80
        Priority 5
    End

    BackEnd
        Address  10.0.0.53
        Port     80
        Priority 5
    End
End

[root@lab ~]# sed -i -e "s/^PIDFile/#PIDFile/" /usr/lib/systemd/system/pound.service 
[root@lab ~]# systemctl start pound 
[root@lab ~]# systemctl enable pound 
```

## 1.3 配置Rsyslog日志系统

```Shell
[root@lab ~]# vi /etc/rsyslog.conf

# line 54: change like follows
*.info;mail.none;authpriv.none;cron.none,local1.none   /var/log/messages
 local1.*                                                /var/log/pound.log

[root@lab ~]# systemctl restart rsyslog
```

## 1.4 配置后端httpd日志系统

```Shell
[root@www ~]# vi /etc/httpd/conf/httpd.conf

# line 196: change
LogFormat "\"%{X-Forwarded-For}i\" %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined

[root@www ~]# systemctl restart httpd 
```

# 2. 设置SSL/TLS

## 2.1 创建数字证书

```Shell
[root@lab ~]# cd /etc/pki/tls/certs 

[root@lab certs]# openssl req -x509 -nodes -newkey rsa:2048 -keyout /etc/pki/tls/certs/pound.pem -out /etc/pki/tls/certs/pound.pem 
Generating a 2048 bit RSA private key
......++++++
.......++++++
writing new private key to '/etc/pki/tls/certs/pound.pem'
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
Email Address []:admin@centos.org# admin email

[root@lab certs]# chmod 600 pound.pem
```

## 2.2 配置SSL

```Shell
[root@lab ~]# vi /etc/pound.cfg

# add like follows
ListenHTTP
    Address 0.0.0.0
    Port 80
End
 ListenHTTPS
    Address 0.0.0.0
    Port 443
    Cert "/etc/pki/tls/certs/pound.pem"
End

[root@lab ~]# systemctl restart pound 
```

# 3. URL重定向

## 3.1 实验环境

```
(1) [10.0.0.30]   - Frontend Pound
(2) [10.0.0.51]   - Backend Web Server#1
(3) [10.0.0.52]   - Backend Web Server#2
(4) [10.0.0.53]   - Backend Web Server#3
```

## 3.2 实验目的

```
访问lab.centos.org的请求转发到Backend#1
访问lab.virtual.host的请求转发到Backend#2
其他访问请求转发到Backend#3
```

## 3.3 配置

```Shell
[root@lab ~]# mv /etc/pound.cfg /etc/pound.cfg.org 
[root@lab ~]# vi /etc/pound.cfg

 User "pound"
Group "pound"
LogLevel 3
LogFacility local1
Alive 30

ListenHTTP
    Address 0.0.0.0
    Port 80
End

Service

    # define for lab.centos.org
    HeadRequire "Host: .*lab.centos.org"
    BackEnd
        Address  10.0.0.51
        Port     80
        Priority 5
    End
End

Service
    # define for lab.virtual.host
    HeadRequire "Host: .*lab.virtual.host"
    BackEnd
        Address  10.0.0.52
        Port     80
        Priority 5
    End
End

Service
    # define for others
    HeadRequire "Host: .*"
    BackEnd
        Address  10.0.0.53
        Port     80
        Priority 5
    End
End

[root@lab ~]# systemctl restart pound 
```
