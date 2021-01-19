# 1. ISO format and nameing rule

download: `https://vault.centos.org/7.2.1512/isos/CentOS-7-x86_64-DVD-1512.iso`

- filename: CentOS-7-x86_64-DVD-1512.iso
	- product: `CentOS`
	- version: `7`
	- arch: `x86_64`
	- laser-disc-type: `DVD`
	- release date: `1512`
	- expanded-name: `iso`

# 2. create yum repository

```shell
#> createrepo -g ./c7-x86_64-comps.xml ./
```
- yum repository meta data format:`3eda3fefdbaf4777fcab430c80bc438293c512f22fd706b12c6567d535b2142a-c7-x86_64-comps.xml`
	- sha256sum: `3eda3fefdbaf4777fcab430c80bc438293c512f22fd706b12c6567d535b2142a`
	- product-version: `c7`
	- arch: `x86_64`
	- name: `comps`
	- expanded-name: `xml`

# 3. lorax tools

## 3.1 lorax argments 

example:
```
 --product="CentOS" 
 --version="7" 
 --release="151209"  #15年12月9日
 --source="/root/centos_repo" 
 --bugurl="http://bugs.centos.org" 
 --isfinal 
 /root/centos_tree
```

> centos8: lorax --product="CentOS" --version="8" --release="2002" --bugurl="http://bugs.centos.org" --source="/root/pub/AppStream/" --source="/root/pub/BaseOS/" --isfinal /root/centos_install_images --nomacboot
## 3.2 add some file

```
EULA
GPL
RPM-GPG-KEY-CentOS-7
RPM-GPG-KEY-CentOS-Testing-7
Packages/
repodata/
CentOS_BuildTag 
```

```shell
#> cp /root/centos_repo/{Packages,repodate} -a ./
# cp /root/centos_orig/{GPL,EULA,RPM-GPG-KEY-CentOS-7,RPM-GPG-KEY-CentOS-Testing-7} -a ./
# cat /srv/.buildstamp | grep UUID | awk -F '=' '{print $2}' |  awk -F '.' '{print $1}'
201607220419
#  echo 20160722-0419 > CentOS_BuildTag
```

# 4. make iso images

mkiso:
```shell
mkisofs
-b isolinux/isolinux.bin 
-c isolinux/boot.cat 
-no-emul-boot
-boot-load-size 4 
-boot-info-table 
-eltorito-alt-boot
-e images/efiboot.img 
-R -V -T -graft-points
-V "CentOS-7-x86_64" 
-verbose
-o "/root/centos_isos/CentOS-7-x86_64-DVD-1512.iso"
/root/centos_tree
```

> centos7_aarch64: genisoimage -V 'CentOS 7 aarch64' -R -J -T -graft-points -verbose -e images/efiboot.img -no-emul-boot -c boot.catalog -hide boot.catalog -o /root/centos_isos/CentOS-7-aarch64-Everything-1810-variant.iso /root/centos_tree

> centos8_aarch64: genisoimage -V 'CentOS-8-aarch64' -R -J -T -graft-points -verbose -e images/efiboot.img -no-emul-boot -c boot.catalog -hide boot.catalog -o /root/centos_isos/CentOS-8.3.2011-aarch64-variant.iso /root/centos_install_images

genisoimage -V 'CentOS-8-aarch64' -R -J -T -graft-points -verbose -e images/efiboot.img -no-emul-boot -c boot.catalog -hide boot.catalog -o /root/centos_isos/CentOS-7-aarch64-Everything-1810-variant.iso /root/centos_install_images/

post:
```
#> isohybrid --uefi "/root/centos_isos/CentOS-7-x86_64-DVD-1512.iso"
#> implantisomd5 --force "/root/centos_isos/CentOS-7-x86_64-DVD-1512.iso"
```

# 5. other

- `/root/centos_repo`: yum repository
- `/root/centos_tree`: iso tree
- `/root/centos_isos`: iso images 
- `/root/centos_orig`: upstream mount point


<CentOS 8.3>
anaconda-gui-33.16.3.26-2.el8.centos.aarch64.rpm --> subscription(subscription.glade)

<CentOS 8.2>
https://mirrors.tuna.tsinghua.edu.cn/centos-vault//centos/8.2.2004/isos/aarch64/CentOS-8.2.2004-aarch64-dvd1.iso


```
[Main]
Product=CentOS Linux
Version=8
BugURL=your distribution provided bug reporting tool
IsFinal=True
UUID=202006082128.aarch64
Variant=BaseOS
[Compose]
Lorax=28.14.30-1
```




upstream:
```
media.repo:

[InstallMedia]
name=CentOS Linux 8
mediaid=None
metadata_expire=-1
gpgcheck=0
cost=500

.discinfo
1591654049.628473
CentOS Linux 8
aarch64
ALL

.treeinfo
[checksums]
images/efiboot.img = sha256:557221c243d2878d0d69180f696bdd2337635c93e86323b8f633114647bf6348
images/install.img = sha256:ac39d770822b7d87e66c424c707c56cf477788532d7c24a30540b56dcd3536ce
images/pxeboot/initrd.img = sha256:a0caf1ee0644880de543db2fbec8058c81edf2ccaa738fed58b23fbb7f3fcd3f
images/pxeboot/vmlinuz = sha256:f3c14ca492154f132287f14644bd1ab9405bdf984c51283e31f27bd3fffa2ffb

[general]
; WARNING.0 = This section provides compatibility with pre-productmd treeinfos.
; WARNING.1 = Read productmd documentation for details about new format.
arch = aarch64
family = CentOS Linux
name = CentOS Linux 8
packagedir = AppStream/Packages
platforms = aarch64
repository = AppStream
timestamp = 1591653985
variant = AppStream
variants = AppStream,BaseOS
version = 8

[header]
type = productmd.treeinfo
version = 1.2

[images-aarch64]
efiboot.img = images/efiboot.img
initrd = images/pxeboot/initrd.img
kernel = images/pxeboot/vmlinuz

[media]
discnum = 1
totaldiscs = 1

[release]
name = CentOS Linux
short = CentOS
version = 8

[stage2]
mainimage = images/install.img

[tree]
arch = aarch64
build_timestamp = 1591653985
platforms = aarch64
variants = AppStream,BaseOS

[variant-AppStream]
id = AppStream
name = AppStream
packages = AppStream/Packages
repository = AppStream
type = variant
uid = AppStream

[variant-BaseOS]
id = BaseOS
name = BaseOS
packages = BaseOS/Packages
repository = BaseOS
type = variant
uid = BaseOS
```

costom:
```
[general]
timestamp = 1610610230.454694
family = CentOS
version = 8
name = CentOS-8
variant =
arch = aarch64
packagedir =

[stage2]
mainimage = images/install.img

[images-aarch64]
kernel = images/pxeboot/vmlinuz
initrd = images/pxeboot/initrd.img
efiboot.img = images/efiboot.img
boot.iso = images/boot.iso

[tree]
arch = aarch64
build_timestamp = 1591653985
platforms = aarch64
variants = AppStream,BaseOS

[variant-AppStream]
id = AppStream
name = AppStream
packages = AppStream/Packages
repository = AppStream
type = variant
uid = AppStream

[variant-BaseOS]
id = BaseOS
name = BaseOS
packages = BaseOS/Packages
repository = BaseOS
type = variant
uid = BaseOS
```

./configure  --enable-debug  --target-list=aarch64-softmmu --enable-sdl --disable-gtk

## todo:

lorax build images: add `variants = AppStream,BaseOS` args.

./MOK.der `find -iname "*.ko" |xargs`

62657.881599] Plx8000_NT: Supports Linux kernel v4.19.166-80.el8.aarch64
[62657.888173] Plx8000_NT: ERROR - No supported devices found
[62657.893632] Plx8000_NT: Unload: PLX 8000_NT driver v8.00
[62657.898949] Plx8000_NT: ...driver unloaded
[62661.561748] PKCS#7 signature not signed with a trusted key


CONFIG_MODULE_SIG=n



> qemu-system-aarch64 -machine virt,gic_version=3 -cpu cortex-a57 -smp 4 -m 4096 -enable-kvm -drive file=/disk/centos7.qcow2,if=virtio -cdrom /opt/CentOS-8.2.2004-aarch64-dvd1.iso -boot menu=on,splash-time=5000


qemu-img create -f qcow2 centos7.qcow2 500G

yum install AAVMF.noarch
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

/usr/local/share/qemu
	edk2-arm-code.fd

	http://llxwj.top/post/linux/qemu-install-ceontos-7.4-aarch64-system/


# 1. Create Disk

```
wget https://releases.linaro.org/components/kernel/uefi-linaro/latest/release/qemu64/QEMU_EFI.img.gz
gunzip QEMU_EFI.img.gz

qemu-img create -f qcow2 rootfs.img 256G
qemu-img create -f qcow2 varstore.img 64M
```

# 2. install centos8

```
qemu-system-aarch64 \
    -cpu host -M virt,accel=kvm -m 4096 -nographic \
    -drive if=pflash,format=raw,file=QEMU_EFI.img \
    -drive if=pflash,file=varstore.img \
    -drive if=virtio,file=rootfs.img \
    -drive if=virtio,format=raw,file=CentOS-8.2.2004-aarch64-dvd1.iso
```

# 3. startup VM

```
qemu-system-aarch64 \
    -cpu host -M virt,accel=kvm -m 64G -smp 8 -nographic \
    -drive if=pflash,format=raw,file=QEMU_EFI.img \
    -drive if=pflash,file=varstore.img \
    -drive if=virtio,file=rootfs.img \
    -drive if=virtio,format=raw,file=CentOS-8.2.2004-aarch64-dvd1.iso
```
### ATF
> qemu-system-aarch64 -nographic -smp 2 -machine virt,secure=on -cpu cortex-a57 -d unimp -semihosting-config enable,target=native -m 512 -bios ./bl1.bin

https://blog.csdn.net/weixin_44124323/article/details/110758865
dd



BUild:  make PLAT=qemu all fip BL33=/disk/QEMU_EFI.img DEBUG=1




Trusted Boot Firmware BL2: offset=0xB0, size=0x9201, cmdline="--tb-fw"
EL3 Runtime Firmware BL31: offset=0x92B1, size=0xA089, cmdline="--soc-fw"
Non-Trusted Firmware BL33: offset=0x1333A, size=0x4000000, cmdline="--nt-fw"




```
[root@centos7 fiptool]# ./fiptool create
fiptool create [opts] FIP_FILENAME

Options:
  --align <value>               Each image is aligned to <value> (default: 1).
  --blob uuid=...,file=...      Add an image with the given UUID pointed to by file.
  --plat-toc-flags <value>      16-bit platform specific flag field occupying bits 32-47 in 64-bit ToC header.

Specific images are packed with the following options:
  --scp-fwu-cfg      FILENAME   SCP Firmware Updater Configuration FWU SCP_BL2U
  --ap-fwu-cfg       FILENAME   AP Firmware Updater Configuration BL2U
  --fwu              FILENAME   Firmware Updater NS_BL2U
  --fwu-cert         FILENAME   Non-Trusted Firmware Updater certificate
  --tb-fw            FILENAME   Trusted Boot Firmware BL2
  --scp-fw           FILENAME   SCP Firmware SCP_BL2
  --soc-fw           FILENAME   EL3 Runtime Firmware BL31
  --tos-fw           FILENAME   Secure Payload BL32 (Trusted OS)
  --tos-fw-extra1    FILENAME   Secure Payload BL32 Extra1 (Trusted OS Extra1)
  --tos-fw-extra2    FILENAME   Secure Payload BL32 Extra2 (Trusted OS Extra2)
  --nt-fw            FILENAME   Non-Trusted Firmware BL33
  --fw-config        FILENAME   FW_CONFIG
  --hw-config        FILENAME   HW_CONFIG
  --tb-fw-config     FILENAME   TB_FW_CONFIG
  --soc-fw-config    FILENAME   SOC_FW_CONFIG
  --tos-fw-config    FILENAME   TOS_FW_CONFIG
  --nt-fw-config     FILENAME   NT_FW_CONFIG
  --rot-cert         FILENAME   Root Of Trust key certificate
  --trusted-key-cert FILENAME   Trusted key certificate
  --scp-fw-key-cert  FILENAME   SCP Firmware key certificate
  --soc-fw-key-cert  FILENAME   SoC Firmware key certificate
  --tos-fw-key-cert  FILENAME   Trusted OS Firmware key certificate
  --nt-fw-key-cert   FILENAME   Non-Trusted Firmware key certificate
  --tb-fw-cert       FILENAME   Trusted Boot Firmware BL2 certificate
  --scp-fw-cert      FILENAME   SCP Firmware content certificate
  --soc-fw-cert      FILENAME   SoC Firmware content certificate
  --tos-fw-cert      FILENAME   Trusted OS Firmware content certificate
  --nt-fw-cert       FILENAME   Non-Trusted Firmware content certificate
  --sip-sp-cert      FILENAME   SiP owned Secure Partition content certificate
  --plat-sp-cert     FILENAME   Platform owned Secure Partition content certificate
```


```
0xffffffff +----------+
           :          :
           |----------|
           |HW_CONFIG |
0x83000000 |----------|  (non-secure)
           |          |
0x80000000 +----------+

           Trusted SRAM
0x04040000 +----------+  loaded by BL2  +----------------+
           | BL1 (rw) |  <<<<<<<<<<<<<  |                |
           |----------|  <<<<<<<<<<<<<  |  BL31 NOBITS   |
           |   BL2    |  <<<<<<<<<<<<<  |                |
           |----------|  <<<<<<<<<<<<<  |----------------|
           |          |  <<<<<<<<<<<<<  | BL31 PROGBITS  |
           |          |  <<<<<<<<<<<<<  |----------------|
           |          |  <<<<<<<<<<<<<  |     BL32       |
0x04003000 +----------+                 +----------------+
           |  CONFIG  |
0x04001000 +----------+
           |  Shared  |
0x04000000 +----------+

           Trusted ROM
0x04000000 +----------+
           | BL1 (ro) |
0x00000000 +----------+
```

### QEMU virt Armv8-A



make PLAT=qemu BL33=bl33.bin BL32_RAM_LOCATION=tdram SPD=opteed all fip

https://trustedfirmware-a.readthedocs.io/en/latest/plat/qemu.html?highlight=qemu#