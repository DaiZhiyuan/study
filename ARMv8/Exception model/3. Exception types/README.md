# Exception types

异常是一种任何可能导致当前正在执行的程序被`suspend`的事件，随后改变执行状态去处理该事件。
其他处理器架构可能将此描述为中断。
在Armv8-A体系结构中，`interrupt`是一种外部产生的异常。
Armv8-A架构将异常分为两类：`synchronous exceptions`与`asynchronous exceptions`。

## 1.1 Synchronous exceptions

`Synchronous exceptions`可能是由刚刚执行的指令引起的异常。
这意味着同步异常是与执行流是同步的。

同步异常可能是由于试图执行`invalid instruction`而引起的，无论是当前异常级别不允许的执行的指令还是已禁用的指令。

同步异常也可能是由于内存访问造成的，原因要么是地址不对齐，要么是MMU权限检查失败。
由于这些错误是同步的，异常可以在尝试内存访问之前发生。
当然内存访问也可以产生异步异常，我们在后续章节讨论。
内存访问错误将在内存管理文章中详细讨论。

Armv8-A架构有一族用来产生异常的指令：`SVC, HVC, and SMC`。
它们针对不同的异常级别，并且它们的优先级不同。
这些指令用于实现`system call interface`，允许`less privilege code`去请求`more privileged code`的服务。

`Debug exceptions`也是同步异常。

## 1.2 Asynchronous exceptions

有些些类型的异常是外部生成的，因此与当前指令流不同步。
这意味着不可能确切地知道异步异常何时会发生。
Armv8-A体系结构只要求它在有限的时间内发生。

异步异常也可以暂时的屏蔽。
这意味着异步异常在异常发生之前可以处于`pending`状态。

异步异常类型有：
- Physical interrupts
    - SError(System Error)
    - IRQ
    - FIQ
- Virtual Interrupts
    - vSError(Virtual System Error)
    - vIRQ(Virtual IRQ)
    - vFIQ(virtual FIRQ)

物理中断是响应`PE`以外产生的信号。虚拟中断可以在外部生成，也可以由在`EL2`中执行的软件生成的。
我们会在虚拟化相关文章中讨论虚拟中断。先让我们看看不同类型的物理中断。

## 1.3 IRQ and FIQ

Armv8-A体系结构有两种物理中断类型（`IRQ`和`FIQ`），用于`generate peripheral interrupts`。
在ARM体系结构的其他版本中，`FIQ`被用作更高优先级的快速中断。
这与Armv8-A不同，其中`FIQ`与`IRQ`具有相同的优先级。

## 1.4 SError

`SError`是一种异常类型，它是为了响应错误的内存访问，它是由内存系统产生的。

`SError`一个典型用法是`asynchronous abort`，例如，内存访问，它通过了所有MMU检查，但在内存总线上遇到错误。
这是异步报告。
`SError`也可能是由奇偶校验或错误校正代码(ECC)检查一些RAM，例如，内置缓存。