- 1 配置Ceph存储集群
    - 1.1 配置JerryDai用户sudo权限
    - 1.2 配置Ceph源
    - 1.3 在所有节点上设置防火墙策略允许访问ssh服务
    - 1.4 在Monitor Node(Monitor Daemon)上添加防火墙策略
    - 1.5 在所有Storage Nodes(Object Storage)上添加防火墙策略
    - 1.6 登录Ceph管理员用户JerryDai，设置管理节点与所有存储节点的SSH双机互信。
    - 1.7 在管理节点部署所有存储节点
    - 1.8 配置管理节点
- 2 重新配置
- 3 作为块设备使用
    - 3.1 在管理节点上部署client并授权
    - 3.2 创建一个块设备并挂载使用
- 4 作为文件系统使用
    - 4.1 在node1节点上创建MDS(Meta Data Server)
    - 4.2 在MDS节点上创建至少2个RADOS池并激活MDS
    - 4.3 挂载CephFS文件系统

# 1. 配置Ceph存储集群

```
安装分布式文件系统Ceph(Jewel)存储集群，1个管理节点与3个存储节点。

管理节点：
    [Ceph-Depoly]
    domain name：lab.centos.org / 10.0.0.30

对象存储：
    [Object Storage]
    [Monitor Daemon]
    domain name: node01.centos.org / 10.0.0.51
    
    [Object Storage]
    domain name: node02.centos.org / 10.0.0.52
    
    [Object Storage]
    domain name: node03.centos.org / 10.0.0.53

客户端：
    10.0.0.x
```

 

## 1.1 配置JerryDai用户sudo权限
```Shell
[root@lab ~]# echo -e 'Defaults:JerryDai !requiretty\nJerryDai ALL = (root) NOPASSWD:ALL' | tee /etc/sudoers.d/ceph 
[root@lab ~]# chmod 440 /etc/sudoers.d/ceph 
```

## 1.2 配置Ceph源

```Shell
[root@lab ~]# yum -y install epel-release yum-plugin-priorities \
https://download.ceph.com/rpm-jewel/el7/noarch/ceph-release-1-1.el7.noarch.rpm 
[root@lab ~]# sed -i -e "s/enabled=1/enabled=1\npriority=1/g" /etc/yum.repos.d/ceph.repo
```

## 1.3 在所有节点上设置防火墙策略允许访问ssh服务

```Shell
[root@lab ~]# firewall-cmd --add-service=ssh --permanent 
[root@lab ~]# firewall-cmd --reload 
```

## 1.4 在Monitor Node(Monitor Daemon)上添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-port=6789/tcp --permanent 
[root@lab ~]# firewall-cmd --reload 
```

## 1.5 在所有Storage Nodes(Object Storage)上添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-port=6800-7100/tcp --permanent 
[root@lab ~]# firewall-cmd --reload 
```

## 1.6 登录Ceph管理员用户JerryDai，设置管理节点与所有存储节点的SSH双机互信。
```Shell
[JerryDai@lab ~]$ ssh-keygen 
Generating public/private rsa key pair.
Enter file in which to save the key (/home/JerryDai/.ssh/id_rsa):
Created directory '/home/JerryDai/.ssh'.
Enter passphrase (empty for no passphrase):
Enter same passphrase again:
Your identification has been saved in /home/JerryDai/.ssh/id_rsa.
Your public key has been saved in /home/JerryDai/.ssh/id_rsa.pub.
The key fingerprint is:
54:c3:12:0e:d3:65:11:49:11:73:35:1b:e3:e8:63:5a JerryDai@lab.centos.org
The key's randomart image is:

[JerryDai@lab ~]$ vi ~/.ssh/config
# create new ( define all nodes and users )
Host lab
    Hostname lab.centos.org
    User JerryDai
Host node01
    Hostname node01.centos.org
    User JerryDai
Host node02
    Hostname node02.centos.org
    User JerryDai
Host node03
    Hostname node03.centos.org
    User JerryDai

[JerryDai@lab ~]$ chmod 600 ~/.ssh/config

# transfer key file
[JerryDai@lab ~]$ ssh-copy-id node01 
JerryDai@node01.centos.org's password: 

Number of key(s) added: 1

Now try logging into the machine, with:   "ssh 'node01'"
and check to make sure that only the key(s) you wanted were added.

[JerryDai@lab ~]$ ssh-copy-id node02 
[JerryDai@lab ~]$ ssh-copy-id node03
```

## 1.7 在管理节点部署所有存储节点
```Shell
[JerryDai@lab ~]$ sudo yum -y install ceph-deploy
[JerryDai@lab ~]$ mkdir ceph 
[JerryDai@lab ~]$ cd ceph 
[JerryDai@lab ~]$ ceph-deploy new node01 

[JerryDai@lab ~]$ vi ./ceph.conf
# add to the end
osd pool default size = 2

# install Ceph on each Node
[JerryDai@lab ~]$ ceph-deploy install lab node01 node02 node03

# settings for monitoring and keys
[JerryDai@lab ~]$ ceph-deploy mon create-initial 
```

## 1.8 配置管理节点
```Shell
# prepare Object Storage Daemon
[JerryDai@lab ~]$ ceph-deploy osd prepare node01:/var/lib/ceph/osd node02:/var/lib/ceph/osd node03:/var/lib/ceph/osd

# activate Object Storage Daemon
[JerryDai@lab ~]$ ceph-deploy osd activate node01:/var/lib/ceph/osd node02:/var/lib/ceph/osd node03:/var/lib/ceph/osd

# transfer config files
[JerryDai@lab ~]$ ceph-deploy admin lab node01 node02 node03 
[JerryDai@lab ~]$ sudo chmod 644 /etc/ceph/ceph.client.admin.keyring

# show status (display like follows if no ploblem)
[JerryDai@lab ~]$ ceph health 
HEALTH_OK
```

# 2. 重新配置

```Shell
# remove packages
[JerryDai@lab ceph]$ ceph-deploy purge lab node01 node02 node03

# remove settings
[JerryDai@lab ceph]$ ceph-deploy purgedata lab node01 node02 node03 
[JerryDai@lab ceph]$ ceph-deploy forgetkeys
```

# 3. 作为块设备使用

## 3.1 在管理节点上部署client并授权

```Shell
[JerryDai@lab ceph]$ ceph-deploy install client 
[JerryDai@lab ceph]$ ceph-deploy admin client
```

## 3.2 创建一个块设备并挂载使用
```Shell
[JerryDai@client ~]$ sudo chmod 644 /etc/ceph/ceph.client.admin.keyring 

# create a disk with 10G
[JerryDai@client ~]$ rbd create disk01 --size 10G --image-feature layering

# show list
[JerryDai@client ~]$ rbd ls -l 
NAME     SIZE PARENT FMT PROT LOCK
disk01 10240M          2

# map the image to device
[JerryDai@client ~]$ sudo rbd map disk01 
/dev/rbd0

# show mapping
[JerryDai@client ~]$ rbd showmapped 
id pool image  snap device
0  rbd  disk01 -    /dev/rbd0

# format with XFS
[JerryDai@client ~]$ sudo mkfs.xfs /dev/rbd0

# mount device
[JerryDai@client ~]$ sudo mount /dev/rbd0 /mnt
[JerryDai@client ~]$ df -hT 
Filesystem          Type      Size  Used Avail Use% Mounted on
/dev/mapper/cl-root xfs        26G  1.9G   25G   7% /
devtmpfs            devtmpfs  2.0G     0  2.0G   0% /dev
tmpfs               tmpfs     2.0G     0  2.0G   0% /dev/shm
tmpfs               tmpfs     2.0G  8.4M  2.0G   1% /run
tmpfs               tmpfs     2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/vda1           xfs      1014M  230M  785M  23% /boot
tmpfs               tmpfs     396M     0  396M   0% /run/user/0
/dev/rbd0           xfs        10G   33M   10G   1% /mnt
```

# 4. 作为文件系统使用

## 4.1 在node1节点上创建MDS(Meta Data Server)

```Shell
[JerryDai@lab ceph]$ ceph-deploy mds create node01 
```

## 4.2 在MDS节点上创建至少2个RADOS池并激活MDS

```
[JerryDai@node01 ~]$ sudo chmod 644 /etc/ceph/ceph.client.admin.keyring 

# create pools
[JerryDai@node01 ~]$ ceph osd pool create cephfs_data 128 
pool 'cephfs_data' created

[JerryDai@node01 ~]$ ceph osd pool create cephfs_metadata 128 
pool 'cephfs_metadata' created

# enable pools
[JerryDai@node01 ~]$ ceph fs new cephfs cephfs_metadata cephfs_data 
new fs with metadata pool 2 and data pool 1

# show list
[JerryDai@node01 ~]$ ceph fs ls 
name: cephfs, metadata pool: cephfs_metadata, data pools: [cephfs_data ]

[JerryDai@node01 ~]$ ceph mds stat 
e5: 1/1/1 up {0=node01=up:active}
```

## 4.3 挂载CephFS文件系统

```Shell
[root@client ~]# yum -y install ceph-fuse

# get admin key
[root@client ~]# ssh JerryDai@node01.centos.org "sudo ceph-authtool -p /etc/ceph/ceph.client.admin.keyring" > admin.key 
JerryDai@node01.centos.org's password:

[root@client ~]# chmod 600 admin.key
[root@client ~]# mount -t ceph node01.centos.org:6789:/ /mnt -o name=admin,secretfile=admin.key
[root@client ~]# df -hT 
Filesystem          Type      Size  Used Avail Use% Mounted on
/dev/mapper/cl-root xfs        26G  1.9G   25G   7% /
devtmpfs            devtmpfs  2.0G     0  2.0G   0% /dev
tmpfs               tmpfs     2.0G     0  2.0G   0% /dev/shm
tmpfs               tmpfs     2.0G  8.4M  2.0G   1% /run
tmpfs               tmpfs     2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/vda1           xfs      1014M  230M  785M  23% /boot
tmpfs               tmpfs     396M     0  396M   0% /run/user/0
10.0.0.51:6789:/    ceph       78G   21G   58G  27% /mnt
```