<!-- TOC -->

- [1. Linux SCSI Traget](#1-linux-scsi-traget)
- [2. Setup iSCSI Target](#2-setup-iscsi-target)
- [3. Initiator Configuration](#3-initiator-configuration)
- [4. vdbench](#4-vdbench)
    - [4.1 Mellanox Technologies MT27800 Family (25Gbps == 3125MB/s)](#41-mellanox-technologies-mt27800-family-25gbps--3125mbs)
    - [4.2 Intel Corporation 82599ES 10-Gigabit (10Gbps == 1250MB/s)](#42-intel-corporation-82599es-10-gigabit-10gbps--1250mbs)
    - [4.3 PCIe 3.0x4 (32GT/s * (128b/130b) ~= 32Gbps <= 4000MB/s)](#43-pcie-30x4-32gts--128b130b--32gbps--4000mbs)

<!-- /TOC -->


# 1. Linux SCSI Traget

LinuxIO (LIO) has been the Linux SCSI target since kernel version 2.6.38. 
It supports a rapidly growing number of `fabric modules`, and all existing Linux block devices as `backstores`.

`targetcli` provides a comprehensive, powerful and easy CLI tool to configure and manage LIO. targetcli was developed by Datera.

The LIO core (`target_core.ko`, see Linux kernel driver database) was released with Linux kernel 2.6.38 on January 14, 2011.

Fabric modules:
- Fibre Channel: SCSI access over FC networks with QLogic HBAs (qla2xxx.ko, released).
- Fibre Channel over Ethernet (FCoE): FC access over Ethernet (tcm_fc.ko, released).
- iSCSI: SCSI access over IP networks (iscsi.ko, released).
- iSER: iSCSI access over InfiniBand networks with Mellanox HCAs (ib_sert.ko, released)
- SRP: SCSI access over InfiniBand networks with Mellanox HCAs (srpt.ko, released).
- vHost: QEMU virtio and virtio-scsi PV guests (tcm_vhost.ko, released).

Backstores：

- `FILEIO` (Linux VFS devices): any file on a mounted filesystem. It may be backed by a file or an underlying real block device. FILEIO is using struct file to serve block I/O with various methods (synchronous or asynchronous) and (buffered or direct). The Linux kernel code for filesystems resides in linux/fs. By default, FILEIO uses non-buffered mode (O_SYNC set).
- `IBLOCK` (Linux BLOCK devices): any block device that appears in /sys/block. The Linux kernel code for block devices is in linux/block.
- `PSCSI` (Linux pass-through SCSI devices): any storage object that supports direct pass-through of SCSI commands without SCSI emulation. This assumes an underlying SCSI device that appears with lsscsi in /proc/scsi/scsi, such as a SAS hard drive, etc. SCSI-3 and higher is supported with this subsystem, but only for control CDBs capable by the device firmware.
- `Memory Copy RAMDISK` (Linux RAMDISK_MCP): Memory Copy ram disks (rd_mcp) provide ram disks with full SCSI emulation and separate memory mappings using memory copy for initiators, thus providing multi-session capability. This is most useful for fast volatile mass storage for production.

Term:
- Target/Initiator
- TPG（Target Portal Group）/LUN((Logical Unit Number)/WWN(World Widename)
    - IQN（iSCSI Qualified Name）: iqn.yyyy-mm.naming-authority:unique name
    - EUI（Extend Unique Identifier）: eui.<16 hex digits>
    - NAA（Network Address Authority）:  naa.<16 ascii> or naa.<32 ascii>

Target Engine Core ConfigFS Infrastructure v5.0：
```
Fabric module name: iscsi
ConfigFS path: /sys/kernel/config/target/iscsi
Allowed WWN types: iqn, naa, eui
Fabric module features: discovery_auth, acls, auth, nps, tpgts
Corresponding kernel module: iscsi_target_mod
```

> https://community.mellanox.com/s/article/howto-configure-lio-enabled-with-iser-for-rhel7-inbox-driver

# 2. Setup iSCSI Target

Kernel Module:
```
[root@centos7 ~]# modinfo target_core_mod
[root@centos7 ~]# modinfo target_core_iblock
```

Install Tools:
```
[root@centos7 ~]# yum install targetcli python-rtslib
```

Systemd Service:
```
/usr/lib/systemd/system/target.service
```

targetcli:
```
[root@centos7 ~]# targetcli
targetcli shell version 2.1.51
Copyright 2011-2013 by Datera, Inc and others.
For help on commands, type 'help'.

/>
/> ls
o- / ......................................................................................................................... [...]
  o- backstores .............................................................................................................. [...]
  | o- block .................................................................................................. [Storage Objects: 0]
  | o- fileio ................................................................................................. [Storage Objects: 0]
  | o- pscsi .................................................................................................. [Storage Objects: 0]
  | o- ramdisk ................................................................................................ [Storage Objects: 0]
  o- iscsi ............................................................................................................ [Targets: 0]
  o- loopback ......................................................................................................... [Targets: 0]
  o- srpt ............................................................................................................. [Targets: 0]
/>
```

Support Module:
```
Fabric module name: srpt
ConfigFS path: /sys/kernel/config/target/srpt
Allowed WWN types: ib
Allowed WWNs list: ib.fe800000000000000e42a1fffe7fe7dd, ib.fe800000000000000e42a1fffe7fe7dc
Fabric module features: acls
Corresponding kernel module: ib_srpt

Fabric module name: iscsi
ConfigFS path: /sys/kernel/config/target/iscsi
Allowed WWN types: iqn, naa, eui
Fabric module features: discovery_auth, acls, auth, nps, tpgts
Corresponding kernel module: iscsi_target_mod
```

1. Create backstore
```
/backstores/block> create name=nvmedisk dev=/dev/nvme1n1
Created block storage object nvmedisk using /dev/nvme1n1.
/> ls /
o- / ......................................................................................................................... [...]
  o- backstores .............................................................................................................. [...]
  | o- block .................................................................................................. [Storage Objects: 1]
  | | o- nvmedisk ................................................................... [/dev/nvme1n1 (1.8TiB) write-thru deactivated]
  | |   o- alua ................................................................................................... [ALUA Groups: 1]
  | |     o- default_tg_pt_gp ....................................................................... [ALUA state: Active/optimized]
  | o- fileio ................................................................................................. [Storage Objects: 0]
  | o- pscsi .................................................................................................. [Storage Objects: 0]
  | o- ramdisk ................................................................................................ [Storage Objects: 0]
  o- iscsi ............................................................................................................ [Targets: 0]
  o- loopback ......................................................................................................... [Targets: 0]
  o- srpt ............................................................................................................. [Targets: 0]
```

2. Create iscsi target

```
/iscsi> create wwn=iqn.2020-12.phytium.com.cn
Created target iqn.2020-12.phytium.com.cn.
Created TPG 1.
Global pref auto_add_default_portal=true
Created default portal listening on all IPs (0.0.0.0), port 3260.

/iscsi> ls /
o- / ....................................................................................................................... [...]
  o- backstores ............................................................................................................ [...]
  | o- block ................................................................................................ [Storage Objects: 1]
  | | o- nvme_disk ................................................................ [/dev/nvme1n1 (1.8TiB) write-thru deactivated]
  | |   o- alua ................................................................................................. [ALUA Groups: 1]
  | |     o- default_tg_pt_gp ..................................................................... [ALUA state: Active/optimized]
  | o- fileio ............................................................................................... [Storage Objects: 0]
  | o- pscsi ................................................................................................ [Storage Objects: 0]
  | o- ramdisk .............................................................................................. [Storage Objects: 0]
  o- iscsi .......................................................................................................... [Targets: 1]
  | o- iqn.2020-12.phytium.com.cn ...................................................................................... [TPGs: 1]
  |   o- tpg1 ............................................................................................. [no-gen-acls, no-auth]
  |     o- acls ........................................................................................................ [ACLs: 0]
  |     o- luns ........................................................................................................ [LUNs: 0]
  |     o- portals .................................................................................................. [Portals: 1]
  |       o- 0.0.0.0:3260 ................................................................................................... [OK]
  o- loopback ....................................................................................................... [Targets: 0]
  o- srpt ........................................................................................................... [Targets: 0]
```

3. Create LUN

```
/iscsi/iqn.20....cn/tpg1/luns> create storage_object=/backstores/block/nvme_disk
Created LUN 0.
/iscsi/iqn.20....cn/tpg1/luns> ls /
o- / ....................................................................................................................... [...]
  o- backstores ............................................................................................................ [...]
  | o- block ................................................................................................ [Storage Objects: 1]
  | | o- nvme_disk .................................................................. [/dev/nvme1n1 (1.8TiB) write-thru activated]
  | |   o- alua ................................................................................................. [ALUA Groups: 1]
  | |     o- default_tg_pt_gp ..................................................................... [ALUA state: Active/optimized]
  | o- fileio ............................................................................................... [Storage Objects: 0]
  | o- pscsi ................................................................................................ [Storage Objects: 0]
  | o- ramdisk .............................................................................................. [Storage Objects: 0]
  o- iscsi .......................................................................................................... [Targets: 1]
  | o- iqn.2020-12.phytium.com.cn ...................................................................................... [TPGs: 1]
  |   o- tpg1 ............................................................................................. [no-gen-acls, no-auth]
  |     o- acls ........................................................................................................ [ACLs: 0]
  |     o- luns ........................................................................................................ [LUNs: 1]
  |     | o- lun0 ............................................................ [block/nvme_disk (/dev/nvme1n1) (default_tg_pt_gp)]
  |     o- portals .................................................................................................. [Portals: 1]
  |       o- 0.0.0.0:3260 ................................................................................................... [OK]
  o- loopback ....................................................................................................... [Targets: 0]
  o- srpt ........................................................................................................... [Targets: 0]
```

4. Create ACL.


```
/iscsi/iqn.20....cn/tpg1/acls> pwd
/iscsi/iqn.2020-12.phytium.com.cn/tpg1/acls
/iscsi/iqn.20....cn/tpg1/acls> create wwn=iqn.2020-12.phytium.com.cn:cmcc
Created Node ACL for iqn.2020-12.phytium.com.cn:cmcc
Created mapped LUN 0.
/iscsi/iqn.20....cn/tpg1/acls> ls /
o- / ....................................................................................................................... [...]
  o- backstores ............................................................................................................ [...]
  | o- block ................................................................................................ [Storage Objects: 1]
  | | o- nvme_disk .................................................................. [/dev/nvme1n1 (1.8TiB) write-thru activated]
  | |   o- alua ................................................................................................. [ALUA Groups: 1]
  | |     o- default_tg_pt_gp ..................................................................... [ALUA state: Active/optimized]
  | o- fileio ............................................................................................... [Storage Objects: 0]
  | o- pscsi ................................................................................................ [Storage Objects: 0]
  | o- ramdisk .............................................................................................. [Storage Objects: 0]
  o- iscsi .......................................................................................................... [Targets: 1]
  | o- iqn.2020-12.phytium.com.cn ...................................................................................... [TPGs: 1]
  |   o- tpg1 ............................................................................................. [no-gen-acls, no-auth]
  |     o- acls ........................................................................................................ [ACLs: 1]
  |     | o- iqn.2020-12.phytium.com.cn:cmcc .................................................................... [Mapped LUNs: 1]
  |     |   o- mapped_lun0 ........................................................................... [lun0 block/nvme_disk (rw)]
  |     o- luns ........................................................................................................ [LUNs: 1]
  |     | o- lun0 ............................................................ [block/nvme_disk (/dev/nvme1n1) (default_tg_pt_gp)]
  |     o- portals .................................................................................................. [Portals: 1]
  |       o- 0.0.0.0:3260 ................................................................................................... [OK]
  o- loopback ....................................................................................................... [Targets: 0]
  o- srpt ........................................................................................................... [Targets: 0]
```

5. Create portal
```
/iscsi/iqn.20.../tpg1/portals> pwd
/iscsi/iqn.2020-12.phytium.com.cn/tpg1/portals

/iscsi/iqn.20.../tpg1/portals> delete ip_address=0.0.0.0 ip_port=3260
Deleted network portal 0.0.0.0:3260

/iscsi/iqn.20.../tpg1/portals> create ip_address=192.168.100.8 ip_port=3260
Using default IP port 3260
Created network portal 192.168.100.8:3260.

/iscsi/iqn.20.../tpg1/portals> ls /
o- / ....................................................................................................................... [...]
  o- backstores ............................................................................................................ [...]
  | o- block ................................................................................................ [Storage Objects: 1]
  | | o- nvme_disk .................................................................. [/dev/nvme1n1 (1.8TiB) write-thru activated]
  | |   o- alua ................................................................................................. [ALUA Groups: 1]
  | |     o- default_tg_pt_gp ..................................................................... [ALUA state: Active/optimized]
  | o- fileio ............................................................................................... [Storage Objects: 0]
  | o- pscsi ................................................................................................ [Storage Objects: 0]
  | o- ramdisk .............................................................................................. [Storage Objects: 0]
  o- iscsi .......................................................................................................... [Targets: 1]
  | o- iqn.2020-12.phytium.com.cn ...................................................................................... [TPGs: 1]
  |   o- tpg1 ............................................................................................. [no-gen-acls, no-auth]
  |     o- acls ........................................................................................................ [ACLs: 1]
  |     | o- iqn.2020-12.phytium.com.cn:cmcc .................................................................... [Mapped LUNs: 1]
  |     |   o- mapped_lun0 ........................................................................... [lun0 block/nvme_disk (rw)]
  |     o- luns ........................................................................................................ [LUNs: 1]
  |     | o- lun0 ............................................................ [block/nvme_disk (/dev/nvme1n1) (default_tg_pt_gp)]
  |     o- portals .................................................................................................. [Portals: 1]
  |       o- 192.168.100.8:3260 ............................................................................................. [OK]
  o- loopback ....................................................................................................... [Targets: 0]
  o- srpt ........................................................................................................... [Targets: 0]
```

6. Enable iSER

```
/iscsi/iqn.20...68.100.8:3260> pwd
/iscsi/iqn.2020-12.phytium.com.cn/tpg1/portals/192.168.100.8:3260

/iscsi/iqn.20...68.100.8:3260> enable_iser boolean=true
iSER enable now: True

/iscsi/iqn.20...68.100.8:3260> ls /
o- / ....................................................................................................................... [...]
  o- backstores ............................................................................................................ [...]
  | o- block ................................................................................................ [Storage Objects: 1]
  | | o- nvme_disk .................................................................. [/dev/nvme1n1 (1.8TiB) write-thru activated]
  | |   o- alua ................................................................................................. [ALUA Groups: 1]
  | |     o- default_tg_pt_gp ..................................................................... [ALUA state: Active/optimized]
  | o- fileio ............................................................................................... [Storage Objects: 0]
  | o- pscsi ................................................................................................ [Storage Objects: 0]
  | o- ramdisk .............................................................................................. [Storage Objects: 0]
  o- iscsi .......................................................................................................... [Targets: 1]
  | o- iqn.2020-12.phytium.com.cn ...................................................................................... [TPGs: 1]
  |   o- tpg1 ............................................................................................. [no-gen-acls, no-auth]
  |     o- acls ........................................................................................................ [ACLs: 1]
  |     | o- iqn.2020-12.phytium.com.cn:cmcc .................................................................... [Mapped LUNs: 1]
  |     |   o- mapped_lun0 ........................................................................... [lun0 block/nvme_disk (rw)]
  |     o- luns ........................................................................................................ [LUNs: 1]
  |     | o- lun0 ............................................................ [block/nvme_disk (/dev/nvme1n1) (default_tg_pt_gp)]
  |     o- portals .................................................................................................. [Portals: 1]
  |       o- 192.168.100.8:3260 ........................................................................................... [iser]
  o- loopback ....................................................................................................... [Targets: 0]
  o- srpt ........................................................................................................... [Targets: 0]
```

7. Allow access from any server (change ACL)

```
/iscsi/iqn.20...m.com.cn/tpg1> pwd
/iscsi/iqn.2020-12.phytium.com.cn/tpg1

/iscsi/iqn.20...m.com.cn/tpg1> set attribute authentication=0 demo_mode_write_protect=0 generate_node_acls=1 cache_dynamic_acls=1
Parameter demo_mode_write_protect is now '0'.
Parameter authentication is now '0'.
Parameter generate_node_acls is now '1'.
Parameter cache_dynamic_acls is now '1'.

/iscsi/iqn.20...m.com.cn/tpg1> ls /
o- / ....................................................................................................................... [...]
  o- backstores ............................................................................................................ [...]
  | o- block ................................................................................................ [Storage Objects: 1]
  | | o- nvme_disk .................................................................. [/dev/nvme1n1 (1.8TiB) write-thru activated]
  | |   o- alua ................................................................................................. [ALUA Groups: 1]
  | |     o- default_tg_pt_gp ..................................................................... [ALUA state: Active/optimized]
  | o- fileio ............................................................................................... [Storage Objects: 0]
  | o- pscsi ................................................................................................ [Storage Objects: 0]
  | o- ramdisk .............................................................................................. [Storage Objects: 0]
  o- iscsi .......................................................................................................... [Targets: 1]
  | o- iqn.2020-12.phytium.com.cn ...................................................................................... [TPGs: 1]
  |   o- tpg1 ................................................................................................ [gen-acls, no-auth]
  |     o- acls ........................................................................................................ [ACLs: 1]
  |     | o- iqn.2020-12.phytium.com.cn:cmcc .................................................................... [Mapped LUNs: 1]
  |     |   o- mapped_lun0 ........................................................................... [lun0 block/nvme_disk (rw)]
  |     o- luns ........................................................................................................ [LUNs: 1]
  |     | o- lun0 ............................................................ [block/nvme_disk (/dev/nvme1n1) (default_tg_pt_gp)]
  |     o- portals .................................................................................................. [Portals: 1]
  |       o- 192.168.100.8:3260 ........................................................................................... [iser]
  o- loopback ....................................................................................................... [Targets: 0]
  o- srpt ........................................................................................................... [Targets: 0]
```

/etc/target/saveconfig.json:
```json
{
  "fabric_modules": [],
  "storage_objects": [
    {
      "alua_tpgs": [
        {
          "alua_access_state": 0,
          "alua_access_status": 0,
          "alua_access_type": 3,
          "alua_support_active_nonoptimized": 1,
          "alua_support_active_optimized": 1,
          "alua_support_offline": 1,
          "alua_support_standby": 1,
          "alua_support_transitioning": 1,
          "alua_support_unavailable": 1,
          "alua_write_metadata": 0,
          "implicit_trans_secs": 0,
          "name": "default_tg_pt_gp",
          "nonop_delay_msecs": 100,
          "preferred": 0,
          "tg_pt_gp_id": 0,
          "trans_delay_msecs": 0
        }
      ],
      "attributes": {
        "block_size": 4096,
        "emulate_3pc": 1,
        "emulate_caw": 1,
        "emulate_dpo": 1,
        "emulate_fua_read": 1,
        "emulate_fua_write": 1,
        "emulate_model_alias": 1,
        "emulate_rest_reord": 0,
        "emulate_tas": 1,
        "emulate_tpu": 0,
        "emulate_tpws": 0,
        "emulate_ua_intlck_ctrl": 0,
        "emulate_write_cache": 0,
        "enforce_pr_isids": 1,
        "force_pr_aptpl": 0,
        "is_nonrot": 1,
        "max_unmap_block_desc_count": 1,
        "max_unmap_lba_count": 536870911,
        "max_write_same_len": 4294967295,
        "optimal_sectors": 256,
        "pi_prot_format": 0,
        "pi_prot_type": 0,
        "pi_prot_verify": 0,
        "queue_depth": 1023,
        "unmap_granularity": 1,
        "unmap_granularity_alignment": 1,
        "unmap_zeroes_data": 4294967295
      },
      "dev": "/dev/nvme1n1",
      "name": "nvme_disk",
      "plugin": "block",
      "readonly": false,
      "write_back": false,
      "wwn": "608e5e69-d594-4849-983a-2324d02b25a4"
    }
  ],
  "targets": [
    {
      "fabric": "iscsi",
      "tpgs": [
        {
          "attributes": {
            "authentication": 0,
            "cache_dynamic_acls": 1,
            "default_cmdsn_depth": 64,
            "default_erl": 0,
            "demo_mode_discovery": 1,
            "demo_mode_write_protect": 0,
            "fabric_prot_type": 0,
            "generate_node_acls": 1,
            "login_keys_workaround": 1,
            "login_timeout": 15,
            "netif_timeout": 2,
            "prod_mode_write_protect": 0,
            "t10_pi": 0,
            "tpg_enabled_sendtargets": 1
          },
          "enable": true,
          "luns": [
            {
              "alias": "de376b7604",
              "alua_tg_pt_gp_name": "default_tg_pt_gp",
              "index": 0,
              "storage_object": "/backstores/block/nvme_disk"
            }
          ],
          "node_acls": [
            {
              "attributes": {
                "dataout_timeout": 3,
                "dataout_timeout_retries": 5,
                "default_erl": 0,
                "nopin_response_timeout": 30,
                "nopin_timeout": 15,
                "random_datain_pdu_offsets": 0,
                "random_datain_seq_offsets": 0,
                "random_r2t_offsets": 0
              },
              "mapped_luns": [
                {
                  "alias": "ef718d3b52",
                  "index": 0,
                  "tpg_lun": 0,
                  "write_protect": false
                }
              ],
              "node_wwn": "iqn.2020-12.phytium.com.cn:cmcc"
            }
          ],
          "parameters": {
            "AuthMethod": "CHAP,None",
            "DataDigest": "CRC32C,None",
            "DataPDUInOrder": "Yes",
            "DataSequenceInOrder": "Yes",
            "DefaultTime2Retain": "20",
            "DefaultTime2Wait": "2",
            "ErrorRecoveryLevel": "0",
            "FirstBurstLength": "65536",
            "HeaderDigest": "CRC32C,None",
            "IFMarkInt": "Reject",
            "IFMarker": "No",
            "ImmediateData": "Yes",
            "InitialR2T": "Yes",
            "MaxBurstLength": "262144",
            "MaxConnections": "1",
            "MaxOutstandingR2T": "1",
            "MaxRecvDataSegmentLength": "8192",
            "MaxXmitDataSegmentLength": "262144",
            "OFMarkInt": "Reject",
            "OFMarker": "No",
            "TargetAlias": "LIO Target"
          },
          "portals": [
            {
              "ip_address": "192.168.100.8",
              "iser": true,
              "offload": false,
              "port": 3260
            }
          ],
          "tag": 1
        }
      ],
      "wwn": "iqn.2020-12.phytium.com.cn"
    }
  ]
}
```

# 3. Initiator Configuration

```

[root@bogon ~]# yum install iscsi-initiator-utils
[root@bogon ~]# modprobe ib_iser

[root@bogon ~]# iscsiadm -m discovery -t st -p 192.168.100.8:3260
192.168.100.8:3260,1 iqn.2020-12.phytium.com.cn

[root@bogon ~]# iscsiadm -m node -T iqn.2020-12.phytium.com.cn -o update -n iface.transport_name -v iser
[root@bogon ~]# iscsiadm -m node -l
Logging in to [iface: default, target: iqn.2020-12.phytium.com.cn, portal: 192.168.100.8,3260] (multiple)
Login to [iface: default, target: iqn.2020-12.phytium.com.cn, portal: 192.168.100.8,3260] successful.

[root@bogon ~]# iscsiadm -m session
iser: [2] 192.168.100.8:3260,1 iqn.2020-12.phytium.com.cn (non-flash)

[root@bogon ~]# lsscsi --size
[7:0:0:0]    process Marvell  Console          1.01  -               -
[8:0:0:0]    cd/dvd  Byosoft  Virtual CDROM0   1.00  /dev/sr0        -
[9:2:0:0]    disk    AVAGO    MR9361-8i        4.68  /dev/sda    959GB
[10:0:0:0]   disk    LIO-ORG  nvme_disk        4.0   /dev/sdb   2.00TB
```

# 4. vdbench 

cmcc.workload
```
[root@bogon vdbench-aarch64-master]# cat cmcc.workload
hd=default,user=root,shell=ssh
sd=sd1,lun=/dev/sdb,openflags=o_direct,threads=64
wd=wd1,sd=sd1,xfersize=1M,readpct=30,seekpct=70
rd=rd1,wd=wd1,iorate=max,elapsed=120,maxdata=500g,interval=1,warmup=30
```

## 4.1 Mellanox Technologies MT27800 Family (25Gbps == 3125MB/s)
```
[root@bogon vdbench-aarch64-master]# ./vdbench -f cmcc.workload

Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
Vdbench distribution: vdbench50406 Wed July 20 15:49:52 MDT 2016
For documentation, see 'vdbench.pdf'.

16:32:39.847 input argument scanned: '-fcmcc.workload'
16:32:40.174 Starting slave: /opt/vdbench-aarch64-master/vdbench SlaveJvm -m localhost -n localhost-10-201202-16.32.39.783 -l localhost-0 -p 5570
16:32:40.774 All slaves are now connected
16:32:42.002 Starting RD=rd1; I/O rate: Uncontrolled MAX; elapsed=120 warmup=30; For loops: None

Dec 02, 2020  interval        i/o   MB/sec   bytes   read     resp     read    write     resp     resp queue  cpu%  cpu%
                             rate  1024**2     i/o    pct     time     resp     resp      max   stddev depth sys+u   sys
16:35:02.061       140    2156.00  2156.00 1048576  28.48   29.211    7.652   37.796   93.813   22.337  63.4   1.6   0.1
16:35:03.061       141    2117.00  2117.00 1048576  31.27   29.984    8.054   39.961   91.706   23.388  63.4   0.7   0.2
16:35:04.060       142    2126.00  2126.00 1048576  31.09   29.954    8.231   39.755   91.968   22.807  63.4   1.2   0.2
16:35:05.060       143    2137.00  2137.00 1048576  30.18   29.614    7.982   38.966   91.032   23.005  63.5   1.2   0.3
16:35:06.060       144    2142.00  2142.00 1048576  30.07   29.626    7.228   39.255   90.875   23.106  63.4   1.0   0.1
16:35:07.073       145    2132.00  2132.00 1048576  30.16   29.711    7.844   39.154   94.109   23.045  63.4   1.0   0.2
16:35:08.050       146    2126.00  2126.00 1048576  30.34   29.784    8.078   39.238  101.609   22.902  63.3   1.2   0.4
16:35:09.060       147    2144.00  2144.00 1048576  29.80   29.679    7.493   39.099   99.850   22.952  63.5   1.1   0.4
16:35:10.059       148    2137.00  2137.00 1048576  29.85   29.628    8.073   38.802   94.239   22.543  63.4   1.2   0.3
16:35:11.059       149    2121.00  2121.00 1048576  31.35   30.000    8.350   39.889   89.531   23.127  63.4   0.7   0.1
16:35:12.066       150    2142.00  2142.00 1048576  29.55   29.544    7.253   38.895   96.148   22.551  63.5   1.2   0.2
16:35:12.083avg_31-150    2136.74  2136.74 1048576  30.01   29.682    7.794   39.064  106.683   22.591  63.4   1.3   0.2
16:35:13.054 Vdbench execution completed successfully. Output directory: /opt/vdbench-aarch64-master/output
```
> I/O rate = 2136 MB/sec

## 4.2 Intel Corporation 82599ES 10-Gigabit (10Gbps == 1250MB/s)

1. Logout
```
[root@centos7 vdbench-aarch64]# iscsiadm -m node -u
Logging out of session [sid: 1, target: iqn.2020-12.phytium.com.cn, portal: 192.168.100.8,3260]
Logout of [sid: 1, target: iqn.2020-12.phytium.com.cn, portal: 192.168.100.8,3260] successful.

[root@centos7 vdbench-aarch64]# lsscsi
[7:0:0:0]    process Marvell  Console          1.01  -
[8:2:0:0]    disk    AVAGO    MR9361-8i        4.68  /dev/sda
[9:0:0:0]    cd/dvd  Byosoft  Virtual CDROM0   1.00  /dev/sr0
```

2. Change portals
```
/iscsi/iqn.20.../tpg1/portals> pwd
/iscsi/iqn.2020-12.phytium.com.cn/tpg1/portals

/iscsi/iqn.20.../tpg1/portals> delete ip_address=192.168.100.8 ip_port=3260
Deleted network portal 192.168.100.8:3260
/iscsi/iqn.20.../tpg1/portals> create ip_address=192.168.200.8 ip_port=3260
Using default IP port 3260
Created network portal 192.168.200.8:3260.
```

3. Login
```
[root@bogon ~]# systemctl restart target.servcie

[root@bogon ~]# iscsiadm -m discovery -t sendtargets -p 192.168.200.8:3260
192.168.200.8:3260,1 iqn.2020-12.phytium.com.cn

[root@bogon ~]# iscsiadm -m node -T iqn.2020-12.phytium.com.cn -p 192.168.200.8 -l

[root@bogon vdbench-aarch64-master]# lsscsi
[7:0:0:0]    process Marvell  Console          1.01  -
[8:0:0:0]    cd/dvd  Byosoft  Virtual CDROM0   1.00  /dev/sr0
[9:2:0:0]    disk    AVAGO    MR9361-8i        4.68  /dev/sda
[10:0:0:0]   disk    LIO-ORG  nvme_disk        4.0   /dev/sdb

[root@bogon vdbench-aarch64-master]# iscsiadm -m session
tcp: [1] 192.168.200.8:3260,1 iqn.2020-12.phytium.com.cn (non-flash)
```

4. vdbench

```
[root@bogon vdbench-aarch64-master]# ./vdbench -f cmcc.workload

Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
Vdbench distribution: vdbench50406 Wed July 20 15:49:52 MDT 2016
For documentation, see 'vdbench.pdf'.

16:21:35.398 input argument scanned: '-fcmcc.workload'
16:21:35.652 Starting slave: /opt/vdbench-aarch64-master/vdbench SlaveJvm -m localhost -n localhost-10-201202-16.21.35.343 -l localhost-0 -p 5570
16:21:36.228 All slaves are now connected
16:21:38.001 Starting RD=rd1; I/O rate: Uncontrolled MAX; elapsed=120 warmup=30; For loops: None

Dec 02, 2020  interval        i/o   MB/sec   bytes   read     resp     read    write     resp     resp queue  cpu%  cpu%
                             rate  1024**2     i/o    pct     time     resp     resp      max   stddev depth sys+u   sys
16:23:58.058       140     736.00   736.00 1048576  30.57   90.149   48.121  108.654  254.440   45.620  63.5   1.9   0.5
16:23:59.049       141     748.00   748.00 1048576  29.81   81.613   45.065   97.137  212.512   42.283  63.2   1.2   0.3
16:24:00.049       142     660.00   660.00 1048576  30.00   96.629   47.856  117.532  274.577   49.961  63.5   1.1   0.4
16:24:01.050       143     632.00   632.00 1048576  29.91   98.068   50.637  118.303  327.775   52.407  63.5   1.5   0.4
16:24:02.051       144     694.00   694.00 1048576  30.98   94.892   46.901  116.433  295.958   52.117  63.5   1.3   0.3
16:24:03.049       145     793.00   793.00 1048576  29.38   78.364   40.913   93.947  216.102   38.354  63.3   1.3   0.6
16:24:04.050       146     772.00   772.00 1048576  31.87   83.330   43.156  102.118  261.886   43.630  63.4   1.5   0.5
16:24:05.049       147     749.00   749.00 1048576  29.77   85.600   44.076  103.204  246.964   44.166  63.5   1.1   0.5
16:24:06.059       148     715.00   715.00 1048576  30.91   89.418   49.006  107.497  282.091   47.699  63.5   1.6   0.6
16:24:07.060       149     731.00   731.00 1048576  29.96   84.501   40.427  103.354  241.416   43.907  63.3   1.4   0.5
16:24:08.066       150     738.00   738.00 1048576  32.38   87.593   44.270  108.343  274.272   49.586  63.5   1.5   0.4
16:24:08.090avg_31-150     704.31   704.31 1048576  29.89   90.171   47.248  108.472  489.771   47.890  63.5   1.2   0.4
16:24:09.556 Vdbench execution completed successfully. Output directory: /opt/vdbench-aarch64-master/output
```
> I/O rate = 704 MB/sec

## 4.3 PCIe 3.0x4 (32GT/s * (128b/130b) ~= 32Gbps <= 4000MB/s)

```
[root@bogon vdbench-aarch64-master]# nvme list
Node             SN                   Model                                    Namespace Usage                      Format           FW Rev
---------------- -------------------- ---------------------------------------- --------- -------------------------- ---------------- --------
/dev/nvme0n1     BTLJ92030G1X2P0BGN   INTEL SSDPE2KX020T8                      1           2.00  TB /   2.00  TB    512   B +  0 B   VDV10131
/dev/nvme1n1     BTLJ92030FC22P0BGN   INTEL SSDPE2KX020T8                      1           2.00  TB /   2.00  TB      4 KiB +  0 B   VDV10131

[root@bogon vdbench-aarch64-master]# ./vdbench -f cmcc.workload

Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
Vdbench distribution: vdbench50406 Wed July 20 15:49:52 MDT 2016
For documentation, see 'vdbench.pdf'.

16:45:57.282 input argument scanned: '-fcmcc.workload'
16:45:57.630 Starting slave: /opt/vdbench-aarch64-master/vdbench SlaveJvm -m localhost -n localhost-10-201202-16.45.57.218 -l localhost-0 -p 5570
16:45:58.188 All slaves are now connected
16:46:00.001 Starting RD=rd1; I/O rate: Uncontrolled MAX; elapsed=120 warmup=30; For loops: None

Dec 02, 2020  interval        i/o   MB/sec   bytes   read     resp     read    write     resp     resp queue  cpu%  cpu%
                             rate  1024**2     i/o    pct     time     resp     resp      max   stddev depth sys+u   sys
16:48:20.051       140    2469.00  2469.00 1048576  28.88   25.785   10.984   31.795   71.160   10.878  63.7   0.7   0.2
16:48:21.051       141    2442.00  2442.00 1048576  30.75   25.947    6.264   34.689   69.982   13.701  63.7   0.7   0.2
16:48:22.050       142    2431.00  2431.00 1048576  29.66   26.328    6.881   34.528   51.747   13.209  63.7   0.8   0.3
16:48:23.049       143    2462.00  2462.00 1048576  29.73   25.888   10.837   32.256   56.741   10.530  63.7   0.7   0.2
16:48:24.050       144    2395.00  2395.00 1048576  29.85   26.537    5.473   35.501   67.966   14.415  63.6   0.8   0.1
16:48:25.072       145    2471.00  2471.00 1048576  29.87   25.853   10.879   32.230   63.796   10.522  63.8   0.8   0.2
16:48:26.049       146    2423.00  2423.00 1048576  28.39   26.148    9.696   32.671   66.869   11.419  63.6   1.4   0.2
16:48:27.049       147    2396.00  2396.00 1048576  29.47   26.696    4.614   35.921   65.413   14.823  63.8   0.6   0.1
16:48:28.049       148    2496.00  2496.00 1048576  30.93   25.538   11.929   31.632   72.583   10.963  63.7   0.8   0.2
16:48:29.049       149    2410.00  2410.00 1048576  30.37   26.306    6.228   35.064   68.191   14.322  63.6   0.7   0.2
16:48:30.064       150    2469.00  2469.00 1048576  31.43   25.906    8.174   34.034   56.975   12.787  63.7   0.9   0.3
16:48:30.090avg_31-150    2443.31  2443.31 1048576  29.98   26.068    5.389   34.923   76.734   14.107  63.7   0.9   0.2
16:48:31.470 Vdbench execution completed successfully. Output directory: /opt/vdbench-aarch64-master/output
```
>  I/O rate = 2443 MB/sec