- 1 密码认证
    - 1.1 阻止root用户远程登录
    - 1.2 添加防火墙策略
    - 1.3 安装SSH客户端
    - 1.4 链接SSH服务器
    - 1.4 在SSH服务器上执行命令
- 2 使用SSH传输文件
    - 2.1 使用SCP(Secure Copy)传输文件
    - 2.2 使用SFTP(SSH File Transfer Protocol)传输文件
- 3 秘钥认证
    - 3.1 创建秘钥对
    - 3.2 将公钥传输到SSH服务器
    - 3.3 禁用密码登录
- 4 仅启用SFTP服务
    - 4.1 设置/home为sftp的根目录
    - 4.2 验证服务配置
- 5 SSH端口转发
    - 5.1 将主机的TCP协议的8081端口数据转发到5901端口上
- 6 SSH X11转发
    - 6.1 SSH服务开启X11转发
    - 6.2 SSH客户端使用方式
- 7 SSHPass
    - 7.1 安装SSHPass
    - 7.2 使用SSHPass
- 8 SSH-Agent
- 9 Parallel SSH
    - 9.1 安装PSSH
    - 9.2 使用PSSH

# 1. 密码认证

## 1.1 阻止root用户远程登录

```Shell
[root@lab ~]# vi /etc/ssh/sshd_config

# line 38: uncomment and change (prohibit root login remotely)
PermitRootLogin no

[root@lab ~]# systemctl restart sshd
```

## 1.2 添加防火墙策略

SSH服务：使用TCP协议22号端口。

```Shell
[root@lab ~]# firewall-cmd --add-service=ssh --permanent 
success

[root@lab ~]# firewall-cmd --reload 
success
```

## 1.3 安装SSH客户端

```Shell
[root@client ~]# yum -y install openssh-clients
```

## 1.4 链接SSH服务器

```Shell
# ssh [username@(hostname or IP address)]
[root@client ~]# ssh JerryDai@lab.centos.org 
The authenticity of host 'lab.centos.org (<no hostip for proxy command>)' can't be established.
ECDSA key fingerprint is xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:60:90:d8.
Are you sure you want to continue connecting (yes/no)? yes
Warning: Permanently added 'lab.centos.org' (ECDSA) to the list of known hosts.
JerryDai@lab.centos.org's password:      # password of the user

[JerryDai@lab ~]$     # just logined
```

## 1.5 在SSH服务器上执行命令

```Shell
# for example, execute [cat /etc/passwd]
[JerryDai@client ~]$ ssh JerryDai@lab.centos.org "cat /etc/passwd" 
JerryDai@lab.centos.org's password:
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
...
...
postfix:x:89:89::/var/spool/postfix:/sbin/nologin
sshd:x:74:74:Privilege-separated SSH:/var/empty/sshd:/sbin/nologin
```

# 2. 使用SSH传输文件

## 2.1 使用SCP(Secure Copy)传输文件

```Shell
# command ⇒ scp [Option] Source Target
# copy the [test.txt] on local to remote server [lab.centos.org]
[JerryDai@dlp ~]$ scp ./test.txt JerryDai@lab.centos.org:~/ 
JerryDai@lab.centos.org's password:     # password of the user
test.txt                                                100%   10     0.0KB/s   00:00

# copy the [/home/fedora/test.txt] on remote server [lab.centos.org] to the local 
[JerryDai@lab ~]$ scp JerryDai@lab.centos.org:/home/JerryDai/test.txt ./test.txt 
JerryDai@lab.centos.org's password:
test.txt                                                100%   10     0.0KB/s   00:00
```

## 2.2 使用SFTP(SSH File Transfer Protocol)传输文件

SFTP服务默认不启用，需要配置/etc/ssh/sshd_config开启sftp子系统（/usr/libexec/openssh/sftp-server）。

```Shell
# command : sftp [Option] [user@host]
[redhat@lab ~]$ sftp JerryDai@lab.centos.org
JerryDai@lab.centos.org's password:     # password of the user
Connected to lab.centos.org.
sftp>

# show current directory on remote server
sftp> pwd
Remote working directory: /home/JerryDai

# show current directory on local server
sftp> !pwd
/home/redhat

# show files in current directory on FTP server
sftp> ls -l
drwxrwxr-x    2 JerryDai     JerryDai            6 Jul 29 21:33 public_html
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 28 22:53 test.txt

# show files in current directory on local server
sftp> !ls -l
total 4
-rw-rw-r-- 1 redhat redhat 10 Jul 29 21:31 test.txt

# change directory
sftp> cd public_html
sftp> pwd
Remote working directory: /home/JerryDai/public_html

# upload a file to remote server
sftp> put test.txt redhat.txt
Uploading test.txt to /home/JerryDai/redhat.txt
test.txt 100% 10 0.0KB/s 00:00

sftp> ls -l
drwxrwxr-x    2 JerryDai     JerryDai            6 Jul 29 21:33 public_html
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:39 redhat.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 28 22:53 test.txt

# upload some files to remote server
sftp> put *.txt
Uploading test.txt to /home/JerryDai/test.txt
test.txt 100% 10 0.0KB/s 00:00
Uploading test2.txt to /home/JerryDai/test2.txt
test2.txt 100% 0 0.0KB/s 00:00
sftp> ls -l
drwxrwxr-x    2 JerryDai     JerryDai            6 Jul 29 21:33 public_html
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:39 redhat.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:45 test.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:46 test2.txt

# download a file from remote server
sftp> get test.txt
Fetching /home/JerryDai/test.txt to test.txt
/home/JerryDai/test.txt 100% 10 0.0KB/s 00:00

# download some files from remote server
sftp> get *.txt
Fetching /home/JerryDai/redhat.txt to redhat.txt
/home/JerryDai/redhat.txt 100% 10 0.0KB/s 00:00
Fetching /home/JerryDai/test.txt to test.txt
/home/JerryDai/test.txt 100% 10 0.0KB/s 00:00
Fetching /home/JerryDai/test2.txt to test2.txt
/home/JerryDai/test2.txt 100% 10 0.0KB/s 00:00

# create a directory on remote server
sftp> mkdir testdir
sftp> ls -l
drwxrwxr-x    2 JerryDai     JerryDai            6 Jul 29 21:33 public_html
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:39 redhat.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:45 test.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:46 test2.txt
drwxrwxr-x    2 JerryDai     JerryDai            6 Jul 29 21:53 testdir

# delete a directory on remote server
sftp> rmdir testdir
rmdir ok, `testdir' removed
sftp> ls -l
drwxrwxr-x    2 JerryDai     JerryDai            6 Jul 29 21:33 public_html
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:39 redhat.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:45 test.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:46 test2.txt

# delete a file on remote server
sftp> rm test2.txt
Removing /home/JerryDai/test2.txt
sftp> ls -l
drwxrwxr-x    2 JerryDai     JerryDai            6 Jul 29 21:33 public_html
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:39 redhat.txt
-rw-rw-r--    1 JerryDai     JerryDai           10 Jul 29 21:45 test.txt

# execute commands with "![command]"
sftp> !cat /etc/passwd
root:x:0:0:root:/root:/bin/bash
bin:x:1:1:bin:/bin:/sbin/nologin
...
...
redhat:x:1001:1001::/home/redhat:/bin/bash
# exit
sftp> quit
221 Goodbye.
```

# 3. 秘钥认证

使用非对称加密算法RSA，创建秘钥对：
- 私钥：SSH客户端使用
- 公钥：SSH服务段使用

## 3.1 创建秘钥对

```Shell
# create key pair
[JerryDai@lab ~]$ ssh-keygen -t rsa 
Generating public/private rsa key pair.
Enter file in which to save the key (/home/JerryDai/.ssh/id_rsa):   # Enter if keep default
Created directory '/home/JerryDai/.ssh'.
Enter passphrase (empty for no passphrase):   # set passphrase (set no passphrase to Enter with empty)
Enter same passphrase again:
Your identification has been saved in /home/JerryDai/.ssh/id_rsa.
Your public key has been saved in /home/JerryDai/.ssh/id_rsa.pub.
The key fingerprint is:
38:f1:b4:6d:d3:0e:59:c8:fa:1d:1d:48:86:f0:fe:74 JerryDai@lab.centos.org
The key's randomart image is:

[JerryDai@lab ~]$ mv ~/.ssh/id_rsa.pub ~/.ssh/authorized_keys 
```

## 3.2 将公钥传输到SSH服务器

```Shell
[JerryDai@www ~]$ mkdir ~/.ssh 
[JerryDai@www ~]$ chmod 700 ~/.ssh

# copy the secret key to local ssh directory
[cent@www ~]$ scp JerryDai@10.0.0.30:/home/JerryDai/.ssh/id_rsa ~/.ssh/ 
JerryDai@10.0.0.30's password:
id_rsa

[JerryDai@www ~]$ ssh -i ~/.ssh/id_rsa JerryDai@10.0.0.30 
Enter passphrase for key '/home/JerryDai/.ssh/id_rsa':   # passphrase
Last login: Wed Jul 30 21:37:19 2014 from lab.centos.org 
[JerryDai@lab ~]$   # just logined
```

## 3.3 禁用密码登录

```Shell
[root@lab ~]# vi /etc/ssh/sshd_config

# line 65: turn to [no]
PasswordAuthentication no

# line 69: make sure the value is [no]
ChallengeResponseAuthentication no

# line 96: make sure the value is [yes]
UsePAM yes

[root@lab ~]# systemctl restart sshd 
```

# 4. 仅启用SFTP服务

## 4.1 设置/home为sftp的根目录

```Shell
# create a group for SFTP
[root@lab ~]# groupadd sftp_users 

# apply to a user [JerryDai] for SFTP only
[root@lab ~]# usermod -G sftp_users JerryDai 
[root@lab ~]# vi /etc/ssh/sshd_config

# line 132: comment out and add a line like below
#Subsystem sftp /usr/libexec/openssh/sftp-server
Subsystem sftp internal-sftp

# add follows to the end
Match Group sftp_users
  X11Forwarding no
  AllowTcpForwarding no
  ChrootDirectory /home
  ForceCommand internal-sftp
  
[root@lab ~]# systemctl restart sshd 
```

## 4.2 验证服务配置

```Shell
[root@localhost ~]# ssh JerryDai@10.0.0.30 
JerryDai@10.0.0.30's password:
This service allows sftp connections only.
Connection to 10.0.0.30 closed.# denyed normally

[root@localhost ~]# sftp JerryDai@10.0.0.30 
Connecting to 10.0.0.30...
JerryDai@10.0.0.30's password:
sftp> ls -l
drwx------    3 1000     1000         4096 Jul  9 12:06 cent
drwx------    2 1001     1001           59 Jul  8 22:06 hirokun
sftp> pwd
Remote working directory: /
sftp> exit
```

# 5. SSH端口转发

## 5.1 将主机的TCP协议的8081端口数据转发到5901端口上

```Shell
# forward the connection to 8081 to 5901 on local
[JerryDai@lab ~]$ ssh -L 0.0.0.0:8081:localhost:5901 JerryDai@localhost 
JerryDai@localhost's password:   # the password of the working user (it means the login to local to local)
Last login: Thu Jul 10 01:35:15 2014

# confirm
[JerryDai@lab ~]$ netstat -lnp | grep 8081 
(Not all processes could be identified, non-owned process info
 will not be shown, you would have to be root to see it all.)
tcp        0      0 0.0.0.0:8081            0.0.0.0:*               LISTEN      3238/ssh

```

# 6. SSH X11转发

## 6.1 SSH服务开启X11转发

```Shell
[root@lab ~]# vi /etc/ssh/sshd_config

# line 102: uncomment
X11Forwarding yes
X11DisplayOffset 10

[root@lab ~]# systemctl restart sshd
```


## 6.2 SSH客户端使用方式
```Shell
[root@localhost ~]# ssh -XC JerryDai@lab.centos.org
```

# 7. SSHPass

使用密码认证策略下，使用SSHPass自动输入密码。

## 7.1 安装SSHPass

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install sshpass
```

## 7.2 使用SSHPass

```Shell
# -p password : from argument
[JerryDai@lab ~]$ sshpass -p password ssh 10.0.0.51 hostname 
node01.centos.org

# -f file : from file
[JerryDai@lab ~]$ echo 'password' > sshpass.txt 
[JerryDai@lab ~]$ chmod 600 sshpass.txt 
[JerryDai@lab ~]$ sshpass -f sshpass.txt ssh 10.0.0.51 hostname 
node01.centos.org

# -e : from env variable
[JerryDai@lab ~]$ export SSHPASS=password 
[JerryDai@lab ~]$ sshpass -e ssh 10.0.0.51 hostname 
node01.centos.org
```

# 8. SSH-Agent

使用秘钥验证策略下，使用SSH-Agent自动输入密码。

```Shell
# start SSH-Agent
[JerryDai@lab ~]$ eval `ssh-agent` 
Agent pid 2168

# add Identity
[JerryDai@lab ~]$ ssh-add 
Enter passphrase for /home/JerryDai/.ssh/id_rsa:
Identity added: /home/JerryDai/.ssh/id_rsa (/home/JerryDai/.ssh/id_rsa)

# confirm
[JerryDai@lab ~]$ ssh-add -l 
2048 xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:4c:b2 /home/JerryDai/.ssh/id_rsa (RSA)

# try to conenct with SSH without passphrase
[JerryDai@lab ~]$ ssh node01.centos.org hostname 
node01.centos.org

# exit from SSH-Agent
[JerryDai@lab ~]$ eval `ssh-agent -k` 
Agent pid 2168 killed
```

# 9. Parallel SSH

## 9.1 安装PSSH

```Shell

# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install pssh
```

## 9.2 使用PSSH

使用PSSH必须配置秘钥认证策略。

```Shell
# connect to hosts and execute hostname command
[JerryDai@lab ~]$ pssh -H "10.0.0.51 10.0.0.52" -i "hostname" 
[1] 17:28:02 [SUCCESS] 10.0.0.51
node01.centos.org
[2] 17:28:02 [SUCCESS] 10.0.0.52
node02.centos.org

# it's possible to read host list fron a file
[JerryDai@lab ~]$ vi pssh_hosts.txt
# write hosts per line like follows
JerryDai@10.0.0.51
JerryDai@10.0.0.52

[JerryDai@lab ~]$ pssh -h pssh_hosts.txt -i "uptime" 
[1] 19:37:59 [SUCCESS] JerryDai@10.0.0.52
 19:37:59 up  1:35,  0 users,  load average: 0.00, 0.00, 0.00
[2] 19:37:59 [SUCCESS] JerryDai@10.0.0.51
 19:37:59 up  1:35,  0 users,  load average: 0.00, 0.00, 0.00
```

