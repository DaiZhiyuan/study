***SATA硬盘与SATA控制器***

[TOC]

# 1 概述

在使用QEMU/KVM时，有些场景需要透传PCI设备给GuestOS。

> 例如：将某块磁盘透传给GuestOS，主机上有多块SATA磁盘、多个SATA控制器，那么怎么找出你想要透传的磁盘隶属于哪个控制器并得到BDF，从而完成透传的目的？


## 1.1 获取SATA控制器信息
lspci - list all PCI devices
```
[root@centos7 ~]# lspci
00:00.0 Host bridge: Intel Corporation 4th Gen Core Processor DRAM Controller (rev 06)
00:02.0 VGA compatible controller: Intel Corporation 4th Generation Core Processor Family Integrated Graphics Controller (rev 06)
00:03.0 Audio device: Intel Corporation Xeon E3-1200 v3/4th Gen Core Processor HD Audio Controller (rev 06)
00:14.0 USB controller: Intel Corporation 8 Series/C220 Series Chipset Family USB xHCI (rev 04)
00:16.0 Communication controller: Intel Corporation 8 Series/C220 Series Chipset Family MEI Controller #1 (rev 04)
00:1a.0 USB controller: Intel Corporation 8 Series/C220 Series Chipset Family USB EHCI #2 (rev 04)
00:1b.0 Audio device: Intel Corporation 8 Series/C220 Series Chipset High Definition Audio Controller (rev 04)
00:1c.0 PCI bridge: Intel Corporation 8 Series/C220 Series Chipset Family PCI Express Root Port #1 (rev d4)
00:1c.3 PCI bridge: Intel Corporation 8 Series/C220 Series Chipset Family PCI Express Root Port #4 (rev d4)
00:1d.0 USB controller: Intel Corporation 8 Series/C220 Series Chipset Family USB EHCI #1 (rev 04)
00:1f.0 ISA bridge: Intel Corporation C220 Series Chipset Family H81 Express LPC Controller (rev 04)
00:1f.2 SATA controller: Intel Corporation 8 Series/C220 Series Chipset Family 6-port SATA Controller 1 [AHCI mode] (rev 04)
00:1f.3 SMBus: Intel Corporation 8 Series/C220 Series Chipset Family SMBus Controller (rev 04)
02:00.0 Ethernet controller: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller (rev 0c)

```
获取信息如下：
00:1f.2 SATA controller: Intel Corporation 8 Series/C220 Series Chipset Family 6-port SATA Controller 1 [AHCI mode] (rev 04)

- 00:1f.2：PCI设备的BDF
- SATA controller：设备名称
- Intel Corporation 8 Series/C220 Series Chipset Family 6-port SATA Controller 1：厂商信息
- [AHCI mode]：工作模式
- (rev 04)：版本信息

> What does "rev" in lspci output mean?

> Revision.

> Different versions of the same model. 
> Could mean just a different firmware version, but could mean a completely different chipset.

## 1.2 获取SCSI设备信息
lsscsi - list SCSI devices (or hosts) and their attributes
```
[root@centos7 ~]# lsscsi -dv
[0:0:0:0]    disk    ATA      ST1000DM010-2EP1 CC43  /dev/sda [8:0]
  dir: /sys/bus/scsi/devices/0:0:0:0  [/sys/devices/pci0000:00/0000:00:1f.2/ata1/host0/target0:0:0/0:0:0:0]
[5:0:0:0]    cd/dvd  PLDS     DVD+-RW DH-16AES DD12  /dev/sr0 [11:0]
  dir: /sys/bus/scsi/devices/5:0:0:0  [/sys/devices/pci0000:00/0000:00:1f.2/ata6/host5/target5:0:0/5:0:0:0]
```

根据上述信息得知：
- 设备ST1000DM010-2EP1与设备DH-16AES都隶属于0000:00:1f.2的SATA控制器，BDF为00:1f.2。

## 1.3 获取SATA磁盘速率信息
smartctl - Control and Monitor Utility for SMART Disks
```
[root@centos7 ~]# smartctl -x /dev/sda
smartctl 6.2 2017-02-27 r4394 [x86_64-linux-3.10.0-693.el7.centos.x86_64.debug] (local build)
Copyright (C) 2002-13, Bruce Allen, Christian Franke, www.smartmontools.org

=== START OF INFORMATION SECTION ===
Device Model:     ST1000DM010-2EP102
Serial Number:    W9AAEBF3
LU WWN Device Id: 5 000c50 0b938e240
Firmware Version: CC43
User Capacity:    1,000,204,886,016 bytes [1.00 TB]
Sector Sizes:     512 bytes logical, 4096 bytes physical
Rotation Rate:    7200 rpm
Device is:        Not in smartctl database [for details use: -P showall]
ATA Version is:   ATA8-ACS T13/1699-D revision 4
SATA Version is:  SATA 3.0, 6.0 Gb/s (current: 6.0 Gb/s)
Local Time is:    Thu Sep 20 17:04:01 2018 CST
SMART support is: Available - device has SMART capability.
SMART support is: Enabled
AAM feature is:   Unavailable
APM level is:     128 (minimum power consumption without standby)
Rd look-ahead is: Enabled
Write cache is:   Enabled
ATA Security is:  Disabled, frozen [SEC2]
Write SCT (Get) XXX Error Recovery Control Command failed: scsi error badly formed scsi parameters
Wt Cache Reorder: N/A
=== START OF READ SMART DATA SECTION ===
```
获取信息如下：
- 磁盘容量：1TB
- 磁盘转速：7200rpm
- 传输速率：6.0 Gb/s




