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



void  *my_malloc(size_t size);
void   my_free(void *ptr);
void  *my_realloc(void *ptr, size_t size);
void  *my_calloc(size_t nmemb, size_t size);
void   print_heap(void);








