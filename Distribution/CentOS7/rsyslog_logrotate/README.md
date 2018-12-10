- 1 rsyslog
    - 1.1 rsyslog提供了三种远程传输协议
    - 1.2 定位日志文件
    - 1.3 Rsyslog基本配置
    - 1.4 Filter
        - 1.4.1 基于设施/优先级的过滤器
        - 1.4.2 基于属性的过滤器
        - 1.4.3 基于表达式的过滤器
    - 1.5 Action
        - 1.5.1 保存日志到日志文件
        - 1.5.2 通过网络发送syslog
        - 1.5.3 丢弃日志
    - 1.6 rsyslog的使用
        - 1.6.1 bash使用方式
        - 1.6.2 C使用方式
        - 1.6.3 python使用方式
- 2 logrotate
    - 2.1 配置文件
    - 2.2 特殊文件
        - 2.2.1 /var/log/btmp
        - 2.2.2 /var/log/wtmp
        - 2.2.3 /var/account/pacct
    - 2.3 logrotate常用参数

# 1. rsyslog

rsyslog（Rocket-fast System for LOG processing）：增强的系统日志记录和内核消息捕获守护进程。
- 多线程
- UDP/TCP/SSL/TLS/RELP
- 存储日志信息于MySQL, PGSQL, Oracle等RDBMS
- 强大的过滤器，实现过滤日志信息中的任何部分的内容
- 自定义的输出格式


## 1.1 rsyslog提供了三种远程传输协议

1. UDP 传输协议：
基于传统UDP协议进行远程日志传输，也是传统syslog使用的传输协议； 
可靠性比较低，但性能损耗最少， 在网络情况比较差， 
或者接收服务器压力比较高情况下，可能存在丢日志情况。 
在对日志完整性要求不是很高，在可靠的局域网环境下可以使用。

2. TCP 传输协议：
基于传统TCP协议明文传输，需要回传进行确认，可靠性比较高； 
但在接收服务器宕机或者两者之间网络出问题的情况下，会出现丢日志情况。 
这种协议相比于UDP在可靠性方面已经好很多，并且rsyslog原生支持，配置简单， 
同时针对可能丢日志情况，可以进行额外配置提高可靠性，因此使用比较广。

3. RELP 传输协议：
RELP（Reliable Event Logging Protocol）是基于TCP封装的可靠日志消息传输协议； 
是为了解决TCP与UDP协议的缺点而在应用层实现的传输协议，也是三者之中最可靠的。 
需要多安装一个包rsyslog-relp以支持该协议。

## 1.2 定位日志文件

大多数日志文件都位于/var/log/目录中。在该目录中，你可能注意到很多日志文件末尾包含一串数字（如messages-20181202），这说明这些日志文件经过了日志转储，这样可以避免日志文件过大。

> 在软件包logrotate中包含了一个定时任务，根据/etc/logrotate.conf文件和/etc/logrotate.d/目录中的的配置定期的转储日志文件。

## 1.3 Rsyslog基本配置

Rsyslog 的主要配置文件为/etc/rsyslog.conf文件，在配置文件中，我们通过配置filter以及action对日志进行管理。

在rsyslog配置文件中，使用如下格式定义规则:
```
filter        action
```
rsyslog发现符合filter规则的日志后，会将日志发送到action指定的动作进行处理。

## 1.4 Filter

在rsyslog中，提供了三种方式的过滤器方法：
- 基于设施/优先级的过滤器
- 基于属性的过滤器
- 基于表达式的过滤器


### 1.4.1 基于设施/优先级的过滤器

基于设施/优先级的过滤器是最常用的方法，语法如下：
```
FACILITY.PRIORITY
```

FACILITY指定了产生日志消息的子系统，可选值为auth, authpriv, cron, daemon, kern, lpr, mail, news, syslog, user, ftp, uucp, local0 ~ local7。


FACILITY | Commnet
---|---
auth | pam产生的日志
authpriv | ssh登录、验证信息
cron | 计划任务
daemon | 守护进程
kern | 内核
lpr | 打印机
mail | 邮件
news | 新闻组
syslog | rsyslog服务内部的信息，事件标识
user | 用户程序产生的相关信息
ftp | ftp登录、验证信息
uucp | unix to unix copy，UNIX主机之间
local0 ~ local7 | 自定义


PRIORITY指定了日志消息的优先级，可用的优先级包含debug(7), info(6), notice(5), warning(4), err(3), crit(2), alert(1), emerg(0)。

PRIORITY | Commnet
---|---
debug | 有调试信息的，日志信息最多
info | 一般信息的日志，最常用
notice | 最具有重要性的普通条件信息
warning | 警告级别
err | 错误级别，阻止某个功能或者模块不能正常工作的信息
crit | 严重级别，阻止整个系统或者整个软件不能正常工作的信息
alter | 需要立刻修改的信息
emerg | 内核崩溃等严重信息
done | 什么都不记录

前置符号：
- '=' : 表明只有该优先级的消息会被捕获
- '!' : 表明除了该优先级的消息之外的优先级会被捕获
- '*' : 表明所有优先级的消息都会捕获
- ''  ：不指定符号，表明大于等于该优先级的消息会被捕获

> 表示所有的设施或者优先级，对优先级部分使用none关键字会捕获所有没有指定优先级的消息。

> 定义多个设施或者优先级使用,分隔，如果是多个filter的话，则使用;进行分隔。

使用范例：
```
kern.*                    # 选择所有优先级的内核日志
mail.crit                 # 选择所有mail的优先级高于crit的日志
cron.!info,!debug         # 选择除了info和debug优先级的cron日志
```

### 1.4.2 基于属性的过滤器

基于属性的过滤器语法:
```
:PROPERTY, [!]COMPARE_OPERATION, "STRING"
```

:PROPERTY是要比较的日志属性，COMPARE_OPERATION 为要执行的比较操作，这个的!表示取反的意思，"STRING"为比较的值。

可以使用的比较操作：

比较操作 | 描述
---|---
contains | 匹配提供的字符串值是否是属性的一部分，如果不区分大小写，使用contains_i
isequal	| 比较属性和值是否相等
startswith | 属性是否以指定字符串开始(startswith_i)
regex | 正则表达式(POSIX BRE 基本正则)匹配
ereregex | 正则表达式(POSIX ERE 扩展正则)匹配
isempty | 判断属性是否为空，不需要 value

使用范例：
```
:msg, contains, "error"
:hostname, isequal, "host1"
:msg, !regex, "fatal .* error"
```

### 1.4.3 基于表达式的过滤器

基于表达式的过滤器使用了rsyslog自定义的脚本语言RainerScript构建复杂的filter，这里暂时不对这种方法进行讲述。

## 1.5 Action

Action定义了当匹配指定的filter的时候，执行什么操作。

> 如果要指定多个ACTION，使用&连接多个ACTION。

### 1.5.1 保存日志到日志文件

语法：
```
FILTER PATH
```
这里的 PATH 指定了日志要保存到的文件。例如cron.* /var/log/cron.log指定了所有的定时任务日志都写入到/var/log/cron.log文件。

> 默认情况下，每次生成syslog的时候，日志信息会同步到日志文件。可以在文件路径前使用-指定忽略同步（如果系统崩溃，会丢失日志，但是这样可以提高日志性能）。

### 1.5.2 通过网络发送syslog

rsyslog可以使用网络将日志消息发送或者接受日志，使用这个特性，可以实现使用单一的日志服务器统一管理多台服务器日志。

```
@[(zNUMBER)]HOST:[PORT]
```
这里的@告诉syslog使用 UDP 协议发送日志，要使用 TCP 的话，使用 @@。可选值zNUMBER设置了是否允许使用zlib对日志压缩（压缩级别1-9）。

使用范例：
```
*.* @192.168.0.1        # 使用 UDP 发送，默认端口514
*.* @@example.com:18    # 使用 TCP 发送到端口18， 默认10514
*.* @(z9)[2001:db8::1]  # UDP, ipv6，使用zlib级别9压缩
```

### 1.5.3 丢弃日志

要丢弃日志消息，使用~动作。
```
FILTER    ~
```

使用范例：
```
cron.* ~
```

## 1.6 rsyslog的使用

1. 配置rsyslog（在/etc/rsyslog.conf文件末尾加下列内容）：
```
# Save Demo message to demo.log
local0.info                                                /var/log/demo.log
```

2. 重启rsyslog服务：
```
#> systemctl restart rsyslog.service
```

### 1.6.1 bash使用方式

1. 使用logger向rsyslog打印日志

test_rsyslog.sh
```bash
#!/bin/bash

level="emerg alert crit err warning notice info debug emerg err warning"

for l in $level; do
    logger -p "local0.$l" -t "test_logger.sh" "[$l]This is a message"
done
```

2. 查看rsyslog输出信息
```
[root@lab logrotate.d]# cat /var/log/demo.log
Dec 10 16:53:16 lab test_logger.sh: [emerg]This is a message
Dec 10 16:53:16 lab test_logger.sh: [alert]This is a message
Dec 10 16:53:16 lab test_logger.sh: [crit]This is a message
Dec 10 16:53:16 lab test_logger.sh: [err]This is a message
Dec 10 16:53:16 lab test_logger.sh: [warning]This is a message
Dec 10 16:53:16 lab test_logger.sh: [notice]This is a message
Dec 10 16:53:16 lab test_logger.sh: [info]This is a message
Dec 10 16:53:16 lab test_logger.sh: [emerg]This is a message
Dec 10 16:53:16 lab test_logger.sh: [err]This is a message
Dec 10 16:53:16 lab test_logger.sh: [warning]This is a message
```

### 1.6.2 C使用方式

1. 使用syslog库函数向rsyslog打印日志

test_rsyslog.c
```c
#include <syslog.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    syslog(LOG_LOCAL0| LOG_INFO, "informative message, pid=%d", getpid());

    return 0;
}

```

2. 查看rsyslog输出信息
```
[root@lab logrotate.d]# cat /var/log/demo.log
Dec 10 17:06:05 lab test_rsyslog: informative message, pid=23989
```

### 1.6.3 python使用方式

1. 使用syslog模块向rsyslog打印日志

test_rsyslog.py
```python
#!/usr/bin/env python

from syslog import *

if __name__ == '__main__':
    syslog(LOG_LOCAL0|LOG_INFO, "This is a message")

```

2. 查看rsyslog输出信息

```
[root@lab logrotate.d]# cat /var/log/demo.log
Dec 10 17:12:02 lab test_rsyslog.py: This is a message
```

# 2. logrotate

logrotate是一个日志管理程序，用来把旧的日志文件删除（备份），并创建新的日志文件，这个过程称为"转储"，我们可以根据日志的大小，或者根据其使用的天数来转储。

logrotate的执行由anacron服务来完成。在/etc/cron.daily目录中，有个文件logrotate，它实际上是个shell script，用来启动logrotate。

Linux下计划任务at、crontab与anacron区别：
- at -- 计划任务（依赖于atd服务）
- crontab -- 周期性计划任务（依赖crond服务）
- anacron -- 系统级周期性计划任务（依赖于anacron服务)

crontab与anacron比较：

- cron和anacron都是守护进程，可以将重复任务的执行调度到用户定义的某个时间点。
- 与cron服务不同，anacron不会错过调度作业的执行，即使系统断电。该活动将在系统下一个可用时执行。
- 这使得anacron成为启动诸如备份或磁盘空间恢复等基本系统管理任务的首选选择。


anacron配置文件：
```
# /etc/anacrontab: configuration file for anacron

# See anacron(8) and anacrontab(5) for details.

SHELL=/bin/sh
PATH=/sbin:/bin:/usr/sbin:/usr/bin
MAILTO=root

# the maximal random delay added to the base delay of the jobs
RANDOM_DELAY=45

# the jobs will be started during the following hours only
START_HOURS_RANGE=3-22

#period in days   delay in minutes    job-identifier    command
1                   5                   cron.daily      nice run-parts /etc/cron.daily
7                   25                  cron.weekly     nice run-parts /etc/cron.weekly
@monthly            45                  cron.monthly    nice run-parts /etc/cron.monthly
```

cron.daily解读：
```
[root@lab ~]# run-parts --list /etc/cron.daily/
/etc/cron.daily/logrotate
/etc/cron.daily/man-db.cron
/etc/cron.daily/mlocate
```

/etc/cron.daily/logrotate脚本：
```bash
[root@lab cron.daily]# cat /etc/cron.daily/logrotate
#!/bin/sh

/usr/sbin/logrotate -s /var/lib/logrotate/logrotate.status /etc/logrotate.conf
EXITVALUE=$?
if [ $EXITVALUE != 0 ]; then
    /usr/bin/logger -t logrotate "ALERT exited abnormally with [$EXITVALUE]"
fi
exit 0
```
最终得知：/usr/sbin/logrotate -s /var/lib/logrotate/logrotate.status /etc/logrotate.conf


常用Debug方式：
```
#> logrotate -d -s /var/lib/logrotate/logrotate.status /etc/logrotate.conf
#> logrotate -d -fv -s /var/lib/logrotate/logrotate.status /etc/logrotate.conf
```

## 2.1 配置文件

默认配置文件存放在：/etc/logrotate.conf

```
[root@lab ~]# cat /etc/logrotate.conf
# see "man logrotate" for details

# rotate log files weekly
# 每周轮转一次
weekly

# keep 4 weeks worth of backlogs
# 保留4个副本
rotate 4

# create new (empty) log files after rotating old ones
# rotate后，创建一个新的空文件
create

# use date as a suffix of the rotated file
# rotate后，增加日期后缀
dateext

# uncomment this if you want your log files compressed
# 默认是不压缩的
#compress

# RPM packages drop log rotation information into this directory
include /etc/logrotate.d

# no packages own wtmp and btmp -- we'll rotate them here
/var/log/wtmp {
    monthly
    create 0664 root utmp
        minsize 1M
    rotate 1
}

/var/log/btmp {
    missingok
    monthly
    create 0600 root utmp
    rotate 1
}

# system-specific logs may be also be configured here.
```

> /etc/logrotate.conf为全局配置文件，具体程序可以在/etc/logrotate.d目录下创建自己的转储策略（优先级高于全局配置）。

## 2.2 特殊文件

### 2.2.1 /var/log/btmp

此文件是记录错误登录的日志。

使用lastb命令打开：

```bash
[root@lab ~]# lastb
root      tty1                            Sat Dec 28 23:53 - 23:53  (00:00)
root      tty1                            Sun Aug 24 17:28 - 17:28  (00:00)
root      ssh:notty     192.168.30.254    Thu Mar 12 03:09 - 03:09  (00:00)
root      ssh:notty     192.168.30.254    Thu Mar 12 03:09 - 03:09  (00:00)
root      tty1                            Tue Oct 15 09:28 - 09:28  (00:00)
btmp begins Mon Dec  3 14:23:02 2018
```

### 2.2.2 /var/log/wtmp

/var/log/wtmp也是一个二进制文件，记录每个用户的登录次数和持续时间等信息。
 
该日志文件永久记录每个用户登录、注销及系统的启动、停机的事件。
因此随着系统正常运行时间的增加，该文件的大小也会越来越大，增加的速度取决于系统用户登录的次数。该日志文件可以用来查看用户的登录记录，
last命令就通过访问这个文件获得这些信息，并以反序从后向前显示用户的登录记录，last也能根据用户、终端tty或时间显示相应的记录。
 
使用lst命令查看：
```bash
[root@CentOS7u4 ~]# last
root     pts/0        :0               Mon Dec 10 02:46   still logged in   
root     :0           :0               Mon Dec 10 02:45   still logged in   
reboot   system boot  3.10.0-693.el7.x Mon Dec 10 02:45 - 02:46  (00:01)    
root     pts/0        :0               Fri Dec  7 04:07 - 04:07  (00:00)    
root     :0           :0               Fri Dec  7 04:06 - down   (00:00)    
root     pts/0        192.168.30.196   Tue Dec  4 21:57 - 22:15  (00:17)    
root     pts/0        192.168.30.196   Tue Dec  4 21:56 - 21:57  (00:00)    
reboot   system boot  3.10.0-693.el7.x Tue Dec  4 21:52 - 04:07 (2+06:15)   
root     pts/1        192.168.30.196   Tue Dec  4 02:31 - 03:45  (01:14)    
root     pts/1        192.168.30.196   Mon Dec  3 04:01 - 21:18  (17:16)    
root     pts/0        :0               Fri Nov 30 01:56 - crash (4+19:55)   
root     :0           :0               Fri Nov 30 01:56 - crash (4+19:55)   
reboot   system boot  3.10.0-693.el7.x Fri Nov 30 01:55 - 04:07 (7+02:11)   
root     pts/0        :0               Fri Nov 23 04:15 - 04:15  (00:00)    
root     :0           :0               Fri Nov 23 04:14 - 04:15  (00:00)    
reboot   system boot  3.10.0-693.el7.x Fri Nov 23 03:39 - 04:15  (00:35)    
root     pts/0        :0               Thu Nov 22 02:32 - 04:39  (02:06)    
root     :0           :0               Thu Nov 22 02:32 - down   (02:07)    
```

last命令常用参数：
-a : 把从何处登入系统的主机名称或IP地址，显示在最后一行。
-d ：将IP地址转换成主机名称。
-f : 指定记录文件。
-n ：设置列出名单的显示列数。
-R ：不显示登入系统的主机名称或IP地址。
-x ：显示系统关机，重新开机，以及执行等级的改变等信息。

### 2.2.3 /var/account/pacct

pacct是ProcessACCounting的缩写，表示进程记账。

1. 开启进程记账
```bash
[root@lab ~]# accton on
Turning on process accounting, file set to the default '/var/account/pacct'.
```

2. 关闭进程记账
```bash
[root@lab ~]# accton off
Turning off process accounting.
```

3. 查看进行记账信息
```bash
[root@lab ~]# lastcomm
accton                 root     pts/2      0.00 secs Mon Dec 10 15:52
systemd-cgroups  S     root     __         0.00 secs Mon Dec 10 15:52
crond            SF    root     __         0.00 secs Mon Dec 10 15:52
dump_replay            root     __         0.00 secs Mon Dec 10 15:52
grep                   root     __         0.00 secs Mon Dec 10 15:52
cat                    root     __         0.00 secs Mon Dec 10 15:52
bash              F    root     pts/2      0.00 secs Mon Dec 10 15:51
bash              F    root     pts/2      0.00 secs Mon Dec 10 15:51
git                    root     pts/2      0.00 secs Mon Dec 10 15:51
sshd             S     root     __         0.00 secs Mon Dec 10 15:51
ksmtuned          F    root     __         0.00 secs Mon Dec 10 15:51
awk                    root     __         0.00 secs Mon Dec 10 15:51
ksmtuned          F    root     __         0.00 secs Mon Dec 10 15:51
ksmtuned          F    root     __         0.00 secs Mon Dec 10 15:51
pgrep                  root     __         0.01 secs Mon Dec 10 15:51
ksmtuned          F    root     __         0.00 secs Mon Dec 10 15:51
awk                    root     __         0.00 secs Mon Dec 10 15:51
sleep                  root     __         0.00 secs Mon Dec 10 15:50
```

flags:
- S -- command executed by super-user
- F -- command executed after a fork but without a following exec
- C -- command run in PDP-11 compatibility mode (VAX only)
- D -- command terminated with the generation of a core file
- X -- command was terminated with the signal SIGTERM

## 2.3 logrotate常用参数

```bash
[root@lab logrotate.d]# cat /etc/logrotate.d/syslog
/var/log/cron
/var/log/maillog
/var/log/messages
/var/log/secure
/var/log/spooler

{
    missingok
    sharedscripts
    postrotate
        /bin/kill -HUP `cat /var/run/syslogd.pid 2> /dev/null` 2> /dev/null || true
    endscript
}

```

args | comment
---|---
compress | 通过gzip 压缩转储以后的日志
nocompress | 不做gzip压缩处理
copytruncate | 用于还在打开中的日志文件，把当前日志备份并截断；是先拷贝再清空的方式，拷贝和清空之间有一个时间差，可能会丢失部分日志数据。
nocopytruncate | 备份日志文件不过不截断
create mode owner group | 轮转时指定创建新文件的属性，如create 0777 nobody nobody
nocreate | 不建立新的日志文件
delaycompress | 和compress 一起使用时，转储的日志文件到下一次转储时才压缩
nodelaycompress | 覆盖 delaycompress 选项，转储同时压缩。
missingok | 如果日志丢失，不报错继续滚动下一个日志
errors address | 专储时的错误信息发送到指定的Email 地址
ifempty | 即使日志文件为空文件也做轮转，这个是logrotate的缺省选项。
notifempty | 当日志文件为空时，不进行轮转
mail address | 把转储的日志文件发送到指定的E-mail 地址
nomail | 转储时不发送日志文件
olddir directory | 转储后的日志文件放入指定的目录，必须和当前日志文件在同一个文件系统
noolddir | 转储后的日志文件和当前日志文件放在同一个目录下
sharedscripts | 运行postrotate脚本，作用是在所有日志都轮转后统一执行一次脚本。如果没有配置这个，那么每个日志轮转后都会执行一次脚本
prerotate | 在logrotate转储之前需要执行的指令，例如修改文件的属性等动作；必须独立成行
postrotate | 在logrotate转储之后需要执行的指令，例如重新启动 (kill -HUP) 某个服务！必须独立成行
daily | 指定转储周期为每天
weekly | 指定转储周期为每周
monthly | 指定转储周期为每月
rotate count | 指定日志文件删除之前转储的次数，0 指没有备份，5 指保留5个备份
dateext | 使用当期日期作为命名格式
dateformat .%s | 配合dateext使用，紧跟在下一行出现，定义文件切割后的文件名，必须配合dateext使用，只支持 %Y %m %d %s 这四个参数
size(或minsize) log-size | 当日志文件到达指定的大小时才转储，log-size能指定bytes(缺省)及KB (sizek)或MB(sizem).