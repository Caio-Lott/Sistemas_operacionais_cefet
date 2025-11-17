#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/wait.h>

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
        printf(" %" PRIxPTR , vaddr);
		printf(" %" PRIxPTR " \n", paddr);
}

int main(){
	int a;
	vtop((intptr_t)&a);
}

