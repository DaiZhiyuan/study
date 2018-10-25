- 1 完全访问的共享文件夹
    - 1.1 安装配置Samba
    - 1.2 添加防火墙策略
    - 1.3 更新SELinux上下文
- 2 有限访问共享文件夹
    - 2.1 安装配置Samba
    - 2.2 添加防火墙策略
    - 2.3 更新SELinux上下文

# 1. 完全访问的共享文件夹

## 1.1 安装配置Samba

```Shell
[root@smb ~]# yum -y install samba samba-client
[root@smb ~]# mkdir /home/share 
[root@smb ~]# chmod 777 /home/share 
[root@smb ~]# vi /etc/samba/smb.conf
# near line 66: add follows
unix charset = UTF-8
dos charset = CP932

# line 90: change (Windows' default)
workgroup = WORKGROUP

# line 96: uncomment and change IP address you allow
hosts allow = 127. 10.0.0.

# line 126: add ( no auth )
security = user
passdb backend = tdbsam
map to guest = Bad User

# add follows to the end
[Share]# any name you like
    path = /home/share# shared directory
    writable = yes# writable
    guest ok = yes# guest allowed
    guest only = yes# guest only
    create mode = 0777# fully accessed file
    directory mode = 0777# fully accessed directory
    
[root@smb ~]# systemctl start smb nmb 
[root@smb ~]# systemctl enable smb nmb 
```

## 1.2 添加防火墙策略

```Shell
[root@smb ~]# firewall-cmd --add-service=samba --permanent 
success
[root@smb ~]# firewall-cmd --reload 
success
```

## 1.3 更新SELinux策略

```Shell
[root@smb ~]# setsebool -P samba_enable_home_dirs on 
[root@smb ~]# restorecon -R /home/share 
```

# 2. 有限访问共享文件夹

## 2.1 安装配置Samba

```Shell
[root@smb ~]# yum -y install samba samba-client
[root@smb ~]# groupadd security 
[root@smb ~]# mkdir /home/security 
[root@smb ~]# chgrp security /home/security 
[root@smb ~]# chmod 770 /home/security 

[root@smb ~]# vi /etc/samba/smb.conf
# near line 66: add follows
unix charset = UTF-8

# line 89: change (Windows' default)
workgroup = WORKGROUP

# line 95: uncomment and change IP address you allow
hosts allow = 127. 10.0.0.

# add follows to the end
[Security]# any name you like
    path = /home/security
    writable = yes
    create mode = 0770
    directory mode = 0770
    guest ok = no# guest not allowed
    valid users = @security# allow only security group
    
[root@smb ~]# systemctl start smb nmb 
[root@smb ~]# systemctl enable smb nmb 

# add a user in Samba
[root@smb ~]# smbpasswd -a JerryDai
New SMB password:     # set password
Retype new SMB password:     # confirm
Added user JerryDai.
[root@smb ~]# usermod -G security JerryDai 
```

## 2.2 添加防火墙策略

```Shell
[root@smb ~]# firewall-cmd --add-service=samba --permanent 
success
[root@smb ~]# firewall-cmd --reload 
success
```

## 2.3 更新SELinux上下文

```Shell
[root@smb ~]# setsebool -P samba_enable_home_dirs on 
[root@smb ~]# restorecon -R /home/security 
```

