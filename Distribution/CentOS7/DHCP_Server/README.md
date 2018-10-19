[TOC]

# 1. 配置DHCP服务

DHCP ( Dynamic Host Configuration Protocol ) 

## 1.1 安装配置

```Shell
[root@lab ~]# yum -y install dhcp

[root@lab ~]# vi /etc/dhcp/dhcpd.conf
# create new
# specify domain name
option domain-name "centos.org";

# specify name server's hostname or IP address
option domain-name-servers lab.centos.org;

# default lease time
default-lease-time 600;

# max lease time
max-lease-time 7200;

# this DHCP server to be declared valid
authoritative;

# specify network address and subnet mask
subnet 10.0.0.0 netmask 255.255.255.0 {
    # specify the range of lease IP address
    range dynamic-bootp 10.0.0.200 10.0.0.254;
    
    # specify broadcast address
    option broadcast-address 10.0.0.255;
    
    # specify default gateway
    option routers 10.0.0.1;
}

[root@lab ~]# systemctl start dhcpd 
[root@lab ~]# systemctl enable dhcpd 
```

## 1.2 添加防火墙策略 

DHCP服务：使用UDP协议67号端口。

```Shell
[root@lab ~]# firewall-cmd --add-service=dhcp --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

# 2. 配置DHCP客户端

DHCP客户端：使用UDP协议的68号端口。

```Shell
[root@client ~]# nmcli c modify enp0s3 ipv4.method auto 
[root@client ~]# nmcli c down enp0s3; nmcli c up enp0s3
```

或者：

```Shell
[root@client ~]# dhclient enp0s3
```