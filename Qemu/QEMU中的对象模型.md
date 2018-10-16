***QEMU中的对象模型——QEMU Object Module***

[TOC]

# 1. 概述

&nbsp;&nbsp;&nbsp;&nbsp; QEMU提供了一套面向对象编程的模型——QOM，即QEMU Object Module，几乎所有的设备如CPU、内存、总线等都是利用这一面向对象的模型来实现的。QOM模型的实现代码位于qom/文件夹下的文件中。对于开发者而言，只要知道如何利用QOM模型创建类和对象就可以了，但是开发者只有理解了QOM的相关数据结构，才能清楚如何利用QOM模型。因此本文先对QOM的必要性展开叙述，然后说明QOM的相关数据结构，在读者了解了QOM的数据结构的基础上，我们向读者阐述如何使用QOM模型创建新对象和新类，在下一篇中，介绍QOM是如何实现的。同时对于阅读代码的人来说，理解和掌握QOM是学习QEMU代码的重要一步。

# 2. 为什么QEMU中要实现对象模型


- 各种架构CPU的模拟和实现 

&nbsp;&nbsp;&nbsp;&nbsp; QEMU中要实现对各种CPU架构的模拟，而且对于一种架构的CPU，比如X86_64架构的CPU，由于包含的特性不同，也会有不同的CPU模型。任何CPU中都有CPU通用的属性，同时也包含各自特有的属性。为了便于模拟这些CPU模型，面向对象的变成模型是必不可少的。

- 模拟device与bus的关系 


&nbsp;&nbsp;&nbsp;&nbsp; 在主板上，一个device会通过bus与其他的device相连接，一个device上可以通过不同的bus端口连接到其他的device，而其他的device也可以进一步通过bus与其他的设备连接，同时一个bus上也可以连接多个device，这种device连bus、bus连device的关系，qemu是需要模拟出来的。为了方便模拟设备的这种特性，面向对象的编程模型也是必不可少的。


# 3. QOM模型的数据结构

这些数据结构中TypeImpl定义在qom/object.c中，ObjectClass、Object、TypeInfo定义在include/qom/object.h中。 
在include/qom/object.h的注释中，对它们的每个字段都有比较明确的说明，并且说明了QOM模型的用法。 

## 3.1 TypeImpl：对数据类型的抽象数据结构
```
typedef struct TypeImpl TypeImpl;

struct InterfaceImpl
{
    const char *typename;
};

#define MAX_INTERFACES 32
typedef struct TypeImpl TypeImpl;
struct TypeImpl
{
    const char *name;

    size_t class_size; /* 该数据类型所代表的类的大小 */

    size_t instance_size; /* 该数据类型所代表的类的大小 */

    /* 类的 Constructor & Destructor */
    void (*class_init)(ObjectClass *klass, void *data);
    void (*class_base_init)(ObjectClass *klass, void *data);
    void (*class_finalize)(ObjectClass *klass, void *data);

    void *class_data; 

    /* 实例的Contructor & Destructor */
    void (*instance_init)(Object *obj);
    void (*instance_post_init)(Object *obj);
    void (*instance_finalize)(Object *obj);

    bool abstract; /* 表示类是否是抽象类 */

    const char *parent; /* 父类的名字 */
    TypeImpl *parent_type; /* 指向父类TypeImpl的指针 */

    ObjectClass *class; /* 该类型对应的类的指针 */

    int num_interfaces; /* 所实现的接口的数量 */
    InterfaceImpl interfaces[MAX_INTERFACES]; /* 类型的名字 */
};
```

## 3.2 ObjectClass: 是所有类的基类

```
#define OBJECT_CLASS_CAST_CACHE 4

struct ObjectClass
{
    /*< private >*/
    Type type;
    GSList *interfaces;

    const char *object_cast_cache[OBJECT_CLASS_CAST_CACHE];
    const char *class_cast_cache[OBJECT_CLASS_CAST_CACHE];

    ObjectUnparent *unparent;
};
```

## 3.3 Object: 是所有对象的base Object

```
struct Object
{
    /*< private >*/
    ObjectClass *class;
    ObjectFree *free; /* 当对象的引用为0时，清理垃圾的回调函数 */
    GHashTable *properties; /* Hash表记录Object的属性 */
    uint32_t ref;  /* 该对象的引用计数 */
    Object *parent;
};
```

## 3.4 TypeInfo：类型信息
TypeInfo：是用户用来定义一个Type的工具型的数据结构，用户定义了一个TypeInfo，然后调用type_register(TypeInfo )或者type_register_static(TypeInfo )函数，就会生成相应的TypeImpl实例，将这个TypeInfo注册到全局的TypeImpl的hash表中。

```
/*TypeInfo的属性与TypeImpl的属性对应，实际上qemu就是通过用户提供的TypeInfo创建的TypeImpl的对象*/
struct TypeInfo
{
    const char *name;
    const char *parent;

    size_t instance_size;
    void (*instance_init)(Object *obj);
    void (*instance_post_init)(Object *obj);
    void (*instance_finalize)(Object *obj);

    bool abstract;
    size_t class_size;

    void (*class_init)(ObjectClass *klass, void *data);
    void (*class_base_init)(ObjectClass *klass, void *data);
    void (*class_finalize)(ObjectClass *klass, void *data);
    void *class_data;

    InterfaceInfo *interfaces;
};
```