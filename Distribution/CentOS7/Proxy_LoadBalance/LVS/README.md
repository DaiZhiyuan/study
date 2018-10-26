- 1 Linux Virtual Server
    - 1.1 安装ipvsadm
    - 1.2 配置负载均衡
    - 1.3 调度算法
- 2 LVS + Keepalived案例
    - 2.1 网络拓扑
    - 2.2 安装ipvsadm和keepalived
    - 2.3 配置Keepalived

# 1. Linux Virtual Server

网络拓扑：
```
                          |
                          |
                      eth0|192.168.0.30
                    +----------+
--------------------|    LVS   |----------------------
                    +-----+----+
                      eth1|10.0.0.30
                          |
+------------+            |             +------------+
|  Backend01 |10.0.0.51   |    10.0.0.52|  Backend02 |
| Web Server +------------+-------------+ Web Server |
|            |eth0                  eth0|            |
+------------+                          +------------+
```

## 1.1 安装ipvsadm

```Shell
[root@lab ~]# yum -y install ipvsadm

# enable IP forward
[root@lab ~]# echo 'net.ipv4.ip_forward = 1' >> /etc/sysctl.conf 
[root@lab ~]# sysctl -p

[root@lab ~]# touch /etc/sysconfig/ipvsadm 
[root@lab ~]# systemctl start ipvsadm 
[root@lab ~]# systemctl enable ipvsadm 
```

## 1.2 配置负载均衡

```Shell
# clear tables
[root@lab ~]# ipvsadm -C

# add virtual service
# [ ipvsadm -A -t (Service IP:Port) -s (Distribution method) ]
[root@lab ~]# ipvsadm -A -t 192.168.0.30:80 -s wlc 

# add backend real servers
# [ ipvsadm -a -t (Service IP:Port) -r (Real Server's IP:Port) -m ] ("m" means masquerading (NAT))
[root@lab ~]# ipvsadm -a -t 192.168.0.30:80 -r 10.0.0.51:80 -m 
[root@lab ~]# ipvsadm -a -t 192.168.0.30:80 -r 10.0.0.52:80 -m 

# confirm tables
[root@lab ~]# ipvsadm -l 
IP Virtual Server version 1.2.1 (size=4096)
Prot LocalAddress:Port Scheduler Flags
  -> RemoteAddress:Port           Forward Weight ActiveConn InActConn
TCP  www.centos.org:http wlc
  -> 10.0.0.51:http               Masq    1      0          0
  -> 10.0.0.52:http               Masq    1      0          0
```

## 1.3 调度算法

Method | Description
---|---
rr | Round Robin
wrr	| Weighted Round Robin
lc | Least-Connection
wlc	| Weighted Least-Connection
lblc | Locality-Based Least-Connection
lblcr | Locality-Based Least-Connection with Replication
dh | Destination Hashing
sh | Source Hashing
sed	| Shortest Expected Delay
nq | Never Queue

# 2. LVS + Keepalived案例

## 2.1 网络拓扑

```
                              |
             +----------------+-----------------+
             |                                  |
 192.168.0.30|eth0 --- VIP:192.168.0.29 --- eth0|192.168.0.31
     +-------+--------+                +--------+-------+
     | LVS+Keepalived |                | LVS+Keepalived |
     +-------+--------+                +--------+-------+
    10.0.0.30|eth1 ----- VIP:10.0.0.29 ---- eth1|10.0.0.31
             |                                  |
             +----------------+-----------------+
                              |
    +------------+            |             +------------+
    |  Backend01 |10.0.0.51   |    10.0.0.52|  Backend02 |
    | Web Server +------------+-------------+ Web Server |
    |            |eth0                  eth0|            |
    +------------+                          +------------+
```

## 2.2 安装ipvsadm和keepalived

```Shell
[root@lab ~]# yum -y install ipvsadm keepalived

# enable IP forward
[root@lab ~]# echo 'net.ipv4.ip_forward = 1' >> /etc/sysctl.conf 
[root@lab ~]# sysctl -p

[root@lab ~]# touch /etc/sysconfig/ipvsadm 
[root@lab ~]# systemctl start ipvsadm 
[root@lab ~]# systemctl enable ipvsadm 
```

## 2.3 配置Keepalived

```Shell
[root@lab ~]# mv /etc/keepalived/keepalived.conf /etc/keepalived/keepalived.conf.org 
[root@lab ~]# vi /etc/keepalived/keepalived.conf

# create new
global_defs {
    notification_email {
        root@www.centos.sorg
    }
    notification_email_from admin@centos.org
    smtp_server 127.0.0.1
    smtp_connect_timeout 30
    router_id LVS_Server
}

vrrp_instance VI_1 {
    state BACKUP
    
    # monitored interface
    interface eth0
    
    # virtual router's ID
    virtual_router_id 51
    
    # set priority (change this value on each server)
    # (large number means priority is high)
    priority 100
    nopreempt
    
    # VRRP sending interval
    advert_int 1
    
    # authentication info between Keepalived servers
    authentication {
        auth_type PASS
        auth_pass password
    }

    virtual_ipaddress {
        # virtual IP address
        192.168.0.29 dev eth0
        10.0.0.29/24 dev eth1
    }
}

virtual_server 192.168.0.29 80 {

    # monitored interval
    delay_loop 3
    
    # distribution method
    lvs_sched rr
    
    # routing method
    lvs_method NAT
    protocol TCP

    # backend server#1
    real_server 10.0.0.51 80 {
        weight 1
        HTTP_GET {
            url {
                # monitored path
                path /
                # status code for normally state
                status_code 200
            }
            # timeout(sec)
            connect_timeout 3
        }
    }
    
    # backend server#2
    real_server 10.0.0.52 80 {
        weight 1
        HTTP_GET {
            url {
                path /
                status_code 200
            }
            connect_timeout 3
        }
    }
}

[root@lab ~]# systemctl start keepalived 
[root@lab ~]# systemctl enable keepalived 
```







