- 1 安装配置DRBD(Distributed Replicated Block Device)分布式存储
    - 1.1 在所有节点上更新系统安装依赖软件包
    - 1.2 在所有节点上安装DRBD9
    - 1.3 在所有节点上创建drbdpool卷组
    - 1.4 添加防火墙策略
    - 1.5 在Node01上初始化DRBD集群并且添加节点
    - 1.6 除node01节点的所有节点
    - 1.7 确认集群状态
    - 1.8 在集群中创建一个资源
    - 1.9 创建文件系统挂载使用
    - 1.10 在第二个主机挂载DRDB设备

# 1. 安装配置DRBD(Distributed Replicated Block Device)分布式存储

```
DRBD Node#1：
    domain name：node01.centos.org / 10.0.0.51
    
DRBD Node#2：
    domain name：node02.centos.org / 10.0.0.52
```

## 1.1 在所有节点上更新系统安装依赖软件包

```Shell
[root@node01 ~]# yum -y update 
[root@node01 ~]# yum -y install gcc gcc-c++ make automake autoconf help2man libxslt libxslt-devel flex rpm-build kernel-devel 
[root@node01 ~]# reboot
```

## 1.2 在所有节点上安装DRBD9

```Shell
[root@node01 ~]# mkdir -p rpmbuild/{BUILD,BUILDROOT,RPMS,SOURCES,SPECS,SRPMS} 

[root@node01 ~]# wget www.linbit.com/downloads/drbd/9.0/drbd-9.0.14-1.tar.gz \
www.linbit.com/downloads/drbdmanage/drbdmanage-0.99.18.tar.gz \
www.linbit.com/downloads/drbd/utils/drbd-utils-9.5.0.tar.gz

[root@node01 ~]# tar zxvf drbd-9.0.14-1.tar.gz 
[root@node01 ~]# cd drbd-9.0.14-1 
[root@node01 drbd-9.0.14-1]# make kmp-rpm 
[root@node01 drbd-9.0.14-1]# cd
[root@node01 ~]# tar zxvf drbdmanage-0.99.18.tar.gz 
[root@node01 ~]# cd drbdmanage-0.99.18 
[root@node01 drbdmanage-0.99.18]# make rpm 
[root@node01 drbdmanage-0.99.18]# cd
[root@node01 ~]# tar zxvf drbd-utils-9.5.0.tar.gz 

[root@node01 ~]# cd drbd-utils-9.5.0 
[root@node01 drbd-utils-9.5.0]# vi drbd.spec.in
# line 34: add
%bcond_without sbinsymlinks
%undefine with_sbinsymlinks

[root@node01 drbd-utils-9.5.0]# ./configure 
[root@node01 drbd-utils-9.5.0]# make rpm 
[root@node01 drbd-utils-9.5.0]# cd /root/rpmbuild/RPMS/x86_64
[root@node01 x86_64]# rpm -Uvh drbd-utils-9.5.0-1.el7.x86_64.rpm \
drbd-udev-9.5.0-1.el7.x86_64.rpm \
drbd-bash-completion-9.5.0-1.el7.x86_64.rpm \
kmod-drbd-9.0.14_3.10.0_862.6.3-1.el7.x86_64.rpm 
Preparing...                          ################################# [100%]
Updating / installing...
   1:drbd-utils-9.5.0-1.el7           ################################# [ 25%]
   2:drbd-udev-9.5.0-1.el7            ################################# [ 50%]
   3:drbd-bash-completion-9.5.0-1.el7 ################################# [ 75%]
   4:kmod-drbd-9.0.14_3.10.0_862.6.3-1################################# [100%]
[root@node01 x86_64]# rpm -Uvh /root/drbdmanage-0.99.18/dist/drbdmanage-0.99.18-1.noarch.rpm 
Preparing...                          ################################# [100%]
Updating / installing...
   1:drbdmanage-0.99.18-1             ################################# [100%]
```

## 1.3 在所有节点上创建drbdpool卷组
```
[root@node01 ~]# pvcreate /dev/sdb1 
Physical volume "/dev/sdb1" successfully created

[root@node01 ~]# vgcreate drbdpool /dev/sdb1 
Volume group "vg_volume01" successfully created
```

## 1.4 添加防火墙策略

```Shell
[root@node01 ~]# firewall-cmd --add-port=6996-7800/tcp --permanent 
success

[root@node01 ~]# firewall-cmd --reload 
success
```

## 1.5 在node01上初始化DRBD集群并且添加节点

```Shell
[root@node01 ~]# drbdmanage init 10.0.0.51 
You are going to initialize a new drbdmanage cluster.
CAUTION! Note that:
  * Any previous drbdmanage cluster information may be removed
  * Any remaining resources managed by a previous drbdmanage installation
    that still exist on this system will no longer be managed by drbdmanage

Confirm:

  yes/no: yes
[ 2420.329129] drbd .drbdctrl: Starting worker thread (from drbdsetup [13925])
[ 2420.451483] drbd .drbdctrl/0 drbd0: disk( Diskless -> Attaching )
[ 2420.453513] drbd .drbdctrl/0 drbd0: Maximum number of peer devices = 31
.....
.....
[ 2421.677920] drbd .drbdctrl: Preparing cluster-wide state change 2615618191 (0->-1 3/1)
[ 2421.680499] drbd .drbdctrl: Committing cluster-wide state change 2615618191 (2ms)
[ 2421.682894] drbd .drbdctrl: role( Secondary -> Primary )
Waiting for server: .
Operation completed successfully

# add a Node to DRBD Cluster
[root@node01 ~]# drbdmanage add-node node02.centos.org 10.0.0.52 
[ 2452.989515] drbd .drbdctrl node02.centos.org: Starting sender thread (from drbdsetup [14020])
[ 2452.997518] drbd .drbdctrl node02.centos.org: conn( StandAlone -> Unconnected )
Operation completed successfully
Operation completed successfully
[ 2453.037235] drbd .drbdctrl node02.centos.org: Starting receiver thread (from drbd_w_.drbdctr [13926])
[ 2453.040902] drbd .drbdctrl node02.centos.org: conn( Unconnected -> Connecting )
Host key verification failed.
Give leader time to contact the new node
Operation completed successfully
Operation completed successfully

Join command for node nnode02.centos.org:
drbdmanage join -p 6999 10.0.0.52 1 node01.centos.org 10.0.0.51 0 twuZE5BAthnZIRyEAAS/
```

## 1.6 除node01节点的所有节点
```
[root@node02 ~]# drbdmanage join -p 6999 10.0.0.52 1 node01.centos.org 10.0.0.51 0 twuZE5BAthnZIRyEAAS/ 
You are going to join an existing drbdmanage cluster.
CAUTION! Note that:
  * Any previous drbdmanage cluster information may be removed
  * Any remaining resources managed by a previous drbdmanage installation
    that still exist on this system will no longer be managed by drbdmanage

Confirm:

  yes/no: yes
[ 2491.338532] drbd: loading out-of-tree module taints kernel.
[ 2491.343082] drbd: module verification failed: signature and/or required key missing - tainting kernel
[ 2491.364065] drbd: initialized. Version: 9.0.14-1 (api:2/proto:86-113)
.....
.....
[ 2553.012505] drbd .drbdctrl node01.centos.org: conn( StandAlone -> Unconnected )
[ 2553.025846] drbd .drbdctrl node01.centos.org: Starting receiver thread (from drbd_w_.drbdctr [13762])
[ 2553.028899] drbd .drbdctrl node01.centos.org: conn( Unconnected -> Connecting )
Operation completed successfully
```

## 1.7 确认集群状态

```Shell
[root@node01 ~]# drbdadm status 
  --==  Thank you for participating in the global usage survey  ==--
The server's response is:

you are the 1527th user to install this version
.drbdctrl role:Primary
  volume:0 disk:UpToDate
  volume:1 disk:UpToDate
  node02.centos.org role:Secondary
    volume:0 peer-disk:UpToDate
    volume:1 peer-disk:UpToDate

[root@node01 ~]# drbdmanage list-nodes 
+---------------------------------------------------+
| Name              | Pool Size | Pool Free | State |
|---------------------------------------------------|
| node01.centos.org |     81916 |     81908 |    ok |
| node02.centos.org |     81916 |     81908 |    ok |
+---------------------------------------------------+
```

## 1.8 在集群中创建一个资源

```Shell
# create [resource01] resource
[root@node01 ~]# drbdmanage add-resource resource01 
Operation completed successfully

[root@node04 ~]# drbdmanage list-resources 
+--------------------+
| Name       | State |
|--------------------|
| resource01 |    ok |
+--------------------+

# add volume with 10G
[root@node01 ~]# drbdmanage add-volume resource01 10GB 
Operation completed successfully
[root@node01 ~]# drbdmanage list-volumes 
+------------------------------------------------+
| Name       | Vol ID |     Size | Minor | State |
|------------------------------------------------|
| resource01 |      0 | 9.31 GiB |   100 |    ok |
+------------------------------------------------+

# deploy resource
# the last number means number of Nodes you'd like to use for this resource
[root@node01 ~]# drbdmanage deploy-resource resource01 2 
Operation completed successfully

# show status : after setting, state is [Inconsistent] (syncing)
[root@node01 ~]# drbdadm status 
.drbdctrl role:Secondary
  volume:0 disk:UpToDate
  volume:1 disk:UpToDate
  node02.centos.org role:Primary
    volume:0 peer-disk:UpToDate
    volume:1 peer-disk:UpToDate

resource01 role:Secondary
  disk:Inconsistent
  node02.centos.org role:Secondary
    replication:SyncTarget peer-disk:UpToDate done:0.07

# after finishing syncing, state turned like follows
[root@node01 ~]# drbdadm status 
.drbdctrl role:Secondary
  volume:0 disk:UpToDate
  volume:1 disk:UpToDate
  node02.centos.org role:Primary
    volume:0 peer-disk:UpToDate
    volume:1 peer-disk:UpToDate

resource01 role:Secondary
  disk:UpToDate
  node02.centos.org role:Secondary
    peer-disk:UpToDate
```

## 1.9 创建文件系统挂载使用

```Shell
# the number of [/dev/drbd***], it's the number of [Minor] value that is shown with the command [drbdmanage list-volumes]
[root@node01 ~]# mkfs.xfs /dev/drbd100 
[root@node01 ~]# mkdir /drbd_disk 
[root@node01 ~]# mount /dev/drbd100 /drbd_disk 
[root@node01 ~]# df -hT 
Filesystem          Type      Size  Used Avail Use% Mounted on
/dev/mapper/cl-root xfs        26G  1.7G   25G   7% /
devtmpfs            devtmpfs  2.0G     0  2.0G   0% /dev
tmpfs               tmpfs     2.0G     0  2.0G   0% /dev/shm
tmpfs               tmpfs     2.0G  8.5M  2.0G   1% /run
tmpfs               tmpfs     2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/vda1           xfs      1014M  238M  777M  24% /boot
tmpfs               tmpfs     396M     0  396M   0% /run/user/0
/dev/drbd100        xfs       9.4G   33M  9.3G   1% /drbd_disk

# create a test file
[root@node01 ~]# echo 'test file' > /drbd_disk/test.txt 
[root@node01 ~]# ll /drbd_disk 
total 4
-rw-r--r--. 1 root root 10 Jul 12 19:54 test.txt
```

## 1.10 在第二个主机挂载DRDB设备

```Shell
########### on primary Node ###########
# unmount and get secondary role
[root@node01 ~]# umount /drbd_disk 
[root@node01 ~]# drbdadm secondary resource01

########### on secondary Node ###########
# get primary role and mount
[root@node02 ~]# drbdadm primary resource01 
[root@node02 ~]# mount /dev/drbd100 /drbd_disk 
[root@node02 ~]# df -hT 
Filesystem          Type      Size  Used Avail Use% Mounted on
/dev/mapper/cl-root xfs        26G  1.6G   25G   6% /
devtmpfs            devtmpfs  2.0G     0  2.0G   0% /dev
tmpfs               tmpfs     2.0G     0  2.0G   0% /dev/shm
tmpfs               tmpfs     2.0G  8.5M  2.0G   1% /run
tmpfs               tmpfs     2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/vda1           xfs      1014M  238M  777M  24% /boot
tmpfs               tmpfs     396M     0  396M   0% /run/user/0
/dev/drbd100        xfs       9.4G   33M  9.3G   1% /drbd_disk

[root@node02 ~]# ll /drbd_disk 
total 4
-rw-r--r--. 1 root root 10 Jul 12 19:54 test.txt
```