#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>




int main(){
    void *mem= mmap(
        NULL, // let the OS pick the address
        2, // size or lenght
        PROT_READ | PROT_WRITE, //permissions
        MAP_PRIVATE | MAP_ANONYMOUS, // not backed by a file 
        -1, // no file discription
        0
    );

    if(mem == MAP_FAILED){
        perror("mmap failed");
        return 1;
    }

    void *a = sbrk(0);
    printf("a= %p\n",a);

    
    printf("mmap gave us: %p\n", mem); 
    // how much the OS gave us memory space 
    
    // check how much the OS gave us:
    long page_size = sysconf(_SC_PAGESIZE);
    printf("Page size:  %ld bytes\n", page_size);
    printf("mmap address: %p\n", mem);

    char *buf = (char *)mem;
    strcpy(buf, "hello from map");
    printf("stored: %s\n", buf);
    // printing what we wrote

    printf("char at pos 0: %c\n", buf[0]);

    //give it back to the OS
    munmap(mem, 4096);
    printf("Memory returned to OS.\n");


    // printf("char at pos 0: %c\n", buf[0]);
    // a
    // its returning: zsh: segmentation fault  ./mmap_tr
    // thats because we gave the ocupaed memory back to OS in munmap, meaning the program is trying
    // to read/reach for an area that its not saposed to reach and thats why its giving 
    // segmentation fault. and its also because munmap actually invalidates the memory, unlike free in most implementations.

    //b
    /*
    The OS always maps in multiples of the page size (4096 bytes on x86-64), 
    so even though you asked for 2 bytes, you physically have a full 4096-byte page. 
    This is why allocators that use mmap internally always round up their requests to 
    the nearest page — requesting 17 bytes wastes almost 4079 bytes of a page, so you 
    batch up requests to make full use of each mapping.
    
    */

    //c
    /*
    You get SIGSEGV the moment you write. The CPU's memory management unit checks the 
    permission bits on the page table entry before every memory access — a write to a 
    read-only page raises a hardware fault which the OS converts to SIGSEGV. This is how 
    the OS enforces memory protection between regions: your code segment (.text) is mapped 
    PROT_READ | PROT_EXEC but not PROT_WRITE, which is why overwriting your own code at 
    runtime also crashes.


    we get a bus error because we are trying to write on to memory when we 
    explicitly said that we want to read only using mmap.
    A bus error is a hardware-level fault that occurs when a process attempts 
    to access memory that the CPU cannot physically address, such as accessing an 
    invalid physical address, performing unaligned memory access on strict-architecture 
    systems, or encountering errors with memory-mapped files (e.g., truncating a mapped 
    file while it is in use).
    */






    return 0;
}







