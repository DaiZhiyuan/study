- 1 安装配置GlusterFS存储集群
    - 1.1 在集群所有节点上安装GlusterFS服务
    - 1.2 在集群所有节点上变更Selinux策略
    - 1.3 在集群所有节点上添加防火墙策略
- 2 配置ClusterFS集群
    - 2.1 分布式配置
    - 2.2 镜像配置
    - 2.3 条带配置
    - 2.4 分布式加镜像配置
    - 2.5 条带加镜像
- 3 客户端配置使用
- 4 GlusterFS与NFS-Ganesha结合
    - 4.1 准备工作
    - 4.2 在集群的一个节点上安装配置NFS-Ganesha
    - 4.3 变更SELinux策略
    - 4.4 添加防火墙策略
    - 4.5 客户端挂载使用

# 1. 安装配置GlusterFS存储集群

```
GlusterFS Server#1：
    domain name：node01.centos.org / 10.0.0.51
    
GlusterFS Server#2：
    domain name：node02.centos.org / 10.0.0.52
    
GlusterFS Server#3：
    domain name：node01.centos.org / 10.0.0.53
    
GlusterFS Server#4：
    domain name：node02.centos.org / 10.0.0.54
```

## 1.1 在集群所有节点上安装GlusterFS服务

```Shell
[root@node01 ~]# yum -y install centos-release-gluster40
[root@node01 ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-Gluster-4.0.repo
[root@node01 ~]# yum --enablerepo=centos-gluster40 -y install glusterfs-server
[root@node01 ~]# systemctl start glusterd 
[root@node01 ~]# systemctl enable glusterd 
```

## 1.2 在集群所有节点上变更Selinux策略
```Shell
[root@node01 ~]# setsebool -P nis_enabled on
```

## 1.3 在集群所有节点上添加防火墙策略

```Shell
[root@node01 ~]# firewall-cmd --add-service=glusterfs --permanent 
success
[root@node01 ~]# firewall-cmd --reload 
success
```

# 2. 配置ClusterFS集群

## 2.1 分布式配置

```Shell
[root@node01 ~]# mkdir /glusterfs/distributed 

# probe the node
[root@node01 ~]# gluster peer probe node02 
peer probe: success.

# show status
[root@node01 ~]# gluster peer status 
Number of Peers: 1

Hostname: node02
Uuid: 3d0b5871-6dc3-42d3-9818-6a43ef035b9f
State: Peer in Cluster (Connected)

# create volume
[root@node01 ~]# gluster volume create vol_distributed transport tcp \
node01:/glusterfs/distributed \
node02:/glusterfs/distributed 
volume create: vol_distributed: success: please start the volume to access data

# start volume
[root@node01 ~]# gluster volume start vol_distributed 
volume start: vol_distributed: success

# show volume info
[root@node01 ~]# gluster volume info 

Volume Name: vol_distributed
Type: Distribute
Volume ID: 67b86c76-6505-4fec-be35-3e79ef7c2dc1
Status: Started
Snapshot Count: 0
Number of Bricks: 2
Transport-type: tcp
Bricks:
Brick1: node01:/glusterfs/distributed
Brick2: node02:/glusterfs/distributed
Options Reconfigured:
transport.address-family: inet
nfs.disable: on
```

## 2.2 镜像配置

```Shell
[root@node01 ~]# mkdir /glusterfs/replica 

# probe the node
[root@node01 ~]# gluster peer probe node02 
peer probe: success.

# show status
[root@node01 ~]# gluster peer status 
Number of Peers: 1

Hostname: node02
Uuid: 0e1917c8-7cd0-4578-af65-46a1489b10b1
State: Peer in Cluster (Connected)

# create volume
[root@node01 ~]# gluster volume create vol_replica replica 2 transport tcp \
node01:/glusterfs/replica \
node02:/glusterfs/replica 
volume create: vol_replica: success: please start the volume to access data

# start volume
[root@node01 ~]# gluster volume start vol_replica 
volume start: vol_replica: success

# show volume info
[root@node01 ~]# gluster volume info 

Volume Name: vol_replica
Type: Replicate
Volume ID: f496e38f-9238-4721-92bf-78fbaace7758
Status: Started
Snapshot Count: 0
Number of Bricks: 1 x 2 = 2
Transport-type: tcp
Bricks:
Brick1: node01:/glusterfs/replica
Brick2: node02:/glusterfs/replica
Options Reconfigured:
transport.address-family: inet
nfs.disable: on
performance.client-io-threads: off
```

## 2.3 条带配置

```Shell
[root@node01 ~]# mkdir /glusterfs/striped 

# probe the node
[root@node01 ~]# gluster peer probe node02 
peer probe: success.

# show status
[root@node01 ~]# gluster peer status 
Number of Peers: 1

Hostname: node02
Uuid: 0e1917c8-7cd0-4578-af65-46a1489b10b1
State: Peer in Cluster (Connected)

# create volume
[root@node01 ~]# gluster volume create vol_striped stripe 2 transport tcp \
node01:/glusterfs/striped \
node02:/glusterfs/striped 
volume create: vol_striped: success: please start the volume to access data

# start volume
[root@node01 ~]# gluster volume start vol_striped 
volume start: vol_replica: success

# show volume info
[root@node01 ~]# gluster volume info 

Volume Name: vol_striped
Type: Stripe
Volume ID: c58cb60b-18d0-4342-bacd-f1399444fc19
Status: Started
Snapshot Count: 0
Number of Bricks: 1 x 2 = 2
Transport-type: tcp
Bricks:
Brick1: node01:/glusterfs/striped
Brick2: node02:/glusterfs/striped
Options Reconfigured:
transport.address-family: inet
nfs.disable: on
```

## 2.4 分布式加镜像配置

```Shell
[root@node01 ~]# mkdir /glusterfs/dist-replica 

# probe the node
[root@node01 ~]# gluster peer probe node02 
peer probe: success. 

[root@node01 ~]# gluster peer probe node03 
peer probe: success. 

[root@node01 ~]# gluster peer probe node04 
peer probe: success.

# show status
[root@node01 ~]# gluster peer status 
Number of Peers: 3

Hostname: node02
Uuid: 2ca22769-28a1-4204-9957-886579db2231
State: Peer in Cluster (Connected)

Hostname: node03
Uuid: 79cff591-1e98-4617-953c-0d3e334cf96a
State: Peer in Cluster (Connected)

Hostname: node04
Uuid: 779ab1b3-fda9-46da-af95-ba56477bf638
State: Peer in Cluster (Connected)

# create volume
[root@node01 ~]# gluster volume create vol_dist-replica replica 2 transport tcp \
node01:/glusterfs/dist-replica \
node02:/glusterfs/dist-replica \
node03:/glusterfs/dist-replica \
node04:/glusterfs/dist-replica 
volume create: vol_dist-replica: success: please start the volume to access data

# start volume
[root@node01 ~]# gluster volume start vol_dist-replica 
volume start: vol_dist-replica: success

# show volume info
[root@node01 ~]# gluster volume info 

Volume Name: vol_dist-replica
Type: Distributed-Replicate
Volume ID: 784d2953-6599-4102-afc2-9069932894cc
Status: Started
Number of Bricks: 2 x 2 = 4
Transport-type: tcp
Bricks:
Brick1: node01:/glusterfs/dist-replica
Brick2: node02:/glusterfs/dist-replica
Brick3: node03:/glusterfs/dist-replica
Brick4: node04:/glusterfs/dist-replica
Options Reconfigured:
transport.address-family: inet
performance.readdir-ahead: on
nfs.disable: on
```

## 2.5 条带加镜像

```Shell
[root@node01 ~]# mkdir /glusterfs/strip-replica 

# probe the node
[root@node01 ~]# gluster peer probe node02 
peer probe: success. 
[root@node01 ~]# gluster peer probe node03 
peer probe: success. 
[root@node01 ~]# gluster peer probe node04 
peer probe: success.

# show status
[root@node01 ~]# gluster peer status 
Number of Peers: 3

Hostname: node02
Uuid: 2ca22769-28a1-4204-9957-886579db2231
State: Peer in Cluster (Connected)

Hostname: node03
Uuid: 79cff591-1e98-4617-953c-0d3e334cf96a
State: Peer in Cluster (Connected)

Hostname: node04
Uuid: 779ab1b3-fda9-46da-af95-ba56477bf638
State: Peer in Cluster (Connected)

# create volume
[root@node01 ~]# gluster volume create vol_strip-replica stripe 2 replica 2 transport tcp \
node01:/glusterfs/strip-replica \
node02:/glusterfs/strip-replica \
node03:/glusterfs/strip-replica \
node04:/glusterfs/strip-replica 
volume create: vol_strip-replica: success: please start the volume to access data

# start volume
[root@node01 ~]# gluster volume start vol_strip-replica 
volume start: vol_strip-replica: success

# show volume info
[root@node01 ~]# gluster volume info 

Volume Name: vol_strip-replica
Type: Striped-Replicate
Volume ID: ec36b0d3-8467-47f6-aa83-1020555f58b6
Status: Started
Number of Bricks: 1 x 2 x 2 = 4
Transport-type: tcp
Bricks:
Brick1: node01:/glusterfs/strip-replica
Brick2: node02:/glusterfs/strip-replica
Brick3: node03:/glusterfs/strip-replica
Brick4: node04:/glusterfs/strip-replica
Options Reconfigured:
transport.address-family: inet
performance.readdir-ahead: on
nfs.disable: on
```

# 3. 客户端配置使用

```Shell
[root@client ~]# yum -y install centos-release-gluster40
[root@client ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-Gluster-4.0.repo
[root@client ~]# yum --enablerepo=centos-gluster40 -y install glusterfs glusterfs-fuse

# for example, mount [vol_distributed] volume on /mnt
[root@client ~]# mount -t glusterfs node01.centos.org:/vol_distributed /mnt 
[root@client ~]# df -hT 
Filesystem                        Type            Size  Used Avail Use% Mounted on
/dev/mapper/cl-root               xfs              26G  1.4G   25G   6% /
devtmpfs                          devtmpfs        2.0G     0  2.0G   0% /dev
tmpfs                             tmpfs           2.0G     0  2.0G   0% /dev/shm
tmpfs                             tmpfs           2.0G  8.4M  2.0G   1% /run
tmpfs                             tmpfs           2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/vda1                         xfs            1014M  234M  781M  24% /boot
tmpfs                             tmpfs           396M     0  396M   0% /run/user/0
node01.centos.org:/vol_distributed fuse.glusterfs   52G  3.2G   49G   7% /mnt
```

# 4. GlusterFS与NFS-Ganesha结合

## 4.1 准备工作

```Shell
# OK if [nfs.disable: on] (default setting)
[root@node01 ~]# gluster volume get vol_distributed nfs.disable 
Option                                  Value
------                                  -----
nfs.disable                             on

# if [nfs.disable: off], turn to disable
[root@node01 ~]# gluster volume set vol_distributed nfs.disable on 
volume set: success

# if NFS server is running, disable it
[root@node01 ~]# systemctl stop nfs-server 
[root@node01 ~]# systemctl disable nfs-server 
```

## 4.2 在集群的一个节点上安装配置NFS-Ganesha

```Shell
[root@node01 ~]# yum --enablerepo=centos-gluster40 -y install nfs-ganesha-gluster
[root@node01 ~]# mv /etc/ganesha/ganesha.conf /etc/ganesha/ganesha.conf.org 
[root@node01 ~]# vi /etc/ganesha/ganesha.conf
# create new
NFS_CORE_PARAM {
    # possible to mount with NFSv3 to NFSv4 Pseudo path
    mount_path_pseudo = true;
    
    # NFS protocol
    Protocols = 3,4;
}

EXPORT_DEFAULTS {
    # default access mode
    Access_Type = RW;
}

EXPORT {
    # uniq ID
    Export_Id = 101;
    
    # mount path of Gluster Volume
    Path = "/vol_distributed";
    
    FSAL {
    	# any name
        name = GLUSTER;
        
        # hostname or IP address of this Node
        hostname="10.0.0.51";
        
        # Gluster volume name
        volume="vol_distributed";
    }
    
    # config for root Squash
    Squash="No_root_squash";
    
    # NFSv4 Pseudo path
    Pseudo="/vfs_distributed";
    
    # allowed security options
    SecType = "sys";
}
LOG {
    # default log level
    Default_Log_Level = WARN;
}

[root@node01 ~]# systemctl start nfs-ganesha 
[root@node01 ~]# systemctl enable nfs-ganesha

# verify mount
[root@node01 ~]# showmount -e localhost 
Export list for localhost:
/vfs_distributed (everyone)
```

## 4.3 变更SELinux策略

```Shell
[root@node01 ~]# vi ganeshanfsd.te

# create new
module ganeshanfsd 1.0;

require {
        type random_device_t;
        type portmap_port_t;
        type reserved_port_t;
        type ganesha_t;
        class capability dac_override;
        class tcp_socket name_connect;
        class chr_file getattr;
}

#============= ganesha_t ==============
allow ganesha_t portmap_port_t:tcp_socket name_connect;
allow ganesha_t random_device_t:chr_file getattr;
allow ganesha_t reserved_port_t:tcp_socket name_connect;
allow ganesha_t self:capability dac_override;

[root@node01 ~]# checkmodule -m -M -o ganeshanfsd.mod ganeshanfsd.te 
checkmodule: loading policy configuration from ganeshanfsd.te
checkmodule: policy configuration loaded
checkmodule: writing binary representation (version 17) to ganeshanfsd.mod

[root@node01 ~]# semodule_package --outfile ganeshanfsd.pp --module ganeshanfsd.mod 
[root@node01 ~]# semodule -i ganeshanfsd.pp 
```

## 4.4 添加防火墙策略

```Shell
[root@node01 ~]# firewall-cmd --add-service=nfs --permanent 
success
[root@node01 ~]# firewall-cmd --reload 
success
```

## 4.5 客户端挂载使用

```Shell
[root@client ~]# yum -y install nfs-utils

# specify Pseudo path set on [Pseudo=***] in ganesha.conf
[root@client ~]# mount -t nfs4 node01.centos.org:/vfs_distributed /mnt 

[root@client ~]# df -hT 
Filesystem                        Type      Size  Used Avail Use% Mounted on
/dev/mapper/cl-root               xfs        26G  1.4G   25G   6% /
devtmpfs                          devtmpfs  2.0G     0  2.0G   0% /dev
tmpfs                             tmpfs     2.0G     0  2.0G   0% /dev/shm
tmpfs                             tmpfs     2.0G  8.4M  2.0G   1% /run
tmpfs                             tmpfs     2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/vda1                         xfs      1014M  234M  781M  24% /boot
tmpfs                             tmpfs     396M     0  396M   0% /run/user/0
node01.centso.org:/vfs_distributed nfs4       52G  3.3G   49G   7% /mnt
```