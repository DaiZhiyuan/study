# Costs of virtualization

虚拟化成本是由`Virtual Machine(VM)`与`hypervisor`切换所需要的时间确定的。
在Arm系统上，这种成本的下限是：
- `31x 64-bit general purpose registers (X0..X30)`
- `32x 128-bit floating point/SIMD registers (V0..V31)`
- `Two stack pointers (SP_EL0, SP_EL1)`

使用`LDP`与`STP`指令，`hypervisor`需要`33`条指令来`save`或`restore`这些寄存器。

> 虚拟化的确切成本取决于`hypervisor`的设计。
