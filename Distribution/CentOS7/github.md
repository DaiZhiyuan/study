# 1. Generating SSH keys

```shell
[root@jerrydai ~]# ssh-keygen -t rsa -b 4096 -C "jerry.zhiyuan.dai@gmail.com"
Generating public/private rsa key pair.
Enter file in which to save the key (/root/.ssh/id_rsa):
Created directory '/root/.ssh'.
Enter passphrase (empty for no passphrase):
Enter same passphrase again:
Your identification has been saved in /root/.ssh/id_rsa.
Your public key has been saved in /root/.ssh/id_rsa.pub.
The key fingerprint is:
SHA256:aUHwbpE7EHCTN2lm5f21n78eJsXdzbl7rylzICgGAIE jerry.zhiyuan.dai@gmail.com
The key's randomart image is:
+---[RSA 4096]----+
|+o  ..=o.o.      |
|E .  ..=Bo .     |
|   .  .==.. .   .|
|    .  o =   ..o*|
|     .  S.    .==|
|      oo... . . +|
|     . .   . o =.|
|            o +.=|
|             ++=*|
+----[SHA256]-----+
```

# 2. view public key

```shell
[root@jerrydai ~]# cat ~/.ssh/id_rsa.pub
ssh-rsa X3U/29EBleQSEnou28hXe4MpZCyaSls1rbCRqQ4Pw== jerry.zhiyuan.dai@gmail.com
```

# 3. check connection github.com

```shell
[root@jerrydai ~]# ssh -T git@github.com
The authenticity of host 'github.com (13.229.188.59)' can't be established.
RSA key fingerprint is SHA256:nThbg6kXUpJWGl7E1IGOCspRomTxdCARLviKw6E5SY8.
RSA key fingerprint is MD5:16:27:ac:a5:76:28:2d:36:63:1b:56:4d:eb:df:a6:48.
Are you sure you want to continue connecting (yes/no)? yes
Warning: Permanently added 'github.com,13.229.188.59' (RSA) to the list of known hosts.
Hi DaiZhiyuan! You've successfully authenticated, but GitHub does not provide shell access.
```
