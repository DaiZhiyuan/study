[TOC]


# 1. 热插拔

热插拔：hot plugging即带点插拔。在服务器硬件中，可实现热插拔的部件主要有SATA硬件、CPU、内存、风扇、SUB、网卡。

KVM主要支持PCI设备的热插拔和CPU热插拔，也可通过ballooning间接实现内存热插拔。

PCI设备的热插拔，主要需要如下几个方面支持：
1. BIOS：QEMU默认使用SeaBIOS作为客户机的BIOS，目前默认的BIOS可以支持PCI设备的热插拔。
2. PCI总线：物理硬件中必须有VT-d的支持，且现在的PCI总线都支持设备的热插拔。
3. 客户机操作系统：内核CONFIG_HOTPLUG=y
4. 客户机中的驱动程序：驱动必须支持热插拔。

CPU和内存的热插拔：CPU和内存的热插拔是RAS（Reliability, Availability, Serviceability）的一个重要特性。
```
CONFIG_HOTPLUG_CPU=y
CONFIG_MEMORY_HOTPLUG=y
```

# 2. 迁移

迁移类型：
- 静态迁移static migration | 动态迁移 live migration
- 冷迁移 cold migration | 热迁移 hot migration 
- 离线迁移 offline migration | 在线迁移 online migration 

迁移效率：
- 整体迁移时间
- 服务器停机时间
- 对服务的性能影响

应用场景：
- 负载均衡
- 解决硬件依赖
- 节约能源
- 实现客户机地理位置上的远程迁移


## 2.1 KVM实现静态迁移

磁盘映像文件为qcow2，启动参数保持一致。QEMU monitor使用 "savevm"与"loadvm"保存快照/恢复快照。


## 2.2 KVM动态迁移过程（基于共享存储系统）

- 在客户机动态迁移开始后，客户机依然在源宿主机上运行，与此同时，客户机的内存也被传输到目的主机之上。
- QEMU会监控并记录下迁移过程中所有已被传输的内存也的任何修改，并在所有内存也都被传输完成后即开始传输前面过程中内存也的更改内容。

1. NFS的挂在目录必须与源主机保持完全一致。
2. 启动客户机命令与源主机保持一致。
3. 增加 -incoming：qemu-kvm进入迁移监听模式.(migration-listen) -incoming tcp:0.0.0.0:6666
4. 在源主机的客户机QEMU monitor migrate tcp:192.168.122.2:6666

# 3. KVM嵌套KVM应用场景
- Iaas
- 测试和调试Hypervisor
- 虚拟机系统动态迁移

L0：硬件层之上就是最底层的宿主机系统。 

L1：L0宿主机中可以运行加载有KVM模块的客户机。

L2：L1客户机通过qemu-kvm启动一个普通客户机我们称L2。

L0加载模块时候：
```
$> modprobe kvm
$> modprobe kvm_intel nested=1
cat /sys/module/kvm_intel/parmeters/nested 
Y
```	

L0启动客户机时：
```
增加-cpu host参数。
```