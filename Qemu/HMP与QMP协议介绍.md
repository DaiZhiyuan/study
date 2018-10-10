***HMP与QMP协议介绍***

[TOC]

# 1. 简介

Qemu Monito Protocol简称qmp，基于json（rfc4627）的协议，便于其他应用控制qemu。
qmp是qemu-kvm虚拟机中的一个重要组成部分。如果使用libvirt起一台虚拟机，libvirt使用qmp给qemu发送命令，qemu通过qmp events（qmp事件）返回，所以说qmp是libvirt和qemu之间交互的一个重要通道。

根据http://wiki.qemu.org/QMP中的介绍，现在我们使用的命令还是放在qmp-commands.hx和hmp-commands.hx中，未来希望都放在qapi-schema.json文件中，就像qga（qemu guest agent）的实现方式一样。

## 1.1 相关文件
- qmp-commands.hx
- qmp-events.txt
- qmp-spec.txt
- qapi-schema.json
- writing-qmp-commands.txt
- hmp-commands.hx

## 1.2 特点

1. 轻量、基于文本、指令格式易于解析，因为它是json格式的；
2. 支持异步消息，主要指通过qmp发送给虚拟机的指令支持异步；
3. Capabilities Negotiation，主要指我们初次建立qmp连接时，进入了capabilities negotiation模式,这时我们不能发送任何指令，除了qmp_capabilities指令，发送了qmp_capabilitie指令，我们就退出了capabilities negotiation模式，进入了指令模式（command mode），这时我们可以发送qmp指令，如{ "execute": "query-status" }，这样就可以查询虚拟机的状态。

# 2. 原理

QMP本质上是一种unix socket本地通信机制，通信内容基于JSON格式，当其他的应用成功连接qemu的QMP监听服务后，会得到一个欢迎消息（表明连接成功），具体的消息如下：
```
{ "QMP": {"version": json-object, "capabilities": json-array } }
```
其中，json-object是一个JSON的对象，json-array是一个JSON对象的数组，一个JSON对象一般表现为一个字典。JSON格式是一种键值对的格式，键值version指定QEMU的版本号，键值capabilities指定QEMU的QMP机制提供全部功能。

## 2.1 发送命令格式
```
{"execute": json-string, "arguments": json-object,"id": json-value }
```
键值execute指定要执行的命令；键值arguments指定命令执行时候需要的参数，是可选的一个键值；键值id是该执行命令的一个标识，是可选的，如果发送命令中包括这个键值，那么返回的响应中也会有同样的键值。

## 2.2 响应格式 

### 2.2.1 成功响应
```
{"return": json-object, "id": json-value }
```
键值return返回命令执行后的结果数据，如果命令执行完毕没有返回数据，则该键值对应的内容为空；键值id返回对应的发送命令中的键值id对应的值。

### 2.2.2 错误响应
```
{"error": { "class": json-string, "desc":json-string }, "id": json-value }
```
键值class指定错误的类型；键值desc指定人类可读的错误描述；键值id意思同上。

## 2.3 异步事件

由于状态改变，QEMU可以单方面向应用发送消息，被称之为异步事件。格式如下：

```
{
    "event": json-string, "data": json-object,
    "timestamp": { "seconds": json-number,"microseconds": json-number }
}
```

键值event包括事件的名字；键值data指定事件关联的数据，可选的；键值timestamp指定了事件发生的具体时间。 
对于发生的事件可以参考qmp-event.txt。


# 3. Capabilities Negotiation模式 
当一个应用连接到QEMU后，QEMU将会处于Capabilities Negotiation模式，在该模式下，只有qmp_capabilities命令可以被使用，当应用发送qmp_capabilities命令并且QEMU成功执行该命令后，QEMU将会处于Command模式，此时，除了qmp_capabilities之外的其他命令便可以使用了。


# 4. QMP样例

## 4.1 QEMU的欢迎消息
```
{
    "QMP": {
        "version": {
            "qemu": {
                "micro": 0,
                "minor": 5,
                "major": 2
            },
            "package": ""
        },
        "capabilities": [
        ]
    }
}

```

## 4.2 执行stop命令
```
Client: { "execute": "stop" } 
Server: {"return": {} }
```
Client表示virsh,telnet,nc等；Server表示QEMU。

## 4.3 查询KVM信息
```
Client: { "execute": "query-kvm","id": "example" } 
Server: { "return": {"enabled": true, "present": true }, "id":"example"}
```

## 4.4 错误返回
```
Client: { "execute": } 
Server: { "error":{ "class": "GenericError", "desc": "InvalidJSON syntax" } }
```

## 4.5 掉电事件
```
Server: { "timestamp": { "seconds":1258551470, "microseconds": 802384 }, "event":"POWERDOWN" }
```


# 5. QMP与HMP区别
qmp和hmp(human monitor protocol)的区别是qmp命令使用的是json格式，hmp直接使用命令，而且两者之间的命令也略有不同。相对来说，hmp比qmp在格式上要简单些。


# 3. 使用

## 3.1 基于QEMU使用方式

1. 启动QEMU
```
qemu-system-x86_64 \
-smp 4,sockets=2 -m 2048 \
-machine q35,accel=kvm \
-drive file=/root/centos.qcow2,if=virtio \
-drive file=/usr/share/ovmf/OVMF_CODE.fd,if=pflash,format=raw,unit=0,readonly=on \
-drive file=/usr/share/ovmf/OVMF_VARS.fd,if=pflash,format=raw,unit=1 \
-boot menu=on,splash-time=5000 \
-chardev socket,id=qmp,port=60009,host=localhost,server,nowait \
-mon chardev=qmp,mode=control,pretty=on
```

2. 连接QEMU并发送命令

- 执行命令qmp_capabilities
- 执行命令stop

```
[root@centos7 ~]# nc 127.0.0.1 60009
{
    "QMP": {
        "version": {
            "qemu": {
                "micro": 0,
                "minor": 5,
                "major": 2
            },
            "package": ""
        },
        "capabilities": [
        ]
    }
}
{ "execute": "qmp_capabilities"}
{
    "return": {
    }
}
{ "execute": "stop" }
{
    "timestamp": {
        "seconds": 1539140291,
        "microseconds": 22723
    },
    "event": "STOP"
}
{
    "return": {
    }
}

```


## 3.2 基于libvirtd使用方式

使用命令 ：
```
# virsh qemu-monitor-command --help
```
可以查到所有的qmp命令以及对命令的简单介绍。


使用命令：
```
# virsh qemu-monitor-command DOMAIN --hmp help
```
可以查到所有的hmp(human monitor protocol)的命令以及对命令的简单介绍。


