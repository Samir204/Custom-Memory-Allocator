#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
// #include <stdint.h>


typedef struct Block {
    size_t size;
    int is_free;
    struct Block *next;
    struct Block *prev;
} Block;












