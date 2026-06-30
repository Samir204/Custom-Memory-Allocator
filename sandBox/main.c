
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


int main(int argc, char *argv[]){

    const char *filename= "Custom Memory Allocator.rtf";
    int fd = open(filename, O_RDONLY);

    if(fd == -1){ // error, didnt open
        perror("open failed");
        return 1;
    }

    struct stat sb;
    if(fstat(fd,&sb) == -1){ // error couldn't opern struct
        perror("fstat failed");
        return 1;
    }

    // map the file in to memory
    char *mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(mapped == MAP_FAILED){
        // failed to map
        perror("map failed");
        close(fd);
        return 1;
    }

    // Use the mapped memory
    printf("First chars: %c\n", mapped[0]);

    // Clean
    munmap(mapped, sb.st_size);
    // munmap is a C system call defined in <sys/mman.h> used to remove (unmap) a previously 
    // created memory mapping from the virtual address space of the calling process.
    close(fd);
    return 0;
    

}