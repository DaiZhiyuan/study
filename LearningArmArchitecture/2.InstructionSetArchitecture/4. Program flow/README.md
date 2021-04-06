# 1. Program flow

通常，处理器按程序顺序执行指令。
这意味着处理器会按照与在内存中设置的指令相同的顺序来执行这些指令。
更改此顺序的一种方法是使用`branch`指令。

分支指令可以改变程序流`(Program flow)`，用于判断(`decisions`)、循环(`loops`)与函数调用(`function calls`)。

`A64`指令集也有一些条件分支`(conditional branch)`指令。

## 1.1 Loops and decisions

在本节中，我们将研究循环和判断是如何使用分支指令来更改程序执行流程。
分支指令有两种类型：无条件指令(`unconditional`)和条件指令(`conditional`)。

## 1.1.1 Unconditional branch instructions

无条件分支指令`B<label>`提供一个`PC-relative`的直接跳转`(<label>)`。
从`current PC`到`destination`的`offset`会被编码到指令当中。
范围收到指令内用于记录偏移量的空间限制`(+/- 128MB)`。

当你使用`BR <Xn>`指令时时，`BR`会对在`Xn`中指定的相对地址或绝对地址进行`branch`。

## 1.1.2 Conditional branch instructions

条件分支指令`B.<cond> <label>`是`B`指令的条件版。
该指令只在`cond`条件为真时，才会执行`branch`动作。
跳转范围被限制在`(+/- 1MB)`以内。

条件测试其实是对`PSTATE`寄存器中的`ALU`标志进行测试，并需要在指令前生成测试指令，例如`compare(CMP)`。
```asm
CBZ <Xn> <label> and CBNZ <Xn> <label>
```
如果`Xn`是`0`即(`CBZ`)，则此指令`branch`到<`label`>。
如果`Xn`不是`0`(`CBNZ`)，则此指令`branch`到<`label`>。

```asm
TBZ <Xn>, #<imm>, <label> and TBNZ <Xn>, #<imm>, <label>
```
`TBX`条件分支指令的工作方式与`CBZ/CBNZ`类似，但测试比较的值是`#<imm>`。

下列这些示例展示了如何使用分支指令，是如何用于循环与判断，我们以C代码与编译器的典型输出为例：

code1:
```c
    if ( a == 5)
        b = 5;
```
compiler1:
```asm
    CMP W0, #5
    B.NE skip
    MOV W8, #5
skip:
    ...
```
code2:
```c
    while ( a != 0) {
        b = b + c;
        a = a - 1;
    }
```
compiler2:
```asm
loop:
    GBZ W8, skip
    ADD W9, W9, W10;
    SUB W8, W8, #1
    B loop
skip:
    ...
```

> 注意：输出中显示的标签不会由编译器创建。它们包括在这里，以帮助可读性。

## 1.2 Generating condition code

在前面我们已经知道，条件测试是对`PSTATE`寄存器中的`ALU`标志进行测试。

数据处理指令的执行可能会影响`ALU`标志位。重点是，以`S`结尾的指令，会更新`ALU`标志位。
这是一个不更新ALU标志的指令示例：
```asm
ADD     X0, X1, X2
```
这是一个使用`S`，更新`ALU`标志的指令示例：
```
ADDS    X0, X1, X2
```
此方法允许软件控制标志更新或不更新。
标志可以在后续条件指令中使用。 
让我们以下面的代码为例：
```asm
SUBS    X0, X5, #1
AND     X1, X7, X9
B.EQ    label
```

`SUBS`指令执行减法并更新`ALU`标志。
然后，`AND`指令执行逻辑与，并且不更新`ALU`标志。
最后，`B.EQ`指令执行条件分支，使用减法结果设置的`ALU`标志。

`ALU flag`是：
- `N(Negative)`
- `C(Carry)`
- `V(Coverflow)`
- `Z(Zero)`

让我们以`Z(Zero)`标志为例。如果操作结果为`Zero`，则`Z`标志设置为`1`。 
例如，如果`X5`为`1`，则`Z`标志设置为`1`，否则`Z`标志设置为`0`。
```asm
SUBS    X0, X5, #1
```

分支指令的条件码`condition codes`会映射到`ALU flag`上，并且会成对出现。
以`EQ(equal)`与`NE(not equal)`为例。看看它是如何映射到`ALU`的`Z`标志的：
`EQ`条件码是`check for Z==1`；`NE`条件码是`check for Z==0`。

以下列代码为例：
```asm
SUBS    W0, W7, W9
W0 = W7 - W9, Z
B.EQ    label
```
在第一行，是一个减法运算。
在第二行，如果结果为零，则设置`Zero`标志。
在最后一行，如果`Z==1`，则`branch`到`label`。

如果`w7==w9`，则减法的结果将为零，并且`Z`标志将被置`1`。
因此，如果`w7`和`w9`相等，则`branch`到`label`。

除了常规的数据处理指令外，还有其他指令只更新ALU标志：
- `CMP(Compare)`
- `TST(Test)`

这些说明这些指令是其他指令的别名。例如：
```asm
CMP X0, X7  // alias of SUBS XZR, X0, X7
TST W5, #1  // alias of ANDS WZR, W5, #1
```
通过使用`Zero`寄存器作为目标寄存器，我们期望丢弃操作结果，只更新`ALU`标志。

## 1.3 Conditional select instructions

到目前为止，我们已经看到了使用分支来处理判断的例子。
`A64`指令集还提供条件选择指令。
在许多情况下，这些指令可以作为分支的替代。

条件选择指令的变形比较多，单基本形式是：
```asm
CSEL    Xd, Xn, Xm, cond
```
意思是：
```c
if (cond == TRUE)
    Xd = Xn
else 
    Xd = Xm
```
你可以在此代码中看到一个示例：
```asm
CMP     W1, #0
CSEL    W5, W6, W7, EQ
```
意思是：
```c
if (W1 == 0)
    W5 = W6
else
    W5 = W7
```
如果有条件地增加，你可以写：
```asm
CSINC   X0, X0, X0, cond
```
等价于：
```c
if (cond == TRUE)
    X0 = X0
else
    X0 = X0 + 1
```

Arm体系结构为该命令提供了别名`CINC`。然而，请注意，`CINC`颠倒了`CSINC`的逻辑：
- `CSINC X0, X0, X0, cond`
    - 条件为真，则`X0`保持不变。
    - 计算条件为假，则自增`X0`。
- `CINC  X0, X0, cond`
    - 条件为真，则自增`X0`
    - 条件为假，则`X0`保持不变。

编译器选择最有效的方法来实现程序中的功能。
编译器处理器`if ... else`代码块，通常会采用条件选择，因为条件选择指令相比条件分支更高效。

Case1(C):
```c
if (a != 0) 
    b = b + 1;
```
Case1(Branch):
```asm
CMP  W0, #0
    b.EQ else
    ADD W1, W1, #1
else:
    ...
```
Case1(Conditional Select):
```asm
CMP  W0, #0
CINC W1, W1, NE
```
Case2(C):
```c
if (a == 0)
    y = y + 1;
else
    y = y - 1;
```
Case2(Branch):
```asm
CMP  W0, #0
    B.NE else
    ADD W1, W1, #1
    B end
else:
    SUB W1, W1, #1
end:
    ...
```
Case2(Conditional Select)
```asm
CMP     W0, #0
SUB     W2, W1, #1
CSINC   W1, W2, W1, NE
```
在这些类型的示例中，条件选择的优点是指令序列更短。更重要的是，条件选择会代替分支的需要。
在现代处理器中，这种分支可能很难让分支预测逻辑进行正确的预测。
一个预测错误的分支可能会对性能产生负面影响，因此你的代码最好尽可能地少使用分支改为条件选择指令。
