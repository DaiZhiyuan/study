<!-- TOC -->

- [1. SPDK Target(nvmf)](#1-spdk-targetnvmf)
- [2. Initiator](#2-initiator)
- [3. vdbench](#3-vdbench)

<!-- /TOC -->

# 1. SPDK Target(nvmf)

Environment:
- NIC: Mellanox Technologies MT27800 Family [ConnectX-5]
- SSD: Intel Corporation NVMe Datacenter SSD P4510
- OS: CentOS Linux 7.6 (AltArch)
- SPDK: v21.01-pre git sha1 0d98a94
- DPDK: 20.08.0

1. Build SPDK
```
yum -y install git && \
cd /usr/src && \
git clone https://github.com/spdk/spdk && \
cd spdk && \
git submodule update --init && \
scripts/pkgdep.sh && \
./configure --with-rdma && \
make
```

2. Setup
```
[root@centos7 spdk]# cat /proc/cmdline
BOOT_IMAGE=/vmlinuz-4.14.0-115.el7a.1.1.aarch64 root=/dev/mapper/centos-root ro rd.lvm.lv=centos/root rd.lvm.lv=centos/swap LANG=en_US.UTF-8 selinux=0 audit=0 transparent_hugepage=never kpti=off net.ifnames=0 biosdevname=0 default_hugepagesz=512M hugepagesz=512M hugepages=64

[root@centos7 spdk]# HUGEMEM=32678 ./scripts/setup.sh config
0000:10:00.0 (8086 0a54): Active mountpoints on nvme0n1:nvme0n1p1, so not binding PCI dev
0000:11:00.0 (8086 0a54): nvme -> uio_pci_generic
modprobe: FATAL: Module msr not found.

[root@centos7 spdk]# ./scripts/setup.sh status
Hugepages
node     hugesize     free /  total
node0      2048kB        0 /      0
node0    524288kB        7 /      8
node1      2048kB        0 /      0
node1    524288kB        8 /      8
node2      2048kB        0 /      0
node2    524288kB        8 /      8
node3      2048kB        0 /      0
node3    524288kB        8 /      8
node4      2048kB        0 /      0
node4    524288kB        8 /      8
node5      2048kB        0 /      0
node5    524288kB        8 /      8
node6      2048kB        0 /      0
node6    524288kB        8 /      8
node7      2048kB        0 /      0
node7    524288kB        8 /      8

BDF             Vendor  Device  NUMA    Driver          Device name

NVMe devices
0000:11:00.0    8086    0a54    0       uio_pci_generic         -
0000:10:00.0    8086    0a54    0       nvme                    nvme0

I/OAT Engine

IDXD Engine

virtio

VMD
```

3. startup

```
[root@centos7 bin]# ./nvmf_tgt --cpumask=0xFF --mem-size=16384
[2020-12-02 17:21:13.869982] Starting SPDK v21.01-pre git sha1 0d98a94 / DPDK 20.08.0 initialization...
[2020-12-02 17:21:13.870089] [ DPDK EAL parameters: [2020-12-02 17:21:13.870114] nvmf [2020-12-02 17:21:13.870133] --no-shconf [2020-12-02 17:21:13.870151] -c 0xFF [2020-12-02 17:21:13.870169] -m 16384 [2020-12-02 17:21:13.870186] --log-level=lib.eal:6 [2020-12-02 17:21:13.870204] --log-level=lib.cryptodev:5 [2020-12-02 17:21:13.870221] --log-level=user1:6 [2020-12-02 17:21:13.870238] --base-virtaddr=0x200000000000 [2020-12-02 17:21:13.870255] --match-allocations [2020-12-02 17:21:13.870272] --file-prefix=spdk_pid9311 [2020-12-02 17:21:13.870289] ]
EAL: No available hugepages reported in hugepages-2048kB
EAL: No legacy callbacks, legacy socket not created
[2020-12-02 17:21:15.702314] app.c: 462:spdk_app_start: *NOTICE*: Total cores available: 8
[2020-12-02 17:21:15.760793] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 1
[2020-12-02 17:21:15.760896] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 2
[2020-12-02 17:21:15.760997] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 3
[2020-12-02 17:21:15.761072] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 4
[2020-12-02 17:21:15.761154] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 5
[2020-12-02 17:21:15.761298] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 6
[2020-12-02 17:21:15.761416] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 7
[2020-12-02 17:21:15.761508] reactor.c: 699:reactor_run: *NOTICE*: Reactor started on core 0
[2020-12-02 17:21:15.761552] accel_engine.c: 692:spdk_accel_engine_initialize: *NOTICE*: Accel engine initialized to use software engine.
```
3. rpc
```
[root@centos7 spdk]# modprobe rdma_ucm
[root@centos7 spdk]# bash -x configure.sh
+ scripts/rpc.py nvmf_create_transport -t RDMA -u 8192 -m 8 -c 0
+ scripts/rpc.py bdev_malloc_create -b Malloc0 4000 4096
Malloc0
+ scripts/rpc.py nvmf_create_subsystem nqn.2020-12.io.spdk:node1 -a -s SPDK00000000000001 -d SPDK_Controller1
+ scripts/rpc.py nvmf_subsystem_add_ns nqn.2020-12.io.spdk:node1 Malloc0
+ scripts/rpc.py nvmf_subsystem_add_listener nqn.2020-12.io.spdk:node1 -t rdma -a 192.168.100.8 -s 4420
+ scripts/rpc.py bdev_nvme_attach_controller -b Nvme0 -t PCIe -a 0000:11:00.0
Nvme0n1
+ scripts/rpc.py nvmf_create_subsystem nqn.2020-12.io.spdk:node2 -a -s SPDK00000000000002 -d SPDK_Controller1
+ scripts/rpc.py nvmf_subsystem_add_ns nqn.2020-12.io.spdk:node2 Nvme0n1
+ scripts/rpc.py nvmf_subsystem_add_listener nqn.2020-12.io.spdk:node2 -t rdma -a 192.168.100.8 -s 4420
___________

[root@centos7 spdk]# bash -x configure.sh
+ scripts/rpc.py nvmf_create_transport -t RDMA -u 8192 -m 8 -c 0
+ scripts/rpc.py bdev_malloc_create -b Malloc0 4000 4096
Malloc0
+ scripts/rpc.py nvmf_create_subsystem nqn.2020-12.io.spdk:cmcc_ram -a -s SPDK00000000000001 -d SPDK_Controller1
+ scripts/rpc.py nvmf_subsystem_add_ns nqn.2020-12.io.spdk:cmcc_ram Malloc0
+ scripts/rpc.py nvmf_subsystem_add_listener nqn.2020-12.io.spdk:cmcc_ram -t rdma -a 192.168.100.8 -s 4420
+ scripts/rpc.py bdev_nvme_attach_controller -b Nvme0 -t PCIe -a 0000:11:00.0
Nvme0n1
+ scripts/rpc.py nvmf_create_subsystem nqn.2020-12.io.spdk:cmcc_nvme -a -s SPDK00000000000002 -d SPDK_Controller1
+ scripts/rpc.py nvmf_subsystem_add_ns nqn.2020-12.io.spdk:cmcc_nvme Nvme0n1
+ scripts/rpc.py nvmf_subsystem_add_listener nqn.2020-12.io.spdk:cmcc_nvme -t rdma -a 192.168.100.8 -s 4420

```


4. nvmf.conf
```json
{
  "subsystems": [
    {
      "subsystem": "accel",
      "config": []
    },
    {
      "subsystem": "vmd",
      "config": []
    },
    {
      "subsystem": "sock",
      "config": [
        {
          "method": "sock_impl_set_options",
          "params": {
            "impl_name": "posix",
            "recv_buf_size": 2097152,
            "send_buf_size": 2097152,
            "enable_recv_pipe": true,
            "enable_zerocopy_send": false
          }
        }
      ]
    },
    {
      "subsystem": "interface",
      "config": null
    },
    {
      "subsystem": "bdev",
      "config": [
        {
          "method": "bdev_set_options",
          "params": {
            "bdev_io_pool_size": 65535,
            "bdev_io_cache_size": 256,
            "bdev_auto_examine": true
          }
        },
        {
          "method": "bdev_nvme_set_options",
          "params": {
            "action_on_timeout": "none",
            "timeout_us": 0,
            "retry_count": 4,
            "arbitration_burst": 0,
            "low_priority_weight": 0,
            "medium_priority_weight": 0,
            "high_priority_weight": 0,
            "nvme_adminq_poll_period_us": 10000,
            "nvme_ioq_poll_period_us": 0,
            "io_queue_requests": 512,
            "delay_cmd_submit": true
          }
        },
        {
          "method": "bdev_nvme_attach_controller",
          "params": {
            "name": "Nvme0",
            "trtype": "PCIe",
            "traddr": "0000:11:00.0",
            "prchk_reftag": false,
            "prchk_guard": false
          }
        },
        {
          "method": "bdev_nvme_set_hotplug",
          "params": {
            "period_us": 100000,
            "enable": false
          }
        },
        {
          "method": "bdev_malloc_create",
          "params": {
            "name": "Malloc0",
            "num_blocks": 1024000,
            "block_size": 4096,
            "uuid": "e0dff819-e259-427c-b0a8-1cba85c25830"
          }
        }
      ]
    },
    {
      "subsystem": "nvmf",
      "config": [
        {
          "method": "nvmf_set_config",
          "params": {
            "acceptor_poll_rate": 10000,
            "admin_cmd_passthru": {
              "identify_ctrlr": false
            }
          }
        },
        {
          "method": "nvmf_set_max_subsystems",
          "params": {
            "max_subsystems": 1024
          }
        },
        {
          "method": "nvmf_create_transport",
          "params": {
            "trtype": "RDMA",
            "max_queue_depth": 128,
            "max_io_qpairs_per_ctrlr": 8,
            "in_capsule_data_size": 0,
            "max_io_size": 131072,
            "io_unit_size": 8192,
            "max_aq_depth": 128,
            "max_srq_depth": 4096,
            "no_srq": false,
            "acceptor_backlog": 100,
            "no_wr_batching": false,
            "abort_timeout_sec": 1
          }
        },
        {
          "method": "nvmf_create_subsystem",
          "params": {
            "nqn": "nqn.2020-12.io.spdk:cmcc_ram",
            "allow_any_host": true,
            "serial_number": "SPDK00000000000001",
            "model_number": "SPDK_Controller1"
          }
        },
        {
          "method": "nvmf_subsystem_add_listener",
          "params": {
            "nqn": "nqn.2020-12.io.spdk:cmcc_ram",
            "listen_address": {
              "trtype": "RDMA",
              "adrfam": "IPv4",
              "traddr": "192.168.100.8",
              "trsvcid": "4420"
            }
          }
        },
        {
          "method": "nvmf_subsystem_add_ns",
          "params": {
            "nqn": "nqn.2020-12.io.spdk:cmcc_ram",
            "namespace": {
              "nsid": 1,
              "bdev_name": "Malloc0",
              "uuid": "e0dff819-e259-427c-b0a8-1cba85c25830"
            }
          }
        },
        {
          "method": "nvmf_create_subsystem",
          "params": {
            "nqn": "nqn.2020-12.io.spdk:cmcc_nvme",
            "allow_any_host": true,
            "serial_number": "SPDK00000000000002",
            "model_number": "SPDK_Controller1"
          }
        },
        {
          "method": "nvmf_subsystem_add_listener",
          "params": {
            "nqn": "nqn.2020-12.io.spdk:cmcc_nvme",
            "listen_address": {
              "trtype": "RDMA",
              "adrfam": "IPv4",
              "traddr": "192.168.100.8",
              "trsvcid": "4420"
            }
          }
        },
        {
          "method": "nvmf_subsystem_add_ns",
          "params": {
            "nqn": "nqn.2020-12.io.spdk:cmcc_nvme",
            "namespace": {
              "nsid": 1,
              "bdev_name": "Nvme0n1",
              "uuid": "c83cde13-4a97-475d-bb52-9f4a7e2c1bc0"
            }
          }
        }
      ]
    },
    {
      "subsystem": "nbd",
      "config": []
    }
  ]
}
```

# 2. Initiator 

1. discovery
```
[root@bogon vdbench-aarch64-master]# modprobe nvme-rdma
[root@bogon vdbench-aarch64-master]# nvme discover -t rdma -a 192.168.100.8 -s 4420

Discovery Log Number of Records 2, Generation counter 8
=====Discovery Log Entry 0======
trtype:  rdma
adrfam:  ipv4
subtype: nvme subsystem
treq:    not required
portid:  0
trsvcid: 4420
subnqn:  nqn.2020-12.io.spdk:cmcc_ram
traddr:  192.168.100.8
rdma_prtype: not specified
rdma_qptype: connected
rdma_cms:    rdma-cm
rdma_pkey: 0x0000
=====Discovery Log Entry 1======
trtype:  rdma
adrfam:  ipv4
subtype: nvme subsystem
treq:    not required
portid:  1
trsvcid: 4420
subnqn:  nqn.2020-12.io.spdk:cmcc_nvme
traddr:  192.168.100.8
rdma_prtype: not specified
rdma_qptype: connected
rdma_cms:    rdma-cm
rdma_pkey: 0x0000
```

2. connect

```
[root@bogon vdbench-aarch64-master]# nvme connect -t rdma -n "nqn.2020-12.io.spdk:cmcc_nvme" -a 192.168.100.8 -s 4420
[root@bogon vdbench-aarch64-master]# nvme list
Node             SN                   Model                                    Namespace Usage                      Format           FW Rev
---------------- -------------------- ---------------------------------------- --------- -------------------------- ---------------- --------
/dev/nvme0n1     BTLJ92030G1X2P0BGN   INTEL SSDPE2KX020T8                      1           2.00  TB /   2.00  TB    512   B +  0 B   VDV10131
/dev/nvme1n1     BTLJ92030FC22P0BGN   INTEL SSDPE2KX020T8                      1           2.00  TB /   2.00  TB      4 KiB +  0 B   VDV10131
/dev/nvme2n1     SPDK00000000000002   SPDK_Controller1                         1           2.00  TB /   2.00  TB      4 KiB +  0 B   21.01
```

# 3. vdbench

workload:
```
[root@bogon vdbench-aarch64-master]# cat cmcc.workload
hd=default,user=root,shell=ssh
sd=sd1,lun=/dev/nvme2n1,openflags=o_direct,threads=64
wd=wd1,sd=sd1,xfersize=1M,readpct=30,seekpct=70
rd=rd1,wd=wd1,iorate=max,elapsed=120,maxdata=500g,interval=1,warmup=30
```

runbenchmark:
```
[root@bogon vdbench-aarch64-master]# ./vdbench -f cmcc.workload

Copyright (c) 2000, 2016, Oracle and/or its affiliates. All rights reserved.
Vdbench distribution: vdbench50406 Wed July 20 15:49:52 MDT 2016
For documentation, see 'vdbench.pdf'.

17:44:03.140 input argument scanned: '-fcmcc.workload'
17:44:03.467 Starting slave: /opt/vdbench-aarch64-master/vdbench SlaveJvm -m localhost -n localhost-10-201202-17.44.03.078 -l localhost-0 -p 5570
17:44:03.952 All slaves are now connected
17:44:05.001 Starting RD=rd1; I/O rate: Uncontrolled MAX; elapsed=120 warmup=30; For loops: None

Dec 02, 2020  interval        i/o   MB/sec   bytes   read     resp     read    write     resp     resp queue  cpu%  cpu%
                             rate  1024**2     i/o    pct     time     resp     resp      max   stddev depth sys+u   sys
17:46:25.060       140    2238.00  2238.00 1048576  29.80   28.448   22.487   30.979  376.343   50.317  63.6   1.4   0.2
17:46:26.051       141    2228.00  2228.00 1048576  31.15   28.218   23.507   30.349  446.971   55.617  63.6   0.9   0.3
17:46:27.060       142    2220.00  2220.00 1048576  29.64   29.237   21.963   32.301  404.183   59.492  63.6   1.7   0.8
17:46:28.060       143    2223.00  2223.00 1048576  30.00   28.329   22.019   31.034  341.951   43.352  63.6   1.1   0.4
17:46:29.060       144    2230.00  2230.00 1048576  31.48   28.071   23.989   29.946  343.353   53.135  63.7   0.9   0.4
17:46:30.073       145    2259.00  2259.00 1048576  28.82   29.644   22.998   32.335  374.342   54.431  63.7   1.1   0.5
17:46:31.049       146    2183.00  2183.00 1048576  31.24   28.467   22.634   31.118  471.724   48.192  63.5   2.4   0.9
17:46:32.059       147    2256.00  2256.00 1048576  29.43   27.515   21.815   29.892  477.645   50.322  63.7   1.3   0.5
17:46:33.060       148    2251.00  2251.00 1048576  28.43   29.041   24.747   30.746  374.027   48.754  63.6   1.1   0.3
17:46:34.050       149    2266.00  2266.00 1048576  29.48   27.761   21.564   30.351  427.687   51.295  63.6   0.9   0.3
17:46:35.085       150    2268.00  2268.00 1048576  29.37   28.058   22.932   30.190  301.354   50.011  64.0   1.0   0.4
17:46:35.163avg_31-150    2232.21  2232.21 1048576  29.99   28.504   23.495   30.650  540.899   49.877  63.6   1.2   0.5
17:46:36.338 Vdbench execution completed successfully. Output directory: /opt/vdbench-aarch64-master/output
```
>  I/O rate = 2232 MB/sec