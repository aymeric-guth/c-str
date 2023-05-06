#ifndef _STACK_H
#define _STACK_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include "allocator.h"

typedef struct {
    size_t total;
    size_t available;
    size_t allocated;
    Handle p;
} MemState;

typedef struct {
    size_t total;
    size_t available;
    size_t allocated;
    size_t freeable;
    Handle sp;
} StackState;

typedef struct {
    MemState ms;
    StackState ss;
} AllocatorState;

void stack_init(size_t size);
void stack_deinit(void);

Handle stack_alloc(size_t size);
Handle stack_realloc(Handle handle, size_t size);
void stack_free(Handle handle);

void stack_read(Handle handle, void *buff, size_t size);
void stack_write(Handle handle, const void *data, size_t size);
size_t stack_handle_cap(Handle handle);

#ifdef PROFILE
AllocatorState allocator_state(void);
void allocator_dump_state(FILE *stream, AllocatorState state);
#endif

static Allocator StackAllocator = {
    .alloc = stack_alloc,
    .realloc = stack_realloc,
    .free = stack_free,
};

#endif
