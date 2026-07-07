#include "allocator.h"

/*
here is just a sea of functions 
that make the project work
the time, hate, anger and energy wasted is impecable 


*/

// WEEK 1:

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

    block->magic = MAGIC_ALLOC;

    // link into list
    if(heap_start == NULL){
        heap_start = block;
    }
    else{
        Block *cur= heap_start;
        while (cur->next!=NULL)
             cur = cur->next;
        
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

static void *split_block(Block *block, size_t size);

void *my_malloc(size_t size){
    pthread_mutex_lock(&alloc_lock);

    if (size == 0) { 
        pthread_mutex_unlock(&alloc_lock); 
        return NULL; 
    }

    size = ALIGN_SIZE(size);

    Block *block = find_free(size);
    if (block) {
        split_block(block, size);
        block->is_free = 0;
        block->magic = MAGIC_ALLOC;
    } else {
        block = extend_heap(size);
        if (!block) { 
            pthread_mutex_unlock(&alloc_lock); 
            return NULL; 
        }
    }

    pthread_mutex_unlock(&alloc_lock);
    return (void*)(block + 1);
    // pointer past the head
}

void *coalesce(Block *block);

void my_free(void *ptr){
    pthread_mutex_lock(&alloc_lock);

    if (ptr == NULL) { pthread_mutex_unlock(&alloc_lock); return; }
    Block *block = (Block*)ptr - 1;

    if (block->magic == MAGIC_FREE) {
        fprintf(stderr, "ERROR: double-free at %p\n", ptr);
        pthread_mutex_unlock(&alloc_lock);
        return;
    }
    if (block->magic != MAGIC_ALLOC) {
        fprintf(stderr, "ERROR: invalid pointer at %p\n", ptr);
        pthread_mutex_unlock(&alloc_lock);
        return;
    }

    block->magic   = MAGIC_FREE;
    block->is_free = 1;
    coalesce(block);

    pthread_mutex_unlock(&alloc_lock);
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
    if(block->size < size + MIN_SPLIT) return NULL;
    // said block is smoler that whats needed, sooo return 

    Block *new_block = (Block*)((char*)(block + 1) + size);
    new_block->size = block->size - size - HEADER_SIZE;
    new_block->is_free = 1;
    new_block->next = block->next;
    new_block->prev= block;
    new_block->magic = MAGIC_ALLOC;

    if(block->next)
        block->next->prev= new_block;

    block->next = new_block;
    block->size = size;
    // shrink the original block to the exact fit
}


void *my_realloc(void *ptr, size_t size){
    pthread_mutex_lock(&alloc_lock);
    
    if (!ptr) {
        pthread_mutex_unlock(&alloc_lock);
        return my_malloc(size);   // my_malloc will lock again — so unlock first
    }
    if (size == 0) {
        pthread_mutex_unlock(&alloc_lock);
        my_free(ptr);
        return NULL;
    }

    size = ALIGN_SIZE(size);
    Block *block = (Block*)ptr - 1;

    // already big enough --> return
    if (block->size >= size) {
        pthread_mutex_unlock(&alloc_lock);
        return ptr;
    }

    // try to absorb the next block if its free
    if (block->next && block->next->is_free){
        size_t combined = block->size + HEADER_SIZE + block->next->size;
        if (combined >= size){ // absorb next block in to this one
            block->size = combined;
            block->next = block->next->next;
            if (block->next){
                block->next->prev = block;
            }
            pthread_mutex_unlock(&alloc_lock);

            return ptr; 
        }
        
    }
    


    // not free, sooo alloc new -> copy -> free old
    // -> means then
    pthread_mutex_unlock(&alloc_lock);   
    // unlock before calling my_malloc/my_free

    void *new_ptr = my_malloc(size);
    if(!new_ptr) return NULL;
    memcpy(new_ptr, ptr, block->size);
    my_free(ptr);
    return new_ptr;
}

void *my_calloc(size_t nmemb, size_t size){
    void *ptr = my_malloc(nmemb * size);   // my_malloc handles its own lock
    if(ptr) memset(ptr, 0, nmemb * size); // zero the memory
    return ptr;
}



// WEEK 2 & 3:

// a system that scans the whole list and 
// returns the smallest block that fits
// ??? instead of the first one ??? 
static Block *find_best_fit(size_t size){

    Block *cur = heap_start;
    Block *best = NULL;
    while (cur){
        if(cur->is_free && cur->size >= size){
            if (best == NULL || cur->size < best->size)
                best = cur;
        }
        cur = cur->next;
    }
    return best;
}

// after a free, merge the block with its neighbors if they're also free
// because why have to free block when i can have one free block with the size of both 
// its also good because in this way i can reduse the amount of checking for free blocks
// meaning. rather thatn checking two maybe free blocks, i can check only one 
void *coalesce(Block *block){

    if (block->next && block->next->is_free) {
        block->size += HEADER_SIZE + block->next->size;
        block->next  = block->next->next;
        if (block->next)
            block->next->prev = block;
    }

    // merge with PREV block if it's free
    if (block->prev && block->prev->is_free) {
        block->prev->size += HEADER_SIZE + block->size;
        block->prev->next  = block->next;
        if (block->next)
            block->next->prev = block->prev;
    }
}


/*
typedef struct Block {
    size_t size;
    int is_free;
    struct Block *next;
    struct Block *prev;
} Block;

-->  static Block *heap_start = NULL;
    
*/












