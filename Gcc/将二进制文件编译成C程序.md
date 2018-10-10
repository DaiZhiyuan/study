***将二进制文件编译成C程序***

[TOC]


# 1. 随机生成4K字节文件

```
#> dd if=/dev/random of=./key.bin bs=1 count=4096
4096+0 records in
4096+0 records out
4096 bytes (4.1 kB) copied, 0.168219 s, 24.3 kB/s
```

# 2. 使用objcopy转换一个二进制数据到目标文件
objdump: supported targets: elf64-x86-64 elf32-i386 elf32-iamcu elf32-x86-64 a.out-i386-linux pei-i386 pei-x86-64 elf64-l1om elf64-k1om elf64-little elf64-big elf32-little elf32-big plugin srec symbolsrec verilog tekhex binary ihex

objdump: supported architectures: i386 i386:x86-64 i386:x64-32 i8086 i386:intel i386:x86-64:intel i386:x64-32:intel i386:nacl i386:x86-64:nacl i386:x64-32:nacl iamcu iamcu:intel l1om l1om:intel k1om k1om:intel plugin

> 用法：objcopy -I binary -O <target_format> -B <target_architecture> <binary_file> <object_file>

``` 
$> objcopy -I binary -O elf64-x86-64 -B i386:x86-64:intel ./key.bin key.o
$> file key.o
key.o: ELF 64-bit LSB relocatable, no machine, version 1 (SYSV), not stripped
```  

# 3. 验证目标文件信息

```
$> objdump -h key.o

key.o:     file format elf64-x86-64

Sections:
Idx Name          Size      VMA               LMA               File off  Algn
  0 .data         00001000  0000000000000000  0000000000000000  00000040  2**0
                  CONTENTS, ALLOC, LOAD, DATA
                  
$> nm key.o
0000000000001000 D _binary___key_bin_end
0000000000001000 A _binary___key_bin_size
0000000000000000 D _binary___key_bin_start
```
  
# 4. 测试

## 4.1 编写测试程序

```c
#include <stdio.h>

/*
 * 0000000000001000 D _binary___key_bin_end
 * 0000000000001000 A _binary___key_bin_size
 * 0000000000000000 D _binary___key_bin_start
 */

extern unsigned char _binary___key_bin_start;
extern unsigned char _binary___key_bin_end;
extern unsigned char _binary___key_bin_size;

int main(void)
{
    unsigned char *pblob = &_binary___key_bin_start;

    while(pblob < &_binary___key_bin_end)
    {
        printf("%d: %02X\n", pblob - &_binary___key_bin_start, *pblob);
        pblob++;
    }
    printf("size: %d\n", &_binary___key_bin_size);

    return 0;
}

```
## 4.2 编译测试程序

```
$> gcc main.c key.o -o main
```

## 4.3 验证结果
```
$> ./main
......
4083: F7
4084: 8F
4085: 4A
4086: 0F
4087: A2
4088: 5D
4089: 75
4090: F7
4091: 3B
4092: CA
4093: 55
4094: 36
4095: 70
size: 4096

$> hexdump key.bin
......
0000fd0 bf92 bf7e 5a98 8be0 8940 c355 8e38 8806
0000fe0 9de7 4805 6e90 4767 2120 bcac 2e6a 864f
0000ff0 0752 f761 4a8f a20f 755d 3bf7 55ca 7036
0001000
```