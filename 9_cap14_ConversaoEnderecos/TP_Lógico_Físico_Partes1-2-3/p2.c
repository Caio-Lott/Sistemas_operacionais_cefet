/*

TP Capítulo 9 - Laboratório de Sistemas Operacionais - CEFET-MG 2025.2

Henrique Rodrigues Lima         20193009473
Caio Fernandes lott Primola     20193001742

*/

#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <inttypes.h>

// Shared memory parameters
#define SHM_NAME "pi_shm"
#define SIZE 4096 // Tamanho do segmento de memória compartilhada (4KB é suficiente)

void vtop(uintptr_t vaddr) {

    FILE *pagemap;
    intptr_t paddr = 0;
    unsigned long offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
    uint64_t e;
    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e, sizeof(uint64_t), 1, pagemap)) {                
                if (e & (1ULL << 63)) { // page present ?
                    paddr = e & ((1ULL << 54) - 1); // pfn mask
                    paddr = paddr * sysconf(_SC_PAGESIZE);
                    paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
                }
            }
        }
        fclose(pagemap);
    }
        printf("Virt:\t%" PRIxPTR , vaddr);
		printf("\tPhys:\t%" PRIxPTR " \n", paddr);
}

int main() {

    // Configuration parameters
    int shm_fd;
    size_t shm_size = sizeof(int); // int used as example

    // Create the shared memory object
    shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open failed");
        exit(1);
    }

    // Define segment size
    if (ftruncate(shm_fd, shm_size) == -1) {
        perror("ftruncate failed");
        shm_unlink(SHM_NAME); // Try to clear memory
        exit(1); 
    }
    
    // Create another process
    int f_val = fork();
    
    // Child
    if(f_val==0){
        int a, b, c;
        a = rand();
        b = rand();
        c = rand();
        // printf("Child: a=%i, b=%i, c=%i\n", a, b, c);
    }
    
    // Parent (terminal waits for this one)
    if(f_val>0){
        char text [102] = "Sample text";
        for(int i=0;i<10;i++){
            strcat(text,"ahhhhhhh\n");
        }
        // printf("Parent: text is \'%s\'\n", text );
    }
    
    // Pointer to the mapping
    int *sm_pointer;
    sm_pointer = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sm_pointer == MAP_FAILED) {
        perror("mmap failed");
        shm_unlink(SHM_NAME); // Try to clear memory
        exit(1);
    }
    
    // Close file descriptor, but the mapping stays
    close(shm_fd); 
    // printf("Created shared memory successfully!\n");

    // Must use the pointer to initialize the physical memory
    *sm_pointer = rand();

    if(f_val == 0){
        printf("Child:\t");
        vtop((uintptr_t) mmap);
        exit(0); // Child quits and notifies the parent
    }
    
    if(f_val > 0){
        wait(NULL);
        printf("Parent:\t");
        vtop((uintptr_t) mmap);
    }
}

