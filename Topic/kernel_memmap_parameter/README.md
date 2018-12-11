[TOC]

# 1. memmap参数介绍

```
    memmap=nn[KMG]$ss[KMG]
            [KNL,ACPI] Mark specific memory as reserved.
            Region of memory to be used, from ss to ss+nn.
```

> ACPI ACPI support is enabled.
> KNL Is a kernel start-up parameter


例如：保留64MB连续物理内存，范围是[1c0000000 - 1c3ffffff]

可以写成：
```
memmap=64M$0x1c0000000 or memmap=0x4000000$0x1c0000000
```


# 2. Grub2菜单修改

> 注意：grub2参数中加入特殊字符必须使用'$'转移字符。

```
grub2：
linuxefi /vmlinuz-3.10.0-693.el7.x86_64 root=/dev/mapper/centos-root ro crashkernel=auto rd.lvm.lv=centos/root rd.lvm.lv=centos/swap rhgb  memmap=0x4000000\$0x1c0000000 console=tty0 console=ttyS0,115200n8 modprobe.blacklist=kvm,kvm_intel
```

# 3. 查看iomem

```
[root@lab ~]# cat /proc/iomem  | grep reserved
00000000-00000fff : reserved
00058000-00058fff : reserved
0009d000-0009efff : reserved
d1a3c000-d1e61fff : reserved
d7ee4000-d7ffffff : reserved
d875e000-d87fffff : reserved
db81e000-dbffffff : reserved
dd000000-df1fffff : reserved
    f8000000-fbffffff : reserved
fec00000-fec00fff : reserved
fed00000-fed03fff : reserved
fed1c000-fed1ffff : reserved
  fee00000-fee00fff : reserved
ff000000-ffffffff : reserved
1c0000000-1c3ffffff : reserved
```

# 4. 16进制速算

## 4.1 公式
- 8bit = 1byte
- 1KB = 2的10次方byte, 0x400，表示地址的范围是0x00~0x3FF。
- 2KB = 2的11次方byte, 0x800，表示的地址范围为0x00~7FF。
- 4KB = 2的12次方byte，0x1000，表示的地址范围0x000~0xFFF。
- 1MB = 2的20次方byte，0x100000，表示的地址范围为0x00000~0xFFFFF。
- 1GB = 2的30次方byte，0x40000000，表示的地址范围为0x00000~0x3FFFFFFF。


## 4.2 例子

### 4.2.1 那么对于一个任意给定的地址范围（必然是1KB的整数倍）怎么反推字节数呢？

例如：0x000000~0x1FFFFF
```
第一种解法，记住1M是5个F，多一位是2倍，所以是2M。
第二种解法，记住FFF是4KB，1FF是512, 512 X 4KB = 2048KB也就是2M。
```

例如：0x1c0000000~1c3ffffff
```
   1c3ffffff
-  1c0000000
---------------------
     3ffffff
     
解：因为5个F是1M，所以3f个1M。3f是64，所以答案是64M。
```

### 4.2.2 给你一个字节数让你计算地址范围？

例如：0x3000表示的地址范围
```
解：4K是0x1000，3个4K就是12K。4K的地址范围是0x000 ~ 0xFFF，12K的地址范围是 0x0000 ~ 0x2FFF。
```


