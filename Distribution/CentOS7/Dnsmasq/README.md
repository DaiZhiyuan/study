[TOC]

# 1. Dnsmasq

Dnsmsq是一个轻量级的DNS转发者和DHCP服务软件。

## 1.1 安装Dnsmasq

```Shell
[root@lab ~]# yum -y install dnsmasq
```

## 1.2 配置Dnsmasq

```Shell
[root@lab ~]# vi /etc/dnsmasq.conf

# line 19: uncomment (never forward plain names)
domain-needed

# line 21: uncomment (never forward addresses in the non-routed address spaces)
bogus-priv

# line 41: uncomment (query with each server strictly in the order in resolv.conf)
strict-order

# line 55: add if you need
# query the specific domain name to the specific DNS server
# the example follows means query [server.education] domain to the [10.0.0.10] server
server=/server.education/10.0.0.10

# line 123: uncomment (add domain name automatically)
expand-hosts

# line 133: add (define domain name)
domain=centos.org

[root@lab ~]# systemctl start dnsmasq 
[root@lab ~]# systemctl enable dnsmasq 
```

## 1.3 配置静态主机名解析

```Shell
[root@lab ~]# vi /etc/hosts
127.0.0.1   localhost localhost.localdomain localhost4 localhost4.localdomain4
::1         localhost localhost.localdomain localhost6 localhost6.localdomain6
# add records
10.0.0.30   lab.centos.org lab 

[root@lab ~]# systemctl restart dnsmasq 
```

## 1.4 增加防火墙规则

```Shell
[root@lab ~]# firewall-cmd --add-service=dns --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

## 1.4 测试解析域名

```
[root@desktop ~]# yum -y install bind-utils

# change DNS setting to Dnsmasq Server (replace "enp0s3" to your own environment)
[root@desktop ~]# nmcli c modify enp0s3 ipv4.dns 10.0.0.30 
[root@desktop ~]# nmcli c down enp0s3; nmcli c up enp0s3
[root@desktop ~]# dig lab.centos.org. 

; <<>> DiG 9.9.4-RedHat-9.9.4-29.el7_2.3 <<>> lab.centos.org
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 11613
;; flags: qr aa rd ra ad; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 0

;; QUESTION SECTION:
;lab.centos.org.                 IN      A

;; ANSWER SECTION:
lab.centos.org.          0       IN      A       10.0.0.30

;; Query time: 2 msec
;; SERVER: 10.0.0.30#53(10.0.0.30)
;; WHEN: Wed Aug 31 17:22:51 JST 2016
;; MSG SIZE  rcvd: 47

[root@desktop ~]# dig -x 10.0.0.30 

; <<>> DiG 9.9.4-RedHat-9.9.4-29.el7_2.3 <<>> -x 10.0.0.30
;; global options: +cmd
;; Got answer:
;; ->>HEADER<<- opcode: QUERY, status: NOERROR, id: 61937
;; flags: qr aa rd ra ad; QUERY: 1, ANSWER: 1, AUTHORITY: 0, ADDITIONAL: 0

;; QUESTION SECTION:
;30.0.0.10.in-addr.arpa.                IN      PTR

;; ANSWER SECTION:
30.0.0.10.in-addr.arpa. 0       IN      PTR     lab.centos.org.

;; Query time: 3 msec
;; SERVER: 10.0.0.30#53(10.0.0.30)
;; WHEN: Wed Aug 31 17:24:00 JST 2016
;; MSG SIZE  rcvd: 67
```

# 2. 启动DHCP功能

## 2.1 配置Dnsmasq

```Shell
[root@lab ~]# vi /etc/dnsmasq.conf

# line 146: add (range of IP address to lease and term of lease)
dhcp-range=10.0.0.200,10.0.0.250,12h

# line 316: add (define default gateway)
dhcp-option=option:router,10.0.0.1

# line 325: add (define NTP, DNS, server and subnetmask)
dhcp-option=option:ntp-server,10.0.0.10
dhcp-option=option:dns-server,10.0.0.10
dhcp-option=option:netmask,255.255.255.0

[root@lab ~]# systemctl restart dnsmasq 
```

## 2.2 添加防火墙规则

```Shell
[root@lab ~]# firewall-cmd --add-service=dhcp --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```