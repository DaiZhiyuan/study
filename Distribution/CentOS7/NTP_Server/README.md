- 1 配置NTP服务器
    - 1.1 安装Chrony
    - 1.2 添加防火墙规则
    - 1.3 验证服务可用性
- 2 安装配置NTP客户端

# 1. 配置NTP服务器

## 1.1 安装Chrony
```
[root@lab ~]# yum -y install chrony

[root@lab ~]# vi /etc/chrony.conf

# line 3: change servers for synchronization
#server 0.centos.pool.ntp.org iburst
#server 1.centos.pool.ntp.org iburst
#server 2.centos.pool.ntp.org iburst
#server 3.centos.pool.ntp.org iburst
server ntp1.jst.mfeed.ad.jp iburst
server ntp2.jst.mfeed.ad.jp iburst
server ntp3.jst.mfeed.ad.jp iburst

# line 25: add the network range you allow to receive requests
allow 10.0.0.0/24

[root@lab ~]# systemctl start chronyd 
[root@lab ~]# systemctl enable chronyd 
```
## 1.2 添加防火墙规则

NTP服务：使用UDP协议123号端口。

```Shell
[root@lab ~]# firewall-cmd --add-service=ntp --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

## 1.3 验证服务可用性

```Shell
[root@lab ~]# chronyc sources 
210 Number of sources = 3
MS Name/IP address         Stratum Poll Reach LastRx Last sample
===============================================================================
^+ ntp1.jst.mfeed.ad.jp          2   6    17     8  -1028us[-1261us] +/-   14ms
^* ntp2.jst.mfeed.ad.jp          2   6    17     8  -1241us[-1474us] +/-   15ms
^? ntp3.jst.mfeed.ad.jp          0   6     0   10y     +0ns[   +0ns] +/-    0ns
```

# 2. 安装配置NTP客户端

```Shell
[root@lab ~]# yum -y install ntpdate

[root@lab ~]# ntpdate ntp1.jst.mfeed.ad.jp 
24 Jan 11:35:15 ntpdate[5740]: adjust time server xxx.xxx.xxx.xxx offset 0.004626 sec

[root@lab ~]# systemctl enable ntpdate 
```