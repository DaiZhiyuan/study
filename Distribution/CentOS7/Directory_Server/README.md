- 1 配置LDAP服务
    - 1.1 安装OpenLDAP
    - 1.2 设置OpenLDAP管理员密码
    - 1.3 导入基本Schemas
    - 1.4 在LDAP数据库设置你的域名
    - 1.5 添加防火墙策略
- 2 添加LDAP用户
    - 2.1 添加一个JerryDai用户
    - 2.2 将本地passwd/group记录的用户和组添加到LDAP
    - 2.3 在LDAP中删除用户和组
- 3 配置LDAP客户端
    - 3.1 启用LDAP认证
    - 3.2 变更SELinux策略
- 4 基于SSL/TLS的LDAP
    - 4.1 创建SSL数字证书
    - 4.2 配置LDAP服务
    - 4.3 配置客户端采用TLS
- 5 安装phpLDAPadmin

# 1. 配置LDAP服务

## 1.1 安装OpenLDAP

```Shell
[root@lab ~]# yum -y install openldap-servers openldap-clients
[root@lab ~]# cp /usr/share/openldap-servers/DB_CONFIG.example /var/lib/ldap/DB_CONFIG 
[root@lab ~]# chown ldap. /var/lib/ldap/DB_CONFIG 
[root@lab ~]# systemctl start slapd 
[root@lab ~]# systemctl enable slapd 
```

## 1.2 设置OpenLDAP管理员密码

```Shell
# generate encrypted password
[root@lab ~]# slappasswd 
New password:
Re-enter new password:
{SSHA}xxxxxxxxxxxxxxxxxxxxxxxx

[root@lab ~]# vi chrootpw.ldif
# specify the password generated above for "olcRootPW" section
 dn: olcDatabase={0}config,cn=config
changetype: modify
add: olcRootPW
olcRootPW: {SSHA}xxxxxxxxxxxxxxxxxxxxxxxx

[root@lab ~]# ldapadd -Y EXTERNAL -H ldapi:/// -f chrootpw.ldif 
SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
modifying entry "olcDatabase={0}config,cn=config"
```

## 1.3 	导入基本Schemas

```Shell
[root@lab ~]# ldapadd -Y EXTERNAL -H ldapi:/// -f /etc/openldap/schema/cosine.ldif 
SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
adding new entry "cn=cosine,cn=schema,cn=config"

[root@lab ~]# ldapadd -Y EXTERNAL -H ldapi:/// -f /etc/openldap/schema/nis.ldif 
SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
adding new entry "cn=nis,cn=schema,cn=config"

[root@lab ~]# ldapadd -Y EXTERNAL -H ldapi:/// -f /etc/openldap/schema/inetorgperson.ldif 
SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
adding new entry "cn=inetorgperson,cn=schema,cn=config"
```

## 1.4 在LDAP数据库设置你的域名

```Shell
# generate directory manager's password
[root@lab ~]# slappasswd 
New password:
Re-enter new password:
{SSHA}xxxxxxxxxxxxxxxxxxxxxxxx

[root@lab ~]# vi chdomain.ldif
# replace to your own domain name for "dc=***,dc=***" section
# specify the password generated above for "olcRootPW" section
 dn: olcDatabase={1}monitor,cn=config
changetype: modify
replace: olcAccess
olcAccess: {0}to * by dn.base="gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth"
  read by dn.base="cn=Manager,dc=centos,dc=org" read by * none

dn: olcDatabase={2}hdb,cn=config
changetype: modify
replace: olcSuffix
olcSuffix: dc=centos,dc=org

dn: olcDatabase={2}hdb,cn=config
changetype: modify
replace: olcRootDN
olcRootDN: cn=Manager,dc=centos,dc=org

dn: olcDatabase={2}hdb,cn=config
changetype: modify
add: olcRootPW
olcRootPW: {SSHA}xxxxxxxxxxxxxxxxxxxxxxxx

dn: olcDatabase={2}hdb,cn=config
changetype: modify
add: olcAccess
olcAccess: {0}to attrs=userPassword,shadowLastChange by
  dn="cn=Manager,dc=centos,dc=org" write by anonymous auth by self write by * none
olcAccess: {1}to dn.base="" by * read
olcAccess: {2}to * by dn="cn=Manager,dc=centos,dc=org" write by * read

[root@lab ~]# ldapmodify -Y EXTERNAL -H ldapi:/// -f chdomain.ldif 
SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
modifying entry "olcDatabase={1}monitor,cn=config"

modifying entry "olcDatabase={2}hdb,cn=config"

modifying entry "olcDatabase={2}hdb,cn=config"

modifying entry "olcDatabase={2}hdb,cn=config"

[root@lab ~]# vi basedomain.ldif
# replace to your own domain name for "dc=***,dc=***" section
 dn: dc=centos,dc=org
objectClass: top
objectClass: dcObject
objectclass: organization
o: CentOS Org
dc: CentOS

dn: cn=Manager,dc=centos,dc=org
objectClass: organizationalRole
cn: Manager
description: Directory Manager

dn: ou=People,dc=centos,dc=org
objectClass: organizationalUnit
ou: People

dn: ou=Group,dc=centos,dc=org
objectClass: organizationalUnit
ou: Group

[root@lab ~]# ldapadd -x -D cn=Manager,dc=centos,dc=org -W -f basedomain.ldif 
Enter LDAP Password: # directory manager's password
adding new entry "dc=centos,dc=org"

adding new entry "cn=Manager,dc=centos,dc=org"

adding new entry "ou=People,dc=centos,dc=org"

adding new entry "ou=Group,dc=centos,dc=org"
```

## 1.5 添加防火墙策略

```Shell
[root@lab ~]# firewall-cmd --add-service=ldap --permanent 
success
[root@lab ~]# firewall-cmd --reload 
success
```

# 2. 添加LDAP用户

## 2.1 添加一个JerryDai用户

```Shell
# generate encrypted password
[root@lab ~]# slappasswd 
New password:
Re-enter new password:
{SSHA}xxxxxxxxxxxxxxxxx

[root@dlp ~]# vi ldapuser.ldif
# create new
# replace to your own domain name for "dc=***,dc=***" section
 dn: uid=cent,ou=People,dc=centos,dc=org
objectClass: inetOrgPerson
objectClass: posixAccount
objectClass: shadowAccount
cn: JerryDai
sn: Linux
userPassword: {SSHA}xxxxxxxxxxxxxxxxx
loginShell: /bin/bash
uidNumber: 1000
gidNumber: 1000
homeDirectory: /home/JerryDai

dn: cn=JerryDai,ou=Group,dc=centos,dc=org
objectClass: posixGroup
cn: Cent
gidNumber: 1000
memberUid: JerryDai

[root@lab ~]# ldapadd -x -D cn=Manager,dc=centos,dc=org -W -f ldapuser.ldif 
Enter LDAP Password:
adding new entry "uid=JerryDai,ou=People,dc=centos,dc=org"

adding new entry "cn=JerryDai,ou=Group,dc=centos,dc=org"
```

## 2.2 将本地passwd/group记录的用户和组添加到LDAP

```Shell
[root@lab ~]# vi ldapuser.sh

# extract local users and groups who have 1000-9999 digit UID
# replace "SUFFIX=***" to your own domain name
# this is an example
#!/bin/bash

SUFFIX='dc=centos,dc=org'
LDIF='ldapuser.ldif'

echo -n > $LDIF
GROUP_IDS=()
grep "x:[1-9][0-9][0-9][0-9]:" /etc/passwd | (while read TARGET_USER
do
    USER_ID="$(echo "$TARGET_USER" | cut -d':' -f1)"

    USER_NAME="$(echo "$TARGET_USER" | cut -d':' -f5 | cut -d' ' -f1,2)"
    [ ! "$USER_NAME" ] && USER_NAME="$USER_ID"

    LDAP_SN="$(echo "$USER_NAME" | cut -d' ' -f2)"
    [ ! "$LDAP_SN" ] && LDAP_SN="$USER_NAME"

    LASTCHANGE_FLAG="$(grep "${USER_ID}:" /etc/shadow | cut -d':' -f3)"
    [ ! "$LASTCHANGE_FLAG" ] && LASTCHANGE_FLAG="0"

    SHADOW_FLAG="$(grep "${USER_ID}:" /etc/shadow | cut -d':' -f9)"
    [ ! "$SHADOW_FLAG" ] && SHADOW_FLAG="0"

    GROUP_ID="$(echo "$TARGET_USER" | cut -d':' -f4)"
    [ ! "$(echo "${GROUP_IDS[@]}" | grep "$GROUP_ID")" ] && GROUP_IDS=("${GROUP_IDS[@]}" "$GROUP_ID")

    echo "dn: uid=$USER_ID,ou=People,$SUFFIX" >> $LDIF
    echo "objectClass: inetOrgPerson" >> $LDIF
    echo "objectClass: posixAccount" >> $LDIF
    echo "objectClass: shadowAccount" >> $LDIF
    echo "sn: $LDAP_SN" >> $LDIF
    echo "givenName: $(echo "$USER_NAME" | awk '{print $1}')" >> $LDIF
    echo "cn: $USER_NAME" >> $LDIF
    echo "displayName: $USER_NAME" >> $LDIF
    echo "uidNumber: $(echo "$TARGET_USER" | cut -d':' -f3)" >> $LDIF
    echo "gidNumber: $(echo "$TARGET_USER" | cut -d':' -f4)" >> $LDIF
    echo "userPassword: {crypt}$(grep "${USER_ID}:" /etc/shadow | cut -d':' -f2)" >> $LDIF
    echo "gecos: $USER_NAME" >> $LDIF
    echo "loginShell: $(echo "$TARGET_USER" | cut -d':' -f7)" >> $LDIF
    echo "homeDirectory: $(echo "$TARGET_USER" | cut -d':' -f6)" >> $LDIF
    echo "shadowExpire: $(passwd -S "$USER_ID" | awk '{print $7}')" >> $LDIF
    echo "shadowFlag: $SHADOW_FLAG" >> $LDIF
    echo "shadowWarning: $(passwd -S "$USER_ID" | awk '{print $6}')" >> $LDIF
    echo "shadowMin: $(passwd -S "$USER_ID" | awk '{print $4}')" >> $LDIF
    echo "shadowMax: $(passwd -S "$USER_ID" | awk '{print $5}')" >> $LDIF
    echo "shadowLastChange: $LASTCHANGE_FLAG" >> $LDIF
    echo >> $LDIF
done

for TARGET_GROUP_ID in "${GROUP_IDS[@]}"
do
    LDAP_CN="$(grep ":${TARGET_GROUP_ID}:" /etc/group | cut -d':' -f1)"

    echo "dn: cn=$LDAP_CN,ou=Group,$SUFFIX" >> $LDIF
    echo "objectClass: posixGroup" >> $LDIF
    echo "cn: $LDAP_CN" >> $LDIF
    echo "gidNumber: $TARGET_GROUP_ID" >> $LDIF

    for MEMBER_UID in $(grep ":${TARGET_GROUP_ID}:" /etc/passwd | cut -d':' -f1,3)
    do
        UID_NUM=$(echo "$MEMBER_UID" | cut -d':' -f2)
        [ $UID_NUM -ge 1000 -a $UID_NUM -le 9999 ] && echo "memberUid: $(echo "$MEMBER_UID" | cut -d':' -f1)" >> $LDIF
    done
    echo >> $LDIF
done
)

[root@lab ~]# sh ldapuser.sh 
[root@lab ~]# ldapadd -x -D cn=Manager,dc=centos,dc=org -W -f ldapuser.ldif 
Enter LDAP Password:
adding new entry "uid=JerryDai,ou=People,dc=centos,dc=org"
adding new entry "uid=redhat,ou=People,dc=centos,dc=org"
adding new entry "uid=ubuntu,ou=People,dc=centos,dc=org"
adding new entry "uid=debian,ou=People,dc=centos,dc=org"
adding new entry "cn=JerryDai,ou=Group,dc=centos,dc=org"
adding new entry "cn=redhat,ou=Group,dc=centos,dc=org"
adding new entry "cn=ubuntu,ou=Group,dc=centos,dc=org"
adding new entry "cn=debian,ou=Group,dc=centos,dc=org"
```

## 2.3 在LDAP中删除用户和组

```Shell
[root@lab ~]# ldapdelete -x -W -D 'cn=Manager,dc=centos,dc=org' "uid=JerryDai,ou=People,dc=centos,dc=org" 
Enter LDAP Password:

[root@lab ~]# ldapdelete -x -W -D 'cn=Manager,dc=centos,dc=org' "cn=JerryDai,ou=Group,dc=centos,dc=org" 
Enter LDAP Password:
```

# 3. 配置LDAP客户端

## 3.1 启用LDAP认证

```
[root@www ~]# yum -y install openldap-clients nss-pam-ldapd
# ldapserver=(LDAP server's hostname or IP address)
# ldapbasedn="dc=(your own domain name)"
[root@www ~]# authconfig --enableldap \
--enableldapauth \
--ldapserver=lab.centos.org \
--ldapbasedn="dc=centos,dc=org" \
--enablemkhomedir \
--update

[root@www ~]# exit 
logout
CentOS Linux 7 (Core)
Kernel 3.10.0-123.20.1.el7.x86_64 on an x86_64
www login: redhat # LDAP user
Password:# password
Creating directory '/home/redhat'.

[redhat@www ~]$ # logined normally 
[redhat@www ~]$ passwd # try to change the LDAP password
Changing password for user redhat.
Enter login(LDAP) password: # current password
New password: # new password
Retype new password:
LDAP password information changed for redhat
passwd: all authentication tokens updated successfully.
```

## 3.2 变更SELinux策略
```Shell
[root@www ~]# vi mkhomedir.te
# create new
module mkhomedir 1.0;

require {
        type unconfined_t;
        type oddjob_mkhomedir_exec_t;
        class file entrypoint;
}

#============= unconfined_t ==============
allow unconfined_t oddjob_mkhomedir_exec_t:file entrypoint;

[root@www ~]# checkmodule -m -M -o mkhomedir.mod mkhomedir.te 
checkmodule: loading policy configuration from mkhomedir.te
checkmodule: policy configuration loaded
checkmodule: writing binary representation (version 17) to mkhomedir.mod

[root@www ~]# semodule_package --outfile mkhomedir.pp --module mkhomedir.mod 
[root@www ~]# semodule -i mkhomedir.pp 
```

# 4. 基于SSL/TLS的LDAP

## 4.1 创建SSL数字证书
```Shell
[root@www ~]# cd /etc/pki/tls/certs 
[root@www certs]# make server.key 
umask 77 ; \
/usr/bin/openssl genrsa -aes128 2048 > server.key
Generating RSA private key, 2048 bit long modulus
...
...
e is 65537 (0x10001)
Enter pass phrase:# set passphrase
Verifying - Enter pass phrase:# confirm

# remove passphrase from private key
[root@www certs]# openssl rsa -in server.key -out server.key 
Enter pass phrase for server.key:# input passphrase
writing RSA key

[root@www certs]# make server.csr 
umask 77 ; \
/usr/bin/openssl req -utf8 -new -key server.key -out server.csr
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [XX]:CN# country
State or Province Name (full name) []:Beijing   # state
Locality Name (eg, city) [Default City]:Beijing# city
Organization Name (eg, company) [Default Company Ltd]:Tsinghua   # company
Organizational Unit Name (eg, section) []:CentOS    # department
Common Name (eg, your name or your server's hostname) []:lab.centos.org   # server's FQDN
Email Address []:admin@centos.org# email address
Please enter the following 'extra' attributes
to be sent with your certificate request
A challenge password []:# Enter
An optional company name []:# Enter


[root@www certs]# openssl x509 -in server.csr -out server.crt -req -signkey server.key -days 3650
Signature ok
subject=/C=CN/ST=Beijing/L=Beijing/O=Tsinghua/OU=CentOS/CN=lab.centos.org/emailAddress=admin@centos.org
Getting Private key
```
## 4.2 配置LDAP服务

```Shell
[root@lab ~]# cp /etc/pki/tls/certs/server.key \
/etc/pki/tls/certs/server.crt \
/etc/pki/tls/certs/ca-bundle.crt \
/etc/openldap/certs/ 

[root@lab ~]# chown ldap. /etc/openldap/certs/server.key \
/etc/openldap/certs/server.crt \
/etc/openldap/certs/ca-bundle.crt

[root@lab ~]# vi mod_ssl.ldif
# create new
 dn: cn=config
changetype: modify
add: olcTLSCACertificateFile
olcTLSCACertificateFile: /etc/openldap/certs/ca-bundle.crt
-
replace: olcTLSCertificateFile
olcTLSCertificateFile: /etc/openldap/certs/server.crt
-
replace: olcTLSCertificateKeyFile
olcTLSCertificateKeyFile: /etc/openldap/certs/server.key


[root@lab ~]# ldapmodify -Y EXTERNAL -H ldapi:/// -f mod_ssl.ldif 
SASL/EXTERNAL authentication started
SASL username: gidNumber=0+uidNumber=0,cn=peercred,cn=external,cn=auth
SASL SSF: 0
modifying entry "cn=config"

[root@dlp ~]# vi /etc/sysconfig/slapd
# line 9: add
SLAPD_URLS="ldapi:/// ldap:/// ldaps:///"

[root@dlp ~]# systemctl restart slapd 
```

## 4.3 配置客户端采用TLS

```Shell
[root@www ~]# echo "TLS_REQCERT allow" >> /etc/openldap/ldap.conf 
[root@www ~]# echo "tls_reqcert allow" >> /etc/nslcd.conf 

[root@www ~]# authconfig --enableldaptls --update 
getsebool: SELinux is disabled
[root@www ~]# exit 
logout

CentOS Linux 7 (Core)
Kernel 3.10.0-123.20.1.el7.x86_64 on an x86_64
www login: redhat 
Password:
Last login: Tue Aug 19 19:55:52 on ttyS0
[redhat@www ~]$ # logined normally
```

# 5. 安装phpLDAPadmin

```Shell
# install from EPEL
[root@lab ~]# yum --enablerepo=epel -y install phpldapadmin

[root@lab ~]# vi /etc/phpldapadmin/config.php
# line 397: uncomment, line 398: comment out
$servers->setValue('login','attr','dn');
// $servers->setValue('login','attr','uid');

[root@lab ~]# vi /etc/httpd/conf.d/phpldapadmin.conf
Alias /phpldapadmin /usr/share/phpldapadmin/htdocs
Alias /ldapadmin /usr/share/phpldapadmin/htdocs
<Directory /usr/share/phpldapadmin/htdocs>
  <IfModule mod_authz_core.c>
    # Apache 2.4
    # line 12: add access permission
    Require local
    Require ip 10.0.0.0/24
    
[root@lab ~]# systemctl restart httpd 
```