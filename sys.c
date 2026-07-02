#include "lib.h"



#define HEADER_SIZE sizeof(Block)

static Block *heap_start = NULL;


// finding the first free block with a size greater/equal to the size
static Block *find_free(size_t size){
    Block *cur = heap_start;
    if(cur == NULL) return NULL;
    while(cur){
        if(cur->is_free && cur->size >= size)
            return cur;
        cur = cur->next;
    }
    return NULL;
}



// aksing the OS for more space
static Block *extend_heap(size_t size){
    


}






























