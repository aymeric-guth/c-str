#include "str.h"
#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "stack.h"

int main(void)
{
    /* char s[] = "lol"; */
    /* str s1 = str_from_std(s, 4); */
    /* char *s2 = str_to_std(s1); */
    /* str s3 = str_new(4); */
    /* printf("s2=%s\n", s2); */

    /* char s1[] = "test"; */
    /* char s2[] = "test"; */
    /* char s3[] = "test101"; */
    /* char s4[] = "tes "; */
    /* str s11 = str_from_std(s1, 5); */
    /* str s12 = str_from_std(s2, 5); */
    /* str s13 = str_from_std(s3, 8); */
    /* str s14 = str_from_std(s4, 5); */
    /* assert(str_equal(s11, s12) == true); */
    /* assert(str_equal(s11, s13) == false); */
    /* assert(str_equal(s11, s14) == false); */

    /* str s = str_new(100); */
    /* char s1[] = "lol"; */
    /* char s2[] = "test"; */
    /* str s11 = str_from_std(s1, strlen(s1)); */
    /* str s12 = str_from_std(s2, strlen(s2)); */
    /* str_concat(s11, &s); */
    /* str_concat(s12, &s); */
    /* printf("str=%s\n", str_to_std(s)); */

    /* stack_init(100000); */
    /* size_t handle = StackAllocator.alloc(1000); */
    /* const char *data = "test1001 oijefij32hf -9jv9vj-wjv"; */
    /* char buff[4096] = {0}; */
    /* stack_write(handle, data, strlen(data) + 1); */
    /* stack_read(handle, buff, 4096); */
    /* printf("data=%s\n", buff); */
    /* const char *data2 = "wooooot"; */
    /* stack_write(handle, data2, strlen(data2) + 1); */
    /* stack_read(handle, buff, 4096); */
    /* printf("data=%s\n", buff); */
    /* StackAllocator.free(handle); */

    /* // initialize allocator */
    /* stack_init(100000); */
    /* // allocates 1000 bytes */
    /* Handle s1 = StackAllocator.alloc(1000); */
    /* // writes data to handle s1 */
    /* { */
    /*     char *data = "module level __getattr__ (intermediate) anthony explains"; */
    /*     stack_write(s1, data, strlen(data)); */
    /* } */
    /* /1* // allocates 1000 bytes *1/ */
    /* Handle s2 = StackAllocator.alloc(1000); */
    /* { */
    /*     char data[] = "Yves Bréchet, audition Souveraineté et indépendance énergétique de la France, 29 novembre 2022"; */
    /*     stack_write(s1, data, strlen(data)); */
    /* } */
    /* { */
    /*     char buff[1000] = {0}; */
    /*     stack_read(s1, buff, stack_handle_cap(s1)); */
    /*     printf("s1=%s\n", buff); */
    /* } */
    /* StackAllocator.free(s1); */
    /* { */
    /*     char buff[1000] = {0}; */
    /*     stack_read(s2, buff, stack_handle_cap(s2)); */
    /*     printf("s2=%s\n", buff); */
    /* } */
    /* stack_free(s2); */
    /* stack_deinit(); */
    /* printf("sizeof=%zu\n", sizeof(size_t)); */
}
