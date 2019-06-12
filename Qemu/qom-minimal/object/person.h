#ifndef __PERSON_H__
#define __PERSON_H__

#include "qom/object.h"

#define TYPE_PERSON "person"


typedef struct Person {
    Object parent;

    char *name;
    int age;
    bool gender;
} Person;

typedef struct PersonClass {
    ObjectClass parent_class;

    void (*about_me)(void *);
    void (*set_name)(char *name);
} PersonClass;

Person *Person_new(void);
void Person_register(void);

#define PERSON_GET_CLASS(obj) \
        OBJECT_GET_CLASS(PersonClass, obj, TYPE_PERSON)

#define PERSON_CLASS(klass) \
        OBJECT_CLASS_CHECK(PersonClass, klass, TYPE_PERSON)

#define PERSON(obj) \
        OBJECT_CHECK(Person, obj, TYPE_PERSON)

#endif
