***QEMU网桥模型部署实验***

[TOC]

# 1. 实验环境

- QEMU 2.5.0
- CentOS 7.4
- Interface
```
[root@centos7 ~]# ifconfig
lo: flags=73<UP,LOOPBACK,RUNNING>  mtu 65536
        inet 127.0.0.1  netmask 255.0.0.0
        inet6 ::1  prefixlen 128  scopeid 0x10<host>
        loop  txqueuelen 1  (Local Loopback)
        RX packets 44712  bytes 477510036 (455.3 MiB)
        RX errors 0  dropped 0  overruns 0  frame 0
        TX packets 44712  bytes 477510036 (455.3 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0

p4p1: flags=4163<UP,BROADCAST,RUNNING,MULTICAST>  mtu 1500
        inet 192.168.30.81  netmask 255.255.255.0  broadcast 192.168.30.255
        inet6 fe80::7779:d1a4:dfef:1550  prefixlen 64  scopeid 0x20<link>
        ether 64:00:6a:1a:cb:63  txqueuelen 1000  (Ethernet)
        RX packets 363302  bytes 53576355 (51.0 MiB)
        RX errors 0  dropped 2553  overruns 0  frame 0
        TX packets 65805  bytes 9658960 (9.2 MiB)
        TX errors 0  dropped 0 overruns 0  carrier 0  collisions 0
```


# 1. 开启报文转发

创建一个文件/etc/sysctl.d/ip_forward.conf并添加：
```
#> net.ipv4.ip_forward=1
```

上面设置重启后生效，现在设置当前运行环境：
```
#> sysctl -w net.ipv4.ip_forward=1
```
# 2. 设置防火墙规则

如果开启防火墙服务则需设置：
```
#> firewall-cmd --permanent --zone=public --add-masquerade
#> firewall-cmd --reload
```

# 4. 创建网桥bridge0
```
#> brctl addbr bridge0
#> brctl stp bridge0 on
#> brctl show
bridge name     bridge id               STP enabled     interfaces
bridge0         8000.000000000000       yes
```

# 3. 创建tap设备
```
#> ip tuntap add mode tap tap0
#> ip link set tap0 up 
```

# 5. 将网络接口介入网桥

```
#> brctl addif bridge0 tap0
#> brctl addif bridge0 p4p1
#> brctl show
bridge name     bridge id               STP enabled     interfaces
bridge0         8000.64006a1acb63       yes             p4p1
                                                        tap0
```

# 6. QEMU启动参数

> 加入参数：-net nic -net tap,ifname=tap0,script=no,downscript=no

例如:

```
/usr/local/bin/qemu-system-x86_64 \
-smp 4 -m 2048 \
-machine q35,accel=kvm \
-drive file=/root/centos.qcow2,if=virtio \
-drive file=/usr/share/ovmf/OVMF_CODE.fd,if=pflash,format=raw,unit=0,readonly=on \
-drive file=/usr/share/ovmf/OVMF_VARS.fd,if=pflash,format=raw,unit=1 \
-boot menu=on,splash-time=5000 \
-chardev socket,id=qmp,port=60009,host=localhost,server,nowait \
-monitor stdio \
-device ada_pci_minimal \
-cdrom /root/CentOS-7-x86_64-DVD-1708.iso \
-net nic -net tap,ifname=tap0,script=no,downscript=no
```
