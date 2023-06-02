#include "stack.h"
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    size_t start;
    size_t capacity;
    bool freed;
} Entry;

static size_t max_size = 0;
static char *mem = NULL;
static Handle p = 0;

#define STACK_SIZE 1024
static Entry Stack[STACK_SIZE] = {0};
static Handle sp = 0;


void stack_init(size_t size)
{
    if (mem != NULL)
        free(mem);

    mem = malloc(size);
    assert(mem != NULL && "Couldn't allocate base memory");
    max_size = size;
}

void stack_deinit(void)
{
    assert(mem != NULL && "allocator is not initialized");
    free(mem);
}

Handle stack_alloc(size_t size)
{
    assert(mem != NULL && "allocator is not initialized");
    assert(p + size < max_size && "overflowing base memory");
    assert(sp + 1 < STACK_SIZE && "overflowing stack size");
    Stack[sp] = (Entry) {
        .start = p, .capacity = size, .freed = false
    };
    p += size;
    return sp++;
}

Handle stack_realloc(Handle handle, size_t size)
{
    // size <= capacity -> do nothing
    // size > capacity -> realloc
    // passer par un modele ou la handle est un uuid, l'array servira a stocker les handle valides
    assert(true && "not implemented");
    return 0;
}

void stack_free(Handle handle)
{
    assert(mem != NULL && "allocator is not initialized");
    assert(handle <= sp && "handle not found");
    Entry *e = &Stack[handle];
    assert(e->freed == false && "handle has already been freed");
    e->freed = true;

    while (handle == sp - 1 && e->freed == true) {
#ifdef PROFILE
        printf("deallocating handle=%zu\n", handle);
        printf("=== STATE BEFORE FREE ===\n");
        allocator_dump_state(NULL, allocator_state());
#endif
        memset(mem, 0, e->capacity);
        p -= e->capacity;
        memset(Stack + sp - 1, 0, sizeof(Entry));
        sp--;
        handle--;
        e = &Stack[handle];
#ifdef PROFILE
        printf("=== STATE AFTER FREE ===\n");
        allocator_dump_state(NULL, allocator_state());
#endif
    }
}

void stack_read(Handle handle, void *buf, size_t cap)
{
    assert(mem != NULL && "allocator is not initialized");
    assert(buf != NULL && "buff is NULL");
    assert(handle < sp && "handle does not exist");
    Entry e = Stack[handle];
    assert(e.freed == false && "handle has been freed");
    assert(cap >= e.capacity && "overflowing buffer capacity");
    memcpy(buf, mem + e.start, e.capacity);
}

void stack_write(Handle handle, const void *data, size_t size)
{
    assert(mem != NULL && "allocator is not initialized");
    assert(size > 0 && "size must be > 0");
    assert(handle < sp && "handle does not exist");
    Entry e = Stack[handle];
    assert(size <= e.capacity && "overflowing handle capacity");
    memcpy(mem + e.start, data, size);
}

size_t stack_handle_cap(Handle handle)
{
    assert(mem != NULL && "allocator is not initialized");
    assert(handle < sp  && "handle does not exist");
    Entry e = Stack[handle];
    assert(e.freed == false && "handle has been freed");
    return e.capacity;
}

#ifdef PROFILE
AllocatorState allocator_state(void)
{
    assert(mem != NULL && "allocator is not initialized");
    size_t freeable = 0;
    size_t capacity = 0;

    for (size_t i = 0; i < sp; i++) {
        Entry e = Stack[i];

        if (e.freed == true)
            freeable++;

        capacity += e.capacity;
    }

    return (AllocatorState) {
        .ms = (MemState) {
            .available = max_size - p,
            .total = max_size,
            .p = p,
        },
        .ss = (StackState) {
            .available = STACK_SIZE - sp,
            .total  = STACK_SIZE,
            .allocated = capacity,
            .freeable = freeable,
            .sp = sp,
        }
    };
}
#endif

#ifdef PROFILE
void allocator_dump_state(FILE *stream, AllocatorState state)
{
    assert(mem != NULL && "allocator is not initialized");

    if (stream == NULL)
        stream = stderr;

    fprintf(stream, "AllocatorState(\n\t.MemState(.available=%zu, .total=%zu, .p=%zu)\n\t.StackState(.available=%zu, .total=%zu, .allocated=%zu, .freeable=%zu, .sp=%zu)\n)\n",
        state.ms.available,
        state.ms.total,
        state.ms.p,
        state.ss.available,
        state.ss.total,
        state.ss.allocated,
        state.ss.freeable,
        state.ss.sp
    );
}
#endif
