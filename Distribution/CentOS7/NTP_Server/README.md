[TOC]

# 1. 安装配置NTP服务

```
[root@lab ~]# yum -y install ntp

[root@lab ~]# vi /etc/ntp.conf

# line 18: add the network range you allow to receive requests
restrict 10.0.0.0 mask 255.255.255.0 nomodify notrap

# change servers for synchronization
#server 0.centos.pool.ntp.org iburst
#server 1.centos.pool.ntp.org iburst
#server 2.centos.pool.ntp.org iburst
#server 3.centos.pool.ntp.org iburst
server ntp1.jst.mfeed.ad.jp iburst
server ntp2.jst.mfeed.ad.jp iburst
server ntp3.jst.mfeed.ad.jp iburst

[root@dlp ~]# systemctl start ntpd 
[root@dlp ~]# systemctl enable ntpd 
```
# 2. 添加防火墙规则

```Shell
[root@lab ~]# firewall-cmd --add-service=ntp --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

# 3. 验证服务可用性

```Shell
[root@lab ~]# ntpq -p
     remote           refid      st t when poll reach   delay   offset  jitter
==============================================================================
+ntp1.jst.mfeed. 172.29.1.100     2 u   29   64    1   18.826   -0.126   0.000
+ntp2.jst.mfeed. 172.29.2.50      2 u   28   64    1   21.592    0.018   0.000
*ntp3.jst.mfeed. 133.243.236.18   2 u   28   64    1   22.666   -1.033   0.000
```