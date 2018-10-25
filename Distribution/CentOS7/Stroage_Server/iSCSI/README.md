- 1 配置iSCSI Target
    - 1.1 安装配置Target
    - 1.2 添加防火墙策略
- 2 配置iSCSI Initiator
    - 2.1 安装配置Initiator
    - 2.2 使用iSCSI磁盘

# 1. 配置iSCSI Target

配置iSCSI存储服务器，实验环境如下：

```
[iSCSI Target]
domain name：lab.centos.org
IP：10.0.030

[iSCSI Initiator]
domain name：www.centos.org
IP: 10.0.0.31
```

## 1.1 安装配置Target

```Shell
[root@lab ~]# yum -y install targetcli

# create a directory
[root@lab ~]# mkdir /var/lib/iscsi_disks 

# enter the admin console
[root@lab ~]# targetcli 
targetcli shell version 2.1.fb34
Copyright 2011-2013 by Datera, Inc and others.
For help on commands, type 'help'.

/> cd backstores/fileio
# create a disk-image with the name "disk01" on /var/lib/iscsi_disks/disk01.img with 10G
/backstores/fileio> create disk01 /var/lib/iscsi_disks/disk01.img 10G 
Created fileio disk01 with size 10737418240


/backstores/fileio> cd /iscsi
# create a target
/iscsi> create iqn.2014-07.centos.org:storage.target00 
Created target iqn.2014-07.centos.org:storage.target00.
Created TPG 1.
Global pref auto_add_default_portal=true
Created default portal listening on all IPs (0.0.0.0), port 3260.

/iscsi> cd iqn.2014-07.centos.org:storage.target00/tpg1/luns
# set LUN
/iscsi/iqn.20...t00/tpg1/luns> create /backstores/fileio/disk01 
Created LUN 0.

/iscsi/iqn.20...t00/tpg1/luns> cd ../acls
# set ACL (it's the IQN of an initiator you permit to connect)
/iscsi/iqn.20...t00/tpg1/acls> create iqn.2014-07.centos.org:www.centos.org 
Created Node ACL for iqn.2014-07.centos.org:www.centos.org
Created mapped LUN 0.
/iscsi/iqn.20...t00/tpg1/acls> cd iqn.2014-07.centos.org:www.centos.org

# set UserID for authentication
/iscsi/iqn.20....srv.world> set auth userid=username 
Parameter userid is now 'username'.
/iscsi/iqn.20....srv.world> set auth password=password 
Parameter password is now 'password'.
/iscsi/iqn.20....srv.world> exit 
Global pref auto_save_on_exit=true
Last 10 configs saved in /etc/target/backup.
Configuration saved to /etc/target/saveconfig.json

# after configuration above, the target enters in listening like follows.
[root@lab ~]# ss -napt | grep 3260 
LISTEN     0      256          *:3260                     *:*

[root@lab ~]# systemctl enable target
```

## 1.2 添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-service=iscsi-target --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

# 2. 配置iSCSI Initiator

## 2.1 安装配置Initiator

```Shell
[root@www ~]# yum -y install iscsi-initiator-utils

[root@www ~]# vi /etc/iscsi/initiatorname.iscsi
# change to the same IQN you set on the iSCSI target server
InitiatorName=iqn.2014-07.centos.org:www.centos.org

[root@www ~]# vi /etc/iscsi/iscsid.conf

# line 57: uncomment
node.session.auth.authmethod = CHAP

# line 61,62: uncomment and specify the username and password you set on the iSCSI target server
node.session.auth.username = username
node.session.auth.password = password

# discover target
[root@www ~]# iscsiadm -m discovery -t sendtargets -p 10.0.0.30 
[  635.510656] iscsi: registered transport (tcp)
10.0.0.30:3260,1 iqn.2014-07.centos.org:storage.target00

# confirm status after discovery
[root@www ~]# iscsiadm -m node -o show 
# BEGIN RECORD 6.2.0.873-21
node.name = iqn.2014-07.centos.org:storage.target00
node.tpgt = 1
node.startup = automatic
node.leading_login = No
...
...
...
node.conn[0].iscsi.IFMarker = No
node.conn[0].iscsi.OFMarker = No
# END RECORD

# login to the target
[root@www ~]# iscsiadm -m node --login 
Logging in to [iface: default, target: iqn.2014-07.centos.org:storage.target00, portal: 10.0.0.30,3260] (multiple)
[  708.383308] scsi2 : iSCSI Initiator over TCP/IP
[  709.393277] scsi 2:0:0:0: Direct-Access     LIO-ORG  disk01           4.0  PQ: 0 ANSI: 5
[  709.395709] scsi 2:0:0:0: alua: supports implicit and explicit TPGS
[  709.398155] scsi 2:0:0:0: alua: port group 00 rel port 01
[  709.399762] scsi 2:0:0:0: alua: port group 00 state A non-preferred supports TOlUSNA
[  709.401763] scsi 2:0:0:0: alua: Attached
[  709.402910] scsi 2:0:0:0: Attached scsi generic sg0 type 0
Login to [iface: default, target: iqn.2014-07.centos.org:storage.target00, portal: 10.0.0.30,3260] successful.

# confirm the established session
[root@www ~]# iscsiadm -m session -o show 
tcp: [1] 10.0.0.30:3260,1 iqn.2014-07.centos.org:storage.target00 (non-flash)

# confirm the partitions
[root@www ~]# cat /proc/partitions 
major minor  #blocks  name

 252        0   52428800 sda
 252        1     512000 sda1
 252        2   51915776 sda2
 253        0    4079616 dm-0
 253        1   47833088 dm-1
   8        0   20971520 sdb
# added new device provided from the target server as "sdb"
```

## 2.2 使用iSCSI磁盘
```Shell
# create label
[root@www ~]# parted --script /dev/sdb "mklabel msdos" 

# create partiton
[root@www ~]# parted --script /dev/sdb "mkpart primary 0% 100%" 

# format with XFS
[root@www ~]# mkfs.xfs -i size=1024 -s size=4096 /dev/sdb1 
meta-data=/dev/sdb1        isize=1024   agcount=16, agsize=327616 blks
         =                 sectsz=4096  attr=2, projid32bit=1
         =                 crc=0
data     =                 bsize=4096   blocks=5241856, imaxpct=25
         =                 sunit=0      swidth=0 blks
naming   =version 2        bsize=4096   ascii-ci=0 ftype=0
log      =internal log     bsize=4096   blocks=2560, version=2
         =                 sectsz=4096  sunit=1 blks, lazy-count=1
realtime =none             extsz=4096   blocks=0, rtextents=0

# mount it
[root@www ~]# mount /dev/sdb1 /mnt 
[ 6894.010661] XFS (sdb1): Mounting Filesystem
[ 6894.031358] XFS (sdb1): Ending clean mount
[root@www ~]# df -hT 
Filesystem              Type      Size  Used Avail Use% Mounted on
/dev/mapper/centos-root xfs        46G 1023M   45G   3% /
devtmpfs                devtmpfs  1.9G     0  1.9G   0% /dev
tmpfs                   tmpfs     1.9G     0  1.9G   0% /dev/shm
tmpfs                   tmpfs     1.9G  8.3M  1.9G   1% /run
tmpfs                   tmpfs     1.9G     0  1.9G   0% /sys/fs/cgroup
/dev/sda1               xfs       497M  120M  378M  25% /boot
/dev/sdb1               xfs        20G   33M   20G   1% /mnt
Matched Con
```
