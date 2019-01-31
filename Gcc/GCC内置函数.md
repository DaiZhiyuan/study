# 1. 概述

记录常用的GCC内置函数。

## 1.1 __builtin_constant_p

\_\_builtin\_constant\_p是GCC的内建函数，可以用该函数判断传入的参数是否为编译时常量。

如果是编译时常量则该函数返回1，否则返回0。

如果该函数返回0，并不代表参数不是常数，而仅仅是在“-O”优化参数下，GCC编译器无法证明其是否为常量。

```c
#include <stdio.h>

int main(int argc, char **argv)
{
    int var = 3;

    printf("%d\n", __builtin_constant_p(var));
    printf("%d\n", __builtin_constant_p(3));

    return 0;
}
```

## 1.2 __sync_synchronize()

gcc的手册中有一节叫做”Built-in functions for atomic memory access”，然后里面列举了这样一个函数：

```
__sync_synchronize (…)
This builtin issues a full memory barrier.
```

```c
int main(){
  __sync_synchronize();
  return 0;
}
```

```asm
00000000004004cd <main>:
  4004cd: 55                    push   %rbp
  4004ce: 48 89 e5              mov    %rsp,%rbp
  4004d1: 0f ae f0              mfence
  4004d4: b8 00 00 00 00        mov    $0x0,%eax
  4004d9: 5d                    pop    %rbp
  4004da: c3                    retq
  4004db: 0f 1f 44 00 00        nopl   0x0(%rax,%rax,1)
```

> mfence: Serializes all store and load operations that occurred prior to the MFENCE instruction in the 
program instruction stream.


