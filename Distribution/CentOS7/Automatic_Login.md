# 1. Auto login on all terminals

1. edit
```
[root@centos7 ~]# vi /etc/systemd/system/getty.target.wants/getty\@tty1.service 
```

2. Find the line
```
ExecStart=-/sbin/agetty  --noclear %I $TERM
```

3. and change to
```
ExecStart=-/sbin/agetty --autologin root --noclear %I $TERM
```

# 2. GDM enable Automatic login 

Update GDM settings: Add the following stanza into a [daemon] section of your /etc/gdm/custom.conf while replacing keyword "linuxconfig" with a username of your autologin user:
```
[daemon]
AutomaticLogin=linuxconfig
AutomaticLoginEnable=True
```