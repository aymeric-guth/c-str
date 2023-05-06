#include "str.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

str str_from_std(char *in, size_t size)
{
    // memcpy in dans buffer alloué
    return (str) {
        .data = in, .capacity = size, .size = size
    };
}

char *str_to_std(str in)
{
    if (in.size <= in.capacity)
        in.data[in.size + 1] = '\0';
    else
        in.data[in.size - 1] = '\0';

    return in.data;
}

str str_new(size_t size)
{
    char *p = malloc(size);
    assert(p != NULL);
    memset(p, 0, size);
    return (str) {
        .size = 0, .capacity = size, .data = p
    };
}

bool str_equal(str s1, str s2)
{
    if (s1.size != s2.size)
        return false;

    size_t n = 0;

    while (n < s1.size) {
        if (s1.data[n] != s2.data[n])
            return false;

        n++;
    }

    return true;
}

void str_concat(str in, str *out)
{
    if (out->capacity < (in.size + out->size)) {
        fprintf(stderr, "out str does not have required capacity");
        exit(0);
    }

    size_t ofst = out->size > 0 ? out->size : 0;
    memcpy(out->data + ofst, in.data, in.size);
    out->size += in.size;
}

void str_print(str s)
{
    printf("data=%s\nsize=%zu\ncapacity=%zu\n", s.data, s.size, s.capacity);
}
