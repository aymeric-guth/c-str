#ifndef _STR_H
#define _STR_H

#include <stdlib.h>
#include <stdbool.h>

#include "allocator.h"

typedef struct {
    char *data;
    size_t size;
    size_t capacity;
    Allocator allocator;
} str;

str str_from_std(char *, size_t);
char *str_to_std(str);
str str_new(size_t);
bool str_equal(str, str);
void str_concat(str in, str *out);
void str_print(str);

#endif
