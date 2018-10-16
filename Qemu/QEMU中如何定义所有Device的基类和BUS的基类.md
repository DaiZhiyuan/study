***QEMU中如何定义所有Device的基类和BUS的基类***

[TOC]

# 1. 概述

为了模拟出虚拟机的主板（board）以及主板上所有设备（Device）、总线（Bus），并且描述它们的关系，qemu设计了所有Device和Bus的基类。

QEMU借助自己实现的面向对象模型QOM实现了设备和BUS的基类，为后面实现更加细致的CPU类和内存类打下基础。前面已经提到，要想创建新类，我们只需实现Device的类和对象的数据结构、Bus的类和数据结构，并且对一个TypeInfo的实例赋值就可以了。我们接下来会重点看它们的代码。

# 2. 虚拟机的设备树

我们知道，一个设备上可以很多可以连接bus的接口，也就是说一个设备可以连接多个bus，一个bus也可以连接多个设备，而无论设备和bus，在qom中都是一种对象，为了表达这种一个设备连多个bus，一个bus也可以连多个设备。（如果你写过verilog之类的硬件编码语言，那么对这种关系应该比较熟悉）

这种连接关系的表达就是通过将对象设置为另一个对象的孩子对象来表示的。在QOM中，如果一个对象是另一个对象的孩子，那么孩子对象指针或者指针的指针会被保存到父对象的属性hash表中。

这种孩子对象的关系，是单方向的，因此，最终所有的设备会形成一棵逻辑上的设备树。我们可以在qemu命令行模式下输入info qtree命令查看这颗树的构成。

# 3. QOM中Object的子类对象属性

在解释QEMU如何定义设备和BUS的基类之前，我们需要了解QEMU面向对象模型QOM除了面向对象编程都有的封装、继承、多态等特性之外，它还可以在对象的属性中添加多个子类对象属性，此时属性的类型为“child”， 或者将对象的指针的指针保存到对象的属性hash表中，此时属性的类型为“link”。

为了向对象中添加子类对象，可以调用object_property_add_child函数：
```c
void object_property_add_child(Object *obj, const char *name,
                               Object *child, Error **errp)
{
    Error *local_err = NULL;
    gchar *type;
    ObjectProperty *op;

    if (child->parent != NULL) {
        error_setg(errp, "child object is already parented");
        return;
    }

    type = g_strdup_printf("child<%s>", object_get_typename(OBJECT(child)));

    op = object_property_add(obj, name, type, object_get_child_property, NULL,
                             object_finalize_child_property, child, &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        goto out;
    }

    op->resolve = object_resolve_child_property;
    object_ref(child);
    child->parent = obj;

out:
    g_free(type);
}
```
将对象指针的指针保存到另一个对象的属性hash表中:
```c
typedef struct {
    Object **child;
    void (*check)(Object *, const char *, Object *, Error **);
    ObjectPropertyLinkFlags flags;
} LinkProperty;

void object_property_add_link(Object *obj, const char *name,
                              const char *type, Object **child,
                              void (*check)(Object *, const char *,
                                            Object *, Error **),
                              ObjectPropertyLinkFlags flags,
                              Error **errp)
{
    Error *local_err = NULL;
    LinkProperty *prop = g_malloc(sizeof(*prop));
    gchar *full_type;
    ObjectProperty *op;

    prop->child = child;
    prop->check = check;
    prop->flags = flags;

    full_type = g_strdup_printf("link<%s>", type);

    op = object_property_add(obj, name, full_type,
                             object_get_link_property,
                             check ? object_set_link_property : NULL,
                             object_release_link_property,
                             prop,
                             &local_err);
    if (local_err) {
        error_propagate(errp, local_err);
        g_free(prop);
        goto out;
    }

    op->resolve = object_resolve_link_property;

out:
    g_free(full_type);
}
```

# 4. 数据结构实现

Device、Bus的数据结构包括DeviceClass、DeviceState、BusClass、BusState，实现都在include/hw/qdev-core.h中，读者可以自行查看相关代码文件。值得注意的是：DeviceClass、BusClass中的相关回调函数，都是虚拟的函数指针，因此这些类都是抽象类，我们要想实现某种设备或总线，必须继承这两个类，并且必须实现相关的回调函数，给这些函数指针赋值。


## 4.1 DeviceClass

Device对应类的数据结构如下：

```c
typedef struct DeviceClass {
    /*< private >*/
    ObjectClass parent_class;  //父类的实例
    /*< public >*/

    DECLARE_BITMAP(categories, DEVICE_CATEGORY_MAX); 
    const char *fw_name;
    const char *desc;
    Property *props;

    /*
     * Shall we hide this device model from -device / device_add?
     * All devices should support instantiation with device_add, and
     * this flag should not exist.  But we're not there, yet.  Some
     * devices fail to instantiate with cryptic error messages.
     * Others instantiate, but don't work.  Exposing users to such
     * behavior would be cruel; this flag serves to protect them.  It
     * should never be set without a comment explaining why it is set.
     * TODO remove once we're there
     */
    bool cannot_instantiate_with_device_add_yet;
    /*
     * Does this device model survive object_unref(object_new(TNAME))?
     * All device models should, and this flag shouldn't exist.  Some
     * devices crash in object_new(), some crash or hang in
     * object_unref().  Makes introspecting properties with
     * qmp_device_list_properties() dangerous.  Bad, because it's used
     * by -device FOO,help.  This flag serves to protect that code.
     * It should never be set without a comment explaining why it is
     * set.
     * TODO remove once we're there
     */
    bool cannot_destroy_with_object_finalize_yet;

    bool hotpluggable;

    /* callbacks */
    void (*reset)(DeviceState *dev);
    DeviceRealize realize;
    DeviceUnrealize unrealize;

    /* device state */
    const struct VMStateDescription *vmsd;  //这个属性在

    /* Private to qdev / bus.  */
    qdev_initfn init; /* TODO remove, once users are converted to realize */
    qdev_event exit; /* TODO remove, once users are converted to unrealize */
    const char *bus_type;
} DeviceClass;
```
- 这个数据结构直接继承的是ObjectClass数据结构，拥有ObjectClass的一切变量，可以通过include/qom/object.h中暴露的一些函数接口，对它的parent_class进行操作。
- 这个数据结构中包含了设备类的几个非常重要的回调函数。其中reset函数用于设备的重置，realize函数用于将设备真正的实现出来（以CPU为例，如果调用这个函数，也就会生成一个新的vCPU线程来模拟一个新的CPU），而unrealize函数与realize函数相反。
- 同时，这些属性也表达了设备的重要特性：fw_name（固件名字）、hotpluggable（是否支持热插拔）。
- 需要特别注意的是vmsd属性，DeviceClass中的属性vmsd是实现热迁移、保存虚拟机系统还原点的基础。它对应的数据结构，我们会在讲热迁移的文档中详细说明，这里先列出它的代码。（代码在include/migration/vmstate.h中）。其中fields变量用于描述某个含有VMStateDescription的指针的数据结构中，需要在保存系统还原点时保存的或者在热迁移时传输的相关的属性。

```c
struct VMStateDescription {
    const char *name;
    int unmigratable;
    int version_id;
    int minimum_version_id;
    int minimum_version_id_old;
    LoadStateHandler *load_state_old;
    int (*pre_load)(void *opaque);
    int (*post_load)(void *opaque, int version_id);
    void (*pre_save)(void *opaque);
    bool (*needed)(void *opaque);
    VMStateField *fields;
    const VMStateDescription **subsections;
};
```

## 4.2 DeviceState

DeviceState数据结构是qemu中实现设备的对象的数据结构。一个DeviceState就代表一个设备。这个数据结构如下： 
```c
struct DeviceState {
    /*< private >*/
    Object parent_obj;
    /*< public >*/

    const char *id;  
    bool realized;  // 指示设备是否实现，然后调用callback
    bool pending_deleted_event;
    QemuOpts *opts;
    int hotplugged;
    BusState *parent_bus;   
    QLIST_HEAD(, NamedGPIOList) gpios;
    QLIST_HEAD(, BusState) child_bus;
    int num_child_bus;
    int instance_id_alias;
    int alias_required_for_version;
};
```
- 我们可以看到parent_bus，这个属性表示，连接这个设备的上一级的bus。
- 我们可以看到设备有两个链表：gpios和child_bus。gpios是设备通用输入输出接口，对应的数据结构定义如下代码。而child_bus正如我们之前说过的是设备上连接的多个bus。
```c
struct NamedGPIOList {
    char *name;
    qemu_irq *in;
    int num_in;
    int num_out;
    QLIST_ENTRY(NamedGPIOList) node;
};
```

## 4.3 Device的TypeInfo

给Device的TypeInfo的相关属性赋值，从而可以将Device的基类注册到由QOM保存qemu中全部类型的hash表中。

```c
static const TypeInfo device_type_info = {
    .name = TYPE_DEVICE,
    .parent = TYPE_OBJECT,
    .instance_size = sizeof(DeviceState),
    .instance_init = device_initfn,
    .instance_post_init = device_post_init,
    .instance_finalize = device_finalize,
    .class_base_init = device_class_base_init,
    .class_init = device_class_init,
    .abstract = true,
    .class_size = sizeof(DeviceClass),
};
```

## 4.4 BusClass

BusClass是实现bus的类的数据结构:

```c
struct BusClass {
    ObjectClass parent_class;

    /* FIXME first arg should be BusState */
    void (*print_dev)(Monitor *mon, DeviceState *dev, int indent);
    char *(*get_dev_path)(DeviceState *dev);
    /*
     * This callback is used to create Open Firmware device path in accordance
     * with OF spec http://forthworks.com/standards/of1275.pdf. Individual bus
     * bindings can be found at http://playground.sun.com/1275/bindings/.
     */
    char *(*get_fw_dev_path)(DeviceState *dev);
    void (*reset)(BusState *bus);
    BusRealize realize;
    BusUnrealize unrealize;

    /* maximum devices allowed on the bus, 0: no limit. */
    int max_dev;
    /* number of automatically allocated bus ids (e.g. ide.0) */
    int automatic_ids;
};
```
- BusClass中包含了一系列回调函数。reset用于重置bus、realize用于实现bus，unrealize与realize相反。
- 其包含的属性包括：max_dev（表示bus上能够连接的最大的设备数）、automatic_ids（bus上能够给设备赋值的最大的id）

## 4.5 BusState

BusState是实现bus的对象的数据结构:

```c
typedef struct BusChild {
    DeviceState *child;
    int index;
    QTAILQ_ENTRY(BusChild) sibling;
} BusChild;

struct BusState {
    Object obj;
    DeviceState *parent;
    const char *name;
    HotplugHandler *hotplug_handler;
    int max_index;
    bool realized;
    QTAILQ_HEAD(ChildrenHead, BusChild) children;
    QLIST_ENTRY(BusState) sibling;
};
```

- 其中parent是连接这个bus的上一级设备
- max_index用于指示目前bus上连接了多少设备（这是一个索引，所以它的value是设备数-1）
- realized指示bus是否实现
- children是bus上连接的所有设备

## 4.6 Bus的TypeInfo

给Bus的TypeInfo的相关属性赋值。从而可以将Bus的基类注册到QOM保存qemu中全部类型的hash表中:

```c
static const TypeInfo bus_info = {
    .name = TYPE_BUS,
    .parent = TYPE_OBJECT,
    .instance_size = sizeof(BusState),
    .abstract = true,
    .class_size = sizeof(BusClass),
    .instance_init = qbus_initfn,
    .instance_finalize = qbus_finalize,
    .class_init = bus_class_init,
};
```

# 5. 总结
通过以上这些数据结构，qemu设计出了所有device和bus的基类，它们包含了device或者bus的共有属性。将一个device连接多个bus，一个bus连接多个device的这种关系设计成对象的孩子属性，或者对象的link属性（保存另一个对象的指针的指针），从而为qemu模拟出包含多个设备、多个总线的主板奠定基础。
