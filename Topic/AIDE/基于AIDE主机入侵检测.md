***基于AIDE主机入侵检测***

[TOC]

# 1. 简介

AIDE（Adevanced Intrusion Detection Environment）高级入侵检测环境)是个入侵检测工具，主要用途是检查文件的完整性。

- 多种完整性检验算法
- 权限(permission)
- 索引节点序号(inode number)
- 所属用户(user) 所属用户组(group)、文档大小(size)
- 最后修改时间(mtime)、创建时间(ctime)、最后访问时间(atime)
- Acl 访问控制列表
- SELinux 安全上下文
- xattrs扩展文件属性

# 2. 默认配置文件

AIDE配置文件/etc/aide.conf

```
# Example configuration file for AIDE.

@@define DBDIR /var/lib/aide
@@define LOGDIR /var/log/aide

# The location of the database to be read.
database=file:@@{DBDIR}/aide.db.gz

# The location of the database to be written.
#database_out=sql:host:port:database:login_name:passwd:table
#database_out=file:aide.db.new
database_out=file:@@{DBDIR}/aide.db.new.gz

# Whether to gzip the output to database.
gzip_dbout=yes

# Default.
verbose=5

report_url=file:@@{LOGDIR}/aide.log
report_url=stdout

# These are the default rules.
#
#p:      permissions
#i:      inode:
#n:      number of links
#u:      user
#g:      group
#s:      size
#b:      block count
#m:      mtime
#a:      atime
#c:      ctime
#S:      check for growing size
#acl:           Access Control Lists
#selinux        SELinux security context
#xattrs:        Extended file attributes
#md5:    md5 checksum
#sha1:   sha1 checksum
#sha256:        sha256 checksum
#sha512:        sha512 checksum
#rmd160: rmd160 checksum
#tiger:  tiger checksum

#haval:  haval checksum (MHASH only)
#gost:   gost checksum (MHASH only)
#crc32:  crc32 checksum (MHASH only)
#whirlpool:     whirlpool checksum (MHASH only)

FIPSR = p+i+n+u+g+s+m+c+acl+selinux+xattrs+sha256

#R:             p+i+n+u+g+s+m+c+acl+selinux+xattrs+md5
#L:             p+i+n+u+g+acl+selinux+xattrs
#E:             Empty group
#>:             Growing logfile p+u+g+i+n+S+acl+selinux+xattrs

# You can create custom rules like this.
# With MHASH...
# ALLXTRAHASHES = sha1+rmd160+sha256+sha512+whirlpool+tiger+haval+gost+crc32
ALLXTRAHASHES = sha1+rmd160+sha256+sha512+tiger

# Everything but access time (Ie. all changes)
EVERYTHING = R+ALLXTRAHASHES

# Sane, with one good hash.
# NORMAL = sha256
NORMAL = sha256

# For directories, don't bother doing hashes.
DIR = p+i+n+u+g+acl+selinux+xattrs

# Access control only.
PERMS = p+u+g+acl+selinux+xattrs

# Access + inode changes + file type.
STATIC = p+u+g+acl+selinux+xattrs+i+n+b+c+ftype

# Logfiles only check access w/o xattrs.
LOG = p+u+g+n+acl+selinux+ftype

# Content + file type.
CONTENT = sha256+ftype

# Extended content + file type + access.
CONTENT_EX = sha256+ftype+p+u+g+n+acl+selinux+xattrs

# Some files get updated automatically, so the inode/ctime/mtime change
# but we want to know when the data inside them changes.
DATAONLY =  p+n+u+g+s+acl+selinux+xattrs+sha256

# Next decide what directories/files you want in the database. Aide
# uses a first match system. Put file specific instructions before generic
# matches. e.g. Put file matches before directories.

/boot/   CONTENT_EX
/bin/    CONTENT_EX
/sbin/   CONTENT_EX
/lib/    CONTENT_EX
/lib64/  CONTENT_EX
/opt/    CONTENT

# Admin's dot files constantly change, just check perms.
/root/\..* PERMS
# Otherwise get all of /root.
/root/   CONTENT_EX

# These are too volatile.
!/usr/src/
!/usr/tmp/
# Otherwise get all of /usr.
/usr/    CONTENT_EX

# Check only permissions, user, group, seliunx for /etc, but
# cover some important files closely.
!/etc/mtab$

# Ignore backup files
!/etc/.*~

# trusted databases
/etc/hosts$ CONTENT_EX
/etc/host.conf$ CONTENT_EX
/etc/hostname$ CONTENT_EX
/etc/issue$ CONTENT_EX
/etc/issue.net$ CONTENT_EX
/etc/protocols$ CONTENT_EX
/etc/services$ CONTENT_EX
/etc/localtime$ CONTENT_EX
/etc/alternatives/ CONTENT_EX
/etc/mime.types$ CONTENT_EX
/etc/terminfo/ CONTENT_EX
/etc/exports$  CONTENT_EX
/etc/fstab$    CONTENT_EX
/etc/passwd$   CONTENT_EX
/etc/group$    CONTENT_EX
/etc/gshadow$  CONTENT_EX
/etc/shadow$   CONTENT_EX
/etc/security/opasswd$   CONTENT_EX
/etc/skel/ CONTENT_EX

# networking
/etc/hosts.allow$   CONTENT_EX
/etc/hosts.deny$    CONTENT_EX
/etc/firewalld/ CONTENT_EX
/etc/NetworkManager/ CONTENT_EX
/etc/networks$ CONTENT_EX
/etc/dhcp/ CONTENT_EX
/etc/wpa_supplicant/ CONTENT_EX
/etc/resolv.conf$ DATAONLY
/etc/nscd.conf$ NORMAL

# logins and accounts
/etc/login.defs$ CONTENT_EX
/etc/libuser.conf$ CONTENT_EX
/var/log/faillog$ PERMS
/var/log/lastlog$ PERMS
/var/run/faillock/ PERMS
/etc/pam.d/ CONTENT_EX
/etc/security$ CONTENT_EX
/etc/securetty$ CONTENT_EX
/etc/polkit-1/ CONTENT_EX
/etc/sudo.conf$ CONTENT_EX
/etc/sudoers$ CONTENT_EX
/etc/sudoers.d/ CONTENT_EX

# Shell/X starting files
/etc/profile$ CONTENT_EX
/etc/profile.d/ CONTENT_EX
/etc/bashrc$ CONTENT_EX
/etc/bash_completion.d/ CONTENT_EX
/etc/zprofile$ CONTENT_EX
/etc/zshrc$ CONTENT_EX
/etc/zlogin$ CONTENT_EX
/etc/zlogout$ CONTENT_EX
/etc/X11/ CONTENT_EX
/etc/shells$ CONTENT_EX

# Pkg manager
/etc/yum.conf$ CONTENT_EX
/etc/yumex.conf$ CONTENT_EX
/etc/yumex.profiles.conf$ CONTENT_EX
/etc/yum/ CONTENT_EX
/etc/yum.repos.d/ CONTENT_EX

# This gets new/removes-old filenames daily.
!/var/log/sa/
# As we are checking it, we've truncated yesterdays size to zero.
!/var/log/aide.log

# auditing
# AIDE produces an audit record, so this becomes perpetual motion.
# /var/log/audit/ PERMS+ANF+ARF
/etc/audit/ CONTENT_EX
/etc/audisp/ CONTENT_EX
/etc/libaudit.conf$ CONTENT_EX
/etc/aide.conf$  CONTENT_EX

# System logs
/etc/rsyslog.conf$ CONTENT_EX
/etc/rsyslog.d/ CONTENT_EX
/etc/logrotate.conf$ CONTENT_EX
/etc/logrotate.d/ CONTENT_EX
/var/log/ LOG+ANF+ARF
/var/run/utmp$ LOG

# secrets
/etc/pkcs11/ CONTENT_EX
/etc/pki/ CONTENT_EX
/etc/ssl/ CONTENT_EX
/etc/certmonger/ CONTENT_EX

# init system
/etc/systemd/ CONTENT_EX
/etc/sysconfig/ CONTENT_EX
/etc/rc.d/ CONTENT_EX
/etc/tmpfiles.d/ CONTENT_EX
/etc/machine-id$ CONTENT_EX

# boot config
/etc/grub.d/ CONTENT_EX
/etc/grub2.cfg$ CONTENT_EX
/etc/dracut.conf$ CONTENT_EX
/etc/dracut.conf.d/ CONTENT_EX

# glibc linker
/etc/ld.so.cache$ CONTENT_EX
/etc/ld.so.conf$ CONTENT_EX
/etc/ld.so.conf.d/ CONTENT_EX

# kernel config
/etc/sysctl.conf$ CONTENT_EX
/etc/sysctl.d/ CONTENT_EX
/etc/modprobe.d/ CONTENT_EX
/etc/modules-load.d/ CONTENT_EX
/etc/depmod.d/ CONTENT_EX
/etc/udev/ CONTENT_EX
/etc/crypttab$ CONTENT_EX

#### Daemons ####

# cron jobs
/var/spool/at/ CONTENT
/etc/at.allow$ CONTENT
/etc/at.deny$ CONTENT
/etc/cron.allow$ CONTENT_EX
/etc/cron.deny$ CONTENT_EX
/etc/cron.d/ CONTENT_EX
/etc/cron.daily/ CONTENT_EX
/etc/cron.hourly/ CONTENT_EX
/etc/cron.monthly/ CONTENT_EX
/etc/cron.weekly/ CONTENT_EX
/etc/crontab$ CONTENT_EX
/var/spool/cron/root/ CONTENT
/etc/anacrontab$ CONTENT_EX

# time keeping
/etc/ntp.conf$ CONTENT_EX
/etc/ntp/ CONTENT_EX
/etc/chrony.conf$ CONTENT_EX
/etc/chrony.keys$ CONTENT_EX

# mail
/etc/aliases$ CONTENT_EX
/etc/aliases.db$ CONTENT_EX
/etc/postfix/ CONTENT_EX
/etc/mail.rc$ CONTENT_EX
/etc/mailcap$ CONTENT_EX

# ssh
/etc/ssh/sshd_config$ CONTENT_EX
/etc/ssh/ssh_config$ CONTENT_EX

# stunnel
/etc/stunnel/ CONTENT_EX

# ftp
/etc/vsftpd.conf$ CONTENT
/etc/vsftpd/ CONTENT

# printing
/etc/cups/ CONTENT_EX
/etc/cupshelpers/ CONTENT_EX
/etc/avahi/ CONTENT_EX

# web server
/etc/httpd/ CONTENT_EX

# dns
/etc/named/ CONTENT_EX
/etc/named.conf$ CONTENT_EX
/etc/named.iscdlv.key$ CONTENT_EX
/etc/named.rfc1912.zones$ CONTENT_EX
/etc/named.root.key$ CONTENT_EX

# xinetd
/etc/xinetd.d/ CONTENT_EX

# Now everything else in /etc.
/etc/    PERMS

# With AIDE's default verbosity level of 5, these would give lots of
# warnings upon tree traversal. It might change with future version.
#
#=/lost\+found    DIR
#=/home           DIR

# Ditto /var/log/sa/ same reason...
!/var/log/httpd/

```

# 3. 初始化数据库

```
$> aide --init

AIDE, version 0.15.1

### AIDE database at /var/lib/aide/aide.db.new.gz initialized.

$> mv /var/lib/aide/aide.db.new.gz /var/lib/aide/aide.db.gz

```

# 4. GunPG初始化

## 4.1 生成秘钥
```
$> gpg --gen-key
	算法：RSA
	长度：2048
	有效期：0 永不过期
	标识：JerryDai <zhiyuan.dai@qq.com>
```

## 4.2 查看系统中秘钥
```	
$> gpg --list-keys
/root/.gnupg/pubring.gpg
------------------------
pub   2048R/A4AC3ADE 2018-10-10
uid                  JerryDai <zhiyuan.dai@qq.com>
```

## 4.3 导出密钥

### 4.3.1 导出公钥文件
公钥文件（.gnupg/pubring.gpg）以二进制形式储存，armor参数可以将其转换为ASCII码显示。

```
$> gpg --armor --output public_key.pem --export JerryDai
```

### 4.3.2 导出私钥文件
私钥文件（.gnupg/secring.gpg）以二进制形式储存，armor参数可以将其转换为ASCII码显示。
```
$> gpg --armor --output private_key.pem --export-secret-subkeys JerryDai
```

## 5. fingerprint
由于公钥服务器没有检查机制，任何人都可以用你的名义上传公钥，所以没有办法保证服务器上的公钥的可靠性。
通常，你可以在网站上公布一个公钥指纹，让其他人核对下载到的公钥是否为真。fingerprint参数生成公钥指纹。

```
$> gpg --fingerprint JerryDai
pub   2048R/A4AC3ADE 2018-10-10
      Key fingerprint = 3363 2A47 BD6A 471E 1B8C  D5DA 0D5A AA7C A4AC 3ADE
uid                  JerryDai <zhiyuan.dai@qq.com>
```

## 6. 导入密钥

### 6.1 导入公钥文件
```
$> gpg --import public_key.pem
```
### 6.2 导入私钥文件
```
$> gpg --import --allow-secret-key-import private_key.pem 
```
### 6.3 显示导入的秘钥
```
$> gpg --list-options show-unusable-subkeys --list-keys
```

## 7. 文件签名
```
$> gpg --armor --detach-sign aide.db.new.gz
$> cat aide.db.new.gz.asc
-----BEGIN PGP SIGNATURE-----
Version: GnuPG v2.0.22 (GNU/Linux)

iQEcBAABAgAGBQJbvcKYAAoJEA1aqnykrDreZpQIANcoiLJdPRx2bHYRF5Ub4ue7
1EVVkeFGRdcUzz1s52VgAs1ZOjx58nfhIQPr1mrddfWGrYBXhgYIQb6sQUnFhZp9
QrfHHFz0RYqIgJ9oM8CLGDho6Q5S6ADilTO+ifGzaunNty51sj2IEOJV+tqQOA9z
KWlvd2GodFcXP7RdgCrR9vkUqAnLMXg2FGafOn68nVamVjBO95x414fWNED0eAqc
/5fXPUMgf7MDaQK84/ywnxrRkRyWzGT8lxD6gWC9LuzH2k+G1TWcyhzrUengEu3w
8KZTyHVs874Qoj3JkdpowOCZj0kka3TyHKYKLxWG6PdeNvjx9MTqNJPxSgnNeJU=
=vZPd
-----END PGP SIGNATURE-----
```

## 8. 文件校验
```
$> gpg --verify aide.db.new.gz.asc
gpg: Signature made Wed 10 Oct 2018 05:12:56 PM CST using RSA key ID A4AC3ADE
gpg: Good signature from "JerryDai <zhiyuan.dai@qq.com>"

```

# 8. 检测根文件系统的完整性
```
$> aide -C

AIDE, version 0.15.1

### All files match AIDE database. Looks okey!

$> echo $?
0
```