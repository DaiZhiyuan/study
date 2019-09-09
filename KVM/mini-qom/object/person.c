#include <stdio.h>
#include "person.h"

static void about_me(void *obj)
{
    printf("My name is [%s], age is [%d], gender is [%s].\n", \
        PERSON(obj)->name, PERSON(obj)->age, \
        PERSON(obj)->gender ? "male":"female ");
}

static void instance_init(Object *obj)
{
    Person *this = PERSON(obj);
    this->name = "Jerry Dai";
    this->age = 32;
    this->gender = true;
}

Person *Person_new(void)
{
    return (Person *)object_new(TYPE_PERSON);
}

static void class_init(ObjectClass *oc, void *data)
{
    PersonClass *person = PERSON_CLASS(oc);
    person->about_me = about_me;
}

static const TypeInfo type_info = {
    .name = TYPE_PERSON,
    .parent = TYPE_OBJECT,
    .instance_size = sizeof(Person),
    .abstract = false,
    .class_size = sizeof(PersonClass),
    .instance_init = instance_init,
    .class_init = class_init,
};

void __attribute__((constructor)) Person_register(void)
{
     type_register_static(&type_info);
}
