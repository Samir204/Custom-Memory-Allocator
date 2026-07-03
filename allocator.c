#include "allocator.h"

/*
here is just a sea of functions 
that make the project work
the time, hate, anger and energy wasted is impecable 


*/


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
    Block *block = sbrk(HEADER_SIZE + size);
    if(block == (void*)-1) return NULL;
    block->size = size;
    block->is_free = 0;
    block->next = NULL;
    block->prev = NULL;

    // link into list
    if(heap_start == NULL){
        heap_start = block;
    }
    else{
        Block *cur= heap_start;
        while (cur!=NULL) cur = cur->next;
        cur->next = block;
        block->prev = cur;        
    }
    return block;
}

// here its 8 because CPU wants addresses that are mult of 8
#define ALIGN 8
#define ALIGN_SIZE(size) (((size) + (ALIGN - 1)) & ~(ALIGN -1))
/*
this is to round up every requested size before allocating
the reason is, the old my_malloc just works on the input size then 
returns pointers that might not be properly aligned. On 64-bit systems,
 the CPU expects data to sit at addresses that are multiples of 8 (or 16 
 for some types like double and SIMD).
 so if it returns a misaligned pointer, i WILL get silent performance penalties 
 or a crashes on some architectures, which is fucked 
*/

void *my_malloc(size_t size){
    if(size == 0) return;
    size = ALIGN_SIZE(size);
    Block *block = find_free(size);
    if(block){
        block->is_free = 0;
    }
    else{
        block = extend_heap(size);
        if(!block) return NULL;
    }
    return (void*)(block+1);
    // pointer past the head
}


void my_free(void *ptr){
    if(ptr == NULL) return;
    Block *block = (void*)ptr -1;
    // recover the head
    block->is_free = 1;
    // freeing is just saying that this block 
    // is free to use, but the data is still there
}



void prinf_heap(){
    if(heap_start == NULL) return;
    Block *cur = heap_start;
    int i=0; // the cur block
    while (cur != NULL){
        printf("Block: %d | address: %p | size= %-5zu | %s\n",
            i++, (void*)cur, cur->size, cur->is_free? "FREE" : "USED");
        cur = cur->next;
    }
}
/*
    block spliter system, 
    the main reosen is because is if, for example, a block has a size of 10 bytes
    and we only need 2 bytes, we are wasting 8 for nothing, so why dont we use these 8 bytes 
    for a nother thing that can fit init? 
    so i made a system that checks the the remaning size of each block
    and check if its posible to save something init.
    efitions? yes
    did i learn something? yes
    did i wast 3h researching and understanding how and if its acctualy posible to 
    do it? yes 
    do i hate myself? also yes 
*/

// so here im checking if the leftover is big enough to hold a header + AT LEAST 8 BYTES 
// then take it
#define MIN_SPLIT (HEADER_SIZE + 8)

static void *split_block(Block *block, size_t size){
    if(block->size < size + MIN_SPLIT) return;
    // said block is smoler that whats needed, sooo return 

    Block *new_block = (Block*)((char*)(block + 1) + size);
    new_block->size = size;
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev= block;

    if(block->next)
        block->next->prev= new_block;

    block->next = new_block;
    block->size = size;
    // shrink the original block to the exact fit
}


void *my_realloc(void *ptr, size_t size){
    if(!ptr) return my_malloc(size);
    if(size == 0){
        my_free(ptr);
        return NULL;
    }

    Block *block = (Block*)ptr - 1;
    if(block->size == 0) return ptr;

    void *new_ptr = my_malloc(size);
    if(!new_ptr) return NULL;
    memcpy(new_ptr, ptr, block->size);
    my_free(ptr);
    return new_ptr;
}

void *my_calloc(size_t nmemb, size_t size){
    void *ptr = my_malloc(nmemb * size);
    if(ptr) memset(ptr, 0, nmemb * size); // zero the memory
    return ptr;
}






























