***OVMF定制Logo图片限制以及编译方法***

[TOC]

# 1 OVMF LOGO格式

## 1.1 图片格式限制

- 8bit位图
- 256色
- Windows 3.x format

验证方式：
> [root@centos7 ~]# file tsinghua.bmp
tsinghua.bmp: PC bitmap, Windows 3.x format, 500 x 204 x 8

## 1.2 图片大小限制

- 图片大小应该小于1024KB。

验证方式：
> [root@centos7 ~]# du -sh tsinghua.bmp
104K    tsinghua.bmp

## 1.3 图片分辨率限制
- 分辨率并无特殊需求，OVMF会居中显示。建议使用特定比例如：4：3、19:6等。

## 1.3 JPG转换格式

1. 安装
> $> yum install ImageMagick

2. 转换JPG到BMP
> $> convert /root/tsinghua.jpg -alpha off -type palette -compress none  BMP3:tsinghua.bmp

# 2 OVMF编译方法

## 2.1 建立EDKII编译环境

下载源代码：
> #>  git clone https://github.com/tianocore/edk2.git

## 2.2 安装编译依赖

编译依赖列表：
- ACPI Source Language (ASL) Compiler Setup
- The NASM assembler
- Python2.7

> #>  yum -y install nasm acpica-tools python-devel libuuid-devel

## 2.3 初始化编译环境和基本组件

> #> source edksetup.sh BaseTools

```bash
WORKSPACE: /root/edk2
EDK_TOOLS_PATH: /root/edk2/BaseTools
CONF_PATH: /root/edk2/Conf/
build_rule.txt
tools_def.txt       -->  Information about 3rd party tools 
target.txt            -->  Restricts a build to defined values

编辑target.txt文件
TARGET = RELEASE
ACTIVE_PLATFORM  = OvmfPkg/OvmfPkgX64.dsc
TOOL_CHAIN_TAG = GCC48
TARGET_ARCH  = X64

编译基本组件：
make -C edk2/BaseTools
配置Conf/target.txt 可直接使用build

```

## 2.4 直接编译

```bash
#> git submodule update --force --init --recursive
#> build -a X64 -p MdeModulePkg/MdeModulePkg.dsc -t GCC48 -b RELEASE
#> build -a X64 -p OvmfPkg/OvmfPkgX64.dsc -DSECURE_BOOT_ENABLE -t GCC48 -b RELEASE --cmd-len=65536 --hash
```

## 2.5 重新配置

> #> . edksetup.sh --reconfig

## 2.6 运行QEMU测试OVMF

QEMU加入下列参数：
```bash
-drive if=pflash,format=raw,readonly,file=/usr/share/ovmf/OVMF_CODE.fd 
-drive if=pflash,format=raw,file=/usr/share/ovmf/OVMF_VARS.fd
```

# 3. OVMF增加debug调试信息

## 3.1 开启debug I/O port

编译时指定target为DEBUG：
```bash
$> build -a X64 -p OvmfPkg/OvmfPkgX64.dsc -t GCC48 -b DEBUG --cmd-len=65536 --hash
```

## 3.2 增加QEMU参数

参数：-debugcon file:guestbios.log --global isa-debugcon.iobase=0x402

指定日志文件为：guestbios.log

例如：
```bash
qemu-system-x86_64 \
-smp 4 -m 2048 \
-machine q35,accel=kvm \
-drive file=/root/centos.qcow2,if=virtio \
-drive file=/usr/share/ovmf/OVMF_CODE.fd,if=pflash,format=raw,unit=0,readonly=on \
-drive file=/usr/share/ovmf/OVMF_VARS.fd,if=pflash,format=raw,unit=1 \
-boot menu=on,splash-time=5000 \
-chardev socket,id=qmp,port=60009,host=localhost,server,nowait \
-net nic -net tap,ifname=tap0,script=no,downscript=no \
-debugcon file:guestbios.log --global isa-debugcon.iobase=0x402
```

guestbios.log
```
SecCoreStartupWithStack(0xFFFCC000, 0x820000)
Register PPI Notify: DCD0BE23-9586-40F4-B643-06522CED4EDE
Install PPI: 8C8CE578-8A3D-4F1C-9935-896185C32DD3
Install PPI: 5473C07A-3DCB-4DCA-BD6F-1E9689E7349A
The 0th FV start address is 0x00000820000, size is 0x000E0000, handle is 0x820000
Register PPI Notify: 49EDB1C1-BF21-4761-BB12-EB0031AABB39
Register PPI Notify: EA7CA24B-DED5-4DAD-A389-BF827E8F9B38
Install PPI: B9E0ABFE-5979-4914-977F-6DEE78C278A6
Install PPI: DBE23AA9-A345-4B97-85B6-B226F1617389
Loading PEIM 9B3ADA4F-AE56-4C24-8DEA-F03B7558AE50
Loading PEIM at 0x0000082F400 EntryPoint=0x0000083189A PcdPeim.efi
Install PPI: 06E81C58-4AD7-44BC-8390-F10265F72480
Install PPI: 01F34D25-4DE2-23AD-3FF3-36353FF323F1
Install PPI: 4D8B155B-C059-4C8F-8926-06FD4331DB8A
Install PPI: A60C6B59-E459-425D-9C69-0BCC9CB27D81
Register PPI Notify: 605EA650-C65C-42E1-BA80-91A52AB618C6
Loading PEIM A3610442-E69F-4DF3-82CA-2360C4031A23
Loading PEIM at 0x000008357A0 EntryPoint=0x000008375AA ReportStatusCodeRouterPei.efi
Install PPI: 0065D394-9951-4144-82A3-0AFC8579C251
Install PPI: 229832D3-7A30-4B36-B827-F40CB7D45436
Loading PEIM 9D225237-FA01-464C-A949-BAABC02D31D0
Loading PEIM at 0x00000838EA0 EntryPoint=0x0000083ACF5 StatusCodeHandlerPei.efi
Loading PEIM 222C386D-5ABC-4FB4-B124-FBB82488ACF4
Loading PEIM at 0x0000083CFA0 EntryPoint=0x000008406B0 PlatformPei.efi
Select Item: 0x0
FW CFG Signature: 0x554D4551
Select Item: 0x1
FW CFG Revision: 0x3
QemuFwCfg interface (DMA) is supported.
Platform PEIM Loaded
CMOS:
00: 39 00 05 00 03 00 04 05 12 18 26 02 00 80 00 00
10: 00 00 00 00 06 80 02 FF FF 00 00 00 00 00 00 00
20: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
30: FF FF 20 00 00 7F 00 20 30 00 00 00 00 12 00 00
40: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
50: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 03
60: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
70: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
    ......
```