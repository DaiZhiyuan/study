## Overview

本系列文章介绍了Armv8-A的`excetion model`与`privilege model`。
其中涵盖Arm体系结构中不同类型的异常，以及处理器在产生异常时的行为。

本系列文章对开发`low-level`代码很有帮助，例如`boot code`或`drivers`。

在学完本系列文章，您将能够列出`Exception Level`，并理解它们是如何切换的，以及执行状态的命名与描述。 
当然，您还可以编写一个简单的`AArch64 vector table`和`exception handler`。
 