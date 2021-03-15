# 1. Overview

本系列文章介绍`ARMv8-A`中的`memory translation`，这是内存管理的核心。
它解释了如何将`virtual addresses`转换为`physical addresses`、页表(`translation table`)格式、以及软件如何使用`Translation Lookaside Buffers(TLB)s`。

这些信息对于`low-level`代码的人都是非常有用的，例如`boot code`或`drivers`。