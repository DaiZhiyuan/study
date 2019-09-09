#include <stdio.h>

#include "person.h"

int main()
{
    Person *obj = Person_new();
    PERSON_GET_CLASS(obj)->about_me(obj);

    return 0;
}
