- 1 安装KVM
    - 1.1 安装软件包
    - 1.2 配置桥接网卡
- 2 创建虚拟机
    - 2.1 文本模式下网络安装CentOS7
    - 2.2 在HostOS与GuestOS切换控制台
    - 2.3 创建虚拟机模板
- 3 Libvirt基本操作
    - 3.1 启动虚拟机
    - 3.2 关闭虚拟机
    - 3.3 设置自动启动虚拟机
    - 3.4 列出所有虚拟机
    - 3.5 切换控制台
    - 3.6 帮助信息
- 4 实用虚拟机管理工具
    - 4.1 安装工具
    - 4.2 列出虚拟机中指定文件
    - 4.3 查看虚拟机中指定文件
    - 4.4 编辑虚拟机中指定文件
    - 4.5 显示虚拟机中磁盘使用信息
    - 4.6 显示虚拟机中挂载信息
    - 4.7 显示虚拟机运行状态
- 5 实时迁移
    - 5.1 使用NFS共享存储需要更改SELinux策略
    - 5.2 添加防火墙策略
    - 5.3 进行实时迁移
- 6 存储迁移
    - 6.1 更新QEMU
    - 6.2 添加防火墙策略
    - 6.3 在其他KVM机器上建立同等大小的空磁盘
    - 6.4 进行存储迁移
- 7 SPICE
    - 7.1 远程桌面协议浅析（VNC/SPICE/RDP）
    - 7.2 配置SPICE服务器
    - 7.3 配置SPICE客户端
- 8 嵌套虚拟化
    - 8.1 启用嵌套虚拟化
    - 8.2 编辑虚拟机配置文件
- 9 GPU Passthrough
    - 9.1 在KVM主机启用IOMMU
    - 9.2 自动加载vfio-pci模块
    - 9.3 更新QEMU
    - 9.4 创建一个虚拟机并透传GPU
    - 9.5 在虚拟机中确认
- 10 UEFI引导
    - 10.1 Install OVMF (Open Virtual Machine Firmware).
    - 10.2 更新QEMU
    - 10.3 创建一个虚拟机

# 1. 安装KVM

## 1.1 安装软件包

```Shell
[root@lab ~]# yum -y install qemu-kvm libvirt virt-install bridge-utils

# make sure modules are loaded
[root@lab ~]# lsmod | grep kvm 
kvm_intel       138567  0
kvm             441119  1 kvm_intel

[root@lab ~]# systemctl start libvirtd 
[root@lab ~]# systemctl enable libvirtd 
```

## 1.2 配置桥接网卡

```Shell
# add bridge "br0"
[root@lab ~]# nmcli connection add type bridge autoconnect yes con-name br0 ifname br0 
Connection 'br0' (0f4b7bc8-8c7a-461a-bff1-d516b941a6ec) successfully added.

# set IP for br0
[root@lab ~]# nmcli connection modify br0 ipv4.addresses 10.0.0.30/24 ipv4.method manual 

# set Gateway for br0
[root@lab ~]# nmcli connection modify br0 ipv4.gateway 10.0.0.1 

# set DNS for "br0"
[root@lab ~]# nmcli connection modify br0 ipv4.dns 10.0.0.1 

# remove the current setting
[root@lab ~]# nmcli connection delete eth0 

# add an interface again as a member of br0
[root@lab ~]# nmcli connection add type bridge-slave autoconnect yes con-name eth0 ifname eth0 master br0 

# restart
[root@lab ~]# reboot

[root@lab ~]# ip addr 
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> 
    mtu 1500 qdisc pfifo_fast master br0 state UP group default qlen 1000
    link/ether 00:0c:29:9f:9b:d3 brd ff:ff:ff:ff:ff:ff
3: virbr0: <NO-CARRIER,BROADCAST,MULTICAST,UP> mtu 1500 qdisc noqueue state DOWN group default
    link/ether 22:f8:64:25:97:44 brd ff:ff:ff:ff:ff:ff
    inet 192.168.122.1/24 brd 192.168.122.255 scope global virbr0
       valid_lft forever preferred_lft forever
4: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default
    link/ether 00:0c:29:9f:9b:d3 brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.30/24 brd 10.0.0.255 scope global br0
       valid_lft forever preferred_lft forever
    inet6 fe80::20c:29ff:fe9f:9bd3/64 scope link
       valid_lft forever preferred_lft forever
```

# 2. 创建虚拟机

## 2.1 文本模式下网络安装CentOS7

```Shell
[root@lab ~]# mkdir -p /var/kvm/images # create a new Storage Pool
[root@lab ~]# virt-install \
--name centos7 \
--ram 4096 \
--disk path=/var/kvm/images/centos7.img,size=30 \
--vcpus 2 \
--os-type linux \
--os-variant rhel7 \
--network bridge=br0 \
--graphics none \
--console pty,target_type=serial \
--location 'https://mirrors.tuna.tsinghua.edu.cn/centos/7/os/x86_64/' \
--extra-args 'console=ttyS0,115200n8 serial'
Starting install...# start installation
```

## 2.2 在HostOS与GuestOS切换控制台

```Shell
[root@localhost ~]# # push Ctrl + ]

[root@lab ~]# # Host's console

[root@lab ~]# virsh console centos7 # move to Guest
Connected to domain www

Escape character is ^] # Enter key
[root@localhost ~]# # Guest's console
```

## 2.3 创建虚拟机模板

```Shell
[root@lab ~]# virt-clone --original centos7 --name template --file /var/kvm/images/template.img 
Allocating 'template.img' | 20 GB 01:44
Clone 'template' created successfully.

[root@lab ~]# ll /var/kvm/images/template.img   # disk image
-rwxr-xr-x 1 root root 32212254720 Jul 11 23:34 /var/kvm/images/template.img

[root@lab ~]# ll /etc/libvirt/qemu/template.xml   # xml file
-rw------- 1 root root 1843 Jul 11 23:32 /etc/libvirt/qemu/template.xml
```

# 3. Libvirt基本操作

## 3.1 启动虚拟机

```Shell
[root@lab ~]# virsh start centos7 # Start Virtual Machine 'centos7'
Domain centos7 started

[root@lab ~]# virsh start centos7 --console # start and connect to console of 'centos7'
Domain centos7 started
Connected to domain centos7
```

## 3.2 关闭虚拟机

```Shell
[root@lab ~]# virsh shutdown centos7 # Stop Virtual Machine 'centos7'
Domain centos7 is being shutdown

[root@lab ~]# virsh destroy centos7 # Stop fourcely Virtual Machine 'centos7'
Domain centos7 destroyed
```

## 3.3 设置自动启动虚拟机

```Shell
[root@lab ~]# virsh autostart centos7 # Enable auto-start for 'centos7'
Domain centos7 marked as autostarted

[root@lab ~]# virsh autostart --disable centos7 # Disable auto-start for 'centos7'
Domain centos7 unmarked as autostarted
```

## 3.4 列出所有虚拟机

```Shell
[root@lab ~]# virsh list # List all active Virtual Machines
 Id    Name               State
----------------------------------------
 2     centos7            running

[root@lab ~]# virsh list --all # List all Virtual Machines included inactives
 Id    Name               State
----------------------------------------
 -     centos7            shut off
 -     template           shut off
 -     Win2k12            shut off
```

## 3.5 切换控制台

```Shell
[root@lab ~]# virsh console centos7 # connect to 'centos7'
Connected to domain centos7
Escape character is ^]# Enter

CentOS Linux 7 (Core)
Kernel 3.10.0-123.el7.x86_64 on an x86_64
localhost login:# just switched on Guest

Password:
Last login: Sat Jul 12 20:51:10 2014

[root@localhost ~]# # Ctrl + ] key
[root@lab ~]# # just switched on Host
```

## 3.6 帮助信息

```Shell
[root@lab ~]# virsh --help
virsh [options]... [<command_string>]
virsh [options]... <command> [args...]

  options:
    -c | --connect=URI      hypervisor connection URI
    -r | --readonly         connect readonly
    -d | --debug=NUM        debug level [0-4]
    -h | --help             this help
    -q | --quiet            quiet mode
    -t | --timing           print timing information
    -l | --log=FILE         output logging to file
    -v                      short version
    -V                      long version
         --version[=TYPE]   version, TYPE is short or long (default short)
    -e | --escape <char>    set escape sequence for console

  commands (non interactive mode):

 Domain Management (help keyword 'domain')
    attach-device                  attach device from an XML file
    attach-disk                    attach disk device
    attach-interface               attach network interface
    autostart                      autostart a domain
    blkdeviotune                   Set or query a block device I/O tuning parameters.
    blkiotune                      Get or set blkio parameters
    blockcommit                    Start a block commit operation.
    blockcopy                      Start a block copy operation.
    blockjob                       Manage active block operations
    blockpull                      Populate a disk from its backing image.
    blockresize                    Resize block device of domain.
    change-media                   Change media of CD or floppy drive
    console                        connect to the guest console
    cpu-baseline                   compute baseline CPU
    cpu-compare                    compare host CPU with a CPU described by an XML file
    cpu-stats                      show domain cpu statistics
    create                         create a domain from an XML file
    define                         define (but don't start) a domain from an XML file
    desc                           show or set domain's description or title
    destroy                        destroy (stop) a domain
    detach-device                  detach device from an XML file
    detach-disk                    detach disk device
    detach-interface               detach network interface
    domdisplay                     domain display connection URI
    domfstrim                      Invoke fstrim on domain's mounted filesystems.
    domhostname                    print the domain's hostname
    domid                          convert a domain name or UUID to domain id
    domif-setlink                  set link state of a virtual interface
    domiftune                      get/set parameters of a virtual interface
    domjobabort                    abort active domain job
    domjobinfo                     domain job information
    domname                        convert a domain id or UUID to domain name
    dompmsuspend                   suspend a domain gracefully using power management functions
    dompmwakeup                    wakeup a domain from pmsuspended state
    domuuid                        convert a domain name or id to domain UUID
    domxml-from-native             Convert native config to domain XML
    domxml-to-native               Convert domain XML to native config
    dump                           dump the core of a domain to a file for analysis
    dumpxml                        domain information in XML
    edit                           edit XML configuration for a domain
    inject-nmi                     Inject NMI to the guest
    send-key                       Send keycodes to the guest
    send-process-signal            Send signals to processes
    lxc-enter-namespace            LXC Guest Enter Namespace
    managedsave                    managed save of a domain state
    managedsave-remove             Remove managed save of a domain
    maxvcpus                       connection vcpu maximum
    memtune                        Get or set memory parameters
    migrate                        migrate domain to another host
    migrate-setmaxdowntime         set maximum tolerable downtime
    migrate-compcache              get/set compression cache size
    migrate-setspeed               Set the maximum migration bandwidth
    migrate-getspeed               Get the maximum migration bandwidth
    numatune                       Get or set numa parameters
    qemu-attach                    QEMU Attach
    qemu-monitor-command           QEMU Monitor Command
    qemu-agent-command             QEMU Guest Agent Command
    reboot                         reboot a domain
    reset                          reset a domain
    restore                        restore a domain from a saved state in a file
    resume                         resume a domain
    save                           save a domain state to a file
    save-image-define              redefine the XML for a domain's saved state file
    save-image-dumpxml             saved state domain information in XML
    save-image-edit                edit XML for a domain's saved state file
    schedinfo                      show/set scheduler parameters
    screenshot                     take a screenshot of a current domain console and store it into a file
    setmaxmem                      change maximum memory limit
    setmem                         change memory allocation
    setvcpus                       change number of virtual CPUs
    shutdown                       gracefully shutdown a domain
    start                          start a (previously defined) inactive domain
    suspend                        suspend a domain
    ttyconsole                     tty console
    undefine                       undefine a domain
    update-device                  update device from an XML file
    vcpucount                      domain vcpu counts
    vcpuinfo                       detailed domain vcpu information
    vcpupin                        control or query domain vcpu affinity
    emulatorpin                    control or query domain emulator affinity
    vncdisplay                     vnc display

 Domain Monitoring (help keyword 'monitor')
    domblkerror                    Show errors on block devices
    domblkinfo                     domain block device size information
    domblklist                     list all domain blocks
    domblkstat                     get device block stats for a domain
    domcontrol                     domain control interface state
    domif-getlink                  get link state of a virtual interface
    domiflist                      list all domain virtual interfaces
    domifstat                      get network interface stats for a domain
    dominfo                        domain information
    dommemstat                     get memory statistics for a domain
    domstate                       domain state
    list                           list domains

 Host and Hypervisor (help keyword 'host')
    capabilities                   capabilities
    freecell                       NUMA free memory
    hostname                       print the hypervisor hostname
    node-memory-tune               Get or set node memory parameters
    nodecpumap                     node cpu map
    nodecpustats                   Prints cpu stats of the node.
    nodeinfo                       node information
    nodememstats                   Prints memory stats of the node.
    nodesuspend                    suspend the host node for a given time duration
    sysinfo                        print the hypervisor sysinfo
    uri                            print the hypervisor canonical URI
    version                        show version

 Interface (help keyword 'interface')
    iface-begin                    create a snapshot of current interfaces settings, 
                                   which can be later committed (iface-commit) or restored (iface-rollback)
    iface-bridge                   create a bridge device and attach an existing network device to it
    iface-commit                   commit changes made since iface-begin and free restore point
    iface-define                   define (but don't start) a physical host interface from an XML file
    iface-destroy                  destroy a physical host interface (disable it / "if-down")
    iface-dumpxml                  interface information in XML
    iface-edit                     edit XML configuration for a physical host interface
    iface-list                     list physical host interfaces
    iface-mac                      convert an interface name to interface MAC address
    iface-name                     convert an interface MAC address to interface name
    iface-rollback                 rollback to previous saved configuration created via iface-begin
    iface-start                    start a physical host interface (enable it / "if-up")
    iface-unbridge                 undefine a bridge device after detaching its slave device
    iface-undefine                 undefine a physical host interface (remove it from configuration)

 Network Filter (help keyword 'filter')
    nwfilter-define                define or update a network filter from an XML file
    nwfilter-dumpxml               network filter information in XML
    nwfilter-edit                  edit XML configuration for a network filter
    nwfilter-list                  list network filters
    nwfilter-undefine              undefine a network filter

 Networking (help keyword 'network')
    net-autostart                  autostart a network
    net-create                     create a network from an XML file
    net-define                     define (but don't start) a network from an XML file
    net-destroy                    destroy (stop) a network
    net-dumpxml                    network information in XML
    net-edit                       edit XML configuration for a network
    net-info                       network information
    net-list                       list networks
    net-name                       convert a network UUID to network name
    net-start                      start a (previously defined) inactive network
    net-undefine                   undefine an inactive network
    net-update                     update parts of an existing network's configuration
    net-uuid                       convert a network name to network UUID

 Node Device (help keyword 'nodedev')
    nodedev-create                 create a device defined by an XML file on the node
    nodedev-destroy                destroy (stop) a device on the node
    nodedev-detach                 detach node device from its device driver
    nodedev-dumpxml                node device details in XML
    nodedev-list                   enumerate devices on this host
    nodedev-reattach               reattach node device to its device driver
    nodedev-reset                  reset node device

 Secret (help keyword 'secret')
    secret-define                  define or modify a secret from an XML file
    secret-dumpxml                 secret attributes in XML
    secret-get-value               Output a secret value
    secret-list                    list secrets
    secret-set-value               set a secret value
    secret-undefine                undefine a secret

 Snapshot (help keyword 'snapshot')
    snapshot-create                Create a snapshot from XML
    snapshot-create-as             Create a snapshot from a set of args
    snapshot-current               Get or set the current snapshot
    snapshot-delete                Delete a domain snapshot
    snapshot-dumpxml               Dump XML for a domain snapshot
    snapshot-edit                  edit XML for a snapshot
    snapshot-info                  snapshot information
    snapshot-list                  List snapshots for a domain
    snapshot-parent                Get the name of the parent of a snapshot
    snapshot-revert                Revert a domain to a snapshot

 Storage Pool (help keyword 'pool')
    find-storage-pool-sources-as   find potential storage pool sources
    find-storage-pool-sources      discover potential storage pool sources
    pool-autostart                 autostart a pool
    pool-build                     build a pool
    pool-create-as                 create a pool from a set of args
    pool-create                    create a pool from an XML file
    pool-define-as                 define a pool from a set of args
    pool-define                    define (but don't start) a pool from an XML file
    pool-delete                    delete a pool
    pool-destroy                   destroy (stop) a pool
    pool-dumpxml                   pool information in XML
    pool-edit                      edit XML configuration for a storage pool
    pool-info                      storage pool information
    pool-list                      list pools
    pool-name                      convert a pool UUID to pool name
    pool-refresh                   refresh a pool
    pool-start                     start a (previously defined) inactive pool
    pool-undefine                  undefine an inactive pool
    pool-uuid                      convert a pool name to pool UUID

 Storage Volume (help keyword 'volume')
    vol-clone                      clone a volume.
    vol-create-as                  create a volume from a set of args
    vol-create                     create a vol from an XML file
    vol-create-from                create a vol, using another volume as input
    vol-delete                     delete a vol
    vol-download                   download volume contents to a file
    vol-dumpxml                    vol information in XML
    vol-info                       storage vol information
    vol-key                        returns the volume key for a given volume name or path
    vol-list                       list vols
    vol-name                       returns the volume name for a given volume key or path
    vol-path                       returns the volume path for a given volume name or key
    vol-pool                       returns the storage pool for a given volume key or path
    vol-resize                     resize a vol
    vol-upload                     upload file contents to a volume
    vol-wipe                       wipe a vol

 Virsh itself (help keyword 'virsh')
    cd                             change the current directory
    connect                        (re)connect to hypervisor
    echo                           echo arguments
    exit                           quit this interactive terminal
    help                           print help
    pwd                            print the current directory
    quit                           quit this interactive terminal


  (specify help <group> for details about the commands in the group)

  (specify help <command> for details about the command)
```

# 4. 实用虚拟机管理工具

## 4.1 安装工具

```Shell
[root@lab ~]# yum -y install libguestfs-tools virt-top
```

## 4.2 列出虚拟机中指定文件

```Shell
[root@lab ~]# virt-ls -l -d centos7 /root 
total 36
dr-xr-x---.  2 root root 4096 Jan  8 22:38 .
drwxr-xr-x. 17 root root 4096 Jan  8 22:36 ..
-rw-------.  1 root root   61 Jan  8 22:38 .bash_history
-rw-r--r--.  1 root root   18 Dec 29  2013 .bash_logout
-rw-r--r--.  1 root root  176 Dec 29  2013 .bash_profile
-rw-r--r--.  1 root root  176 Dec 29  2013 .bashrc
...
```

## 4.3 查看虚拟机中指定文件

```Shell
[root@lab ~]# virt-cat -d centos7 /etc/passwd 
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
daemon:x:2:2:daemon:/sbin:/sbin/nologin
adm:x:3:4:adm:/var/adm:/sbin/nologin
lp:x:4:7:lp:/var/spool/lpd:/sbin/nologin
sync:x:5:0:sync:/sbin:/bin/sync
...
```

## 4.4 编辑虚拟机中指定文件

```Shell
[root@lab ~]# virt-edit -d centos7 /etc/fstab 
#
# /etc/fstab
# Created by anaconda on Thu Jan  8 13:20:43 2015
#
# Accessible filesystems, by reference, are maintained under '/dev/disk'
# See man pages fstab(5), findfs(8), mount(8) and/or blkid(8) for more info
#
/dev/mapper/centos-root /                       xfs     defaults        1 1
UUID=537b215f-30a1-4e82-b05d-f480aa8e1034 /boot xfs     defaults        1 2
/dev/mapper/centos-swap swap                    swap    defaults        0 0
```

## 4.5 显示虚拟机中磁盘使用信息

```Shell
[root@lab ~]# virt-df -d centos7 
Filesystem                     1K-blocks       Used  Available  Use%
centos7:/dev/sda1                 508588      72348     436240   15%
centos7:/dev/centos/root         8910848     779252    8131596    9%
```

## 4.6 显示虚拟机中挂载信息

```Shell
[root@lab ~]# guestmount -d centos7 -i /media 
[root@lab ~]# ll /media 
total 32
lrwxrwxrwx.  1 root root    7 Jan  8 22:22 bin -> usr/bin
dr-xr-xr-x.  4 root root 4096 Jan  8 22:37 boot
drwxr-xr-x.  2 root root    6 Jan  8 22:20 dev
drwxr-xr-x. 74 root root 8192 Jan  8 22:36 etc
...
```

## 4.7 显示虚拟机运行状态

```Shell
[root@lab ~]# virt-top 
virt-top 22:32:14 - x86_64 4/4CPU 2801MHz 11968MB
2 domains, 1 active, 1 running, 0 sleeping, 0 paused, 1 inactive D:0 O:0 X:0
CPU: 0.2%  Mem: 500 MB (500 MB by guests)

   ID S RDRQ WRRQ RXBY TXBY %CPU %MEM    TIME   NAME
    6 R    0    0            0.2  4.0   0:09.14 guestfs-o7nss1p3kxvyl1r5
    -                                           (centos7)
```

# 5. 实时迁移

实验环境：
```
                      +----------------------+
                      |   [  NFS Servver  ]  |
                      |     nfs.centos.org   |
                      |                      |
                      +-----------+----------+
                                  |10.0.0.30
                                  |
+----------------------+          |          +----------------------+
|  [   KVM Host #1  ]  |10.0.0.21 | 10.0.0.22|  [  KVM Host #2   ]  |
|                      +----------+----------+                      |
|    kvm01.centos.org  |                     |    kvm02.centos.org  |
+----------------------+                     +----------------------+
```

1. 需要由共享存储如NFS、iSCSI、GlusterFS等用于存储虚拟机磁盘文件。
2. 共享存储的挂载点是/var/kvm/images。

## 5.1 使用NFS共享存储需要更改SELinux策略

```Shell
[root@kvm01 ~]# setsebool -P virt_use_nfs on
```

## 5.2 添加防火墙策略

```Shell
[root@kvm01 ~]# firewall-cmd --add-service={libvirt,libvirt-tls} --permanent 
[root@kvm01 ~]# firewall-cmd --add-port=49152-49215/tcp --permanent 
[root@kvm01 ~]# firewall-cmd --reload 
```

## 5.3 进行实时迁移

```Shell
# edit setting of a VM

root@kvm01:~# virsh edit centos7
 <disk type='file' device='disk'>
      # add: change cache mode to [none]
      <driver name='qemu' type='qcow2' cache='none'/>
      <source file='/var/kvm/images/centos7.img'/>

root@kvm01:~# virsh start centos7 
root@kvm01:~# virsh list 
 Id    Name                           State
----------------------------------------------------
 1     centos7                        running
 
 
[root@kvm01 ~]# virsh migrate --live centos7 qemu+ssh://10.0.0.22/system 
root@10.0.0.22's password:     # root password

[root@kvm01 ~]# virsh list 
 Id    Name             State
--------------------------------------
# just migrated

### on another KVM Host ###
[root@kvm02 ~]# virsh list 
 Id    Name              State
---------------------------------------
 1     centos7           running

# back to the KVM Host again like follows
[root@kvm02 ~]# virsh migrate --live centos7 qemu+ssh://10.0.0.21/system 
root@10.0.0.21's password:

[root@kvm02 ~]# virsh list 
 Id    Name             State
--------------------------------------
```

# 6. 存储迁移

实验环境：
```
                         Storage Migration
                        <------------------->
                        
+----------------------+                     +----------------------+
|  [   KVM Host #1  ]  |10.0.0.21   10.0.0.22|  [  KVM Host #2   ]  |
|                      +---------------------+                      |
|   kvm01.centos.org   |                     |    kvm02.centos.org  |
+----------------------+                     +----------------------+
```

在没有共享存储情况下可以使用存储迁移。

## 6.1 更新QEMU

```Shell
[root@kvm01 ~]# /usr/libexec/qemu-kvm -version 
QEMU emulator version 1.5.3 (qemu-kvm-1.5.3-156.el7), Copyright (c) 2003-2008 Fabrice Bellard

[root@kvm01 ~]# yum -y install centos-release-qemu-ev

# disable on default
[root@kvm01 ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-QEMU-EV.repo

# for this installing, [qemu-kvm] package is replaced to [qemu-kvm-ev] package
[root@kvm01 ~]# yum --enablerepo=centos-qemu-ev -y install qemu-kvm-ev

[root@kvm01 ~]# systemctl restart libvirtd 

[root@kvm01 ~]# /usr/libexec/qemu-kvm -version 
QEMU emulator version 2.10.0(qemu-kvm-ev-2.10.0-21.el7_5.2.1)
Copyright (c) 2003-2017 Fabrice Bellard and the QEMU Project developers
```

## 6.2 添加防火墙策略

```Shell
[root@kvm01 ~]# firewall-cmd --add-service={libvirt,libvirt-tls} --permanent 
[root@kvm01 ~]# firewall-cmd --add-port=49152-49215/tcp --permanent 
[root@kvm01 ~]# firewall-cmd --reload 
```

## 6.3 在其他KVM机器上建立同等大小的空磁盘

```Shell
# show the size of Virtual machine
root@kvm01:~# ll /var/kvm/images 
total 1344456
-rw-------. 1 root root 32217432064 May 19 23:44 centos7.img

### on another KVM host ###
# create a disk which is the same size of a Virtual Machine
root@kvm02:~# qemu-img create -f qcow2 /var/kvm/images/centos7.img 32217432064 
root@kvm02:~# ll /var/kvm/images 
total 196
-rw-r--r--. 1 root root 197120 May 20 00:19 centos7.img
```

## 6.4 进行存储迁移

```
root@kvm01:~# virsh list 
 Id    Name                           State
----------------------------------------------------
 1     centos7                        running

root@kvm01:~# virsh migrate --live --copy-storage-all centos7 qemu+ssh://10.0.0.22/system 
root@10.0.0.22's password:

root@kvm01:~# virsh list 
 Id    Name             State
--------------------------------------
# just migrated

### on another KVM host ###
root@kvm02:~# virsh list 
 Id    Name                           State
----------------------------------------------------
 1     centos7                        running

# back to another host, it's OK to execute normal Live Migration
root@kvm02:~# virsh migrate --live centos7 qemu+ssh://10.0.0.21/system 
root@10.0.0.21's password:

root@kvm02:~# virsh list 
 Id    Name             State
--------------------------------------

```

# 7. SPICE

## 7.1 远程桌面协议浅析（VNC/SPICE/RDP）

 \  | SPICE | VNC | DRP
---|---|---|---
BIOS屏幕显示 | 能 | 能 | 不能
全彩支持 | 能 | 能 | 能
更改分辨率 | 能 | 能 | 能
多显示器 |多显示器支持（高达4画面）	| 只有一个屏幕 | 多显示器支持
图像传输 | 图像和图形传输 | 图像传输 | 图像和图形传输
视频播放支持| GPU加速支持 | 不能 | GPU加速支持
音频传输 | 双向语音可以控制 | 不能 | 双向语音可以控制
鼠标控制 | 客户端服务器都可以控制 | 服务器端控制 | 服务器端控制
USB传输 | USB可以通过网络传输 | 不能 | USB可以通过网络传输

- VNC:
    - 适用系统：windows、linux
    - 网络流量：较小，常用100k左右
    - 适用场景：主要用于linux的服务器的管理，由于无声音和usb传输，不满足于虚拟桌面的使用
- SPICE：
    - 适用系统：linux
    - 网络流量：较大，正常使用10-20M
    - 适用场景：由于在色彩、音频和usb方面，适用于虚拟桌面，主要用于虚拟机的虚拟桌面应用
- DRP：
    - 适用系统：windows、linux
    - 网络流量：较小，正常使用100-200k左右
    - 适用场景：由于在色彩、音频、usb及本地磁盘映射方面较好，非常适用于虚拟桌面


## 7.2 配置SPICE服务器

**1. 安装**

```Shell
[root@lab ~]# yum -y install spice-server spice-protocol 
```

**2. 配置已创建虚拟机的XML文件**

```Shell
[root@lab ~]# virsh edit centos7 # edit the configration of "centos7"
<domain type='kvm'>
  <name>centos7</name>
  <uuid>b38a50ca-a1ae-4d37-ba10-caf1e05b43ce</uuid>
  <memory unit='KiB'>4194304</memory>
  <currentMemory unit='KiB'>4194304</currentMemory>
  <vcpu placement='static'>2</vcpu>
  .
  .
  .
      # add follows
      # set any password for "passwd=***" section
      # specify a uniq number for "sound" section "slot='0x06'"
      # the "slot='0x02'" in video section is fixed number for graphics
     <graphics type='spice' port='5900' autoport='no' listen='0.0.0.0' passwd='password'>
      <listen type='address' address='0.0.0.0'/>
    </graphics>
    <sound model='ac97'>
      <address type='pci' domain='0x0000' bus='0x00' slot='0x06' function='0x0'/>
    </sound>
    <video>
      <model type='qxl' ram='65536' vram='32768' heads='1'/>
      <address type='pci' domain='0x0000' bus='0x00' slot='0x02' function='0x0'/>
    </video>
     <memballoon model='virtio'>
      <address type='pci' domain='0x0000' bus='0x00' slot='0x05' function='0x0'/>
    </memballoon>
  </devices>
</domain>

Domain centos7 XML configuration edited.

[root@lab ~]# virsh start centos7 # start VM
Domain centos7 started
```

**3. 创建虚拟机时候配置SPICE**

```Shell
[root@lab ~]# virt-install \
--name Win2k12R2 \
--ram 6144 \
--disk path=/var/kvm/images/Win2k12R2.img,size=100 \
--vcpus=4 \
--os-type windows \
--os-variant=win2k12r2 \
--network bridge=br0 \
--graphics spice,listen=0.0.0.0,password=password,keymap=en \
--video qxl \
--cdrom /tmp/X64FRE_SERVER_EVAL_JA-JP-IRM_SSS_X64FREE_JA-JP_DV5.ISO
```

**4. 添加防火墙策略**

```Shell
[root@lab ~]# firewall-cmd --add-port=5900/tcp --permanent 
[root@lab ~]# firewall-cmd --reload 
```

## 7.3 配置SPICE客户端

- Linux图形化客户端：Remote Desktop Viewer
- Windows图形化客户端：Remote viewer

# 8. 嵌套虚拟化

## 8.1 启用嵌套虚拟化

```Shell
[root@lab ~]# vi /etc/modprobe.d/kvm-nested.conf

# create new
options kvm_intel nested=1
[root@lab ~]# modprobe -r kvm_intel # unload
[root@lab ~]# modprobe kvm_intel # reload again

[root@lab ~]# cat /sys/module/kvm_intel/parameters/nested 
Y# just enabled
```
## 8.2 编辑虚拟机配置文件

```Shell
[root@lab ~]# virsh edit centos7 # edit a virtual machine "centos7"

# change like folows for "cpu mode" section
<cpu mode='host-passthrough'>
```

# 9. GPU Passthrough

虚拟机需要部署特殊的应用场景时需要GPU透传，例如：
- GPU加速：CUDA
- 深度学习：TensorFlow

配置之前需要在BIOS中启用Vt-d特性。

## 9.1 在KVM主机启用IOMMU

```Shell
[root@lab ~]# vi /etc/default/grub

# line 6: add (if AMD CPU, add [amd_iommu=on])
GRUB_TIMEOUT=5
GRUB_DISTRIBUTOR="$(sed 's, release .*$,,g' /etc/system-release)"
GRUB_DEFAULT=saved
GRUB_DISABLE_SUBMENU=true
GRUB_TERMINAL_OUTPUT="console"
GRUB_CMDLINE_LINUX="rd.lvm.lv=centos/root rd.lvm.lv=centos/swap rhgb quiet intel_iommu=on"
GRUB_DISABLE_RECOVERY="true"

[root@lab ~]# grub2-mkconfig -o /boot/grub2/grub.cfg 
[root@lab ~]# reboot

# after rebooting, verify IOMMU is enabled like follows
[root@dlp ~]# dmesg | grep -E "DMAR|IOMMU" 
[    0.000000] ACPI: DMAR 00000000bf79e0d0 00118 (v01    AMI  OEMDMAR 00000001 MSFT 00000097)
[    0.000000] DMAR: IOMMU enabled
[    0.069846] DMAR: Host address width 40
[    0.069848] DMAR: DRHD base: 0x000000fbffe000 flags: 0x1
[    0.069856] DMAR: dmar0: reg_base_addr fbffe000 ver 1:0 cap c90780106f0462 ecap f020fe
[    0.069858] DMAR: RMRR base: 0x000000000ec000 end: 0x000000000effff
[    0.069859] DMAR: RMRR base: 0x000000bf7ec000 end: 0x000000bf7fffff
[    0.069861] DMAR: ATSR flags: 0x0
[    0.069863] DMAR-IR: IOAPIC id 6 under DRHD base  0xfbffe000 IOMMU 0
[    0.070128] DMAR-IR: Enabled IRQ remapping in xapic mode
[    0.960840] DMAR: dmar0: Using Queued invalidation
[    0.960860] DMAR: Setting RMRR:
[    0.960890] DMAR: Setting identity map for device 0000:00:1a.0 [0xbf7ec000 - 0xbf7fffff]
[    0.960925] DMAR: Setting identity map for device 0000:00:1a.1 [0xbf7ec000 - 0xbf7fffff]
[    0.960958] DMAR: Setting identity map for device 0000:00:1a.2 [0xbf7ec000 - 0xbf7fffff]
[    0.960994] DMAR: Setting identity map for device 0000:00:1a.7 [0xbf7ec000 - 0xbf7fffff]
[    0.961031] DMAR: Setting identity map for device 0000:00:1d.0 [0xbf7ec000 - 0xbf7fffff]
[    0.961064] DMAR: Setting identity map for device 0000:00:1d.1 [0xbf7ec000 - 0xbf7fffff]
[    0.961100] DMAR: Setting identity map for device 0000:00:1d.2 [0xbf7ec000 - 0xbf7fffff]
[    0.961132] DMAR: Setting identity map for device 0000:00:1d.7 [0xbf7ec000 - 0xbf7fffff]
[    0.961153] DMAR: Setting identity map for device 0000:00:1a.0 [0xec000 - 0xeffff]
[    0.961167] DMAR: Setting identity map for device 0000:00:1a.1 [0xec000 - 0xeffff]
[    0.961182] DMAR: Setting identity map for device 0000:00:1a.2 [0xec000 - 0xeffff]
[    0.961196] DMAR: Setting identity map for device 0000:00:1a.7 [0xec000 - 0xeffff]
[    0.961210] DMAR: Setting identity map for device 0000:00:1d.0 [0xec000 - 0xeffff]
[    0.961224] DMAR: Setting identity map for device 0000:00:1d.1 [0xec000 - 0xeffff]
[    0.961238] DMAR: Setting identity map for device 0000:00:1d.2 [0xec000 - 0xeffff]
[    0.961255] DMAR: Setting identity map for device 0000:00:1d.7 [0xec000 - 0xeffff]
[    0.961270] DMAR: Prepare 0-16MiB unity mapping for LPC
[    0.961287] DMAR: Setting identity map for device 0000:00:1f.0 [0x0 - 0xffffff]
[    0.961424] DMAR: Intel(R) Virtualization Technology for Directed I/O
```

## 9.2 自动加载vfio-pci模块

```Shell
# OK with common kernel
[root@lab ~]# uname -a 
Linux lab.centos.org 3.10.0-693.17.1.el7.x86_64 #1 SMP Thu Jan 25 20:13:58 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux

# show PCI identification number and [vendor-ID:device-ID] of Graphic card
# PCI number ⇒ it matchs [03:00.*] below, vendor-ID:device-ID ⇒ it matchs [10de:***] below
[root@lab ~]# lspci -nn | grep -i nvidia 
03:00.0 VGA compatible controller [0300]: NVIDIA Corporation GP106 [GeForce GTX 1060 6GB] [10de:1c03] (rev a1)
03:00.1 Audio device [0403]: NVIDIA Corporation GP106 High Definition Audio Controller [10de:10f1] (rev a1)

[root@lab ~]# vi /etc/modprobe.d/vfio.conf

# create new: for [ids=***], specify [vendor-ID:device-ID]
options vfio-pci ids=10de:1c03,10de:10f1

[root@lab ~]# echo 'vfio-pci' > /etc/modules-load.d/vfio-pci.conf 
[root@lab ~]# reboot

# it's OK if it's enabled like follows
[root@lab ~]# dmesg | grep -i vfio 
[    4.338817] VFIO - User Level meta-driver version: 0.3
[    4.348806] vfio_pci: add [10de:1c03[ffff:ffff]] class 0x000000/00000000
[    4.348958] vfio_pci: add [10de:10f1[ffff:ffff]] class 0x000000/00000000
```

## 9.3 更新QEMU

```Shell
[root@lab ~]# /usr/libexec/qemu-kvm -version 
QEMU emulator version 1.5.3 (qemu-kvm-1.5.3-141.el7_4.6), Copyright (c) 2003-2008 Fabrice Bellard

[root@lab ~]# yum -y install centos-release-qemu-ev

# disable usually
[root@lab ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-QEMU-EV.repo

# for this installing, [qemu-kvm] package is replaced to [qemu-kvm-ev] package
[root@lab ~]# yum --enablerepo=centos-qemu-ev -y install qemu-kvm-ev
[root@lab ~]# systemctl restart libvirtd
[root@lab ~]# /usr/libexec/qemu-kvm -version 
QEMU emulator version 2.9.0(qemu-kvm-ev-2.9.0-16.el7_4.13.1)
Copyright (c) 2003-2017 Fabrice Bellard and the QEMU Project developers

# verify q35 is included like follows
[root@lab ~]# /usr/libexec/qemu-kvm -machine help 
Supported machines are:
pc                   RHEL 7.4.0 PC (i440FX + PIIX, 1996) (alias of pc-i440fx-rhel7.4.0)
pc-i440fx-rhel7.4.0  RHEL 7.4.0 PC (i440FX + PIIX, 1996) (default)
pc-i440fx-rhel7.3.0  RHEL 7.3.0 PC (i440FX + PIIX, 1996)
pc-i440fx-rhel7.2.0  RHEL 7.2.0 PC (i440FX + PIIX, 1996)
pc-i440fx-rhel7.1.0  RHEL 7.1.0 PC (i440FX + PIIX, 1996)
pc-i440fx-rhel7.0.0  RHEL 7.0.0 PC (i440FX + PIIX, 1996)
rhel6.6.0            RHEL 6.6.0 PC
rhel6.5.0            RHEL 6.5.0 PC
rhel6.4.0            RHEL 6.4.0 PC
rhel6.3.0            RHEL 6.3.0 PC
rhel6.2.0            RHEL 6.2.0 PC
rhel6.1.0            RHEL 6.1.0 PC
rhel6.0.0            RHEL 6.0.0 PC
q35                  RHEL-7.4.0 PC (Q35 + ICH9, 2009) (alias of pc-q35-rhel7.4.0)
pc-q35-rhel7.4.0     RHEL-7.4.0 PC (Q35 + ICH9, 2009)
pc-q35-rhel7.3.0     RHEL-7.3.0 PC (Q35 + ICH9, 2009)
none                 empty machine
```

## 9.4 创建一个虚拟机并透传GPU

```Shell
[root@lab ~]# virt-install \
--name centos7 \
--ram 8192 \
--disk path=/var/kvm/images/centos7.img,size=30 \
--vcpus 4 \
--os-type linux \
--os-variant rhel7 \
--network bridge=br0 \
--graphics none \
--console pty,target_type=serial \
--location 'https://mirrors.aliyun.com/centos/7/os/x86_64/' \
--extra-args 'console=ttyS0,115200n8 serial' \
--host-device 03:00.0 \
--features kvm_hidden=on \
--machine q35
```

## 9.5 在虚拟机中确认

```Shell
[root@localhost ~]# lspci | grep -i nvidia 
04:00.0 VGA compatible controller: NVIDIA Corporation GP106 [GeForce GTX 1060 6GB] (rev a1)
```

# 10. UEFI引导

## 10.1 Install OVMF (Open Virtual Machine Firmware).

```Shell
[root@lab ~]# vi /etc/yum.repos.d/kraxel.repo

# create new
[qemu-firmware-jenkins]
name=firmware for qemu, built by jenkins, fresh from git repos
baseurl=https://www.kraxel.org/repos/jenkins/
enabled=0
gpgcheck=0

[root@lab ~]# yum --enablerepo=qemu-firmware-jenkins -y install OVMF

[root@lab ~]# vi /etc/libvirt/qemu.conf

# line 682: add
nvram = [
    "/usr/share/edk2.git/ovmf-x64/OVMF_CODE-pure-efi.fd:/usr/share/edk2.git/ovmf-x64/OVMF_VARS-pure-efi.fd",
]

[root@lab ~]# systemctl restart libvirtd
```

## 10.2 更新QEMU

```Shell
[root@lab ~]# /usr/libexec/qemu-kvm -version 
QEMU emulator version 1.5.3 (qemu-kvm-1.5.3-141.el7_4.6), Copyright (c) 2003-2008 Fabrice Bellard

[root@lab ~]# yum -y install centos-release-qemu-ev

# disable usually
[root@lab ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-QEMU-EV.repo

# for this installing, [qemu-kvm] package is replaced to [qemu-kvm-ev] package
[root@lab ~]# yum --enablerepo=centos-qemu-ev -y install qemu-kvm-ev
[root@lab ~]# systemctl restart libvirtd

[root@lab ~]# /usr/libexec/qemu-kvm -version 
QEMU emulator version 2.9.0(qemu-kvm-ev-2.9.0-16.el7_4.13.1)
Copyright (c) 2003-2017 Fabrice Bellard and the QEMU Project developers
```

## 10.3 创建一个虚拟机

```Shell
[root@lab ~]# virt-install \
--name Win2k16 \
--ram 8192 \
--disk path=/var/kvm/images/Win2k16.img,size=80 \
--vcpus=4 \
--os-type windows \
--os-variant=win10 \
--network bridge=br0 \
--graphics spice,listen=0.0.0.0,password=password,keymap=ja \
--video qxl \
--cdrom /tmp/Win2016_JA-JP.ISO \
--boot uefi
```