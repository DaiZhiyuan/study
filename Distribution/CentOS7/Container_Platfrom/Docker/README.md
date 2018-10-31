- 1 Docker
    - 1.1 安装Docker
    - 1.2 下载使用官网Centos镜像
    - 1.3 交互式方式连接容器
    - 1.4 宿主机与容器之间切换
- 2 制作一个基于centos_httpd镜像
- 3 向宿主机暴露容器端口
- 4 Dockerfile
    - 4.1 基于Dockerfile创建web_server镜像
- 5 私有镜像仓库
    - 5.1 构建私有镜像仓库
    - 5.2 更改Docker服务配置
    - 5.3 push或pull镜像到私有仓库
    - 5.4 配置SSL
    - 5.5 密码认证
- 6 使用持久存储
    - 6.1 使用一个容器实现持久存储
    - 6.2 使用目录挂载实现持久存储
- 7 使用Docker Compose
    - 7.1 安装
    - 7.2 创建一个httpd容器和一个MariaDB容器
    - 7.3 其他常用命令
- 8 使用Docker Swarm
    - 8.1 在所有节点上安装Docker并确认版本大于1.12
    - 8.2 在所有节点上关闭live-restore，集群模式不支持
    - 8.3 在管理节点上配置
    - 8.4 在其他工作节点上配置
    - 8.5 确认集群状态
    - 8.6 在管理节点上配置服务
    - 8.7 改变服务的副本数

# 1. Docker

## 1.1 安装Docker

```Shell
[root@lab ~]# yum -y install docker

[root@lab ~]# systemctl start docker 
[root@lab ~]# systemctl enable docker
```

## 1.2 下载使用官网Centos镜像

```Shell
# download a centos image
[root@lab ~]# docker pull centos 
Trying to pull repository docker.io/library/centos ...
latest: Pulling from library/centos
47d44cb6f252: Extracting     32 B/32 B
...
...
# run echo inside Container
[root@lab ~]# docker run centos /bin/echo "Welcome to the Docker World" 
Welcome to the Docker World
```

## 1.3 交互式方式连接容器

```Shell
[root@lab ~]# docker run -i -t centos /bin/bash 
[root@06c8cbea8dc3 /]#     # Container's console

bash-4.3# uname -a 
Linux 06c8cbea8dc3 3.10.0-123.13.2.el7.x86_64 #1 SMP Thu Dec 18 14:09:13 UTC 2014 x86_64 x86_64 x86_64 GNU/Linux

[root@06c8cbea8dc3 /]# exit 
exit

[root@lab ~]#     # back
```

## 1.4 宿主机与容器之间切换

```Shell
[root@lab ~]# docker run -i -t centos /bin/bash 
[root@64241ed538ed /]# [root@lab ~]#     # Ctrl+p, Ctrl+q key to back to Host's console

[root@lab ~]# docker ps     # show docker process
CONTAINER ID   IMAGE      COMMAND       CREATED         STATUS         PORTS  NAMES
64241ed538ed   centos:7   "/bin/bash"   35 seconds ago  Up 34 seconds         clever_bartik

# connect to container's session
[root@lab ~]# docker attach 64241ed538ed 
[root@64241ed538ed /]#     # connected

# shutdown container's process from Host's console
[root@lab ~]# docker kill 64241ed538ed 
[root@lab ~]# docker ps 
CONTAINER ID    IMAGE          COMMAND       CREATED          STATUS          PORTS     NAMES
```

# 2. 制作一个基于centos_httpd镜像

```Shell
# show images
[root@lab ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             VIRTUAL SIZE
centos              7                   8efe422e6104        4 days ago          224 MB
centos              centos7             8efe422e6104        4 days ago          224 MB
centos              latest              8efe422e6104        4 days ago          224 MB

# start a Container and install httpd
[root@lab ~]# docker run centos /bin/bash -c "yum -y update; yum -y install httpd" 
[root@lab ~]# docker ps -a | head -2 
CONTAINER ID   IMAGE      COMMAND                CREATED         STATUS                     PORTS  NAMES
a0294a053f8c   centos:7   "/bin/bash -c 'yum -   37 seconds ago  Exited (0) 19 seconds ago         suspicious_morse

# add the Image
[root@dlp ~]# docker commit a0294a053f8c my_image/centos_httpd 
d0938f54bfd62c2a108249c1f969aaeb80be51fbbaee15b594004d4875327609

# show images
[root@lab ~]# docker images 
REPOSITORY              TAG                 IMAGE ID            CREATED             VIRTUAL SIZE
my_image/centos_httpd   latest              d0938f54bfd6        17 seconds ago      338.3 MB
centos                  7                   8efe422e6104        4 days ago          224 MB
centos                  centos7             8efe422e6104        4 days ago          224 MB
centos                  latest              8efe422e6104        4 days ago          224 MB

# Generate a Container from the new image and execute which command to make sure httpd exists
[root@lab ~]# docker run my_image/centos_httpd /usr/bin/which httpd 
/usr/sbin/httpd
```

# 3. 向宿主机暴露容器端口

```Shell
# start the Container and connect to the shell session
# map the port of Host and the port of Container with "-p xxx:xxx"
[root@dlp ~]# docker run -it -p 8081:80 my_image/centos_httpd /bin/bash

# start httpd and exit the Container
[root@821bc61cb2e6 /]# /usr/sbin/httpd & 
[root@821bc61cb2e6 /]# echo "httpd on Docker Container" > /var/www/html/index.html 
[root@821bc61cb2e6 /]# # exit with Ctrl+p, Ctrl+q

[root@dlp ~]# docker ps 
CONTAINER ID  IMAGE                        COMMAND      CREATED        STATUS        PORTS                  
821bc61cb2e6  my_image/centos_httpd:latest "/bin/bash"  54 seconds ago Up 53 seconds 0.0.0.0:8081->80/tcp
```

# 4. Dockerfile

INSTRUCTION | Description
---|---
FROM | It sets the Base Image for subsequent instructions.
MAINTAINER	| It sets the Author field of the generated images.
RUN	| It will execute any commands when Docker image will be created.
CMD	| It will execute any commands when Docker container will be executed.
ENTRYPOINT | It will execute any commands when Docker container will be executed.
LABEL | It adds metadata to an image.
EXPOSE | It informs Docker that the container will listen on the specified network ports at runtime.
ENV | It sets the environment variable.
ADD | It copies new files, directories or remote file URLs.
COPY | It copies new files or directories.The differences of [ADD] are that it's impossible to specify remore URL and also it will not extract archive files automatically.
VOLUME | It creates a mount point with the specified name and marks it as holding externally mounted volumes from native host or other containers.
USER | It sets the user name or UID.
WORKDIR | It sets the working directory.

## 4.1 基于Dockerfile创建web_server镜像

```Shell
[root@lab ~]# vi Dockerfile

# create new
FROM centos
MAINTAINER JerryDai <zhiyuan.dai@qq.com>
RUN yum -y install httpd
RUN echo "Hello DockerFile" > /var/www/html/index.html
EXPOSE 80
CMD ["-D", "FOREGROUND"]
ENTRYPOINT ["/usr/sbin/httpd"]

# build image ⇒ docker build -t [image name]:[tag] . 
[root@lab ~]# docker build -t web_server:latest . 
Sending build context to Docker daemon 10.24 kB
Step 0 : FROM centos
 ---> 7322fbe74aa5
Step 1 : MAINTAINER serverworld <zhiyuan.dai@qq.com>
 ---> Running in fa5364b3d41f
 ---> 57d8fd36b7f7
.....
.....
Removing intermediate container 3efa8e1dcae9
Successfully built 7c39aaa338b4

[root@lab ~]# docker images 
REPOSITORY          TAG         IMAGE ID            CREATED             VIRTUAL SIZE
web_server          latest      7c39aaa338b4        24 seconds ago      283.9 MB
docker.io/centos    latest      ce20c473cd8a        8 weeks ago         172.3 MB

# run Container on background
[root@lab ~]# docker run -d -p 80:80 web_server 
[root@lab ~]# docker ps 
CONTAINER ID  IMAGE       COMMAND                 CREATED         STATUS         PORTS                  NAMES
eda2b1482272  web_server  "/usr/sbin/httpd -D F"  35 seconds ago  Up 34 seconds  0.0.0.0:80->80/tcp  mad_bhabha

[root@node02 ~]# curl http://localhost/ 
Hello DockerFile
```

# 5. 私有镜像仓库

## 5.1 构建私有镜像仓库
```Shell
[root@lab ~]# docker run -d -p 5000:5000 -v /var/lib/registry:/var/lib/registry --restart=always --name registry registry:2 
Trying to pull repository docker.io/library/registry ...
2: Pulling from docker.io/library/registry
81033e7c1d6a: Pull complete
Status: Downloaded newer image for docker.io/registry:2
5188ec6904756070138edfc49d4c0f231841bfbdd620556a85eb44b201b37b31

[root@dlp ~]# docker ps 
CONTAINER ID  IMAGE      COMMAND                 CREATED     STATUS        PORTS                  NAMES
5188ec690475  registry:2 "/entrypoint.sh /e..."  16 sec ago Up 14 seconds  0.0.0.0:5000->5000/tcp registry
```

## 5.2 更改Docker服务配置

```Shell
[root@node01 ~]# vi /etc/sysconfig/docker

# line 4: add Docker-Registry's URL
OPTIONS='--insecure-registry lab.centos.org:5000 --selinux-enabled --log-driver=journald.....'

[root@node01 ~]# systemctl restart docker
```

## 5.3 push或pull镜像到私有仓库

```Shell
[root@node01 ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
docker.io/centos    latest              e934aafc2206        9 days ago          199 MB

# set tag and push
[root@node01 ~]# docker tag centos lab.centos.org:5000/centos 
[root@node01 ~]# docker push lab.centos.org:5000/centos 
The push refers to a repository [lab.centos.org:5000/centos]
43e653f84b79: Pushed
latest: digest: sha256:191c883e479a7da2362b2d54c0840b2e8981e5ab62e11ab925abf8808d3d5d44 size: 529

[root@node01 ~]# docker images 
REPOSITORY                  TAG                 IMAGE ID            CREATED             SIZE
docker.io/centos            latest              e934aafc2206        9 days ago          199 MB
lab.centos.org:5000/centos  latest              e934aafc2206        9 days ago          199 MB

# delete all images once to verify Pull
[root@node01 ~]# docker rmi docker.io/centos 
[root@node01 ~]# docker rmi lab.centos.org:5000/centos 
[root@node01 ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE

[root@node01 ~]# docker pull lab.centos.org:5000/centos 
Using default tag: latest
Trying to pull repository lab.centos.org:5000/centos ...
latest: Pulling from lab.centos.org:5000/centos
469cfcc7a4b3: Pull complete
Digest: sha256:191c883e479a7da2362b2d54c0840b2e8981e5ab62e11ab925abf8808d3d5d44
Status: Downloaded newer image for lab.centos.org:5000/centos:latest

[root@node01 ~]# docker images 
REPOSITORY                  TAG                 IMAGE ID            CREATED             SIZE
lab.centos.org:5000/centos  latest              e934aafc2206        9 days ago          199 MB
```

## 5.4 配置SSL

**1. 创建数字证书**

使用Let's Encrypt提供的90天免费数字证书，官网为letsencrypt.org。

```Shell
# install from EPEL
[root@www ~]# yum --enablerepo=epel -y install certbot

# for the option [--webroot], use a directory under the webroot on your server as a working temp
# -w [document root] -d [FQDN you'd like to get certs]
# FQDN (Fully Qualified Domain Name) : Hostname.Domainname
# if you'd like to get certs for more than 2 FQDNs, specify all like below
# ex : if get [centos.org] and [www.centos.org]
# => specify [-d centos.org -d www.centos.org]

[root@www ~]# certbot certonly --webroot -w /var/www/html -d www.centos.org 
Saving debug log to /var/log/letsencrypt/letsencrypt.log
Plugins selected: Authenticator webroot, Installer None
Enter email address (used for urgent renewal and security notices) 
# for only initial using, register your email address and agree to terms of use
# specify valid email address
(Enter 'c' to cancel): root@mail.centos.org 
Starting new HTTPS connection (1): acme-v01.api.letsencrypt.org

-------------------------------------------------------------------------------
Please read the Terms of Service at
https://letsencrypt.org/documents/LE-SA-v1.2-November-15-2017.pdf. You must
agree in order to register with the ACME server at
https://acme-v01.api.letsencrypt.org/directory
-------------------------------------------------------------------------------
# agree to the terms of use
(A)gree/(C)ancel: A

-------------------------------------------------------------------------------
Would you be willing to share your email address with the Electronic Frontier
Foundation, a founding partner of the Let's Encrypt project and the non-profit
organization that develops Certbot? We'd like to send you email about EFF and
our work to encrypt the web, protect its users and defend digital rights.
-------------------------------------------------------------------------------
# answer Yes or No
(Y)es/(N)o: Y
Starting new HTTPS connection (1): supporters.eff.org
Obtaining a new certificate
Performing the following challenges:
http-01 challenge for www.centos.org
Using the webroot path /var/www/html for all unmatched domains.
Waiting for verification...
Cleaning up challenges

IMPORTANT NOTES:
 - Congratulations! Your certificate and chain have been saved at:
   /etc/letsencrypt/live/www.centos.org/fullchain.pem
   Your key file has been saved at:
   /etc/letsencrypt/live/www.centos.org/privkey.pem
   Your cert will expire on 2018-05-22. To obtain a new or tweaked
   version of this certificate in the future, simply run certbot
   again. To non-interactively renew *all* of your certificates, run
   "certbot renew"
 - Your account credentials have been saved in your Certbot
   configuration directory at /etc/letsencrypt. You should make a
   secure backup of this folder now. This configuration directory will
   also contain certificates and private keys obtained by Certbot so
   making regular backups of this folder is ideal.
 - If you like Certbot, please consider supporting our work by:

   Donating to ISRG / Let's Encrypt:   https://letsencrypt.org/donate
   Donating to EFF:                    https://eff.org/donate-le

# success if [Congratulations] is shown
# certs are created under the [/etc/letsencrypt/live/(FQDN)/] directory

# cert.pem       ⇒ SSL Server cert(includes public-key)
# chain.pem      ⇒ intermediate certificate
# fullchain.pem  ⇒ combined file cert.pem and chain.pem
# privkey.pem    ⇒ private-key file
```

**2. 重新启动registry容器**

```Shell
[root@lab ~]# cp /etc/letsencrypt/live/lab.centos.org/fullchain.pem /etc/docker/certs.d/server.crt 
[root@lab ~]# cp /etc/letsencrypt/live/lab.centos.org/privkey.pem /etc/docker/certs.d/server.key 

[root@lab ~]# docker run -d -p 5000:5000 --restart=always --name registry \
-v /var/lib/registry:/var/lib/registry \
-v /etc/docker/certs.d:/certs \
-e REGISTRY_HTTP_TLS_CERTIFICATE=/certs/server.crt \
-e REGISTRY_HTTP_TLS_KEY=/certs/server.key \
registry:2 
 2: Pulling from docker.io/library/registry
81033e7c1d6a: Pull complete
6ff332201ab29c521fa70e54187d7677f9df1803550f8d61bcaff88f8c602e3b

[root@lab ~]# docker ps 
CONTAINER ID  IMAGE      COMMAND                 CREATED    STATUS      PORTS                  NAMES
6ff332201ab2  registry:2 "/entrypoint.sh /e..."  21 sec ago 18 seconds  0.0.0.0:5000->5000/tcp registry
```

**3. push/pull测试**

```Shell
[root@node01 ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
docker.io/centos    latest              e934aafc2206        9 days ago          199 MB

# set tag and push
[root@node01 ~]# docker tag centos lab.centos.org:5000/centos 
[root@node01 ~]# docker push lab.centos.org:5000/centos 
The push refers to a repository [lab.centos.org:5000/centos]
43e653f84b79: Layer already exists
latest: digest: sha256:191c883e479a7da2362b2d54c0840b2e8981e5ab62e11ab925abf8808d3d5d44 size: 529

[root@node01 ~]# docker images 
REPOSITORY                  TAG                 IMAGE ID            CREATED             SIZE
lab.centos.org:5000/centos  latest              e934aafc2206        9 days ago          199 MB
docker.io/centos            latest              e934aafc2206        9 days ago          199 MB

# delete all images once to verify Pull
[root@node01 ~]# docker rmi docker.io/centos 
[root@node01 ~]# docker rmi lab.centos.org:5000/centos 
[root@node01 ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE

[root@node01 ~]# docker pull lab.centos.org:5000/centos 
Using default tag: latest
Trying to pull repository lab.centos.org:5000/centos ...
latest: Pulling from lab.centos.org:5000/centos
469cfcc7a4b3: Pull complete
Digest: sha256:191c883e479a7da2362b2d54c0840b2e8981e5ab62e11ab925abf8808d3d5d44
Status: Downloaded newer image for lab.centos.org:5000/centos:latest
[root@node01 ~]# docker images 
REPOSITORY                  TAG                 IMAGE ID            CREATED             SIZE
lab.centos.org:5000/centos   latest              e934aafc2206        9 days ago          199 MB
```

## 5.5 密码认证

**1. 安装软件包**

```Shell
[root@lab ~]# yum -y install httpd-tools
```

**2. 添加新用户**

```Shell
[root@lab ~]# htpasswd -Bc /etc/docker/.htpasswd admin 
New password:
Re-type new password:
Adding password for user admin

```

**3. 重新启动registry容器**

```Shell
[root@lab ~]# cp /etc/letsencrypt/live/lab.centos.org/fullchain.pem /etc/docker/certs.d/server.crt 
[root@lab ~]# cp /etc/letsencrypt/live/lab.centos.org/privkey.pem /etc/docker/certs.d/server.key

[root@lab ~]# docker run -d -p 5000:5000 --restart=always --name registry \
-v /var/lib/registry:/var/lib/registry \
-v /etc/docker/certs.d:/certs \
-v /etc/docker:/auth \
-e REGISTRY_HTTP_TLS_CERTIFICATE=/certs/server.crt \
-e REGISTRY_HTTP_TLS_KEY=/certs/server.key \
-e REGISTRY_AUTH=htpasswd \
-e REGISTRY_AUTH_HTPASSWD_PATH=/auth/.htpasswd \
-e REGISTRY_AUTH_HTPASSWD_REALM="Registry Realm" \
registry:2 

Trying to pull repository docker.io/library/registry ...
2: Pulling from docker.io/library/registry
Digest: sha256:672d519d7fd7bbc7a448d17956ebeefe225d5eb27509d8dc5ce67ecb4a0bce54
Status: Downloaded newer image for docker.io/registry:2

[root@lab ~]# docker ps 
CONTAINER ID IMAGE      COMMAND                CREATED  STATUS     PORTS                  NAMES
db00df47085f registry:2 "/entrypoint.sh /e..." 31 sec   Up 29 sec  0.0.0.0:5000->5000/tcp registry
```

**4. 登陆验证**

```Shell
# login with a user you added
[root@node01 ~]# docker login dlp.srv.world:5000 
Username: admin
Password:
Login Succeeded

[root@node01 ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
docker.io/nginx     latest              b175e7467d66        6 days ago          109 MB

[root@node01 ~]# docker tag nginx lab.centos.org:5000/nginx 
[root@node01 ~]# docker push lab.centos.org:5000/nginx 
The push refers to a repository [lab.centos.org:5000/nginx]
77e23640b533: Pushed
757d7bb101da: Pushed
3358360aedad: Pushed
latest: digest: sha256:d903fe3076f89ad76afe1cbd0e476d9692d79b3835895b5b3541654c85422bf1 size: 948

[root@node01 ~]# docker images 
REPOSITORY                 TAG                 IMAGE ID            CREATED             SIZE
docker.io/nginx            latest              b175e7467d66        6 days ago          109 MB
lab.centos.org:5000/nginx  latest              b175e7467d66        6 days ago          109 MB
```

# 6. 使用持久存储

因为当容器删除时，数据会被丢弃，所以容器需要外部文件系统来提供持久存储。

## 6.1 使用一个容器实现持久存储

**1. 创建一个仅用来保存数据的存储服务容器**

```Shell
[root@lab ~]# vi Dockerfile

# create new
FROM busybox
MAINTAINER JerryDai <zhiyuan.dai@qq.com>

VOLUME /storage
CMD /bin/sh

# build image
[root@lab ~]# docker build -t storage .
[root@lab ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             VIRTUAL SIZE
storage             latest              65c5cce81114        20 seconds ago      1.113 MB
docker.io/centos    latest              14dab3d40372        6 days ago          194.7 MB
docker.io/busybox   latest              fc0db02f3072        13 days ago         1.113 MB

# generate a Container with any name you like
[root@lab ~]# docker run -i -t --name storage_server storage 
/ # exit
```

**2. 在其他容器中使用存储容器**

```Shell
[root@lab ~]# docker run -i -t --name centos_server --volumes-from storage_server centos /bin/bash
[root@b9b7a2d35b51 /]# df -hT 
Filesystem                              Type   Size  Used Avail Use% Mounted on
/dev/mapper/docker-253:0-67164897-..... ext4    99G  266M   94G   1% /
tmpfs                                   tmpfs  2.0G     0  2.0G   0% /dev
shm                                     tmpfs   64M     0   64M   0% /dev/shm
tmpfs                                   tmpfs  2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/mapper/centos-root                 xfs     27G  3.2G   24G  13% /storage
tmpfs                                   tmpfs  2.0G     0  2.0G   0% /run/secrets

[root@b9b7a2d35b51 /]# echo "persistent storage" >> /storage/testfile.txt 
[root@b9b7a2d35b51 /]# ll /storage 
total 4
-rw-r--r-- 1 root root 19 Dec 22 02:15 testfile.txt
```

**3. 确认数据是否保存到存储容器**

```Shell
[root@lab ~]# docker start storage_server 
[root@lab ~]# docker attach storage_server 

/ # cat /storage/testfile.txt 
persistent storage
```

## 6.2 使用目录挂载实现持久存储

```Shell

# create a directory
[root@www ~]# mkdir -p /var/docker/disk01 
[root@www ~]# echo "persistent storage" >> /var/docker/disk01/testfile.txt

# run a Container with mounting the directory above on /mnt
[root@www ~]# docker run -i -t -v /var/docker/disk01:/mnt centos /bin/bash
[root@bc9a4d5578a6 /]# df -hT 
Filesystem                              Type   Size  Used Avail Use% Mounted on
/dev/mapper/docker-253:0-67164897-..... ext4    99G  266M   94G   1% /
tmpfs                                   tmpfs  2.0G     0  2.0G   0% /dev
shm                                     tmpfs   64M     0   64M   0% /dev/shm
tmpfs                                   tmpfs  2.0G     0  2.0G   0% /sys/fs/cgroup
/dev/mapper/centos-root                 xfs     27G  3.2G   24G  13% /mnt
tmpfs                                   tmpfs  2.0G     0  2.0G   0% /run/secrets

[root@bc9a4d5578a6 /]# cat /mnt/testfile.txt 
persistent storage
```

# 7. 使用Docker Compose

## 7.1 安装

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install python2-pip

[root@lab ~]# pip install docker-compose
```

## 7.2 创建一个httpd容器和一个MariaDB容器

```Shell
# create Dockerfile with httpd
[root@lab ~]# vi Dockerfile

FROM centos
MAINTAINER JerryDai <zhiyuan.dai@qq.com>

RUN yum -y update
RUN yum -y install httpd

EXPOSE 80
CMD ["/usr/sbin/apachectl", "-D", "FOREGROUND"]

# define application configuration
[root@lab ~]# vi docker-compose.yml

version: '3'
services:
  db:
    image: mariadb
    volumes:
      - /var/lib/docker/disk01:/var/lib/mysql
    environment:
      MYSQL_ROOT_PASSWORD: password
      MYSQL_USER: centos
      MYSQL_PASSWORD: password
      MYSQL_DATABASE: centos_db
    ports:
      - "3306:3306"
  web:
    build: .
    ports:
      - "80:80"
    volumes:
      - /var/lib/docker/disk02:/var/www/html

# if SELinux is Enforcing, change to permissive or disabled
[root@lab ~]# setenforce 0

# build and run application
[root@lab ~]# docker-compose up -d 
Building web
Step 1/6 : FROM centos
Trying to pull repository docker.io/library/centos ...
latest: Pulling from docker.io/library/centos
256b176beaff: Download complete
.....
.....
Creating root_web_1 ...
Creating root_db_1  ...
Creating root_web_1 ... done
Creating root_db_1  ... done

[root@lab ~]# docker ps 
CONTAINER ID   IMAGE        COMMAND                  CREATED          STATUS         PORTS                    NAMES
06ece319c627   mariadb      "docker-entrypoint..."   20 seconds ago   Up 18 seconds  0.0.0.0:3306->3306/tcp   root_db_1
1f3796c137f3   root_web     "/usr/sbin/apachec..."   20 seconds ago   Up 18 seconds  0.0.0.0:80->80/tcp       root_web_1

# confirm possible to access or not
[root@lab ~]# mysql -h 127.0.0.1 -u root -p -e "show variables like 'hostname';" 
Enter password:
+---------------+--------------+
| Variable_name | Value        |
+---------------+--------------+
| hostname      | 06ece319c627 |
+---------------+--------------+

[root@lab ~]# mysql -h 127.0.0.1 -u centos -p -e "show databases;" 
Enter password:
+--------------------+
| Database           |
+--------------------+
| centos_db          |
| information_schema |
+--------------------+

[root@lab ~]# echo "Hello Docker Compose World" > /var/lib/docker/disk02/index.html 
[root@lab ~]# curl localhost 
Hello Docker Compose World
```

## 7.3 其他常用命令

```Shell
# show status of application
[root@lab ~]# docker-compose ps 
   Name                 Command               State           Ports
----------------------------------------------------------------------------
root_db_1    docker-entrypoint.sh mysqld      Up      0.0.0.0:3306->3306/tcp
root_web_1   /usr/sbin/apachectl -D FOR ...   Up      0.0.0.0:80->80/tcp

# show logs of application
[root@lab ~]# docker-compose logs 
Attaching to root_db_1, root_web_1
web_1  | Passing arguments to httpd using apachectl is no longer supported.
web_1  | You can only start/stop/restart httpd using this script.
.....
.....
db_1   | 2018-08-28  2:35:54 0 [Note] Added new Master_info '' to hash table
db_1   | 2018-08-28  2:35:54 0 [Note] mysqld: ready for connections.
db_1   | Version: '10.3.9-MariaDB-1:10.3.9+maria~bionic'  socket: '/var/run/mysqld/mysqld.sock'  port: 3306  mariadb.org binary distribution

# run commands inside a container
# for container name, it's just the name set in [docker-compose.yml]
[root@lab ~]# docker-compose exec db /bin/bash 
root@06ece319c627:/#

# stop allications
[root@lab ~]# docker-compose stop 
Stopping root_web_1 ... done
Stopping root_db_1  ... done

# run an application particularly
# if it has dependency, another container also runs
[root@lab ~]# docker-compose up -d web 
Stopping root_db_1  ...
Stopping root_web_1 ...
Stopping root_db_1  ... done
Stopping root_web_1 ... done

[root@lab ~]# docker-compose ps 
   Name                 Command                State     Ports
--------------------------------------------------------------
root_db_1    docker-entrypoint.sh mysqld      Exit 0
root_web_1   /usr/sbin/apachectl -D FOR ...   Exit 137

# remove applications
# running container won't be removed
[root@lab ~]# docker-compose rm 
Going to remove root_db_1, root_web_1
Are you sure? [yN] y
Removing root_db_1 ... done
Removing root_web_1 ... done
```

# 8. 使用Docker Swarm

实验环境：

```
 -----------+---------------------------+--------------------------+------------
            |                           |                          |
        eth0|10.0.0.51              eth0|10.0.0.52             eth0|10.0.0.53
 +----------+-----------+   +-----------+----------+   +-----------+----------+
 | [ node01.centos.org ]|   | [ node02.centos.org ]|   | [ node03.centos.org ]|
 |       Manager        |   |        Worker        |   |        Worker        |
 +----------------------+   +----------------------+   +----------------------+
```

## 8.1 在所有节点上安装Docker并确认版本大于1.12

```Shell
[root@node01 ~]# docker -v 
Docker version 1.12.6, build 96d83a5/1.12.6
```

## 8.2 在所有节点上关闭live-restore，集群模式不支持

```Shell
[root@node01 ~]# vi /etc/docker/daemon.json
{
    "live-restore": false
}

[root@node01 ~]# systemctl restart docker 
```

## 8.3 在管理节点上配置

```Shell
[root@node01 ~]# docker swarm init 
Swarm initialized: current node (2f7pmmoilf753g7wfctcik7yh) is now a manager.

To add a worker to this swarm, run the following command:

    docker swarm join \
    --token SWMTKN-1-1l6o776w0ktvg6vw4cony5et76gkrjzw4xke0cmas3ftl6ghnm-8ppur7wbl9bej6fjcrdtaufsg \
    10.0.0.51:2377

To add a manager to this swarm, run 'docker swarm join-token manager' and follow the instructions.
```

## 8.4 在其他工作节点上配置

```Shell
[root@node02 ~]# docker swarm join \
--token SWMTKN-1-1l6o776w0ktvg6vw4cony5et76gkrjzw4xke0cmas3ftl6ghnm-8ppur7wbl9bej6fjcrdtaufsg \
10.0.0.51:2377 
This node joined a swarm as a worker.
```

## 8.5 确认集群状态

```Shell
[root@node01 ~]# docker node ls 
ID                           HOSTNAME          STATUS  AVAILABILITY  MANAGER STATUS
2f7pmmoilf753g7wfctcik7yh *  node01.centos.org  Ready   Active        Leader
6ok99v3dkt9mg4jvzg7qi4zqb    node02.centos.org  Ready   Active
a3dtuaqibnrym3u3wg32cuq7p    node03.centos.org  Ready   Active
```

## 8.6 在管理节点上配置服务

```Shell
[root@node01 ~]# docker images 
REPOSITORY          TAG                 IMAGE ID            CREATED             SIZE
web_server          latest              b7cf7440922c        4 minutes ago       306.4 MB
docker.io/centos    latest              98d35105a391        3 weeks ago         192.5 MB

# create a service with 2 repricas
[root@node01 ~]# docker service create --name swarm_cluster --replicas=2 -p 80:80 web_server:latest 
7xg4yssy516xwgkjx2vxbw05d
# show service list
[root@node01 ~]# docker service ls 
ID            NAME           REPLICAS  IMAGE              COMMAND
7xg4yssy516x  swarm_cluster  2/2       web_server:latest

# inspect the service
[root@node01 ~]# docker service inspect swarm_cluster --pretty 
ID:             7xg4yssy516xwgkjx2vxbw05d
Name:           swarm_cluster
Mode:           Replicated
 Replicas:      2
Placement:
UpdateConfig:
 Parallelism:   1
 On failure:    pause
ContainerSpec:
 Image:         web_server:latest
Resources:
Ports:
 Protocol = tcp
 TargetPort = 80
 PublishedPort = 80

# show service status
[root@node01 ~]# docker service ps swarm_cluster 
ID              NAME             IMAGE              NODE              DESIRED STATE  CURRENT STATE               ERROR
78lh8sn1myw...  swarm_cluster.1  web_server:latest  node01.centos.org  Running        Running about a minute ago
4ag74wr59rb...  swarm_cluster.2  web_server:latest  node03.centos.org  Running        Running about a minute ago

# verify it works normally
[root@node01 ~]# curl http://node01.centos.org/ 
Docker Node : node03.centos.org

[root@node01 ~]# curl http://node01.centos.org/ 
Docker Node : node01.centos.org

[root@node01 ~]# curl http://node01.centos.org/ 
Docker Node : node03.centos.org

[root@node01 ~]# curl http://node01.centos.org/ 
Docker Node : node01.centos.org
```

## 8.7 改变服务的副本数

```Shell

# change repricas to 3
[root@node01 ~]# docker service scale swarm_cluster=3 
swarm_cluster scaled to 3

[root@node01 ~]# docker service ps swarm_cluster 
ID              NAME             IMAGE              NODE              DESIRED STATE  CURRENT STATE           ERROR
78lh8sn1myw...  swarm_cluster.1  web_server:latest  node01.centos.org  Running        Running 34 minutes ago
4ag74wr59rb...  swarm_cluster.2  web_server:latest  node03.centos.org  Running        Running 34 minutes ago
46ecs1c0tjv...  swarm_cluster.3  web_server:latest  node02.centos.org  Running        Running 13 seconds ago

# verify working
[root@node01 ~]# curl http://node01.centos.org/ 
Docker Node : node01.centos.org

[root@node01 ~]# curl http://node01.centos.org/ 
Docker Node : node02.centos.org

[root@node01 ~]# curl http://node01.centos.org/ 
Docker Node : node03.centos.org
```

