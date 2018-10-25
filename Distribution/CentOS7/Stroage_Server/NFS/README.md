- 1 配置NFS服务器
    - 1.1 配置NFS服务
    - 1.2 添加防火墙策略
- 2 配置NFS客户端
    - 2.1 配置NFS客户端
    - 2.2 基于fstab自动挂载
    - 2.3 基于autofs自动挂载

# 1. 配置NFS服务器

配置NFS服务器共享目录，实验环境如下：
```
[NFS Server]
domain name：lab.centos.org
IP：10.0.030

[NFS Client]
domain name：www.centos.org
IP: 10.0.0.31
```

## 1.1 配置NFS服务

```Shell
[root@lab ~]# yum -y install nfs-utils

[root@lab ~]# vi /etc/idmapd.conf
# line 5: uncomment and change to your domain name
Domain = centos.org

[root@lab ~]# vi /etc/exports
# write settings for NFS exports
/home 10.0.0.0/24(rw,no_root_squash)

[root@lab ~]# systemctl start rpcbind nfs-server 
[root@lab ~]# systemctl enable rpcbind nfs-server 
```

## 1.2 添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-service=nfs --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

# 2. 配置NFS客户端

## 2.1 配置NFS客户端

```Shell
[root@www ~]# yum -y install nfs-utils

[root@www ~]# vi /etc/idmapd.conf
# line 5: uncomment and change to your domain name
Domain = centos.org

[root@www ~]# systemctl start rpcbind 
[root@www ~]# systemctl enable rpcbind 

[root@www ~]# mount -t nfs lab.centos.org:/home /home 

[root@www ~]# df -hT 
Filesystem              Type      Size  Used Avail Use% Mounted on
/dev/mapper/centos-root xfs        46G  1.4G   45G   4% /
devtmpfs                devtmpfs  1.9G     0  1.9G   0% /dev
tmpfs                   tmpfs     1.9G     0  1.9G   0% /dev/shm
tmpfs                   tmpfs     1.9G  8.3M  1.9G   1% /run
tmpfs                   tmpfs     1.9G     0  1.9G   0% /sys/fs/cgroup
/dev/vda1               xfs       497M  219M  278M  45% /boot
lab.centos.org:/home    nfs4       46G  1.4G   45G   4% /home
# /home from NFS server is mounted
```

## 2.2 基于fstab自动挂载

```Shell
[root@www ~]# vi /etc/fstab
/dev/mapper/centos-root /                       xfs     defaults        1 1
UUID=a18716b4-cd67-4aec-af91-51be7bce2a0b /boot xfs     defaults        1 2
/dev/mapper/centos-swap swap                    swap    defaults        0 0

# add like follows to the end
lab.centos.org:/home  /home                     nfs     defaults        0 0
```

## 2.3 基于autofs自动挂载

```Shell
[root@www ~]# yum -y install autofs
[root@www ~]# vi /etc/auto.master
# add follows to the end
 /-    /etc/auto.mount

[root@www ~]# vi /etc/auto.mount
# create new : [mount point] [option] [location]
 /mntdir -fstype=nfs,rw  lab.centos.org:/home

[root@www ~]# mkdir /mntdir 
[root@www ~]# systemctl start autofs 
[root@www ~]# systemctl enable autofs 
# move to the mount point to make sure it normally mounted
[root@www ~]# cd /mntdir 

[root@www mntdir]# ll 
total 0
drwx------ 2 JerryDai JerryDai 59 Jul  9  2014 JerryDai
[root@www mntdir]# cat /proc/mounts | grep mntdir 
/etc/auto.mount /mntdir autofs rw,relatime,fd=18,pgrp=2093,timeout=300,minproto=5,maxproto=5,direct 0 0
lab.centos.org:/home /mntdir nfs4 rw,relatime,vers=4.0,rsize=524288,wsize=524288,namlen=255,hard,proto=tcp,
port=0,timeo=600,retrans=2,sec=sys,clientaddr=10.0.0.31,local_lock=none,addr=10.0.0.30 0 0
```