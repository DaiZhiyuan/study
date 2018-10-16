#### 查看i915模块相关信息
```
$> modinfo i915
filename:       /lib/modules/3.10.0-693.el7.x86_64/kernel/drivers/gpu/drm/i915/i915.ko.xz
license:        GPL and additional rights
description:    Intel Graphics
author:         Intel Corporation
author:         Tungsten Graphics, Inc.
    [......]
firmware:       i915/bxt_guc_ver8_7.bin
firmware:       i915/skl_guc_ver6_1.bin
rhelversion:    7.4
srcversion:     FB21B15BEA7FC372951373D
alias:          pci:v00008086d0000593Bsv*sd*bc03sc*i*
alias:          pci:v00008086d00005927sv*sd*bc03sc*i*
alias:          pci:v00008086d00005926sv*sd*bc03sc*i*
    [......]
depends:        drm_kms_helper,drm,i2c-core,video,i2c-algo-bit
intree:         Y
vermagic:       3.10.0-693.el7.x86_64 SMP mod_unload modversion
```

- firmware字段

> firmware: i915/bxt_guc_ver8_7.bin

很多设备需要两样东西才能工作，驱动和固件。这个驱动依赖固件，固件从文件系统/lib/firmware读取。

- alias字段

> alias:  pci:v00008086d0000593Bsv*sd*bc03sc*i*

```
v00008086：v表示vendor，它表示一个硬件厂商。例如：0x8086表示"Intel Corporation"。

d00005A84：d表示device，当指定厂商时，它用来唯一标识一个设备。

sv*, sd*: sv表示sub-vendor子硬件厂商，sd表示sub-device子设备名称用于进一步识别设备 (* 匹配任何字符)。

bc03: bc即Base Class表示各种设备的种类如：IDE interface, Ethernet controller, USB Controller等。例如： bc03表示Display controller。 

sc*: sc表示sub Class，用于扩展Base Class。

i*: i表示interface即接口。
```

- vermagic字段

> vermagic: 3.10.0-693.el7.x86_64 SMP mod_unload modversion

当载入模块时，通过vermagic字符来检查是否匹配。如果不匹配禁止插入并打印错误信息。你可以使用modprobe --force命令来绕过限制，使用这个选项很危险。

