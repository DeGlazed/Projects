#include "sack_object.h"

#ifndef DATA_STRUCT_H
#define DATA_STRUCT_H

typedef data_struct
{
    int thread_ID;
    const sack_object *objects;
    int object_count;
    int generations_count;
    int sack_capacity;
}

#endif