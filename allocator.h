#ifndef _ALLOCATOR_H
#define _ALLOCATOR_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

typedef size_t Handle;

typedef struct {
    Handle (*alloc)(size_t);
    Handle (*realloc)(Handle, size_t);
    void (*free)(Handle);
} Allocator;

#endif
