- 1 Nginx
    - 1.1 安装
    - 1.2 基本配置
    - 1.3 添加防火墙策略
- 2 虚拟主机
    - 2.1 配置虚拟主机
    - 2.2 添加测试页面
- 3 启用SSL/TLS
    - 3.1 创建数字证书
    - 3.2 配置SSL/TLS
    - 3.3 添加防火墙策略
- 4 启用密码认证
- 5 反向代理
    - 5.1 将Nginx接收到的http请求转发到后端的httpd服务
    - 5.2 配置反向代理实现负载均衡
- 6 PHP-FPM
    - 6.1 安装PHP和PHP-FPM
    - 6.2 配置Nginx
    - 6.3 创建测试页面

# 1. Nginx

基本功能：

- 处理静态文件，索引文件以及自动索引；
- 反向代理加速(无缓存)，简单的负载均衡和容错；
- FastCGI，简单的负载均衡和容错；
- 模块化的结构。过滤器包括gzipping, byte ranges, chunked responses, 以及 SSI-filter 。在SSI过滤器中，到同一个 proxy 或者 FastCGI 的多个子请求并发处理；
- SSL 和 TLS SNI 支持；

结构与扩展：

- 一个主进程和多个工作进程。工作进程是单线程的，且不需要特殊授权即可运行；
- kqueue (FreeBSD 4.1+), epoll (Linux 2.6+), rt signals (Linux 2.2.19+), /dev/poll (Solaris 7 11/99+), select, 以及 poll 支持；
- kqueue支持的不同功能包括 EV_CLEAR, EV_DISABLE （临时禁止事件）， NOTE_LOWAT, EV_EOF, 有效数据的数目，错误代码；
- sendfile (FreeBSD 3.1+), sendfile (Linux 2.2+), sendfile64 (Linux 2.4.21+), 和 sendfilev (Solaris 8 7/01+) 支持；
- 输入过滤 (FreeBSD 4.1+) 以及 TCP_DEFER_ACCEPT (Linux 2.4+) 支持；
- 10,000 非活动的 HTTP keep-alive 连接仅需要 2.5M 内存。
- 最小化的数据拷贝操作；

## 1.1 安装

```Shell
# install from EPEL
[root@www ~]# yum --enablerepo=epel -y install nginx
```

## 1.2 基本配置

```Shell
[root@www ~]# vi /etc/nginx/nginx.conf

# line 38: change hostname
server_name www.centos.org;

[root@www ~]# systemctl start nginx 
[root@www ~]# systemctl enable nginx 
```

## 1.3 添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=http --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

# 2. 虚拟主机

## 2.1 配置虚拟主机

```Shell
[root@www ~]# vi /etc/nginx/conf.d/virtual.host.conf

# create new
server {
    listen       80;
    server_name  www.virtual.host;

    location / {
        root   /usr/share/nginx/virtual.host;
        index  index.html index.htm;
    }
}

[root@www ~]# mkdir /usr/share/nginx/virtual.host 
[root@www ~]# systemctl restart nginx 
```

## 2.2 添加测试页面

```Shell
[root@www ~]# vi /usr/share/nginx/virtual.host/index.html
 <html>
<body>
<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
Nginx Virtual Host Test Page
</div>
</body>
</html>
```

# 3. 启用SSL/TLS

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
subject=/C=CN/ST=Beijing/L=Beijing/O=GTS/OU=CentOS Org/CN=www.centos.org/emailAddress=admin@centos.org
Getting Private key
```

## 3.2 配置SSL/TLS

```Shell
[root@www ~]# vi /etc/nginx/nginx.conf

# add into "server" section
    server {
        listen       80 default_server;
        listen       [::]:80 default_server;
         listen       443 ssl;
         server_name  www.centos.org;
        root         /usr/share/nginx/html;

        ssl_protocols TLSv1 TLSv1.1 TLSv1.2;
        ssl_prefer_server_ciphers on;
        ssl_ciphers ECDHE+RSAGCM:ECDH+AESGCM:DH+AESGCM:ECDH+AES256:DH+AES256:ECDH+AES128:DH+AES:!aNULL!eNull:!EXPORT:!DES:!3DES:!MD5:!DSS;
        ssl_certificate      /etc/pki/tls/certs/server.crt;
        ssl_certificate_key  /etc/pki/tls/certs/server.key;

[root@www ~]# systemctl restart nginx 
```

## 3.3 添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=https --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

# 4. 启用密码认证

```Shell
[root@www ~]# yum -y install httpd-tools

[root@www ~]# vi /etc/nginx/nginx.conf

# add into "server" section
        location /auth-basic {
            auth_basic            "Basic Auth";
            auth_basic_user_file  "/etc/nginx/.htpasswd";
        }

[root@www ~]# htpasswd -c /etc/nginx/.htpasswd JerryDai 
New password:     # set password
Re-type new password: 
Adding password for user JerryDai

[root@www ~]# systemctl restart nginx 
```

# 5. 反向代理

## 5.1 将Nginx接收到的http请求转发到后端的httpd服务

1. 配置

```Shell
[root@www ~]# vi /etc/nginx/nginx.conf

# change like follows in "server" section
    server {
         listen      80 default_server;
        listen      [::]:80 default_server;
        server_name www.centos.org;

        proxy_redirect           off;
        proxy_set_header         X-Real-IP $remote_addr;
        proxy_set_header         X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header         Host $http_host;

        location / {
            proxy_pass http://node01.centos.org/;
        }
     }

[root@www ~]# systemctl restart nginx 
```

2. 配置后端httpd记录X-Forwarded-For头部信息

```Shell
[root@node01 ~]# vi /etc/httpd/conf/httpd.conf

# line 196: change
LogFormat "\"%{X-Forwarded-For}i\" %l %u %t \"%r\" %>s %b \"%{Referer}i\" \"%{User-Agent}i\"" combined

[root@node01 ~]# systemctl restart httpd 
```

## 5.2 配置反向代理实现负载均衡

1. 实验环境

```Shell
(1) www.centos.org      [10.0.0.31]   - Nginx Server
(2) node01.centos.org   [10.0.0.51]   - Backend Web Server#1
(3) node02.centos.org   [10.0.0.52]   - Backend Web Server#2
(4) node03.centos.org   [10.0.0.53]   - Backend Web Server#3
```

2. 配置

```Shell
[root@www ~]# vi /etc/nginx/nginx.conf

# add into http section
# "backup" means this server is baranced only when other servers are down
# "weight=*" means barancing weight
http {
    upstream backends {
        server node01.srv.world:80 weight=3;
        server node02.srv.world:80;
        server node03.srv.world:80 backup;
    }

# change like follows in "server" section
    server {
         listen      80 default_server;
        listen      [::]:80 default_server;
        server_name www.srv.world;

        proxy_redirect           off;
        proxy_set_header         X-Real-IP $remote_addr;
        proxy_set_header         X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header         Host $http_host;

        location / {
            proxy_pass http://backends;
        }
     }

[root@www ~]# systemctl restart nginx 
```

# 6. PHP-FPM

PHP-FPM：即PHP FastCGI Process Manager。

## 6.1 安装PHP和PHP-FPM

```Shell
# install from EPEL
[root@www ~]# yum --enablerepo=epel -y install php php-mbstring php-pear php-fpm
```

## 6.2 配置Nginx

```Shell
[root@www ~]# vi /etc/php-fpm.d/www.conf

# line 39: change
user = nginx

# line 41: change
group = nginx

[root@www ~]# systemctl start php-fpm 
[root@www ~]# systemctl enable php-fpm

[root@www ~]# vi /etc/nginx/nginx.conf

# add into "server" section
        location ~ \.php$ {
            fastcgi_pass   127.0.0.1:9000;
            fastcgi_param  SCRIPT_FILENAME $document_root$fastcgi_script_name;
            fastcgi_param  PATH_INFO $fastcgi_path_info;
            include        fastcgi_params;
        }

[root@www ~]# systemctl restart nginx 
```

## 6.3 创建测试页面

```Shell
[root@www ~]# echo "<?php phpinfo() ?>" > /usr/share/nginx/html/info.php 
```