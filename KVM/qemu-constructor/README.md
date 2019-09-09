***QEMU在main函数前对模块的初始化过程***

<!-- TOC -->

- [1. 初始化的难题](#1-初始化的难题)
- [2. 模块化管理初始化工作](#2-模块化管理初始化工作)
- [3. init_type_list全局变量](#3-init_type_list全局变量)
- [4. gcc扩展功能构造函数和析构函数](#4-gcc扩展功能构造函数和析构函数)
- [5. gcc中宏的##和#扩展符](#5-gcc中宏的和扩展符)
    - [5.1 #扩展符可以将宏字符串化](#51-扩展符可以将宏字符串化)
    - [5.2 ##扩展符可以将它左右两边连接起来](#52-扩展符可以将它左右两边连接起来)
- [6. 言归正传](#6-言归正传)
- [7. qemu中如何利用module实现大量代码的初始化工作](#7-qemu中如何利用module实现大量代码的初始化工作)
- [8. 总结](#8-总结)

<!-- /TOC -->

# 1. 初始化的难题

QEMU中包含了大量的初始化函数，比如使用QOM模型设计的很多类（CPU、设备等都是利用QOM模型设计实现模拟的），这些类需要注册到管理类型的全局的hash表中，这个注册的过程需要在初始化函数中完成。

想象一下，如果我们把这些注册过程都放到main函数里面调用，main函数中就会有非常长的一段篇幅，仅仅是用于调用大量的初始化函数，这样对于QEMU的代码维护非常不利，因此QEMU将这些初始化函数的指针保存到了链表数据结构中，这样只要遍历一遍链表就可以执行全部的初始化函数。说起来简单，但是其中的设计和实现，以及对GCC中相关特性的使用都有我们值得学习和研究的地方。

# 2. 模块化管理初始化工作

QEMU中的代码的初始化管理是分模块的，实现这种模块化的代码文件包括include/qemu/module.h 和util/module.c。在include/qemu/module.h中所涉及的相关的代码模块包括：

- block （QEMU中的块操作实现代码）
- machine （QEMU中模拟设备的实现代码模块）
- qapi （QEMU向上层调用者提供的接口的代码模块）
- type 或者qom（QEMU中的QOM模型所涉及的代码模块）

# 3. init_type_list全局变量

util/module.c
```cpp
typedef enum {
    MODULE_INIT_BLOCK,
    MODULE_INIT_MACHINE,
    MODULE_INIT_QAPI,
    MODULE_INIT_QOM,
    MODULE_INIT_MAX
} module_init_type;

typedef struct ModuleEntry
{
    void (*init)(void);
    QTAILQ_ENTRY(ModuleEntry) node;
    module_init_type type;
} ModuleEntry;

typedef QTAILQ_HEAD(, ModuleEntry) ModuleTypeList;

static ModuleTypeList init_type_list[MODULE_INIT_MAX];
```

# 4. gcc扩展功能构造函数和析构函数

GCC提供了扩展功能，其中就包括构造函数（contructors），用户可以定义一个函数：
```c
static void __attribute__((constructor)) start(void)
```
这样start函数就会在main函数执行之前执行。
同理gcc中也有__attribute__((destructor))，会在main函数之后执行。

# 5. gcc中宏的##和#扩展符


## 5.1 #扩展符可以将宏字符串化

比如：
```c
#define SSVAR(X,Y) const char X[]=#Y  
SSVAR(InternetGatewayDevice, InternetGatewayDevice.);  
```
它等价于：
```c
const char InternetGatewayDevice[]="InternetGatewayDevice.";
```

## 5.2 ##扩展符可以将它左右两边连接起来

比如：
```c
#define DEV_FILE_NAME    "/dev/test_kft"  

#define OPEN_FILE(fd, n) \  
{  \  
      fd = open(DEV_FILE_NAME ##n, 0); \  
   if (fd < 0) \  
   { \  
      printf("Open device error/n"); \  
      return 0; \  
   } \  
}  
OPEN_FILE(fd1, 1);  
OPEN_FILE(fd2, 2);  
```
它等价于：
```c
{ fd1 = open(DEV_FILE_NAME1, 0); if (fd1 < 0) { printf("Open device error/n"); return 0; } };  
{ fd2 = open(DEV_FILE_NAME2, 0); if (fd2 < 0) { printf("Open device error/n"); return 0; } };
```

# 6. 言归正传

要想看QEMU在main函数之前做了什么，可以直接查找__attribute__((constructor))。通过查找，我们可以发现只有少数几个文件中定义了具有constructor属性的函数，其中就包括include/qemu/module.h文件中do_qemu_init_ ## function(void)，它是一个宏，而不是一个具体的函数。它的定义如下：（代码在include/qemu/module.h中）
```c
#define module_init(function, type)                                         \
static void __attribute__((constructor)) do_qemu_init_ ## function(void)    \
{                                                                           \
    register_module_init(function, type);                                   \
}
```

我们进一步看一下register_module_init的实现：（代码在util/module.c中）。这段代码实际上是将类型为type的代码模块中一个初始化函数的指针存入全局变量init_type_list对应类型的链表中。
```c
void register_module_init(void (*fn)(void), module_init_type type)
{
    ModuleEntry *e;
    ModuleTypeList *l;

    e = g_malloc0(sizeof(*e));
    e->init = fn;
    e->type = type;

    l = find_type(type);

    QTAILQ_INSERT_TAIL(l, e, node);
}
```

这将意味着，如果在全局的代码中调用module_init(f,t)，系统就会定义一个名为module_init_f的函数，并且该函数是constructor，必须在main函数之前执行，该函数要执行的代码就是下列代码。这个函数最终将函数指针f存入全局变量init_type_list数组中的type为t的链表中。

而根据这个module_init宏，qemu分别针对四个代码模块定义了四种宏：

```c
typedef enum {
    MODULE_INIT_BLOCK,
    MODULE_INIT_MACHINE,
    MODULE_INIT_QAPI,
    MODULE_INIT_QOM,
    MODULE_INIT_MAX
} module_init_type;

#define block_init(function) module_init(function, MODULE_INIT_BLOCK)
#define machine_init(function) module_init(function, MODULE_INIT_MACHINE)
#define qapi_init(function) module_init(function, MODULE_INIT_QAPI)
#define type_init(function) module_init(function, MODULE_INIT_QOM)

```

也就是说，在全局的代码中如果调用上述任何一个函数，就会在系统中自动生成一个具有constructor属性的函数，这个函数会在main函数之前执行。举一个例子，我们在hw/i2c/smbus_eeprom.c中看到了type_init(smbus_eeprom_register_types)的一个调用：

```c
static void smbus_eeprom_class_initfn(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    SMBusDeviceClass *sc = SMBUS_DEVICE_CLASS(klass);

    sc->init = smbus_eeprom_initfn;
    sc->quick_cmd = eeprom_quick_cmd;
    sc->send_byte = eeprom_send_byte;
    sc->receive_byte = eeprom_receive_byte;
    sc->write_data = eeprom_write_data;
    sc->read_data = eeprom_read_data;
    dc->props = smbus_eeprom_properties;
    /* Reason: pointer property "data" */
    dc->cannot_instantiate_with_device_add_yet = true;
}

static const TypeInfo smbus_eeprom_info = {
    .name          = "smbus-eeprom",
    .parent        = TYPE_SMBUS_DEVICE,
    .instance_size = sizeof(SMBusEEPROMDevice),
    .class_init    = smbus_eeprom_class_initfn,
};

static void smbus_eeprom_register_types(void)
{
    type_register_static(&smbus_eeprom_info);
}

type_init(smbus_eeprom_register_types)

```

这样就会在系统中生成一个如下函数：
```c
static void __attribute__((constructor)) do_qemu_init_smbus_eeprom_register_types(void)    
{                                                                           
    register_module_init(smbus_eeprom_register_types, MODULE_INIT_QOM);                                   
}
```

# 7. qemu中如何利用module实现大量代码的初始化工作

通过上述定义constructor属性的函数的方式，qemu会在main函数之前，将所有代码模块中的所有的初始化函数指针保存到init_type_list数组中的对应类型的链表中。我们只需调用每种类型链表中每个entry保存的初始化函数指针，就可以实现调用所有代码模块中大量的初始化函数指针。
```c
void module_call_init(module_init_type type)
{
    ModuleTypeList *l;
    ModuleEntry *e;

    module_load(type);
    l = find_type(type);

    QTAILQ_FOREACH(e, l, node) {
        e->init();
    }
}
```

# 8. 总结

qemu中通过设计module的机制，将大量的初始化函数分模块地保存到链表中，简化了这些代码模块的初始化工作，也简化了实现的流程，提高了代码的可维护性。