/*
 * QEMU Object Model
 *
 *
 *  Anthony Liguori   <aliguori@us.ibm.com>
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 *
 */

#ifndef OBJECT_H
#define OBJECT_H

#include <glib.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

struct TypeImpl;
typedef struct TypeImpl *Type;

typedef struct ObjectClass ObjectClass;
typedef struct Object Object;

typedef struct TypeInfo TypeInfo;

typedef struct InterfaceClass InterfaceClass;
typedef struct InterfaceInfo InterfaceInfo;

#define TYPE_OBJECT "object"

/**
 * ObjectUnparent:
 * @obj: the object that is being removed from the composition tree
 *
 * Called when an object is being removed from the QOM composition tree.
 * The function should remove any backlinks from children objects to @obj.
 */
typedef void (ObjectUnparent)(Object *obj);

/**
 * ObjectFree:
 * @obj: the object being freed
 *
 * Called when an object's last reference is removed.
 */
typedef void (ObjectFree)(void *obj);

#define OBJECT_CLASS_CAST_CACHE 4

/**
 * ObjectClass:
 *
 * The base for all classes.  The only thing that #ObjectClass contains is an
 * integer type handle.
 */
struct ObjectClass
{
    /*< private >*/
    Type type;
    GSList *interfaces;

    const char *object_cast_cache[OBJECT_CLASS_CAST_CACHE];
    const char *class_cast_cache[OBJECT_CLASS_CAST_CACHE];

    ObjectUnparent *unparent;

    GHashTable *properties;
};

/**
 * Object:
 *
 * The base for all objects.  The first member of this object is a pointer to
 * a #ObjectClass.  Since C guarantees that the first member of a structure
 * always begins at byte 0 of that structure, as long as any sub-object places
 * its parent as the first member, we can cast directly to a #Object.
 *
 * As a result, #Object contains a reference to the objects type as its
 * first member.  This allows identification of the real type of the object at
 * run time.
 */
struct Object
{
    /*< private >*/
    ObjectClass *class;
    ObjectFree *free;
    GHashTable *properties;
    guint32 ref;
    Object *parent;
};

/**
 * TypeInfo:
 * @name: The name of the type.
 * @parent: The name of the parent type.
 * @instance_size: The size of the object (derivative of #Object).  If
 *   @instance_size is 0, then the size of the object will be the size of the
 *   parent object.
 * @instance_init: This function is called to initialize an object.  The parent
 *   class will have already been initialized so the type is only responsible
 *   for initializing its own members.
 * @instance_finalize: This function is called during object destruction.  This
 *   is called before the parent @instance_finalize function has been called.
 *   An object should only free the members that are unique to its type in this
 *   function.
 * @abstract: If this field is true, then the class is considered abstract and
 *   cannot be directly instantiated.
 * @class_size: The size of the class object (derivative of #ObjectClass)
 *   for this object.  If @class_size is 0, then the size of the class will be
 *   assumed to be the size of the parent class.  This allows a type to avoid
 *   implementing an explicit class type if they are not adding additional
 *   virtual functions.
 * @class_init: This function is called after all parent class initialization
 *   has occurred to allow a class to set its default virtual method pointers.
 *   This is also the function to use to override virtual methods from a parent
 *   class.
 * @class_base_init: This function is called for all base classes after all
 *   parent class initialization has occurred, but before the class itself
 *   is initialized.  This is the function to use to undo the effects of
 *   memcpy from the parent class to the descendants.
 * @class_finalize: This function is called during class destruction and is
 *   meant to release and dynamic parameters allocated by @class_init.
 * @class_data: Data to pass to the @class_init, @class_base_init and
 *   @class_finalize functions.  This can be useful when building dynamic
 *   classes.
 * @type_init_phase: If this filed is set to OBJECT_NEW_PHASE, then type will only
 *   be initialized during the first object of this type is creating.The other
 *   choice is TYPE_REGISTER_PHASE, that means type initialization will happen during
 *   type registration.
 * @interfaces: The list of interfaces associated with this type.  This
 *   should point to a static array that's terminated with a zero filled
 *   element.
 */

typedef enum TypeInitPhase
{
    OBJECT_NEW_PHASE = 0,
    TYPE_REGISTER_PHASE = 1
} TypeInitPhase;

struct TypeInfo
{
    const char *name;
    const char *parent;

    size_t instance_size;
    void (*instance_init)(Object *obj);
    void (*instance_finalize)(Object *obj);

    bool abstract;
    size_t class_size;

    void (*class_init)(ObjectClass *klass, void *data);
    void (*class_base_init)(ObjectClass *klass, void *data);
    void (*class_finalize)(ObjectClass *klass, void *data);
    void *class_data;

    TypeInitPhase type_init_phase; 

    InterfaceInfo *interfaces;
};

/**
 * OBJECT:
 * @obj: A derivative of #Object
 *
 * Converts an object to a #Object.  Since all objects are #Objects,
 * this function will always succeed.
 */
#define OBJECT(obj) \
    ((Object *)(obj))

/**
 * OBJECT_CLASS:
 * @class: A derivative of #ObjectClass.
 *
 * Converts a class to an #ObjectClass.  Since all objects are #Objects,
 * this function will always succeed.
 */
#define OBJECT_CLASS(class) \
    ((ObjectClass *)(class))

/**
 * GET_CLASS: 
 * @class_type: The class type want to get.
 * @name: class name.
 * 
 * This function will terminated if typname is NULL, or type has not registered
 * yet. If class member is null, it typically means type inititialization happens
 * in object new phase and no object of this type has created before.
 * created
 */
#define GET_CLASS(name) \
     (get_class_by_name((name), __FILE__, __LINE__, __func__))


/**
 * OBJECT_CHECK:
 * @type: The C type to use for the return value.
 * @obj: A derivative of @type to cast.
 * @name: The QOM typename of @type
 *
 * A type safe version of @object_dynamic_cast_assert.  Typically each class
 * will define a macro based on this type to perform type safe dynamic_casts to
 * this object type.
 *
 * If an invalid object is passed to this function, a run time assert will be
 * generated.
 */
#define OBJECT_CHECK(type, obj, name) \
    ((type *)object_dynamic_cast_assert(OBJECT(obj), (name), \
                                        __FILE__, __LINE__, __func__))

/**
 * OBJECT_CLASS_CHECK:
 * @class_type: The C type to use for the return value.
 * @class: A derivative class of @class_type to cast.
 * @name: the QOM typename of @class_type.
 *
 * A type safe version of @object_class_dynamic_cast_assert.  This macro is
 * typically wrapped by each type to perform type safe casts of a class to a
 * specific class type.
 */
#define OBJECT_CLASS_CHECK(class_type, class, name) \
    ((class_type *)object_class_dynamic_cast_assert(OBJECT_CLASS(class), (name), \
                                               __FILE__, __LINE__, __func__))

/**
 * OBJECT_GET_CLASS:
 * @class: The C type to use for the return value.
 * @obj: The object to obtain the class for.
 * @name: The QOM typename of @obj.
 *
 * This function will return a specific class for a given object.  Its generally
 * used by each type to provide a type safe macro to get a specific class type
 * from an object.
 */
#define OBJECT_GET_CLASS(class, obj, name) \
    OBJECT_CLASS_CHECK(class, object_get_class(OBJECT(obj)), name)

/**
 * InterfaceInfo:
 * @type: The name of the interface.
 *
 * The information associated with an interface.
 */
struct InterfaceInfo {
    const char *type;
};

/**
 * InterfaceClass:
 * @parent_class: the base class
 *
 * The class for all interfaces.  Subclasses of this class should only add
 * virtual methods.
 */
struct InterfaceClass
{
    ObjectClass parent_class;
    /*< private >*/
    ObjectClass *concrete_class;
    Type interface_type;
};

#define TYPE_INTERFACE "interface"

/**
 * INTERFACE_CLASS:
 * @klass: class to cast from
 * Returns: An #InterfaceClass or raise an error if cast is invalid
 */
#define INTERFACE_CLASS(klass) \
    OBJECT_CLASS_CHECK(InterfaceClass, klass, TYPE_INTERFACE)

/**
 * INTERFACE_CHECK:
 * @interface: the type to return
 * @obj: the object to convert to an interface
 * @name: the interface type name
 *
 * Returns: @obj casted to @interface if cast is valid, otherwise raise error.
 */
#define INTERFACE_CHECK(interface, obj, name) \
    ((interface *)object_dynamic_cast_assert(OBJECT((obj)), (name), \
                                             __FILE__, __LINE__, __func__))

/**
 *  register the object type
 */
void type_object_init(void);

/**
 * object_new:
 * @typename: The name of the type of the object to instantiate.
 *
 * This function will initialize a new object using heap allocated memory.
 * The returned object has a reference count of 1, and will be freed when
 * the last reference is dropped.
 *
 * Returns: The newly allocated and instantiated object.
 */
Object *object_new(const char *typename);


/**
 * object_new:
 * @typename: The name of the type of the object to instantiate.
 *
 * This function will initialize a new object using heap allocated memory.
 * The returned object has a reference count of 1, and will be freed when
 * the last reference is dropped.
 *
 * Returns: The newly allocated and instantiated object.
 */

Object **objects_new(const char *typename, int num_object);

/**
 * object_initialize:
 * @obj: A pointer to the memory to be used for the object.
 * @size: The maximum size available at @obj for the object.
 * @typename: The name of the type of the object to instantiate.
 *
 * This function will initialize an object.  The memory for the object should
 * have already been allocated.  The returned object has a reference count of 1,
 * and will be finalized when the last reference is dropped.
 */
void object_initialize(void *obj, size_t size, const char *typename);

/**
 * object_dynamic_cast:
 * @obj: The object to cast.
 * @typename: The @typename to cast to.
 *
 * This function will determine if @obj is-a @typename.  @obj can refer to an
 * object or an interface associated with an object.
 *
 * Returns: This function returns @obj on success or #NULL on failure.
 */
Object *object_dynamic_cast(Object *obj, const char *typename);

/**
 * object_dynamic_cast_assert:
 *
 * See object_dynamic_cast() for a description of the parameters of this
 * function.  The only difference in behavior is that this function asserts
 * instead of returning #NULL on failure if QOM cast debugging is enabled.
 * This function is not meant to be called directly, but only through
 * the wrapper macro OBJECT_CHECK.
 */
Object *object_dynamic_cast_assert(Object *obj, const char *typename,
                                   const char *file, int line, const char *func);

/**
 * object_get_class:
 * @obj: A derivative of #Object
 *
 * Returns: The #ObjectClass of the type associated with @obj.
 */
ObjectClass *object_get_class(Object *obj);

/**
 * object_get_typename:
 * @obj: A derivative of #Object.
 *
 * Returns: The QOM typename of @obj.
 */
const char *object_get_typename(const Object *obj);

/**
 * type_register_static:
 * @info: The #TypeInfo of the new type.
 *
 * @info and all of the strings it points to should exist for the life time
 * that the type is registered.
 *
 * Returns: the new #Type.
 */
Type type_register_static(const TypeInfo *info);

/**
 * type_register:
 * @info: The #TypeInfo of the new type
 *
 * Unlike type_register_static(), this call does not require @info or its
 * string members to continue to exist after the call returns.
 *
 * Returns: the new #Type.
 */
Type type_register(const TypeInfo *info);

/**
 * type_register_static_array:
 * @infos: The array of the new type #TypeInfo structures.
 * @nr_infos: number of entries in @infos
 *
 * @infos and all of the strings it points to should exist for the life time
 * that the type is registered.
 */
void type_register_static_array(const TypeInfo *infos, int nr_infos);

/**
 * void* get_class_by_name:
 * @typename: The QOM typename of the class to cast to.
 */
void *get_class_by_name(const char *typename,
                        const char *file, int line,
                        const char *func);

/**
 *  
 * bool is_compatible_type:
 * @typename: the name of the type to be tested. 
 * @target_typename: the name of the targert type. 
 */

bool is_compatible_type(const char *typename, const char *target_typename);

/**
 * object_class_dynamic_cast_assert:
 * @klass: The #ObjectClass to attempt to cast.
 * @typename: The QOM typename of the class to cast to.
 *
 * See object_class_dynamic_cast() for a description of the parameters
 * of this function.  The only difference in behavior is that this function
 * asserts instead of returning #NULL on failure if QOM cast debugging is
 * enabled.  This function is not meant to be called directly, but only through
 * the wrapper macros OBJECT_CLASS_CHECK and INTERFACE_CHECK.
 */
ObjectClass *object_class_dynamic_cast_assert(ObjectClass *klass,
                                              const char *typename,
                                              const char *file, int line,
                                              const char *func);

/**
 * object_class_dynamic_cast:
 * @klass: The #ObjectClass to attempt to cast.
 * @typename: The QOM typename of the class to cast to.
 *
 * Returns: If @typename is a class, this function returns @klass if
 * @typename is a subtype of @klass, else returns #NULL.
 *
 * If @typename is an interface, this function returns the interface
 * definition for @klass if @klass implements it unambiguously; #NULL
 * is returned if @klass does not implement the interface or if multiple
 * classes or interfaces on the hierarchy leading to @klass implement
 * it.  (FIXME: perhaps this can be detected at type definition time?)
 */
ObjectClass *object_class_dynamic_cast(ObjectClass *klass,
                                       const char *typename);

/**
 * object_class_get_parent:
 * @klass: The class to obtain the parent for.
 *
 * Returns: The parent for @klass or %NULL if none.
 */
ObjectClass *object_class_get_parent(ObjectClass *klass);

/**
 * object_class_get_name:
 * @klass: The class to obtain the QOM typename for.
 *
 * Returns: The QOM typename for @klass.
 */
const char *object_class_get_name(ObjectClass *klass);

/**
 * object_class_is_abstract:
 * @klass: The class to obtain the abstractness for.
 *
 * Returns: %true if @klass is abstract, %false otherwise.
 */
bool object_class_is_abstract(ObjectClass *klass);

/**
 * object_class_by_name:
 * @typename: The QOM typename to obtain the class for.
 *
 * Returns: The class for @typename or %NULL if not found.
 */
ObjectClass *object_class_by_name(const char *typename);

void object_class_foreach(void (*fn)(ObjectClass *klass, void *opaque),
                          const char *implements_type, bool include_abstract,
                          void *opaque);

/**
 * object_class_get_list:
 * @implements_type: The type to filter for, including its derivatives.
 * @include_abstract: Whether to include abstract classes.
 *
 * Returns: A singly-linked list of the classes in reverse hashtable order.
 */
GSList *object_class_get_list(const char *implements_type,
                              bool include_abstract);

/**
 * object_ref:
 * @obj: the object
 *
 * Increase the reference count of a object.  A object cannot be freed as long
 * as its reference count is greater than zero.
 */
void object_ref(Object *obj);

/**
 * object_unref:
 * @obj: the object
 *
 * Decrease the reference count of a object.  A object cannot be freed as long
 * as its reference count is greater than zero.
 */
void object_unref(Object *obj);

/**
 * object_type_register:
 *
 * Register object type. 
 */

void object_type_register(void);

#endif
