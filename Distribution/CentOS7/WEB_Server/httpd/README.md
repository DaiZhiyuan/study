- 1 安装配置httpd
- 2 CGI
    - 2.1 基于Perf的CGI
    - 2.2 基于PHP的CGI
    - 2.3 基于Ruby的CGI
    - 2.4 基于Python的CGI
- 3 启用用户目录
- 4 虚拟主机
- 5 配置SSL/TLS
- 6 认证
    - 6.1 基于密码认证
    - 6.2 基于密码加PAM认证
    - 6.3 基于密码加LDAP认证
- 7 应用WebDAV
- 8 FastCGI
    - 8.1 Perl加mod_perl
    - 8.2 PHP加PHP-FPM
    - 8.3 Python加mod_wsgi
- 9 模块
    - 9.1 mod_proxy
        - 9.1.1 开启转发功能
        - 9.1.2 配置load balancing
    - 9.2 mod_ratelimit
    - 9.3 mod_evasive

# 1. 安装配置httpd

1. 安装httpd

```Shell
[root@www ~]# yum -y install httpd
# remove welcome page
[root@www ~]# rm -f /etc/httpd/conf.d/welcome.conf
```

2. 配置httpd

```Shell
[root@www ~]# vi /etc/httpd/conf/httpd.conf

# line 86: change to admin's email address
ServerAdmin root@centos.org

# line 95: change to your server's name
ServerName www.centos.org:80

# line 151: change
AllowOverride All

# line 164: add file name that it can access only with directory's name
DirectoryIndex index.html index.cgi index.php
# add follows to the end

# server's response header
ServerTokens Prod

# keepalive is ON
KeepAlive On

[root@www ~]# systemctl start httpd 
[root@www ~]# systemctl enable httpd 
```

3. 添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=http --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

4. 创建测试index页面

```Html
[root@www ~]# vi /var/www/html/index.html
<html>
    <body>
        <div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
            Test Page
        </div>
    </body>
</html>
```
# 2. CGI

## 2.1 基于Perf的CGI

1. 安装Perf

```Shell
[root@www ~]# yum -y install perl perl-CGI
```

2. 创建配置文件

```Shell
# the settings below is the one for CGI
[root@www ~]# grep -n "^ *ScriptAlias" /etc/httpd/conf/httpd.conf 
247: ScriptAlias /cgi-bin/ "/var/www/cgi-bin/"

[root@www ~]# vi /etc/httpd/conf.d/cgi-enabled.conf

# create new
# processes .cgi and .pl as CGI scripts
<Directory "/var/www/html/cgi-enabled">
    Options +ExecCGI
    AddHandler cgi-script .cgi .pl
</Directory>

[root@www ~]# systemctl restart httpd 
```

3. 更改SELinux上下文

```Shell
[root@www ~]# chcon -R -t httpd_sys_script_exec_t /var/www/html/cgi-enabled 
[root@www ~]# semanage fcontext -a -t httpd_sys_script_exec_t /var/www/html/cgi-enabled 
```

4. 创建一个CGI测试页面

```Perl
[root@www ~]# vi /var/www/html/cgi-enabled/index.cgi
 #!/usr/bin/perl

print "Content-type: text/html\n\n";
print "<html>\n<body>\n";
print "<div style=\"width: 100%; font-size: 40px; font-weight: bold; text-align: center;\">\n";
print "CGI Test Page";
print "\n</div>\n";
print "</body>\n</html>\n";

[root@www ~]# chmod 705 /var/www/html/cgi-enabled/index.cgi 
```

## 2.2 基于PHP的CGI

1. 安装PHP

```Shell
[root@www ~]# yum -y install php php-mbstring php-pear

[root@www ~]# vi /etc/php.ini

# line 878: uncomment and add your timezone
date.timezone = "Asia/Shanghai"

[root@www ~]# systemctl restart httpd 
```

2. 创建一个CGI测试页面

```PHP
[root@www ~]# vi /var/www/html/index.php
 <html>
<body>
<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
<?php
   print Date("Y/m/d");
?>
</div>
</body>
</html>
```

## 2.3 基于Ruby的CGI

1. 安装Ruby

```Shell
[root@www ~]# yum -y install ruby
```

2. 创建配置文件

```Shell
# the settings below is the one for CGI
[root@www ~]# grep -n "^ *ScriptAlias" /etc/httpd/conf/httpd.conf 
247: ScriptAlias /cgi-bin/ "/var/www/cgi-bin/"

[root@www ~]# vi /etc/httpd/conf.d/cgi-enabled.conf

# create new
# processes .rb as CGI scripts
<Directory "/var/www/html/cgi-enabled">
    Options +ExecCGI
    AddHandler cgi-script .rb
</Directory>

[root@www ~]# systemctl restart httpd 
```

3. 更新SElinux上下文

```Shell
[root@www ~]# chcon -R -t httpd_sys_script_exec_t /var/www/html/cgi-enabled 
[root@www ~]# semanage fcontext -a -t httpd_sys_script_exec_t /var/www/html/cgi-enabled 
```

4. 创建一个CGI测试页面

```Ruby
[root@www ~]# vi /var/www/html/cgi-enabled/index.rb
 #!/usr/bin/ruby

print "Content-type: text/html\n\n"
print "<html>\n<body>\n"
print "<div style=\"width: 100%; font-size: 40px; font-weight: bold; text-align: center;\">\n"
print "Ruby Script Test Page"
print "\n</div>\n"
print "</body>\n</html>\n" 

[root@www ~]# chmod 705 /var/www/html/cgi-enabled/index.rb 
```

## 2.4 基于Python的CGI

1. 安装Python

```Shell
[root@www ~]# yum -y install python
```

2. 创建配置文件

```Shell
# the settings below is the one for CGI
[root@www ~]# grep -n "^ *ScriptAlias" /etc/httpd/conf/httpd.conf 
247: ScriptAlias /cgi-bin/ "/var/www/cgi-bin/"

[root@www ~]# vi /etc/httpd/conf.d/cgi-enabled.conf
# create new
# processes .py as CGI scripts
<Directory "/var/www/html/cgi-enabled">
    Options +ExecCGI
    AddHandler cgi-script .py
</Directory>
[root@www ~]# systemctl restart httpd 
```

3. 更新SElinux上下文

```Shell
[root@www ~]# chcon -R -t httpd_sys_script_exec_t /var/www/html/cgi-enabled 
[root@www ~]# semanage fcontext -a -t httpd_sys_script_exec_t /var/www/html/cgi-enabled 
```

4. 创建一个CGI测试页面

```Python
[root@www ~]# vi /var/www/html/cgi-enabled/index.py
 #!/usr/bin/env python

print "Content-type: text/html\n\n"
print "<html>\n<body>"
print "<div style=\"width: 100%; font-size: 40px; font-weight: bold; text-align: center;\">"
print "Python Script Test Page"
print "</div>\n</body>\n</html>"

[root@www ~]# chmod 705 /var/www/html/cgi-enabled/index.py 
```

# 3. 启用用户目录

1. 配置httpd

```Shell
[root@www ~]# vi /etc/httpd/conf.d/userdir.conf

# line 17: comment out
#UserDir disabled

# line 24: uncomment
UserDir public_html

# line 31 - 35
<Directory "/home/*/public_html">
    AllowOverride All# change
    Options None# change
    Require method GET POST OPTIONS
</Directory>

[root@www ~]# systemctl restart httpd 
```

2. 创建基于用户访问的测试页面

```Shell
[JerryDai@www ~]$ mkdir public_html 
[JerryDai@www ~]$ chmod 711 /home/JerryDai 
[JerryDai@www ~]$ chmod 755 /home/JerryDai/public_html 
[JerryDai@www ~]$ vi ./public_html/index.html
 <html>
<body>
<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
UserDir Test Page
</div>
</body>
</html>
```

# 4. 虚拟主机

1. 配置虚拟主机

```Shell
[root@www ~]# vi /etc/httpd/conf.d/vhost.conf

# create new
# for original domain
<VirtualHost *:80>
   DocumentRoot /var/www/html
   ServerName www.centos.org
</VirtualHost>

# for virtual domain
<VirtualHost *:80>
   DocumentRoot /home/cent/public_html
   ServerName www.virtual.host
   ServerAdmin webmaster@virtual.host
   ErrorLog logs/virtual.host-error_log
   CustomLog logs/virtual.host-access_log combined
</VirtualHost>

[root@www ~]# systemctl restart httpd 
```

2. 创建测试页面

```PHP
[JerryDai@www ~]$ vi ~/public_html/virtual.php
<html>
<body>
<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
Virtual Host Test Page
</div>
</body>
</html>
```

# 5. 配置SSL/TLS

1. 配置httpd

```Shell
[root@www ~]# yum -y install mod_ssl

[root@www ~]# vi /etc/httpd/conf.d/ssl.conf

# line 59: uncomment
DocumentRoot "/var/www/html"

# line 60: uncomment and specify the server name
ServerName www.centos.org:443

# line 75: change
SSLProtocol -All +TLSv1 +TLSv1.1 +TLSv1.2
SSLCertificateFile /etc/letsencrypt/live/www.centos.org/cert.pem
SSLCertificateKeyFile /etc/letsencrypt/live/www.centos.org/privkey.pem
SSLCertificateChainFile /etc/letsencrypt/live/www.centos.org/chain.pem

[root@www ~]# systemctl restart httpd 
```

2. 创建一个虚拟主机

```Shell
[root@www ~]# vi /etc/httpd/conf.d/vhost.conf

<VirtualHost *:80>
    DocumentRoot /var/www/html
    ServerName www.centos.org
    RewriteEngine On
    RewriteCond %{HTTPS} off
    RewriteRule ^(.*)$ https://%{HTTP_HOST}%{REQUEST_URI} [R=301,L]
</VirtualHost>

[root@www ~]# systemctl restart httpd 
```

3. 添加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-service=https --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

# 6. 认证

## 6.1 基于密码认证

```Shell
[root@www ~]# vi /etc/httpd/conf.d/auth_basic.conf

# create new
<Directory /var/www/html/auth-basic>
    AuthType Basic
    AuthName "Basic Authentication"
    AuthUserFile /etc/httpd/conf/.htpasswd
    require valid-user
</Directory>

# add a user : create a new file with "-c" ( add the "-c" option only for the initial registration )
[root@www ~]# htpasswd -c /etc/httpd/conf/.htpasswd JerryDai 
New password: # set password
Re-type new password: # confirm
Adding password for user JerryDai
[root@www ~]# systemctl restart httpd

[root@www ~]# mkdir /var/www/html/auth-basic 
[root@www ~]# vi /var/www/html/auth-basic/index.html
# create a test page
 <html>
<body>
<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
Test Page for Basic Auth
</div>
</body>
</html>

```

## 6.2 基于密码加PAM认证

```Shell
# install from EPEL
[root@www ~]# yum --enablerepo=epel -y install mod_authnz_external pwauth
[root@www ~]# vi /etc/httpd/conf.d/authnz_external.conf

# add to the end
 <Directory /var/www/html/auth-pam>
    SSLRequireSSL
    AuthType Basic
    AuthName "PAM Authentication"
    AuthBasicProvider external
    AuthExternal pwauth
    require valid-user
</Directory>

[root@www ~]# mkdir /var/www/html/auth-pam 
[root@www ~]# vi /var/www/html/auth-pam/index.html

# create a test page
 <html>
<body>
<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
Test Page for PAM Auth
</div>
</body>
</html>

[root@www ~]# systemctl restart httpd
```

## 6.3 基于密码加LDAP认证

```Shell
[root@www ~]# yum -y install mod_ldap

[root@www ~]# vi /etc/httpd/conf.d/auth_ldap.conf

# add follows to the end
 <Directory /var/www/html/auth-ldap>
    SSLRequireSSL
    AuthName "LDAP Authentication"
    AuthType Basic
    AuthBasicProvider ldap
    AuthLDAPURL ldap://dlp.centos.org/dc=centos,dc=org?uid?sub?(objectClass=*)
    Require ldap-filter objectClass=posixAccount
</Directory>

# create a test page
[root@www ~]# mkdir /var/www/html/auth-ldap 
[root@www ~]# vi /var/www/html/auth-ldap/index.html
 <html>
<body>
<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">
Test Page for LDAP Auth
</div>
</body>
</html>

[root@www ~]# systemctl restart httpd
```

# 7. 应用WebDAV

1. 安装配置WebDAV

```Shell
[root@www ~]# mkdir /home/webdav 
[root@www ~]# chown apache. /home/webdav 
[root@www ~]# chmod 770 /home/webdav 
[root@www ~]# vi /etc/httpd/conf.d/webdav.conf

# create new
DavLockDB "/tmp/DavLock"
Alias /webdav /home/webdav
<Location /webdav>
    DAV On
    SSLRequireSSL
    Options None
    AuthType Basic
    AuthName WebDAV
    AuthUserFile /etc/httpd/conf/.htpasswd
    <RequireAny>
        Require method GET POST OPTIONS
        Require valid-user
    </RequireAny>
</Location>

# add a user : create a new file with "-c" ( add the "-c" option only for the initial registration )
[root@www ~]# htpasswd -c /etc/httpd/conf/.htpasswd JerryDai 
New password:     # set password
Re-type new password:
Adding password for user JerryDai

[root@www ~]# systemctl restart httpd 
```

2. 变更SELinux上下文

```Shell
[root@www ~]# chcon -R -t httpd_sys_rw_content_t /home/webdav 
[root@www ~]# semanage fcontext -a -t httpd_sys_rw_content_t /home/webdav 
```

3. WebDAV客户端

> CarotDAV

# 8. FastCGI

## 8.1 Perl加mod_perl

1. 安装配置

```
# install from EPEL
[root@www ~]# yum --enablerepo=epel -y install mod_perl

[root@www ~]# vi /etc/httpd/conf.d/perl.conf

# line 15: uncomment ( check codes and output warnings to logs )
PerlSwitches -w

# line 24: uncomment
PerlSwitches -T

# line 30-36: uncomment like follows
Alias /perl /var/www/perl

<Directory /var/www/perl> # the directory for mod_perl environment
    SetHandler perl-script # processes files as perl-scripts under this directory
#    AddHandler perl-script .cgi # set specific extension if do do not want to processes all files as CGI
#    PerlResponseHandler ModPerl::Registry
    PerlResponseHandler ModPerl::PerlRun # specify PerlRun mode
    PerlOptions +ParseHeaders
    Options +ExecCGI
</Directory>

# line 43-49: uncomment and add like follows
<Location /perl-status>
    SetHandler perl-script
    PerlResponseHandler Apache2::Status
    Require ip 127.0.0.1 10.0.0.0/24 # add access permission
#    Order deny,allow
#    Deny from all
#    Allow from .example.com
</Location>
[root@www ~]# systemctl restart httpd 
```

2. 创建测试脚本

```perl
[root@www ~]# mkdir /var/www/perl 
[root@www ~]# vi /var/www/perl/test-mod_perl.cgi
 #!/usr/bin/perl

use strict;
use warnings;

print "Content-type: text/html\n\n";
print "<html>\n<body>\n";
print "<div style=\"width:100%; font-size:40px; font-weight:bold; text-align:center;\">";

my $a = 0;
&number();

print "</div>\n</body>\n</html>";

sub number {
    $a++;
    print "number \$a = $a";
}

[root@www ~]# chmod 705 /var/www/perl/test-mod_perl.cgi 
```

## 8.2 PHP加PHP-FPM

1. 安装配置

```Shell
[root@www ~]# yum -y install php-fpm

[root@www ~]# vi /etc/httpd/conf.d/php.conf

# line 5: change like follows
<FilesMatch \.php$>

#    SetHandler application/x-httpd-php
    SetHandler "proxy:fcgi://127.0.0.1:9000" 
</FilesMatch>

[root@www ~]# systemctl start php-fpm 
[root@www ~]# systemctl enable php-fpm 
[root@www ~]# systemctl restart httpd 
```

2. 创建测试页面

```php
[root@www ~]# echo '<?php phpinfo(); ?>' > /var/www/html/info.php 
```

## 8.3 Python加mod_wsgi

1. 安装配置

```Shell
[root@www ~]# yum -y install mod_wsgi

[root@www ~]# vi /etc/httpd/conf.d/wsgi.conf

# create new
WSGIScriptAlias /test_wsgi /var/www/html/test_wsgi.py

[root@www ~]# systemctl restart httpd 
```

2. 创建测试脚本

```Python
[root@www ~]# vi /var/www/html/test_wsgi.py

# create new
 def application(environ,start_response):
    status = '200 OK'
    html = '<html>\n' \
           '<body>\n' \
           '<div style="width: 100%; font-size: 40px; font-weight: bold; text-align: center;">\n' \
           'mod_wsgi Test Page\n' \
           '</div>\n' \
           '</body>\n' \
           '</html>\n'
    response_header = [('Content-type','text/html')]
    start_response(status,response_header)
    return [html]
```

# 9. 模块

## 9.1 mod_proxy

### 9.1.1 开启转发功能

1. 配置

```Shell

# module is enabled by default
[root@www ~]# grep "mod_proxy" /etc/httpd/conf.modules.d/00-proxy.conf 
LoadModule proxy_module modules/mod_proxy.so
LoadModule proxy_ajp_module modules/mod_proxy_ajp.so
.....
LoadModule proxy_http_module modules/mod_proxy_http.so
LoadModule proxy_scgi_module modules/mod_proxy_scgi.so

[root@www ~]# vi /etc/httpd/conf.d/f_proxy.conf
# create new
<IfModule mod_proxy.c>
    # forward proxy function ON
    ProxyRequests On
    <Proxy *>
        # access permission
        Require ip 127.0.0.1 10.0.0.0/24
    </Proxy>
</IfModule>

[root@www ~]# vi /etc/httpd/conf/httpd.conf

# line 42: change listening port
Listen 8080
[root@www ~]# systemctl restart httpd 
```

2. 增加防火墙策略

```Shell
[root@www ~]# firewall-cmd --add-port=8080/tcp --permanent 
success
[root@www ~]# firewall-cmd --reload 
success
```

3. 变更SELinux策略

```Shell
[root@www ~]# setsebool -P httpd_can_network_relay on 
```

### 9.1.2 配置load balancing

1. 测试环境
```
(1) www.centos.org        [10.0.0.31]     - Web Server#1
(2) node01.centos.org     [10.0.0.51]     - Web Server#2
(3) node02.centos.org     [10.0.0.52]     - Web Server#3
```

2. 配置
```Shell
[root@www ~]# vi /etc/httpd/conf.d/r_proxy.conf

# create new
<IfModule mod_proxy.c>
    ProxyRequests Off
    <Proxy *>
        Require all granted
    </Proxy>
    # specify the way of load balancing with "lbmethod". it's also possible to set "bytraffic".
    ProxyPass / balancer://cluster lbmethod=byrequests
    <proxy balancer://cluster>
        BalancerMember http://node01.centos.org/ loadfactor=1
        BalancerMember http://node02.centos.org/ loadfactor=1
    </proxy>
</IfModule>

[root@www ~]# systemctl restart httpd 
```

## 9.2 mod_ratelimit

**限制客户端带宽**

```Shell
[root@www ~]# vi /etc/httpd/conf.modules.d/00-base.conf

# line 72: uncomment
LoadModule ratelimit_module modules/mod_ratelimit.so

[root@www ~]# vi /etc/httpd/conf.d/ratelimit.conf

# create new
# for example, limit bandwidth as 500 KB/sec under the /download location
<IfModule mod_ratelimit.c>
    <Location /download>
        SetOutputFilter RATE_LIMIT
        SetEnv rate-limit 500
    </Location>
</IfModule>
[root@www ~]# systemctl restart httpd 
```

## 9.3 mod_evasive

**防止DDOS攻击**

1. 配置
```Shell
# install from EPEL
[root@www ~]# yum --enablerepo=epel -y install mod_evasive

[root@www ~]# vi /etc/httpd/conf.d/mod_evasive.conf

# line 18: threshhold for the number of requests for the same page per page interval
DOSPageCount   5

# line 24: threshhold for the total number of requests for any object by the same client on the same listener per site interval
DOSSiteCount   50

# line 28: The interval for the page count threshhold
DOSPageInterval   1

# line 32: The interval for the site count threshhold
DOSSiteInterval   1

# line 41: amount of time (in seconds) that a client will be blocked for if they are added to the blocking list
DOSBlockingPeriod   300

# line 48: notification address if IP address becomes blacklisted
DOSEmailNotify   root@localhost

# line 66: specify log directory
DOSLogDir   "/var/log/mod_evasive"

[root@www ~]# mkdir /var/log/mod_evasive 
[root@www ~]# chown apache. /var/log/mod_evasive 
[root@www ~]# systemctl restart httpd 
```

2. 测试

```Shell
[root@www ~]# perl /usr/share/doc/mod_evasive-*/test.pl 
HTTP/1.1 200 OK
HTTP/1.1 200 OK
HTTP/1.1 200 OK
HTTP/1.1 200 OK
HTTP/1.1 200 OK
.....
.....
HTTP/1.1 403 Forbidden
HTTP/1.1 403 Forbidden
HTTP/1.1 403 Forbidden
.....
.....
HTTP/1.1 403 Forbidden

# turn to "403 Forbidden" if blocked
# logs are saved
[root@www ~]# ll /var/log/mod_evasive 
total 4
-rw-r--r-- 1 apache apache 5 Aug  5 15:42 dos-127.0.0.1

# if set notification, it is sent like follows
[root@www ~]# mail 
Heirloom Mail version 12.5 7/5/10.  Type ? for help.
"/var/spool/mail/root": 1 message 1 new
>N  1 Apache                Wed Aug  3 19:42  20/673
& 1
Message  1:
From apache@centos.org  Wed Aug  3 19:42:55 2015
Return-Path: <apache@www.centos.org>
X-Original-To: root@localhost
Delivered-To: root@localhost.centos.org
Date: Wed, 05 Aug 2015 15:42:54 +0900
To: root@localhost.centos.org
User-Agent: Heirloom mailx 12.5 7/5/10
Content-Type: text/plain; charset=us-ascii
From: apache@www.centos.org (Apache)
Status: R

To: root@localhost
Subject: HTTP BLACKLIST 127.0.0.1

mod_evasive HTTP Blacklisted 127.0.0.1
```