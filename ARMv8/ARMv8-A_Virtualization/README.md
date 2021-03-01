## 概述

本系列文章阐述Armv8-A AArch64的虚拟化支持，包含`stage 2 translation`、`virtual exceptions`与`trapping`。

包括一些基本的虚拟化理论作为介绍，并给出了一些例子阐述`hypervisor`是如何使用这些`features`。不涵盖特定的`hypervisor`，而是尝试解释如何从零编写一个`hypervisor`。
但这两个主题都超出了本系列文章的范围。

在学完本系列文章，你将学会两种类型的`hypervisor`以及它们如何映射到`Arm Exception levels`。
你将能够理解`traps`及`emulate`操作，并可以列出`hypervisor`可以生成那些`virtual exceptions`及理解其实现原理。

> 在你开始之前，假设你有一定的虚拟化基础，了解`Virtual Machine`与`hypervisor`的角色与功能。还需要熟悉`Memory management`中`Exception model`与`Address translation`。
