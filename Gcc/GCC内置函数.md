- 1 概述
- 1.1 __builtin_constant_p

# 1. 概述

记录Linux Kernel常用的GCC内置函数。

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

