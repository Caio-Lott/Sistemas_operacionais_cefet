// cd /home/limao/Documents/2025.2/LSO/repo/Sistemas_operacionais_cefet/9_cap14_ConversaoEnderecos/TP_Lógico_Físico_Partes1-2-3 && gcc p1.c -o 1 && ./1

#define _GNU_SOURCE
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

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

int f1(){
    int x;
    for(x=1;x<10;x++);
    return x;
}

int main() {
    
    int a = 0;
    int f_val = fork();

    // Error
    if(f_val<0){
        printf("Error while forking\n");
    }
    
    // Child
    if(f_val==0){
        printf("Filho\n");
        printf("\'a\':\t");
        vtop((uintptr_t) &a);
        printf("\'f1\':\t");
        vtop((uintptr_t) &f1);
    }
    
    //Parent
    if(f_val>0){
        sleep(1);
        printf("Pai\n");
        printf("\'a\':\t");
        vtop((uintptr_t) &a);
        printf("\'f1\':\t");
        vtop((uintptr_t) &f1);
    }
    return 0;
}
