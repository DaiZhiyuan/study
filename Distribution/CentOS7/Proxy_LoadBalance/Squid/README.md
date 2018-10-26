- 1 Squid
    - 1.1 安装
    - 1.2 配置正向代理
    - 1.3 添加防火墙策略
    - 1.4 配置代理客户端
- 2 添加密码认证
    - 2.1 安装软件包
    - 2.2 更新Squid配置
    - 2.3 更新客户端配置
- 3 反向代理
- 4 Squid + SquidClamav
    - 4.1 安装Clamav
    - 4.2 安装Clamav Scanner
    - 4.3 安装c-icap
    - 4.4 安装SquidClamav
- 5 Squid + SquidGuard
    - 5.1 安装配置

# 1. Squid

Squid是一个高性能的代理缓存服务器，Squid支持FTP、gopher、HTTPS和HTTP协议。和一般的代理缓存软件不同，Squid用一个单独的、非模块化的、I/O驱动的进程来处理所有的客户端请求。

## 1.1 安装

```Shell
[root@lab ~]# yum -y install squid
```

## 1.2 配置正向代理

```Shell
[root@lab ~]# vi /etc/squid/squid.conf

acl CONNECT method CONNECT
# line 26: add ( define new ACL )
acl lan src 10.0.0.0/24
http_access allow localhost

# line 54: add ( allow defined ACL above )
http_access allow lan

# add follows to the end
request_header_access Referer deny all
request_header_access X-Forwarded-For deny all
request_header_access Via deny all
request_header_access Cache-Control deny all

# do not display IP address
forwarded_for off

[root@lab ~]# systemctl start squid 
[root@lab ~]# systemctl enable squid 
```

## 1.3 添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-service=squid --permanent 
success
[root@lab ~]# firewall-cmd --reload 
success
```

## 1.4 配置代理客户端

```Shell
[root@client ~]# vi /etc/profile

# add follows to the end (set proxy settings to the environment variables)
MY_PROXY_URL="http://lab.centos.org:3128/"
HTTP_PROXY=$MY_PROXY_URL
HTTPS_PROXY=$MY_PROXY_URL
FTP_PROXY=$MY_PROXY_URL
http_proxy=$MY_PROXY_URL
https_proxy=$MY_PROXY_URL
ftp_proxy=$MY_PROXY_URL
export HTTP_PROXY HTTPS_PROXY FTP_PROXY http_proxy https_proxy ftp_proxy
[root@client ~]# source /etc/profile

# it's OK all, but it's possible to set proxy settings for each application like follows
# for yum
[root@client ~]# vi /etc/yum.conf
# add to the end
proxy=http://lab.centos.org:3128/

# for wget
[root@client ~]# vi /etc/wgetrc
# add to the end
http_proxy = http://lab.centos.org:3128/
https_proxy = http://lab.centos.org:3128/
ftp_proxy = http://lab.centos.org:3128/
```

# 2. 添加密码认证

## 2.1 安装软件包

```Shell
[root@lab ~]# yum -y install httpd-tools
```

## 2.2 更新Squid配置

```Shell
[root@lab ~]# vi /etc/squid/squid.conf

acl CONNECT method CONNECT
# line 26: add follows for Basic Auth
auth_param basic program /usr/lib64/squid/basic_ncsa_auth /etc/squid/.htpasswd
auth_param basic children 5
auth_param basic realm Squid Basic Authentication
auth_param basic credentialsttl 5 hours
acl password proxy_auth REQUIRED
http_access allow password

# add a user : create a new file with "-c" ( add the "-c" option only for the initial registration )
[root@lab ~]# htpasswd -c /etc/squid/.htpasswd JerryDai 
New password:     # set password
Re-type new password: 
Adding password for user JerryDai

[root@lab ~]# systemctl restart squid 
```

## 2.3 更新客户端配置

```Shell
[root@client ~]# vi /etc/profile

# add follows to the end
# username:password@proxyserver:port
MY_PROXY_URL="http://JerryDai:password@lab.centos.org:3128/"
HTTP_PROXY=$MY_PROXY_URL
HTTPS_PROXY=$MY_PROXY_URL
FTP_PROXY=$MY_PROXY_URL
http_proxy=$MY_PROXY_URL
https_proxy=$MY_PROXY_URL
ftp_proxy=$MY_PROXY_URL
export HTTP_PROXY HTTPS_PROXY FTP_PROXY http_proxy https_proxy ftp_proxy
[root@client ~]# source /etc/profile

# it's OK all, but it's possible to set proxy settings for each application like follows
# for yum
[root@client ~]# vi /etc/yum.conf
# add follows to the end
proxy=http://lab.centos.org:3128/
proxy_username=JerryDai
proxy_password=password

# for wget
[root@client ~]# vi /etc/wgetrc
# add follows to the end
http_proxy = http://lab.centos.org:3128/
https_proxy = http://lab.centos.org:3128/
ftp_proxy = http://lab.centos.org:3128/
proxy_user = JerryDai
proxy_passwd = password
```

# 3. 反向代理

```Shell
[root@lab ~]# vi /etc/squid/squid.conf

# line 54: add (allow all http access)
http_access allow all 

# And finally deny all other access to this proxy
http_access deny all

# line 59: specify the backend Web server
http_port 80 accel defaultsite=www.centos.org

# line 62: uncommnet
# number means ⇒ [disk cache size] [number of directories on top level] [number of directories on 2nd level]
cache_dir ufs /var/spool/squid 100 16 256

# add to the end
cache_peer www.centos.org parent 80 0 no-query originserver

# memory cache size
cache_mem 256 MB

# define hostname
visible_hostname lab.centos.org

[root@lab ~]# systemctl start squid 
[root@lab ~]# systemctl enable squid 
```

# 4. Squid + SquidClamav

SquidClamAv是一个专为 Squid 代理服务器提供的用ClamAV的病毒重定向工具。它可以运行在文件名正则，内容类型正则表达式的防病毒检查，等等。它易于安装和工程甚至高负荷的Squid访问。

## 4.1 安装Clamav

1. 安装配置

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install clamav clamav-update
[root@lab ~]# sed -i -e "s/^Example/#Example/" /etc/freshclam.conf

# update pattern files
[root@lab ~]# freshclam
ClamAV update process started at Fri Aug 29 22:03:30 2014
main.cld is up to date (version: 55, sigs: 2424225, f-level: 60, builder: neo)
daily.cvd is up to date (version: 19314, sigs: 1094505, f-level: 63, builder: neo)
bytecode.cvd is up to date (version: 242, sigs: 46, f-level: 63, builder: dgoddard)
```

2. 尝试扫描

```Shell
[root@lab ~]# clamscan --infected --remove --recursive /home
----------- SCAN SUMMARY -----------
Known viruses: 3575245
Engine version: 0.98.4
Scanned directories: 2
Scanned files: 3
Infected files: 0
Data scanned: 0.00 MB
Data read: 0.00 MB (ratio 0.00:1)
Time: 10.369 sec (0 m 10 s)

# download trial virus
[root@lab ~]# curl -O http://www.eicar.org/download/eicar.com
[root@lab ~]# clamscan --infected --remove --recursive .
./eicar.com: Eicar-Test-Signature FOUND
./eicar.com: Removed. # just detected
----------- SCAN SUMMARY -----------
Known viruses: 3575245
Engine version: 0.98.4
Scanned directories: 3
Scanned files: 10
Infected files: 1
Data scanned: 0.00 MB
Data read: 256.57 MB (ratio 0.00:1)
Time: 10.307 sec (0 m 10 s)
```

## 4.2 安装Clamav Scanner

1. 安装配置

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install clamav-scanner clamav-scanner-systemd

[root@lab ~]# vi /etc/clamd.d/scan.conf

# line 8: comment out
#Example

# line 14: uncomment
LogFile /var/log/clamd.scan

# line 66: uncomment
PidFile /var/run/clamd.scan/clamd.pid

# line 70: uncomment
TemporaryDirectory /var/tmp

# line 85: uncomment
LocalSocket /var/run/clamd.scan/clamd.sock

# line 101: uncomment
TCPSocket 3310

[root@lab ~]# touch /var/log/clamd.scan 
[root@lab ~]# chown clamscan. /var/log/clamd.scan 
[root@lab ~]# systemctl start clamd@scan 
[root@lab ~]# systemctl enable clamd@scan 
```

2. 变更SElinux上下文

```Shell
[root@lab ~]# restorecon -v /var/log/clamd.scan
```

## 4.3 安装c-icap

```Shell
[root@lab ~]# yum -y install gcc make 
[root@lab ~]# curl -L -O http://downloads.sourceforge.net/project/c-icap/c-icap/0.4.x/c_icap-0.4.2.tar.gz
[root@lab ~]# tar zxvf c_icap-0.4.2.tar.gz 
[root@lab ~]# cd c_icap-0.4.2 
[root@lab c_icap-0.4.2]# ./configure 
[root@lab c_icap-0.4.2]# make
[root@lab c_icap-0.4.2]# make install 
[root@lab c_icap-0.4.2]# cd 
[root@lab ~]# cp /usr/local/etc/c-icap.conf /etc 
[root@lab ~]# vi /etc/c-icap.conf

# line 161: change admin address
ServerAdmin admin@centos.org

# line 170: change hostname
ServerName lab.centos.org

# line 568: add
Service squidclamav squidclamav.so

[root@lab ~]# vi /etc/tmpfiles.d/c-icap.conf
# create new
d /var/run/c-icap 0755 root root -

[root@lab ~]# vi /usr/lib/systemd/system/c-icap.service
# create new
[Unit]
Description=c-icap service
After=network.target

[Service]
Type=forking
PIDFile=/var/run/c-icap/c-icap.pid
ExecStart=/usr/local/bin/c-icap -f /etc/c-icap.conf
KillMode=process

[Install]
WantedBy=multi-user.target
```

## 4.4 安装SquidClamav

1. 安装配置

```Shell
[root@lab ~]# curl -L -O http://downloads.sourceforge.net/project/squidclamav/squidclamav/6.14/squidclamav-6.14.tar.gz 
[root@lab ~]# tar zxvf squidclamav-6.14.tar.gz 
[root@lab ~]# cd squidclamav-6.14 
[root@lab squidclamav-6.14]# ./configure --with-c-icap 
[root@lab squidclamav-6.14]# make
[root@lab squidclamav-6.14]# make install 
[root@lab squidclamav-6.14]# cd 
[root@lab ~]# ln -s /usr/local/etc/squidclamav.conf /etc/squidclamav.conf 
[root@lab ~]# vi /etc/squidclamav.conf

# line 18: change( destination URL for redirect. Create it first )
redirect http://www.centos.org/error.html

# line 27: change( same with clamd )
clamd_local /var/run/clamd.scan/clamd.sock
```

2. 配置Squid

```Shell
[root@lab ~]# vi /etc/squid/squid.conf

# add follows to the end
icap_enable on
icap_send_client_ip on
icap_send_client_username on
icap_client_username_header X-Authenticated-User
icap_service service_req reqmod_precache bypass=1 icap://127.0.0.1:1344/squidclamav
adaptation_access service_req allow all
icap_service service_resp respmod_precache bypass=1 icap://127.0.0.1:1344/squidclamav
adaptation_access service_resp allow all

[root@lab ~]# systemctl start c-icap 
[root@lab ~]# systemctl enable c-icap 
[root@lab ~]# systemctl restart squid 
```

3. 测试

```
在客户端浏览器设置正向代理并访问http://eicar.org/85-0-Download.html。

期望返回：
        Access Denied！ 
        Virus detected！
```

# 5. Squid + SquidGuard

网络管理中，经常面临的问题是如何过滤互联网中的有害内容或屏蔽特定网站，则可以通过Squid和SquidGuard来进行过滤。Squid是一个代理软件，SquidGuard是一个URL重定向器，用于在Squid中使用黑名单和白名单。 Squidguard有两大优势：速度快，免费。

## 5.1 安装配置

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install squidGuard

[root@lab ~]# mv /etc/squid/squidGuard.conf /etc/squid/squidGuard.conf.org 
[root@lab ~]# vi /etc/squid/squidGuard.conf
# create new
dbhome /var/lib/squidGuard/db
logdir /var/log/squidGuard

# define 'deny' category
dest deny {
    # define prohibited domain list in 'deny' category
    domainlist deny/domains
    # define prohibited URL list in 'deny' category
    urllist deny/urls
    }
    
acl {
    default {
        # permit all except 'deny' category
        pass !deny all
        # the redirected URL if matches 'deny'
        redirect http://www.srv.world/error.html
    }
}

[root@lab ~]# mkdir -p /var/lib/squidGuard/db/deny 
[root@lab ~]# vi /var/lib/squidGuard/db/deny/domains
# write domains you'd like to prohibit to access
yahoo.co.jp
example.com

[root@lab ~]# vi /var/lib/squidGuard/db/deny/urls
# write URLs you'd like to prohibit to access
www.yahoo.co.jp/deny/
www.example.com/

[root@lab ~]# squidGuard -C all 
[root@lab ~]# chown -R squid. /var/lib/squidGuard/db/deny 

[root@lab ~]# vi /etc/squid/squid.conf
# add follows to the end
url_rewrite_program /usr/bin/squidGuard -c /etc/squid/squidGuard.conf

[root@lab ~]# systemctl restart squid 
```