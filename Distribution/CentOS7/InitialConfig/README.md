- 1 添加用户
    - 1.1 添加JerryDai用户
    - 1.2 设置只允许JerryDai用户可以切换到管理员身份
    - 1.3 将root用户的email转发给JerryDai
- 2 禁用防火墙
    - 2.1 CentOS7默认启用防火墙服务
    - 2.2 停止防火墙服务
    - 2.3 禁用防火墙服务
- 3 禁用SELinux
    - 3.1 CentOS7默认启用SElinux强制访问控制
    - 3.2 停止SElinux
    - 3.2 禁用SElinux
- 4 设置网络
    - 4.1 配置静态IP地址
    - 4.2 禁用IPv6协议栈
    - 4.3 使用EthX网络接口命名
- 5 禁用不使用服务
    - 5.1 查看服务状态
    - 5.2 禁用systemd服务
    - 5.3 禁用SysV服务
- 6 更新系统
- 7 添加额外yum源
    - 7.1 安装yum优先级插件
    - 7.2 添加EPEL源
    - 7.3 添加SCLo源

# 1. 添加用户

## 1.1 添加JerryDai用户
```Shell
[root@localhost ~]# useradd JerryDai
[root@localhost ~]# passwd JerryDai
Changing password for user JerryDai.
New password:           # set password
Retype new password:    # confirm
passwd: all authentication tokens updated successfully.
[root@localhost ~]# exit  # logout
```

## 1.2 设置只允许JerryDai用户可以切换到管理员身份
```Shell
[root@localhost ~]# usermod -G wheel JerryDai
[root@localhost ~]# vi /etc/pam.d/su
#%PAM-1.0
auth        sufficient  pam_rootok.so
# Uncomment the following line to implicitly trust users in the "wheel" group.
#auth       sufficient  pam_wheel.so trust use_uid
# Uncomment the following line to require a user to be in the "wheel" group.
auth       required    pam_wheel.so use_uid
auth        substack    system-auth
auth        include     postlogin
account     sufficient  pam_succeed_if.so uid = 0 use_uid quiet
account     include     system-auth
password    include     system-auth
session     include     system-auth
session     include     postlogin
session     optional    pam_xauth.so
```

## 1.3 将root用户的email转发给JerryDai

```Shell
[root@localhost ~]# vi /etc/aliases
# Person who should get root's mail
root:       JerryDai

[root@localhost ~]# newaliases
```

# 2. 禁用防火墙

## 2.1 CentOS7默认启用防火墙服务

```Shell
[root@localhost ~]# systemctl status firewalld
● firewalld.service - firewalld - dynamic firewall daemon
   Loaded: loaded (/usr/lib/systemd/system/firewalld.service; enabled; vendor preset: enabled)
   Active: active (running) since Fri 2018-10-19 11:08:11 CST; 3h 53min ago
     Docs: man:firewalld(1)
 Main PID: 823 (firewalld)
    Tasks: 2
   CGroup: /system.slice/firewalld.service
           └─823 /usr/bin/python -Es /usr/sbin/firewalld --nofork --nopid
```

## 2.2 停止防火墙服务

```Shell
[root@localhost ~]# systemctl stop firewalld
```

## 2.3 禁用防火墙服务

```Shell
[root@localhost ~]# systemctl disable firewalld
Removed symlink /etc/systemd/system/multi-user.target.wants/firewalld.service.
Removed symlink /etc/systemd/system/dbus-org.fedoraproject.FirewallD1.service.
```

# 3. 禁用SELinux

## 3.1 CentOS7默认启用SElinux强制访问控制

```Shell
[root@localhost ~]# getenforce
Enforcing
```

## 3.2 停止SElinux

```Shell
[root@localhost ~]# setenforce Permissive
```

## 3.2 禁用SElinux

```Shell
[root@localhost ~]# vi /etc/selinux/config
# This file controls the state of SELinux on the system.
# SELINUX= can take one of these three values:
#     enforcing - SELinux security policy is enforced.
#     permissive - SELinux prints warnings instead of enforcing.
#     disabled - No SELinux policy is loaded.
SELINUX=disabled   # change to disabled
# SELINUXTYPE= can take one of these two values:
#     targeted - Targeted processes are protected,
#     minimum - Modification of targeted policy. Only selected processes are protected.
#     mls - Multi Level Security protection.
SELINUXTYPE=targeted

# restart to apply new setting
[root@localhost ~]# reboot
```

# 4. 设置网络

## 4.1 配置静态IP地址

```Shell

# set hostname
[root@localhost ~]# hostnamectl set-hostname lab.centos7.org
# display devices

[root@localhost ~]# nmcli d 
DEVICE       TYPE      STATE      CONNECTION
eth0         ethernet  connected  eth0
lo           loopback  unmanaged  --

# set IPv4 address ⇒ nmcli *** [IP address]
[root@localhost ~]# nmcli c modify eth0 ipv4.addresses 10.0.0.30/24 

# set default gateway
[root@localhost ~]# nmcli c modify eth0 ipv4.gateway 10.0.0.1 

# set DNS
[root@localhost ~]# nmcli c modify eth0 ipv4.dns 10.0.0.1 

# set manual for static setting (it's "auto" for DHCP)
[root@localhost ~]# nmcli c modify eth0 ipv4.method manual 

# restart the interface and reload the settings
[root@localhost ~]# nmcli c down eth0; nmcli c up eth0 
Connection successfully activated (D-Bus active path: /org/freedesktop/NetworkManager/ActiveConnection/1)

# show settings
[root@localhost ~]# nmcli d show eth0 
GENERAL.DEVICE:                 eth0
GENERAL.TYPE:                   ethernet
GENERAL.HWADDR:                 00:0C:29:CD:9C:2D
GENERAL.MTU:                    1500
GENERAL.STATE:                  100 (connected)
GENERAL.CONNECTION:             eth0
GENERAL.CON-PATH:               /org/freedesktop/NetworkManager/ActiveConnection/0
WIRED-PROPERTIES.CARRIER:       on
IP4.ADDRESS[1]:                 ip = 10.0.0.30/24, gw = 10.0.0.1
IP4.DNS[1]:                     10.0.0.1
IP6.ADDRESS[1]:                 ip = fe80::20c:29ff:fecd:9c2d/64, gw = ::

# show status
[root@localhost ~]# ip addr show 
1: lo: <LOOPBACK,UP,LOWER_UP> mtu 65536 qdisc noqueue state UNKNOWN
    link/loopback 00:00:00:00:00:00 brd 00:00:00:00:00:00
    inet 127.0.0.1/8 scope host lo
       valid_lft forever preferred_lft forever
    inet6 ::1/128 scope host
       valid_lft forever preferred_lft forever
2: eth0: <BROADCAST,MULTICAST,UP,LOWER_UP> mtu 1500 qdisc pfifo_fast state UP qlen 1000
    link/ether 00:0c:29:cd:9c:2d brd ff:ff:ff:ff:ff:ff
    inet 10.0.0.30/24 brd 10.0.0.255 scope global eth0
       valid_lft forever preferred_lft forever
    inet6 fe80::20c:29ff:fecd:9c2d/64 scope link
       valid_lft forever preferred_lft forever
```

## 4.2 禁用IPv6协议栈

```Shell

[root@localhost ~]# vi /etc/default/grub

# line 6: add
GRUB_CMDLINE_LINUX="ipv6.disable=1 crashkernel=auto rd.lvm.lv=centos/root.....

# apply changing
[root@localhost ~]# grub2-mkconfig -o /boot/grub2/grub.cfg 
[root@localhost ~]# reboot 
```

## 4.3 使用EthX网络接口命名

```Shell
[root@localhost ~]# vi /etc/default/grub

# line 6: add
GRUB_CMDLINE_LINUX="net.ifnames=0 crashkernel=auto rd.lvm.lv=centos/root.....

# apply changing
[root@localhost ~]# grub2-mkconfig -o /boot/grub2/grub.cfg 
[root@localhost ~]# reboot
```

# 5. 禁用不使用服务

## 5.1 查看服务状态

```Shell
# display the list of services which are running
[root@lab ~]# systemctl -t service 
UNIT                                    LOAD   ACTIVE SUB     DESCRIPTION
auditd.service                          loaded active running Security Auditing Service
avahi-daemon.service                    loaded active running Avahi mDNS/DNS-SD Stack
crond.service                           loaded active running Command Scheduler
dbus.service                            loaded active running D-Bus System Message Bus
getty@tty1.service                      loaded active running Getty on tty1
...
...
...
systemd-udevd.service                   loaded active running udev Kernel Device Manager
systemd-update-utmp.service             loaded active exited  Update UTMP about System Reboot/Shutdown
systemd-user-sessions.service           loaded active exited  Permit User Sessions
systemd-vconsole-setup.service          loaded active exited  Setup Virtual Console
tuned.service                           loaded active running Dynamic System Tuning Daemon

LOAD   = Reflects whether the unit definition was properly loaded.
ACTIVE = The high-level unit activation state, i.e. generalization of SUB.
SUB    = The low-level unit activation state, values depend on unit type.

39 loaded units listed. Pass --all to see loaded but inactive units, too.
To show all installed unit files use 'systemctl list-unit-files'.

# the list of all services
[root@lab ~]# systemctl list-unit-files -t service 
UNIT FILE                               STATE
auditd.service                          enabled
autovt@.service                         disabled
avahi-daemon.service                    enabled
blk-availability.service                disabled
brandbot.service                        static
...
...
...
systemd-user-sessions.service           static
systemd-vconsole-setup.service          static
teamd@.service                          static
tuned.service                           enabled
wpa_supplicant.service                  disabled

125 unit files listed.
```

## 5.2 禁用systemd服务

```Shell
[root@lab ~]# systemctl stop postfix 
[root@lab ~]# systemctl disable postfix 
rm '/etc/systemd/system/multi-user.target.wants/postfix.service'
```

## 5.3 禁用SysV服务

```Shell
[root@lab ~]# chkconfig --list 

Note: This output shows SysV services only and does not include native
      systemd services. SysV configuration data might be overridden by native
      systemd configuration.

      If you want to list systemd services use 'systemctl list-unit-files'.
      To see services enabled on particular target use
      'systemctl list-dependencies [target]'.

iprdump         0:off   1:off   2:on    3:on    4:on    5:on    6:off
iprinit         0:off   1:off   2:on    3:on    4:on    5:on    6:off
iprupdate       0:off   1:off   2:on    3:on    4:on    5:on    6:off
netconsole      0:off   1:off   2:off   3:off   4:off   5:off   6:off
network         0:off   1:off   2:on    3:on    4:on    5:on    6:off

# for exmaple, turn OFF auto-start setting for netconsole
[root@lab ~]# chkconfig netconsole off 
```

## 6. 更新系统

```Shell
[root@lab ~]# yum -y update 
Loaded plugins: fastestmirror
Loading mirror speeds from cached hostfile
 * base: ftp.tsinghua.cn
 * extras: ftp.tsinghua.cn
 * updates: ftp.tsinghua.cn
Resolving Dependencies
--> Running transaction check
---> Package NetworkManager.x86_64 1:0.9.9.1-13.git20140326.4dba720.el7 will be updated
---> Package NetworkManager.x86_64 1:0.9.9.1-23.git20140326.4dba720.el7_0 will be an update
...
...
...
--> Finished Dependency Resolution

Dependencies Resolved

= == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
 Package                      Arch      Version                       Repository     Size
= == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
Installing:
 kernel                       x86_64    3.10.0-123.4.2.el7            updates        29 M
Updating:
 NetworkManager               x86_64    1:0.9.9.1-23.git20140326...   updates       1.4 M
 NetworkManager-glib          x86_64    1:0.9.9.1-23.git20140326...   updates       369 k
 NetworkManager-tui           x86_64    1:0.9.9.1-23.git20140326...   updates       190 k
 gnutls                       x86_64    3.1.18-9.el7_0                updates       609 k
 json-c                       x86_64    0.11-4.el7_0                  updates        31 k
 kernel-tools                 x86_64    3.10.0-123.4.2.el7            updates       669 k
 kernel-tools-libs            x86_64    3.10.0-123.4.2.el7            updates       609 k
 kexec-tools                  x86_64    2.0.4-32.el7.centos.1         updates       320 k
 libtasn1                     x86_64    3.3-5.el7_0                   updates       316 k
 mariadb-libs                 x86_64    1:5.5.37-1.el7_0              updates       752 k
 microcode_ctl                x86_64    2:2.1-7.1.el7_0.1             updates       488 k
 openssl                      x86_64    1:1.0.1e-34.el7_0.3           updates       705 k
 openssl-libs                 x86_64    1:1.0.1e-34.el7_0.3           updates       939 k
 selinux-policy               noarch    3.12.1-153.el7_0.10           updates       340 k
 selinux-policy-targeted      noarch    3.12.1-153.el7_0.10           updates       3.8 M
 tuned                        noarch    2.3.0-11.el7_0.3              updates       145 k
 tzdata                       noarch    2014e-1.el7_0                 updates       433 k

Transaction Summary
= == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == == ==
Install   1 Package
Upgrade  17 Packages

Total download size: 41 M
...
...
...
  Verifying  : kernel-tools-libs-3.10.0-123.el7.x86_64                                                34/35
  Verifying  : 1:NetworkManager-tui-0.9.9.1-13.git20140326.4dba720.el7.x86_64                         35/35

Installed:
  kernel.x86_64 0:3.10.0-123.4.2.el7

Updated:
  NetworkManager.x86_64 1:0.9.9.1-23.git20140326.4dba720.el7_0         NetworkManager-glib.x86_64 1:0.9.9.1-2
  NetworkManager-tui.x86_64 1:0.9.9.1-23.git20140326.4dba720.el7_0     gnutls.x86_64 0:3.1.18-9.el7_0
  json-c.x86_64 0:0.11-4.el7_0                                         kernel-tools.x86_64 0:3.10.0-123.4.2.e
  kernel-tools-libs.x86_64 0:3.10.0-123.4.2.el7                        kexec-tools.x86_64 0:2.0.4-32.el7.cent
  libtasn1.x86_64 0:3.3-5.el7_0                                        mariadb-libs.x86_64 1:5.5.37-1.el7_0
  microcode_ctl.x86_64 2:2.1-7.1.el7_0.1                               openssl.x86_64 1:1.0.1e-34.el7_0.3
  openssl-libs.x86_64 1:1.0.1e-34.el7_0.3                              selinux-policy.noarch 0:3.12.1-153.el7
  selinux-policy-targeted.noarch 0:3.12.1-153.el7_0.10                 tuned.noarch 0:2.3.0-11.el7_0.3
  tzdata.noarch 0:2014e-1.el7_0

Complete!
```

# 7. 添加额外yum源

## 7.1 安装yum优先级插件

```Shell
[root@lab ~]# yum -y install yum-plugin-priorities

# set [priority=1] to official repository
[root@lab ~]# sed -i -e "s/\]$/\]\npriority=1/g" /etc/yum.repos.d/CentOS-Base.repo
```

## 7.2 添加EPEL源

```Shell
[root@lab ~]# yum -y install epel-release

# set [priority=5]
[root@lab ~]# sed -i -e "s/\]$/\]\npriority=5/g" /etc/yum.repos.d/epel.repo

# for another way, change to [enabled=0] and use it only when needed
[root@lab ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/epel.repo

# if [enabled=0], input a command to use the repository
[root@lab ~]# yum --enablerepo=epel install [Package]
```

## 7.3 添加SCLo源

```Shell
[root@lab ~]# yum -y install centos-release-scl-rh centos-release-scl

# set [priority=10]
[root@lab ~]# sed -i -e "s/\]$/\]\npriority=10/g" /etc/yum.repos.d/CentOS-SCLo-scl.repo 
[root@lab ~]# sed -i -e "s/\]$/\]\npriority=10/g" /etc/yum.repos.d/CentOS-SCLo-scl-rh.repo

# for another way, change to [enabled=0] and use it only when needed
[root@lab ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-SCLo-scl.repo 
[root@lab ~]# sed -i -e "s/enabled=1/enabled=0/g" /etc/yum.repos.d/CentOS-SCLo-scl-rh.repo

# if [enabled=0], input a command to use the repository
[root@lab ~]# yum --enablerepo=centos-sclo-rh install [Package]
[root@lab ~]# yum --enablerepo=centos-sclo-sclo install [Package]
```

