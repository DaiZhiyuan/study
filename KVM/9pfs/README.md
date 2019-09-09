<!-- TOC -->

- [1. Test environment](#1-test-environment)
- [2. QEMU configure](#2-qemu-configure)
    - [2.1 add Qemu build option](#21-add-qemu-build-option)
    - [2.2 add Qemu runing option](#22-add-qemu-runing-option)
- [3. Rebuild guest kernel](#3-rebuild-guest-kernel)
    - [3.1 Mount 9pfs by mount_tags](#31-mount-9pfs-by-mount_tags)
    - [3.2 Auto mount](#32-auto-mount)
    - [3.3 View /mnt dirctory](#33-view-mnt-dirctory)

<!-- /TOC -->

# 1. Test environment

- Kernel vmlinuz-3.10.0-693.el7.x86_64
- QEMU 2.5.0

# 2. QEMU configure

## 2.1 add Qemu build option 

`--enable-virtfs`:

```
./configure --target-list=x86_64-softmmu --enable-sdl --enable-kvm --enable-uuid --enable-bzip2 --enable-numa --enable-libusb --enable-virtfs --enable-debug --enable-debug-info
```

## 2.2 add Qemu runing option

`-fsdev local,id=shared,path=/data/failed_snapshots,security_model=none -device virtio-9p-pci,fsdev=shared,mount_tag=snapshot`

```
xinit /usr/local/bin/qemu-system-x86_64 \
-cpu host -smp 4 -m 2048 \
-M q35 \
-enable-kvm \
-monitor stdio \
-drive file=/usr/share/ovmf/OVMF_CODE.fd,if=pflash,format=raw,unit=0,readonly=on \
-drive file=/usr/share/ovmf/OVMF_VARS.fd,if=pflash,format=raw,unit=1 \
-drive file=/root/centos.qcow2,if=virtio \
-boot menu=on,splash-time=5000 \
-net nic -net tap,ifname=tap1,script=no,downscript=no \
-fsdev local,id=shared,path=/data/failed_snapshots,security_model=none -device virtio-9p-pci,fsdev=shared,mount_tag=snapshot
```

# 3. Rebuild guest kernel

guest kernel add option `make menuconfig` to `make` && `make install` and reboot.

```
CONFIG_NET_9P=y
CONFIG_NET_9P_VIRTIO=m
CONFIG_9P_FS=y
CONFIG_9P_FS_POSIX_ACL=y
CONFIG_VIRTIO_PCI=m
CONFIG_PCI=y
```

## 3.1 Mount 9pfs by mount_tags

```
[root@localhost ~]# modprobe 9pnet_virtio

[root@localhost ~]# mount -t 9p -o rw,relatime,sync,dirsync,trans=virtio,version=9p2000.L snapshot /mnt

[root@localhost ~]# mount | tail -1
snapshot on /mnt type 9p (rw,relatime,sync,dirsync,trans=virtio,version=9p2000.L)
```

## 3.2 Auto mount

```
[root@localhost ~]# echo "snapshot /mnt 9p rw,relatime,sync,dirsync,trans=virtio,version=9p2000.L 0 0" >> /etc/fstab
```

## 3.3 View /mnt dirctory

```
[root@localhost ~]# ls -l /mnt/
total 60540
drwxr-xr-x. 2 root root     4096 Feb  1  2019 dump_0000000008__2018-12-18-07:10:29__33554433
-rw-r--r--. 1 root root 61986580 Sep  4 15:25 dump_0000000008__2018-12-18-07:10:29__33554433.tar.gz

[root@localhost ~]# ls -l /mnt/dump_0000000008__2018-12-18-07\:10\:29__33554433
total 258268
-rw-r--r--. 1 root root     15248 Feb  1  2019 cpu.bin
-rw-r--r--. 1 root root 134217728 Feb  1  2019 event.bin
-rw-r--r--. 1 root root      2944 Feb  1  2019 itr0_addr_xlate_regs.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci00_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci00_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci01_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci01_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci02_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci02_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci03_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci03_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci04_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci04_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci05_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci05_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci06_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci06_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci07_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci07_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci08_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci08_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci09_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci09_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci10_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci10_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr0_dci11_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr0_dci11_mtr_bar2.bin
-rw-r--r--. 1 root root       148 Feb  1  2019 itr0_error_state.bin
-rw-r--r--. 1 root root        36 Feb  1  2019 itr0_htl_info.bin
-rw-r--r--. 1 root root  33554432 Feb  1  2019 itr0_htl_ram.bin
-rw-r--r--. 1 root root        40 Feb  1  2019 itr0_ptld_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr0_ptld_info.bin
-rw-r--r--. 1 root root        16 Feb  1  2019 itr0_ptlt0_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr0_ptlt0_info.bin
-rw-r--r--. 1 root root        16 Feb  1  2019 itr0_ptlt1_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr0_ptlt1_info.bin
-rw-r--r--. 1 root root        16 Feb  1  2019 itr0_ptlt2_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr0_ptlt2_info.bin
-rw-r--r--. 1 root root      2944 Feb  1  2019 itr1_addr_xlate_regs.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci00_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci00_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci01_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci01_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci02_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci02_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci03_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci03_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci04_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci04_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci05_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci05_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci06_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci06_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci07_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci07_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci08_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci08_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci09_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci09_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci10_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci10_mtr_bar2.bin
-rw-r--r--. 1 root root      4096 Feb  1  2019 itr1_dci11_mtr_bar0.bin
-rw-r--r--. 1 root root   2621440 Feb  1  2019 itr1_dci11_mtr_bar2.bin
-rw-r--r--. 1 root root       148 Feb  1  2019 itr1_error_state.bin
-rw-r--r--. 1 root root        36 Feb  1  2019 itr1_htl_info.bin
-rw-r--r--. 1 root root  33554432 Feb  1  2019 itr1_htl_ram.bin
-rw-r--r--. 1 root root        40 Feb  1  2019 itr1_ptld_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr1_ptld_info.bin
-rw-r--r--. 1 root root        16 Feb  1  2019 itr1_ptlt0_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr1_ptlt0_info.bin
-rw-r--r--. 1 root root        16 Feb  1  2019 itr1_ptlt1_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr1_ptlt1_info.bin
-rw-r--r--. 1 root root        16 Feb  1  2019 itr1_ptlt2_error_state.bin
-rw-r--r--. 1 root root        32 Feb  1  2019 itr1_ptlt2_info.bin
-rw-r--r--. 1 root root      1104 Feb  1  2019 oss_dev_map.bin
-rw-r--r--. 1 root root       352 Feb  1  2019 oss_mem_map.bin
-rw-r--r--. 1 root root        64 Feb  1  2019 payload.bin
-rw-r--r--. 1 root root         4 Feb  1  2019 rcp_err_code.bin
-rw-r--r--. 1 root root        24 Feb  1  2019 sampling_endpos
```