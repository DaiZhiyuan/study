***qemu命令行help的使用***

[TOC]

# 1. qemu命令行help的使用

## 1.1 查看QEMU全局帮助
```
$> qemu-system-x86_64 --help
```

## 1.2 查看某个模块的帮助信息
```
$> qemu-system-x86_64 -device help
```

## 1.3 查看具体模块指定driver的帮助信息
```
$> qemu-system-x86_64 -device vfio-pci,help
```