#include <unistd.h>
#include <stdio.h>

int main() {
    void *result = sbrk(-999999999);
    if (result == (void*)-1) {
        perror("sbrk failed");
        // Output: sbrk failed: Cannot allocate memory
    }
    return 0;
}

/*
void *origen = sbrk(0); is just asking where the current break is
void *new = sbrk(1024); move the program breake 1024 bytes ahead, and itll return the old break
void *cur = sbrk(0); is just asking where the break is, itll return the new break posision
sbrk(-1024); move the break backwords 
printf("break restored: %p\n", sbrk(0)); this will print the old break pos






*/



