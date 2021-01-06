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

> aarch64: genisoimage -V 'CentOS 7 aarch64' -R -J -T -graft-points -verbose -e images/efiboot.img -no-emul-boot -c boot.catalog -hide boot.catalog -o /root/centos_isos/CentOS-7-aarch64-Everything-1810-variant.iso /root/centos_tree

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
