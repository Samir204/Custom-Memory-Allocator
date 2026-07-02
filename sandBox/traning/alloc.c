/*
A tiny first-fit allocator
This is the Week 1 target. 
*/

#include <unistd.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef struct Block {
    size_t  size;
    int     is_free;
    struct Block *next;
    struct Block *prev;

    //
    size_t rem_size;
} Block;

#define HEADER_SIZE sizeof(Block)

static Block *heap_start = NULL;

// Walk the list; return the first free block >= size, or NULL
static Block *find_free(size_t size) {
    Block *cur = heap_start;
    while (cur) {
        if (cur->is_free && cur->rem_size >= size) // <---
            return cur;
        cur = cur->next;
    }
    return NULL;
}

// Ask the OS for more space and append a new block
static Block *extend_heap(size_t size) {
    Block *block = sbrk(HEADER_SIZE + size);
    if (block == (void*)-1) return NULL;
    block->size    = size;
    block->rem_size = size;    ////
    block->is_free = 0;
    block->next    = NULL;
    block->prev    = NULL;

    // Link into list
    if (heap_start == NULL) {
        heap_start = block;
    } else {
        // Find the last block
        Block *cur = heap_start;
        while (cur->next) cur = cur->next;
        cur->next   = block;
        block->prev = cur;
    }
    return block;
}

// block size = 10
// i need 2
// rem = 8


void *my_malloc(size_t size) {
    if (size == 0) return NULL;

    Block *block = find_free(size);
    if (block) {
        block->is_free = 0;
    } else {
        block = extend_heap(size);
        if (!block) return NULL;
    }
    return (void *)(block + 1); // pointer past the header
}

void my_free(void *ptr) {
    if (!ptr) return;
    Block *block = (Block*)ptr - 1; // recover header
    block->is_free = 1;
}

//heap_start
// prints each block's address, size, and is_free
void print_heap(){
    Block *cur= heap_start;
    if(!cur) return;
    while (cur){
        printf("addres: %p\n", (void*)cur);
        printf("size: %zu\n", cur->size);
        printf("is free? %d\n", cur->is_free);
        if (cur->next!=NULL){
            printf("next: \n");
        }
        
        cur=cur->next;
    }
    




}

int main() {
    // Basic allocation
    int *a = my_malloc(sizeof(int) * 4);
    a[0] = 10; a[1] = 20; a[2] = 30; a[3] = 40;
    printf("a = [%d, %d, %d, %d]\n", a[0], a[1], a[2], a[3]);

    char *s = my_malloc(32);
    strcpy(s, "hello allocator");
    printf("s = %s\n", s);

    printf("heap printer before free:\n");
    print_heap();

    my_free(a);
    printf("Freed a. Now allocate again...\n");

    printf("///////////\n");
    printf("heap printer after free:\n");
    print_heap();

    // This should REUSE the block we just freed (first-fit)
    int *b = my_malloc(sizeof(int) * 10);
    b[0] = 99; b[1] = 88;
    printf("b = [%d, %d]  (should reuse a's block)\n", b[0], b[1]);
    printf("b ptr: %p,  a was: %p — same? %s\n",
           (void*)b, (void*)a, b == a ? "YES" : "NO");


    return 0;
}











































































