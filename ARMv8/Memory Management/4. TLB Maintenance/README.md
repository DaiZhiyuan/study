# 1. Translation Lookaside Buffer maintenance

`Translation Lookaside Buffers (TLBs)`缓存最近使用的翻译。
这种缓存允许后续查找时重用翻译，而不需要`wallk tables`。

> 注意：`TLBs`是缓存翻译，而不是缓存翻译表。这种差别很微妙。有多个寄存器字段可控制如何解释翻译表的条目。TLB条目中的内容是对翻译表条目的解释，其中指定了在`wallk tables`时要进行的配置。在`Arm Architecture Reference Manual (Arm ARM)`，这些寄存器字段被称为`permitted to be cached in a TLB`。

如果更改翻译表条目或影响条目解释方式的操作，则需要使TLB中受影响的条目无效。
如果您不使这些条目无效，则处理器可能会继续使用旧的翻译。

不允许处理器将翻译缓存到TLB中，这会导致以下任何错误：
- `translation fault`（未映射的地址）
- `address size fault`（地址超出范围）
- `access flag fault`

因此，首次映射地址时无需发出TLB无效。如果您要执行以下任一操作，则需要TLB无效：
- `Unmap an address`：取以前有效或映射的地址，并将其标记为错误。
- `Change the mapping of an address`：更改输出地址或其中的任何属性。例如，请将地址从只读权限更改为读写权限。
- `Change the way the tables are interpreted`：这并不常见。例如，如果改变了颗粒大小，那么对页表的解释也会发生变化。 因此，TLB无效将是必要的。

## 1.1 Format of a TLB operation

`TLBI`指令用于使`TLBs`中的`invalidate entries`。 本指令的语法为：
```asm
TLBI    <type><level>{IS|OS}    {, <xt>}
```
其中：
- `<type>`：使得哪些`entries`到`invalidate`
    - ALL：所有`entries`
    - VA：匹配`Xt`输入的`VA`与`ASID`
    - VAA：匹配`Xt`输入的`VA`与任意的`ASID`
    - ASID：与`Xt`中的`ASID`相匹配的任何条目
    - 还有更多的信息
- `<level>`：要操作的`address space`
    - EL1：EL0/1虚拟地址空间
    - EL2：EL2虚拟地址空间
    - EL3：EL3虚拟地址空间
- `<IS|OS>`：操作是`Inner Shareable`或`Outer Shareable`
    - `IS`：该操作被广播到`Inner shareable domain`的其他`cores`上。
    - `OS`：该操作被广播到`Outer shareable domain`的其他`cores`上(在`Armv8.4-A`添加)。
- `<Xt>`：要操作的`address`或`ASID`
    - 仅用于描述操作地址或ASID。

例如，操作系统需要更新内核转换表中条目。一个典型的TLB无效看起来是这样的：
```asm
STR     X1, [X5]    // Write to translation table entry
DSB     ISH         // Barrier instructions 
TLBI    VAAE1IS, X0 // Invalidate VA specified by X0, in EL0/1 virtual address space for all ASIDs
DSB     ISH         // Barrier instructions 
ISB                 // Synchronize context on this processor
```

# 2. Address translation instructions

`Address Translation(AT)`指令软件查询特定地址的转换。该翻译结果，包括这些属性，将被写入`Physical Address Register`，`PAR_EL1`。

`AT`指令的语法允许您指定要翻译的`translation regime`。
例如，`EL2`可以查询`EL0/EL1`的`translation regime`。
然而，`EL1`不能使用`AT`指令查询`EL2`的`translation regime`，因为这是违反了权限。

如果请求的翻译会导致`fault`，则不会产生异常。相反，产生的`fault`类型记录在`PAR_EL1`中。
