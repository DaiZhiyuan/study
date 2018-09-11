**Linux内核模块**
- 1 概述
    - 1.1 Hello World
- 2 模块操作
    - 2.1 构建模块
        - 2.1.1 放在内核源码树中
        - 2.1.2 放在内核源码树外
    - 2.2 安装模块
    - 2.3 生成模块依赖性
    - 2.4 装载模块
    - 2.5 Kconfig管理选项
    - 2.6 模块参数
    - 2.7 导出符号表
    - 2.8 获取模块相关信息

# 1 概述

&nbsp;&nbsp;&nbsp;&nbsp; 尽管Linux是“单内核（monolithic）”的操作系统————这是说整个系统内核都运行与一个单独保护域中，但是Linux内核时模块化组成的，它允许内核在运行时动态地向其中插入或从中删除代码。这些代码（包括相关的子例程、数据、函数入口和函数出口）被一并组合在一个单独的二进制映像中，即所谓的可装载内核模块中，或简称为模块。

&nbsp;&nbsp;&nbsp;&nbsp; 支持模块的好处是基本内核映像可以尽可能地小，因为可选的功能和驱动程序可以利用模块形式再提供。模块允许我们方便地删除和重新装入内核代码，也方便了调试工作。而且当热插拔新设备时，可通过命令载入新的驱动程序。

## 1.1 Hello World

&nbsp;&nbsp;&nbsp;&nbsp; 内核模块开发接近编写新的应用系统，因为至少在模块文件中具有入口和出口点。虽然编写“hello，world”程序作为实例实属陈词滥调了，但它的确让人喜爱。内核模块Hello，world出场了：

```cpp
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

static int hello_init(void)
{
    printk(KERN_ALERT "Hi.\n");
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Bye.\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jerry Dai");
MODULE_DESCRIPTION("A Hello, World Module");
```
&nbsp;&nbsp;&nbsp;&nbsp; 这大概是我们所能见到的最简单的内核模块了，hello_init()函数是模块的入口点，他通过module_init()例程注册到系统中，在内核装载时被调用。调用module_init()实际上不是真正的调用，而是一个宏调用，它唯一的参数便是模块的初始化函数。模块的所有初始化函数必须符合下面的形式：

> int my_init(void);

&nbsp;&nbsp;&nbsp;&nbsp; 因为init函数通常不会被外部函数直接调用，所以你不必导出该函数，故它可标记为static类型。

&nbsp;&nbsp;&nbsp;&nbsp; init函数会返回一个init型数值，如果初始化顺利完成，那么它的返回值为零；否则返回一个非零值。

&nbsp;&nbsp;&nbsp;&nbsp; 这个init函数仅仅打印了一条简单的消息，然后返回零。在实际的模块中，init函数会注册资源、初始化硬件、分配数据结构等。如果这个文件被静态编译进内核映像中，其init函数将放在内核映像中，并在内核启动时执行。

&nbsp;&nbsp;&nbsp;&nbsp; hello_exit()函数是在模块的出口函数，它由module_exit()例程注册到系统。在模块从内存卸载时，内核便会调用hello_exit()。退出函数可能会在返回前负责清理资源，比保证硬件处于一致状态；或者做其他的一些操作。简单说来，exit函数负责对init函数以及模块声明周期过程中所做一切事情进行撤销操作，基本上就是清理工作。在退出函数返回后，该模块就被卸载了。

&nbsp;&nbsp;&nbsp;&nbsp; 退出函数必须符合以下形式：

> void my_init(void);

&nbsp;&nbsp;&nbsp;&nbsp; 与init函数一样，你也可以标记其为static。

&nbsp;&nbsp;&nbsp;&nbsp; 如果上述文件被静态地编译到内核映像中，那么退出函数将不被包含，而且永远都不会被调用（因为如果不是编译成模块的话，那么代码就不需要从内核中卸载）。

&nbsp;&nbsp;&nbsp;&nbsp; MODULE_LICENSE()宏用于指定模块的版权。如果载入非GPL模块到系统内存，则会在内核中设置被染污标识————这个标识只起到记录信息的作用。版权许可证具有两大目的。首先，它具有通告的目的。当oops中设置了被染污的标识时，很多内核开发者对bug的报告缺乏信任，因为他们认为二进制模块（也就是开发者不能调试它）被装载到了内核。其次，非GPL模块不能调用GPL_only符号。

&nbsp;&nbsp;&nbsp;&nbsp; 最后还要说明，MODULE_AUTHOR()宏和MODULE_DESCRIPTION()宏指定了代码作者和模块的简要描述，它们完全是用于信息记录目的。

# 2 模块操作

## 2.1 构建模块

&nbsp;&nbsp;&nbsp;&nbsp; 内核采用“Kbuild”构建系统，构建过程的第一步是决定在哪里管理模块源码。你可以把模块代码加入到内核源码树中，或者是作为一个部分或者是最终把你的代码合并到正式的内核源码树中；另外一种可行的方式是在内核源码树之外维护和构建你的模块源码。

### 2.1.1 放在内核源码树中

&nbsp;&nbsp;&nbsp;&nbsp; 最理想的情况莫过于你的模块正式成为Linux内核的一部分，这样就会被存放入内核源代码树中。把你的模块代码正确地置于内核中，开始的时候难免需要更多的维护，但这样通常是一劳永逸的解决之道。

&nbsp;&nbsp;&nbsp;&nbsp; 当你决定了把你的模块放入内核代码树中，下一步要清除你的模块应在内核源码树中处于何处。设备驱动程序存放在内核源码树根目录下/drivers的子目录，在其内部，设备驱动文件被进一步按照类型、类型或特殊驱动程序等更有序组织起来。如字符设备存在于drivers/char/目录下，而块设备存放在drivers/block/目录下，USB设备则存放在drivers/usb/目录下。文件的组织规则并不必须绝对墨守成规，不容打破，你可以看到许多USB设备也属于字符设备。但是不管怎么样，这些组织关系对我们来说相当容易理解，并且很也很准确。

&nbsp;&nbsp;&nbsp;&nbsp; 假设你有一个字符设备，而且希望将它存放在drivers/char/目录下，那么要注意，在该目录下同时会存在大量的C源代码文件和许多其他目录。所以对于仅仅只有一两个源文件的设备驱动程序，可以直接存放在该目录下；但如果驱动程序包含许多源文件和其他辅助文件，那么就可以创建一个新的子目录。这期间并没有什么金科玉律。假设想建立自己代码的子目录，你的驱动程序是一个钓鱼竿和计算机的接口，名为Fish Master XL 3000，那么你就需要在drivers/char/目录下建立一个名为fishing的子目录。

&nbsp;&nbsp;&nbsp;&nbsp; 接下来需要向drivers/char/下的Makefile文件添加一行。编辑drivers/char/Makefile并加入：

```Makefile
obj-m += fishing/
```

&nbsp;&nbsp;&nbsp;&nbsp; 这行编译指令告诉模块构建系统，在编译模块时需要进入fishing/子目录中。更可能发生的情况是，你的驱动程序的编译取决于一个特殊配置选项；比如，可能的CONFIG_FISHING_POLE。如果这样，你就需要在下面指令代替刚才那条指令：

```Makefile
obj-$(CONFIG_FISHING_POLE) += fishing/
```

&nbsp;&nbsp;&nbsp;&nbsp; 最后，在drivers/char/fishing/下，需要添加一个新Makefile文件，其中需要有下面这行指令：

```Makefile
obj-m += fishing.o
```
&nbsp;&nbsp;&nbsp;&nbsp; 一切就绪了，此刻构建系统运行将会进入fishing/目录下，并且将fishing.c编译为fishing.ko模块。虽然你写的扩展名时.o，但是模块被编译后的扩展名是.ko。

&nbsp;&nbsp;&nbsp;&nbsp; 再一个可能，要是你的钓鱼竿驱动程序编译时有编译选项，那么你可能需要这么来做：

```makefile
obj-$(CONFIG_FISHING_POLE) += fishing.o
```

&nbsp;&nbsp;&nbsp;&nbsp; 以后，假如你的钓鱼竿驱动程序需要更加智能化————它可以自动检测钓鱼线，这可能是最新的鱼竿“必备需求”呀。这是驱动程序源文件可能就不再只有一个了。别怕，朋友，你只要把你的Makefile做如下修改就可搞定：

```makefile
obj-$(CONFIG_FISHING_POLE) += fishing.o
fishing-objs := fishing-main.o fishing-line.o
```

&nbsp;&nbsp;&nbsp;&nbsp; 每当设置了CONFIG_FISHING_POLE，fishing-main.c和fishing-line.c就一起呗编译和链接到fishing.ko模块内。

&nbsp;&nbsp;&nbsp;&nbsp; 最后一个注意事项是，在构建文件时你可能需要额外的编译标记，如果这样，你只需在Makefile中添加如下指令：

```makefile
EXTRA_CFLAGS += -DTITANIUM_POLE
```

&nbsp;&nbsp;&nbsp;&nbsp; 如果喜欢把你的源文件至于drivers/char/目录下，并且不建立新目录的话，那么你要做的便是将前面提到的行（也就是原来处于drivers/char/fishing/下你自己的Makefile中的）都加入到drivers/char/Makefile中。

&nbsp;&nbsp;&nbsp;&nbsp; 开始编译吧，运行内核构建过程和原来一样。如果你的模块编译取决于配置选项，比如有CONFIG_FISHING_POLE约束，那么在编译前首先要确保选项被允许。

### 2.1.2 放在内核源码树外

&nbsp;&nbsp;&nbsp;&nbsp; 如果你喜欢脱离内核代码树来维护和构建你的模块，那你要做的就是在自己的源码树目录中建立一个Makefile，它只需要一行指令：

```makefile
obj-m := fishing.o
```

&nbsp;&nbsp;&nbsp;&nbsp; 这条指令就可把fishing.c编译成fishing.ko。如果你有多个源文件，那么用两行就足够：

```makefile
obj-m := fishing.o
fishing-objs := fishing-main.o fishing-line.o
```

&nbsp;&nbsp;&nbsp;&nbsp; 这样一来，fishing-main.c和fishing-line.c就一起被编译和链接到fishing.ko模块内了。
&nbsp;&nbsp;&nbsp;&nbsp; 模块在内核内核在内核外构建的最大区别在于构建过程。当模块在内核源码树外围时，你必须告诉make如何找到内核源代码文件和基础Makefile文件。不过要完成这个工作同样不难：

```bash
make -C /kernel/source/location SUBDIRS=$PWD modules
```

&nbsp;&nbsp;&nbsp;&nbsp; 在这个例子中，/kernel/source/location是你配置的内核源码树。回想一下，不要把要处理的内核源码树放在/usr/src/linux下，而要移到你的home目录下某个方便访问的地方。 
    
## 2.2 安装模块

&nbsp;&nbsp;&nbsp;&nbsp; 编译后的模块将被装入到目录/lib/modules/$VERSION/kernel/下，在kernel/目录下的每一个目录都对应着内核源码树中的模块位置。如果使用的是3.10.0-693.el7.x86_64内核，而且将你的模块源代码直接放在drivers/char/下，那么编译后的钓鱼竿驱动程序的存放路径是：/lib/modules/3.10.0-693.el7.x86_64/kernel/drivers/char/fishing.ko。

&nbsp;&nbsp;&nbsp;&nbsp; 下面的构建命令用来安装编译的模块到合适的目录下：
> $> make modules_install

&nbsp;&nbsp;&nbsp;&nbsp; 通常需要以root权限运行。

## 2.3 生成模块依赖性

&nbsp;&nbsp;&nbsp;&nbsp; Linux模块之间存在依赖性，也就是说钓鱼模块依赖于鱼饵模块，那么当你载入钓鱼模块时，鱼饵模块会被自动载入。这里需要的依赖信息必须事先生成。多数Linux发行版都能自动生成这些依赖关系信息，而且在每次启动时更新。若想要生成内核依赖关系信息，root用户可运行命令：

> $> depmod

&nbsp;&nbsp;&nbsp;&nbsp; 为了执行更快的更新操作，那么可以只为新模块生成依赖信息，而不是生成所有的依赖关系，这时root用户可运行命令：

> $> depmod -A

&nbsp;&nbsp;&nbsp;&nbsp; 模块依赖关系存放在/lib/modules/$VERSION/modules.dep文件中。

## 2.4 装载模块

&nbsp;&nbsp;&nbsp;&nbsp; 载入模块最简单的方法是通过insmod命令，这是个功能有限的命令，它能做的就是请求内核载入指定模块。insmod程序不执行任何依赖性分析或进一步的错误检查。它用法简单，以root身份运行命令：

> $> insmod module.ko

&nbsp;&nbsp;&nbsp;&nbsp; 类似的，卸载一个模块，你可以使用rmmod命令，它同样需要以root身份运行：

> $> rmod module

&nbsp;&nbsp;&nbsp;&nbsp; 这两个命令很简单，但是它们一点也不智能。先进工具modprobe提供了模块依赖性分析、错误只能检查、错误报告以及许多其他功能和选项。强烈建议大家用这个命令，使用modprobe插入模块，需要以root身份运行：

> $> modprobe module [ module parameters ]

&nbsp;&nbsp;&nbsp;&nbsp; 其中，参数module指定了需要装入的模块名称，后面的参数将在模块载入时传入内核。

&nbsp;&nbsp;&nbsp;&nbsp; modprobe命令不但会加载指定的模块，而且会自动加载任何它所依赖的有关模块。所以说它是加载模块的最佳机制。

&nbsp;&nbsp;&nbsp;&nbsp; modprobe命令页可以用来从内核中卸载模块，当然这也需要以root身份运行：

> $> modprobe -r modules 

&nbsp;&nbsp;&nbsp;&nbsp; 参数modules指定一个或多个需要卸载的模块。与rmmod命令不同，modprobe也会卸载给定模块所依赖的相关模块，但其前提是这些相关模块没有被使用。

## 2.5 Kconfig管理选项

&nbsp;&nbsp;&nbsp;&nbsp; 在前面的内容中我们看到，只要设置了CONFIG_FISHING_POLE配置选项，钓鱼竿模块就将被自动编译。虽然配置选项在前面已经讨论过了，但这里我们将继续以钓鱼竿驱动为例，再看看一个新的配置选项如何加入。

&nbsp;&nbsp;&nbsp;&nbsp; 内核进入了Kbuild系统，因此，加入一个新配置选项现在可以说是易如反掌。你所需要做的全部就是向Kconfig文件中添加一项，用以对应内核源码树。对驱动程序而言，Kconfig通常和源码处于同一目录。如果钓鱼竿驱动程序在目录drivers/char/下，那么你便会发现drivers/char/kconfig也同时存在。

&nbsp;&nbsp;&nbsp;&nbsp; 如果你建立一个新子目录，并且也希望kconfig文件存在于该目录中的话，那么你必须在一个已存在的kconfig文件中将它进入。你需要加入下面一行命令：

```Kconfg
source "drivers/char/fishing/Kconfig"
```

&nbsp;&nbsp;&nbsp;&nbsp; 这里所谓存在的Kconfig文件可能是drivers/char/Kconfig。

&nbsp;&nbsp;&nbsp;&nbsp; Kconfig文件很方便加入一个配置选项，请看钓鱼竿模块的选项：

```Kconfig
config FISHING_POLE
    tristate "Fish Master 3000 support"
    defalut n
    help 
        if you say Y here, support for the Fish Master 3000 with computer
        interface will be compiled into the kernel and accessible via a
        device node. You can also say M here and the driver will be built as
        a module named fishing.ko
        
        If unsure, say N.
```

&nbsp;&nbsp;&nbsp;&nbsp; 配置选项第一行定义了该选项所代码的配置目标。注意CONFIG_前缀并不需要写上。

&nbsp;&nbsp;&nbsp;&nbsp; 第二行声明类型为tristate，也就是说可以编译进内核（Y），也可以作为模块编译（M）或者干脆不编译它（N）。如果编译选项代表的是一个系统功能，而不是一个模块，那么编译选项将用bool指令代替tristate，这说明它不允许被编译成模块。处于指令后的引号内文件为该选项指令了名称。

&nbsp;&nbsp;&nbsp;&nbsp; 第三行指定了该选项的默认选项，这里默认操作是不编译它（N）。也可以把默认选项指定为编译进内核（Y），或者编译成一个模块（M）。对驱动程序而言，默认选择通常为不编译进内核（N）。

&nbsp;&nbsp;&nbsp;&nbsp; Help指令的目的是为该选项提供帮助文档。各种配置工具都可以按要求显示这些帮助信息。应为这些帮助是面向编译的用户和开发者的，所以帮助内容简洁扼要。一般的用户通常不会编译内核。

&nbsp;&nbsp;&nbsp;&nbsp; 除了上述选项以外，还存在其他选项。比如depends指令规定了在该选项被设置前，首选要设置的选项。加入依赖性不满足时，那么该选项就会被禁止。比如，如果你加入命令：

```Kconfig
depends on FISH_TANK
```

&nbsp;&nbsp;&nbsp;&nbsp; 到配置选项中，那么就意味着在CONFIG_FISH_TANK被选择前，我们的钓鱼竿模块时不能使用（Y或者M）。Select指令和depends类似，它们只有一点不同之处————只要是select指定了谁，它就会强行将被指定的选项打开。所以这个指令可不能像depends那样滥用一通，因为它会自动的激活其他配置选项。它的用法和depends一样。比如：

```Kconfig
select BAIT
```

&nbsp;&nbsp;&nbsp;&nbsp; 意味着当CONFIG_FISHING_POLE被激活时，配置选项CONFIG_BAIT必然一起被激活。

&nbsp;&nbsp;&nbsp;&nbsp; 如果select和depends同时指定多个选项，那就需要通过&&指令来进行多选。使用depends时，你还可以利用!前缀来指明禁止某个选项。比如：

```Kconfig
depends on EXAMPLE_DRIVERS &&  !NO_FISHING_ALLOWED
```

&nbsp;&nbsp;&nbsp;&nbsp; 这行指令就指定驱动程序安装要求打开CONFIG_EXAMPLE_DRIVERS选项，同时要禁止CONFIG_NO_FISHING_ALLOWED选项。

&nbsp;&nbsp;&nbsp;&nbsp; tristate和bool选项往往会结合if指令一起使用，这表示某个选项取决于另一个配置选项。如果条件不满足，配置选项不但会被禁止，甚至不会显示在配置工具中，比如，要求系统只有在CONFIG_64BIT配置选项时才显示某选项。请看下面命令：

```Kconfig
config 64BIT
    bool "64-bit kernel" if ARCH = "x86"
    default ARCH != "i386"
    ---help---
      Say yes to build a 64-bit kernel - formerly known as x86_64
      Say no to build a 32-bit kernel - formerly known as i386
```

&nbsp;&nbsp;&nbsp;&nbsp; If指令页可与default指令结合使用，强制只有在条件满足时defalut选项才有效。

&nbsp;&nbsp;&nbsp;&nbsp; 配置系统导出了一些元选项(meta-option)以简化生成配置文件。比如选项CONFIG_EMBEDDED是用于关闭哪些用户想要关闭的关键功能（比如要在嵌入系统中节省珍贵的内存）；选项CONFIG_BROKEN_ON_SMP用来表示驱动程序并非多处理器安全的。通常该选项不应设置，标记它的目的是确保用户知道该驱动的弱点。当然，新的驱动程序不应该使用该标志。最后要说明CONFIG_EXPERIMENTAL选项，它是一个用于说明某项功能尚在试验或处于beta版阶段的标志选项。该选项默认情况下关闭，同样，标记它的目的是为了让用户在使用驱动之前明白潜在风险。

## 2.6 模块参数

&nbsp;&nbsp;&nbsp;&nbsp; Linux提供了这样一个简单框架————它可允许驱动程序声明参数，从而用户可以在系统启动或者模块装载时在指定参数值，这些参数对于驱动程序属于全局变量。值得一提的是模块参数同时也将出现在sysfs文件系统中，这样一来，无论是生成模块参数，还是管理模块参数的方式都变得灵活多样了。

&nbsp;&nbsp;&nbsp;&nbsp; 定义一个模块参数可通过宏module_param()完成：

```cpp
module_param(name, type, perm);
```

&nbsp;&nbsp;&nbsp;&nbsp; 参数name即使用户可见的参数名，也是你模块中存放模块参数的变量名。

&nbsp;&nbsp;&nbsp;&nbsp; 参数type则存放了参数的类型，它可以是byte、short、ushort、int、uint、long、ulong、charp、bool或invbool，它们分别代表字节型、短整型、无符号短整型、整型、无符号整型、长整型、无符号长整型、字符串指针、布尔型，以及应用户要求转换得来的布尔型。其中byte类型存放在char类型变量中，boolean类型存放在int变量中，其余的类型都一致对应C语言的变量类型。

&nbsp;&nbsp;&nbsp;&nbsp; 参数perm指定了模块在sysfs文件系统下对应文件的权限，该值可以是八进制的格式，比如0644（所有者可以读写、组内可以读、其他人可以读）；或是S_Ifoo的定义形式；如果该值是零，则表示禁止所有的sysfs项。

&nbsp;&nbsp;&nbsp;&nbsp; 上面的宏其实并没有定义变量，你必须在使用该宏前进行变量定义。通常使用类似下面的语句完成定义：

```cpp
/* 在模块参数控制下，我们允许在钓鱼竿上用活鱼饵 */
static int allow_live_bait = 1;     /* 默认功能允许 */

module_param(allow_live_bait, bool, 0644); /* 一个Boolean类型 */
```

&nbsp;&nbsp;&nbsp;&nbsp; 这个值处于模块代码文件外部，换句话说，allow_live_bait是全局变量。

&nbsp;&nbsp;&nbsp;&nbsp; 有可能模块的外部参数名称不同于它对应的内部变量名称，这时就该使用宏module_param_named()定义了：

```cpp
module_param_named(name, variable, type, perm);
```

&nbsp;&nbsp;&nbsp;&nbsp; 参数name是外部可见的参数名称，参数variable是参数对应的内部全局变量名称。比如：

```cpp
static unsigned int max_test = DEFAULT_MAX_LINE_TEST;

module_param_named(maximum_line_test, max_test, int, 0);
```

&nbsp;&nbsp;&nbsp;&nbsp; 通常，需要一个charp类型来定义模块参数（一个字符串），内核将用户提供的这个字符串拷贝到内存，并且将变量指向该字符串。比如：

```cpp
static char *name;

module_param(name, charp, 0);
```

&nbsp;&nbsp;&nbsp;&nbsp; 如果需要，也可使内核直接拷贝字符串到指定的字符数组。宏module_param_string()可完成上述任务：

```cpp
module_param_string(name, string, len, perm);
```
&nbsp;&nbsp;&nbsp;&nbsp; 这些参数name为外部参数名称，参数string是对应的内部变量名称，参数len是string命名缓冲区的长度（或更小的长度，但是没什么太大意义），参数perm是sysfs文件系统权限（如果为零，则表示完全禁止sysfs项），比如：

```cpp
static char species[BUF_LEN];

module_param_string(specifies, species, BUF_LEN, 0);
```
&nbsp;&nbsp;&nbsp;&nbsp; 你可接受逗号分隔的参数序列，这些参数序列可通过宏module_param_array()存储在C数组中：

```cpp
module_param_array(name, type, nump, perm);
```
&nbsp;&nbsp;&nbsp;&nbsp; 参数name仍然是外部参数以及对应内部变量名，参数type是数据类型，参数perm是sysfs文件系统访问权限，这里新参数是nump，它是一个整型指针，该整型存放数据项数。注意由参数name指定的数组必须是静态分配的，内核需要在编译时确定数组大小，从而确保不会造成溢出。该函数的用法相当简单，比如：

```cpp
static int fish[MAX_FISH];
static int nr_fish;

module_param_array(fish, int, &nr_fish, 0444);
```

&nbsp;&nbsp;&nbsp;&nbsp; 你也可以将内部参数数组命名区别于外部参数，这是你需要使用宏：

```cpp
module_param_array_named(name, array, type, nump, perm);
```

&nbsp;&nbsp;&nbsp;&nbsp; 其中参数和其他宏一致。

&nbsp;&nbsp;&nbsp;&nbsp; 最后，你可以使用MODULE_PARM_DESC()描述你的参数：

```cpp
static unsigned short size = 1;

module_param(size, ushort, 0644);
MODULE_PARM_DESC(size, "The size in inches of the fishing pole.");
```

&nbsp;&nbsp;&nbsp;&nbsp; 上述所有宏需要包含<linux/module.h>头文件。

## 2.7 导出符号表

&nbsp;&nbsp;&nbsp;&nbsp; 模块被导入后，就会被动态地链接到内核。注意，它与用户空间中的动态链接库类似，只有被显式导出后的外部函数，才可以被动态库调用。在内核中，导出内核函数需要使用特殊的指令：

- EXPORT_SYMBOL()
- EXPORT_SYMBOL_GPL()

&nbsp;&nbsp;&nbsp;&nbsp; 导出的内核函数可以被模块调用，而未导出的函数模块则无法被调用。模块代码的链接和调用规则相比内核镜像中的代码而言，要更加严格。内核代码在内核中可以调用任意非静态接口，因为所有的内核源代码文件被链接成了同一个镜像。当然，被导出的符号表所含的函数必须也要是非静态的。

&nbsp;&nbsp;&nbsp;&nbsp; 导出的内核符号表被看做导出的内核接口，甚至成为内核API。导出符号相当简单，在声明后，紧跟上EXPORT_SYMBOL()指令就搞定了，比如：

```cpp
int get_pirate_beard_color(struct pirate *p)
{
    return p->beard.color;
}
EXPORT_SYMBOL(get_pirate_beard_color);
```

&nbsp;&nbsp;&nbsp;&nbsp; 假定get_pirate_beard_color()同时也定义在一个可访问的头文件中，那么现在任何模块都可以访问它。有一些开发者希望自己的接口仅仅对GPL兼容的模块可见，内核连接器使用MODULE_LINCENSE()宏满足这个要求。如果你希望先前的函数仅仅对标记为GPL协议的模块可见，那么你就需要用：

```cpp
EXPORT_SYMBOL_GPL(get_pirate_beard_color);
```
&nbsp;&nbsp;&nbsp;&nbsp; 如果你的代码被配置为模块，那么你就必须确保当前它被编译为模块时，它所用的全部接口都已被导出，否则就会产生链接错误（而且模块不能成功编译）。

## 2.8 获取模块相关信息

&nbsp;&nbsp;&nbsp;&nbsp; 查看模块的基本信息：

> $> modinfo e1000

```text
filename:       /lib/modules/3.10.0-693.el7.x86_64/kernel/drivers/net/ethernet/intel/e1000/e1000.ko.xz
version:        7.3.21-k8-NAPI
license:        GPL
description:    Intel(R) PRO/1000 Network Driver
author:         Intel Corporation, <linux.nics@intel.com>
rhelversion:    7.4
srcversion:     9E0A112E5D47C996E7C4A58
alias:          pci:v00008086d00002E6Esv*sd*bc*sc*i*
alias:          pci:v00008086d000010B5sv*sd*bc*sc*i*
alias:          pci:v00008086d00001099sv*sd*bc*sc*i*
alias:          pci:v00008086d0000108Asv*sd*bc*sc*i*
alias:          pci:v00008086d0000107Csv*sd*bc*sc*i*
alias:          pci:v00008086d0000107Bsv*sd*bc*sc*i*
alias:          pci:v00008086d0000107Asv*sd*bc*sc*i*
alias:          pci:v00008086d00001079sv*sd*bc*sc*i*
alias:          pci:v00008086d00001078sv*sd*bc*sc*i*
alias:          pci:v00008086d00001077sv*sd*bc*sc*i*
alias:          pci:v00008086d00001076sv*sd*bc*sc*i*
alias:          pci:v00008086d00001075sv*sd*bc*sc*i*
alias:          pci:v00008086d00001028sv*sd*bc*sc*i*
alias:          pci:v00008086d00001027sv*sd*bc*sc*i*
alias:          pci:v00008086d00001026sv*sd*bc*sc*i*
alias:          pci:v00008086d0000101Esv*sd*bc*sc*i*
alias:          pci:v00008086d0000101Dsv*sd*bc*sc*i*
alias:          pci:v00008086d0000101Asv*sd*bc*sc*i*
alias:          pci:v00008086d00001019sv*sd*bc*sc*i*
alias:          pci:v00008086d00001018sv*sd*bc*sc*i*
alias:          pci:v00008086d00001017sv*sd*bc*sc*i*
alias:          pci:v00008086d00001016sv*sd*bc*sc*i*
alias:          pci:v00008086d00001015sv*sd*bc*sc*i*
alias:          pci:v00008086d00001014sv*sd*bc*sc*i*
alias:          pci:v00008086d00001013sv*sd*bc*sc*i*
alias:          pci:v00008086d00001012sv*sd*bc*sc*i*
alias:          pci:v00008086d00001011sv*sd*bc*sc*i*
alias:          pci:v00008086d00001010sv*sd*bc*sc*i*
alias:          pci:v00008086d0000100Fsv*sd*bc*sc*i*
alias:          pci:v00008086d0000100Esv*sd*bc*sc*i*
alias:          pci:v00008086d0000100Dsv*sd*bc*sc*i*
alias:          pci:v00008086d0000100Csv*sd*bc*sc*i*
alias:          pci:v00008086d00001009sv*sd*bc*sc*i*
alias:          pci:v00008086d00001008sv*sd*bc*sc*i*
alias:          pci:v00008086d00001004sv*sd*bc*sc*i*
alias:          pci:v00008086d00001001sv*sd*bc*sc*i*
alias:          pci:v00008086d00001000sv*sd*bc*sc*i*
depends:        
intree:         Y
vermagic:       3.10.0-693.el7.x86_64 SMP mod_unload modversions 
signer:         CentOS Linux kernel signing key
sig_key:        DA:18:7D:CA:7D:BE:53:AB:05:BD:13:BD:0C:4E:21:F4:22:B6:A4:9C
sig_hashalgo:   sha256
parm:           TxDescriptors:Number of transmit descriptors (array of int)
parm:           RxDescriptors:Number of receive descriptors (array of int)
parm:           Speed:Speed setting (array of int)
parm:           Duplex:Duplex setting (array of int)
parm:           AutoNeg:Advertised auto-negotiation setting (array of int)
parm:           FlowControl:Flow Control setting (array of int)
parm:           XsumRX:Disable or enable Receive Checksum offload (array of int)
parm:           TxIntDelay:Transmit Interrupt Delay (array of int)
parm:           TxAbsIntDelay:Transmit Absolute Interrupt Delay (array of int)
parm:           RxIntDelay:Receive Interrupt Delay (array of int)
parm:           RxAbsIntDelay:Receive Absolute Interrupt Delay (array of int)
parm:           InterruptThrottleRate:Interrupt Throttling Rate (array of int)
parm:           SmartPowerDownEnable:Enable PHY smart power down (array of int)
parm:           copybreak:Maximum size of packet that is copied to a new buffer on receive (uint)
parm:           debug:Debug level (0=none,...,16=all) (int)
```

&nbsp;&nbsp;&nbsp;&nbsp; 查看内核以装载特定模块导出的API接口：

> $> cat /proc/kallsyms | grep \\[e1000\\] | cut -f1 | grep -v __

```text
ffffffffc0916bf0 t e1000_set_mac_type
ffffffffc091bdf0 t e1000_read_mac_addr
ffffffffc0918e60 t e1000_phy_setup_autoneg
ffffffffc09209c0 t e1000_set_ethtool_ops
ffffffffc0911ae0 t e1000_reset
ffffffffc0916060 t e1000_pci_clear_mwi
ffffffffc09159d0 t e1000_up
ffffffffc0912d90 t e1000_down
ffffffffc0914e10 t e1000_pci_set_mwi
ffffffffc0918770 t e1000_check_for_link
ffffffffc0921faa t cleanup_module
ffffffffc0913f30 t e1000_update_stats
ffffffffc09281a1 d e1000_driver_name
ffffffffc091bfa0 t e1000_rar_set
ffffffffc091c860 t e1000_write_vfta
ffffffffc0913e90 t e1000_has_link
ffffffffc0912fb0 t e1000_setup_all_tx_resources
ffffffffc091ca10 t e1000_led_on
ffffffffc09160a0 t e1000_pcix_set_mmrbc
ffffffffc0916ef0 t e1000_force_mac_fc
ffffffffc0916eb0 t e1000_config_collision_dist
ffffffffc0911a40 t e1000_get_hw_dev
ffffffffc09192d0 t e1000_phy_reset
ffffffffc09191d0 t e1000_phy_hw_reset
ffffffffc0919710 t e1000_validate_mdi_setting
ffffffffc091b860 t e1000_validate_eeprom_checksum
ffffffffc09180c0 t e1000_reset_hw
ffffffffc091ca90 t e1000_led_off
ffffffffc09160d0 t e1000_set_spd_dplx
ffffffffc0922a0b r e1000_driver_version
ffffffffc0917670 t e1000_get_speed_and_duplex
ffffffffc091cc90 t e1000_get_bus_info
ffffffffc091cbb0 t e1000_update_adaptive
ffffffffc0915da0 t e1000_reinit_locked
ffffffffc091bf00 t e1000_hash_mc_addr
ffffffffc0916e20 t e1000_set_media_type
ffffffffc091cb10 t e1000_reset_adaptive
ffffffffc0913ce0 t e1000_free_all_tx_resources
ffffffffc091b690 t e1000_init_eeprom_params
ffffffffc091c030 t e1000_init_hw
ffffffffc0917dd0 t e1000_write_phy_reg
ffffffffc0916080 t e1000_pcix_get_mmrbc
ffffffffc091b960 t e1000_write_eeprom
ffffffffc0919780 t e1000_read_eeprom
ffffffffc0911a60 t e1000_power_up_phy
ffffffffc091aee0 t e1000_setup_link
ffffffffc09160c0 t e1000_io_write
ffffffffc091bce0 t e1000_update_eeprom_checksum
ffffffffc09211d0 t e1000_check_options
ffffffffc0913640 t e1000_setup_all_rx_resources
ffffffffc0916fa0 t e1000_read_phy_reg
ffffffffc091c910 t e1000_setup_led
ffffffffc091cd90 t e1000_enable_mng_pass_thru
ffffffffc091c9c0 t e1000_cleanup_led
ffffffffc0913d30 t e1000_free_all_rx_resources
ffffffffc09193d0 t e1000_phy_get_info
```




&nbsp;&nbsp;&nbsp;&nbsp;
