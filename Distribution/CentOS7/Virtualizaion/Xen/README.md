- 1 Xen
    - 1.1 构建Xen虚拟化环境
    - 1.2 配置桥接网络
- 2 创建虚拟机
    - 2.1 安装libvirt
    - 2.2 创建一个虚拟机
    - 2.3 测试验证

# 1. Xen

Xen 是一个开放源代码虚拟机监视器，由剑桥大学开发。它打算在单个计算机上运行多达100个满特征的操作系统。操作系统必须进行显式地修改（“移植”）以在Xen上运行（但是提供对用户应用的兼容性）。这使得Xen无需特殊硬件支持，就能达到高性能的虚拟化。

## 1.1 构建Xen虚拟化环境

```Shell
[root@lab ~]# yum -y install centos-release-xen
[root@lab ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-Xen.repo 

[root@lab ~]# yum --enablerepo=centos-virt-xen -y update kernel
[root@lab ~]# yum --enablerepo=centos-virt-xen -y install xen

[root@lab ~]# vi /etc/default/grub
# change memory amount for Domain0 (specify proper value on your system)
GRUB_CMDLINE_XEN_DEFAULT="dom0_mem=4096M,max:4096M cpuinfo com1=115200,8n1 .....

[root@lab ~]# /bin/grub-bootxen.sh 
[root@lab ~]# reboot

# display info
[root@lab ~]# xl info 
host                   : lab.centos.org
release                : 3.18.21-17.el7.x86_64
version                : #1 SMP Fri Dec 18 18:04:14 UTC 2015
machine                : x86_64
nr_cpus                : 6
max_cpu_id             : 5
nr_nodes               : 1
cores_per_socket       : 1
threads_per_core       : 1
cpu_mhz                : 2594
hw_caps                : xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx
virt_caps              : hvm
total_memory           : 12287
free_memory            : 8054
sharing_freed_memory   : 0
sharing_used_memory    : 0
outstanding_claims     : 0
free_cpus              : 0
xen_major              : 4
xen_minor              : 6
xen_extra              : .0-9.el7
xen_version            : 4.6.0-9.el7
xen_caps               : xen-3.0-x86_64 xen-3.0-x86_32p hvm-3.0-x86_32 hvm-3.0-x86_32p hvm-3.0-x86_64
xen_scheduler          : credit
xen_pagesize           : 4096
platform_params        : virt_start=0xffff800000000000
xen_changeset          : Thu Jan 14 15:35:35 2016 +0000 git:6e8597a-dirty
xen_commandline        : placeholder dom0_mem=4096M,max:4096M cpuinfo com1=115200,8n1 
                         console=com1,tty loglvl=all guest_loglvl=all
cc_compiler            : gcc (GCC) 4.8.5 20150623 (Red Hat 4.8.5-4)
cc_compile_by          : mockbuild
cc_compile_domain      : centos.org
cc_compile_date        : Wed Jan 20 12:25:53 UTC 2016
xend_config_format     : 4
```

## 1.2 配置桥接网络

```Shell
# add bridge "br0"
[root@lab ~]# nmcli c add type bridge autoconnect yes con-name br0 ifname br0 
Connection 'br0' (0f4b7bc8-8c7a-461a-bff1-d516b941a6ec) successfully added.

# set IP for br0
[root@lab ~]# nmcli c modify br0 ipv4.addresses 10.0.0.30/24 ipv4.method manual 

# set Gateway for br0
[root@lab ~]# nmcli c modify br0 ipv4.gateway 10.0.0.1 

# set DNS for "br0"
[root@lab ~]# nmcli c modify br0 ipv4.dns 10.0.0.1 

# remove the current setting
[root@lab ~]# nmcli c delete eno16777736 

# add an interface again as a member of br0
[root@lab ~]# nmcli c add type bridge-slave autoconnect yes con-name eno16777736 ifname eno16777736 master br0 

# stop and start NetworkManager
[root@lab ~]# systemctl stop NetworkManager; systemctl start NetworkManager 

[root@lab ~]# ip addr 
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN group default
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eno16777736: <BROADCAST,MULTICAST,UP,LOWER_UP> 
    mtu 1500 qdisc pfifo_fast master br0 state UP group default qlen 1000
    link/ether 00:0c:29:9f:9b:d3 brd ff:ff:ff:ff:ff:ff
3: br0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc noqueue state UP group default
    link/ether 00:0c:29:9f:9b:d3 brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.30/24 brd 10.0.0.255 scope global br0
       valid_lft forever preferred_lft forever
    inet6 fe80::20c:29ff:fe9f:9bd3/64 scope link
       valid_lft forever preferred_lft forever
```

# 2. 创建虚拟机

## 2.1 安装libvirt

```Shell
# enable CentOS Xen
[root@lab ~]# yum --enablerepo=centos-virt-xen -y install libvirt libvirt-daemon-xen virt-install

[root@lab ~]# systemctl start libvirtd 
[root@lab ~]# systemctl enable libvirtd 
```

## 2.2 创建一个虚拟机

```Shell
# create a directory for images
[root@lab ~]# mkdir -p /var/xen/images

[root@lab ~]# virt-install \
--connect xen:/// \
--paravirt \
--name centos7 \
--ram 4096 \
--disk path=/var/xen/images/centos7.img,size=20 \
--vcpus 2 \
--os-type linux \
--os-variant rhel7 \
--network bridge=br0 \
--graphics none \
--location 'https://mirrors.tuna.tsinghua.edu.cn/centos/7/os/x86_64/' \
--extra-args 'text console=com1 utf8 console=hvc0'
Starting install...     # installation starts
```

## 2.3 测试验证

```Shell
[root@localhost ~]#     # Ctrl + ] key
[root@lab ~]#     # Host's console

# display active domains
[root@lab ~]# xl list 
Name            ID   Mem VCPUs      State   Time(s)
Domain-0         0  4090     6     r-----     226.3
centos7          2  4096     2     -b----      17.0

[root@lab ~]# xl console centos7     # move to Guest
[root@localhost ~]#     # Guest's console
```
