#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

/*
Build a block header struct
This is the core data structure your allocator will use. 
Nothing runs yet just get comfortable with the layout.
*/

#define MAGIC_NUMBER 0xDEADBEEF


typedef struct Block {
    size_t size;    // usable bytes AFTER this header
    int is_free;     // 1 = available, 0 = in use
    struct Block *next; // next block in the heap linked list
    struct Block *prev; // previous block
    uint32_t magic; // new field
} Block;


// function to free the block
void my_free(void *ptr){
    if(!ptr) return; // check if the pointer is NULL

    //getting the block
    Block *block = (Block*)ptr - 1;

    // validate if the block has the magic number
    // meaning: checking if the block is corrupted or not before touching anything
    if(block->magic != MAGIC_NUMBER){
        fprintf(stderr, "ERROR: invalid or corrupted pointer passed to free!\n");
        return;
    }
    //the block has the magic number, meaning its not corrupted
    // now we can free it
    block->is_free = 1;
}





int main(){

    printf("sizeof(Block) = %zu bytes\n", sizeof(Block));

    // Imagine the allocator placed a block at address 0x1000
    // The user gets a pointer AFTER the header:
    //   [0x1000 .. 0x1018] = Block header (24 bytes on 64-bit)
    //   [0x1018 .. ......] = user data starts here

    // Given a user pointer, you can always recover the header:
    // Block *header = (Block*)user_ptr - 1;

    printf("///\n");
    printf("sizeof(Block) without magic was: 32 bytes (on 64-bit)\n");
    printf("sizeof(Block) with magic is:     %zu bytes\n", sizeof(Block));
    printf("my_free function:\n");
    int x=10;
    my_free(&x);
    printf("////\n");



    // Demonstrate pointer arithmetic
    Block fake_block = {.size = 64, .is_free = 1, .next = NULL, .prev = NULL};
    Block *header = &fake_block;

    // The user pointer is just past the header
    void *user_ptr = (void*)(header + 1);

    printf("Header at: %p\n", (void*)header);
    printf("User date at:%p\n", user_ptr);
    printf("Dif: %zu bytes (=sizeof(Block))\n",
            (char*)user_ptr - (char*)header);

    // Recover header from user pointer
    Block *recovered = (Block*)user_ptr - 1;
    printf("recovered header size of field: %zu\n", recovered->size);

    return 0;
}





















