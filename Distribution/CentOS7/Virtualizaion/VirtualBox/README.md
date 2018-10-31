- 1 VirtualBox
    - 1.1 安装依赖软件包并重启
    - 1.2 安装VirtualBox
    - 1.3 安装扩展包
- 2 安装虚拟机
    - 2.1 创建虚拟机
    - 2.2 启动虚拟机
- 3 安装虚拟机附加组件
    - 3.1 挂载VBoxGuestAdditions.iso
    - 3.2 安装GuestAdditions

# 1. VirtualBox

## 1.1 安装依赖软件包并重启

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install gcc make glibc kernel-headers kernel-devel dkms 
[root@lab ~]# yum -y update kernel 
[root@lab ~]# reboot 
```

## 1.2 安装VirtualBox

```Shell
[root@lab ~]# curl http://download.virtualbox.org/virtualbox/rpm/rhel/virtualbox.repo -o /etc/yum.repos.d/virtualbox.repo 
[root@lab ~]# yum search virtualbox 
Loaded plugins: fastestmirror
Loading mirror speeds from cached hostfile
 * base: www.ftp.ne.jp
 * extras: www.ftp.ne.jp
 * updates: www.ftp.ne.jp
=========================== N/S matched: virtualbox ============================
VirtualBox-4.3.x86_64 : Oracle VM VirtualBox
VirtualBox-5.0.x86_64 : Oracle VM VirtualBox

  Name and summary matches only, use "search all" for everything.
  
[root@lab ~]# yum -y install VirtualBox-5.0
[root@lab ~]# VBoxManage -v 
5.0.14r105127
```

## 1.3 安装扩展包

```Shell
[root@lab ~]# curl -O http://download.virtualbox.org/virtualbox/5.0.14/Oracle_VM_VirtualBox_Extension_Pack-5.0.14.vbox-extpack 
[root@lab ~]# VBoxManage extpack install Oracle_VM_VirtualBox_Extension_Pack-5.0.14.vbox-extpack 
0%...10%...20%...30%...40%...50%...60%...70%...80%...90%...100%
Successfully installed "Oracle VM VirtualBox Extension Pack".

[root@lab ~]# VBoxManage list extpacks 
Extension Packs: 1
Pack no. 0:   Oracle VM VirtualBox Extension Pack
Version:      5.0.14
Revision:     104061
Edition:
Description:  USB 2.0 and USB 3.0 Host Controller, Host Webcam, VirtualBox RDP, PXE ROM, Disk Encryption.
VRDE Module:  VBoxVRDP
Usable:       true
Why unusable:
```

# 2. 安装虚拟机

## 2.1 创建虚拟机

```Shell
# create a directory for VM
[root@lab ~]# mkdir /var/vbox

# create a VM
[root@lab ~]# VBoxManage createvm \
--name CentOS_7 \
--ostype RedHat_64 \
--register \
--basefolder /var/vbox 
Virtual machine 'CentOS_7' is created and registered.
UUID: bdceab21-504f-47c2-9107-17ec778dbcf3
Settings file: '/var/vbox/CentOS_7/CentOS_7.vbox'

# modify VM
[root@lab ~]# VBoxManage modifyvm CentOS_7 \
--cpus 4 \
--memory 4096 \
--nic1 bridged \
--bridgeadapter1 eth0 \
--boot1 dvd \
--vrde on \
--vrdeport 5001

# configure storage for VM
[root@lab ~]# VBoxManage storagectl CentOS_7 --name "CentOS_7_SATA" --add sata 
[root@lab ~]# VBoxManage createhd \
--filename /var/vbox/CentOS_7/CentOS_7.vdi \
--size 20480 \
--format VDI \
--variant Standard 
0%...10%...20%...30%...40%...50%...60%...70%...80%...90%...100%
Medium created. UUID: dc515d33-fa38-4c70-92c7-6f2f17a39487
[root@lab ~]# VBoxManage storageattach CentOS_7 \
--storagectl CentOS_7_SATA \
--port 1 \
--type hdd \
--medium /var/vbox/CentOS_7/CentOS_7.vdi

# configure DVD drive for VM (the example specifies an ISO file)
[root@lab ~]# VBoxManage storageattach CentOS_7 \
--storagectl CentOS_7_SATA \
--port 0 \
--type dvddrive \
--medium /tmp/CentOS-7-x86_64-DVD-1511.iso

# confirm settings for VM
[root@lab ~]# VBoxManage showvminfo CentOS_7 
Name:            CentOS_7
Groups:          /
Guest OS:        Red Hat (64-bit)
UUID:            bdceab21-504f-47c2-9107-17ec778dbcf3
Config file:     /var/vbox/CentOS_7/CentOS_7.vbox
Snapshot folder: /var/vbox/CentOS_7/Snapshots
Log folder:      /var/vbox/CentOS_7/Logs
Hardware UUID:   bdceab21-504f-47c2-9107-17ec778dbcf3
Memory size:     4096MB
Page Fusion:     off
VRAM size:       8MB
CPU exec cap:    100%
HPET:            off
Chipset:         piix3
Firmware:        BIOS
Number of CPUs:  4
PAE:             on
Long Mode:       on
CPUID Portability Level: 0
.....
.....
```

## 2.2 启动虚拟机

```Shell
[root@lab ~]# VBoxManage startvm CentOS_7 --type headless 
Waiting for VM "CentOS_7" to power on...
VM "CentOS_7" has been successfully started.
```

# 3. 安装虚拟机附加组件

## 3.1 挂载VBoxGuestAdditions.iso

```Shell
[root@lab ~]# VBoxManage storageattach CentOS_7 \
--storagectl CentOS_7_SATA \
--port 0 \
--type dvddrive \
--medium /usr/share/virtualbox/VBoxGuestAdditions.iso
```

## 3.2 安装GuestAdditions

```Shell
# install some packgaes
[root@guest ~]# yum -y update kernel 
[root@guest ~]# reboot

[root@guest ~]# yum -y install gcc make bzip2 kernel-devel
[root@guest ~]# mount /dev/cdrom /mnt 
[root@guest ~]# cd /mnt 

[root@guest mnt]# ./VBoxLinuxAdditions.run 
[root@guest mnt]# reboot
```