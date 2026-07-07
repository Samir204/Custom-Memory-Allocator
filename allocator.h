#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <pthread.h>

static pthread_mutex_t alloc_lock = PTHREAD_MUTEX_INITIALIZER;


typedef struct Block {
    size_t size;
    int is_free;
    struct Block *next;
    struct Block *prev;

    uint32_t magic;
} Block;

typedef enum {
    STRATEGY_FIRST_FIT,
    STRATEGY_BEST_FIT,
    STRATEGY_BUDDY
} AllocStrategy;

void set_strategy(AllocStrategy s);

#define MAGIC_ALLOC 0xDEADBEEF // block in use
#define MAGIC_FREE 0xDEADDEAD // block was free 





void  *my_malloc(size_t size);
void   my_free(void *ptr);
void  *my_realloc(void *ptr, size_t size);
void  *my_calloc(size_t nmemb, size_t size);
void   print_heap(void);

double fragmentation_ratio(void);

void *buddy_malloc(size_t size);
void buddy_free(void *ptr);




