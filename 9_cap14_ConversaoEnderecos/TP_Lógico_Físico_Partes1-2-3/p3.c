/*

TP Capítulo 9 - Laboratório de Sistemas Operacionais - CEFET-MG 2025.2

Henrique Rodrigues Lima         20193009473
Caio Fernandes lott Primola     20193001742

clear ; cd /home/limao/Documents/2025.2/LSO/repo/Sistemas_operacionais_cefet/9_cap14_ConversaoEnderecos/TP_Lógico_Físico_Partes1-2-3 && gcc p3.c -o 3 && ./3

*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h> // Include for malloc and free
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <inttypes.h>

#define VETOR_SIZE 1025
#define PAGE_SHIFT 12 // Assumes 4096-byte pages (4KB)

// Function to convert virtual address to physical address
intptr_t vtop(uintptr_t vaddr, long page_size) {
    FILE *pagemap;
    intptr_t paddr = 0;
    // Calculates the offset in the /proc/self/pagemap file
    unsigned long offset = (vaddr / page_size) * sizeof(uint64_t);
    uint64_t e;

    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e, sizeof(uint64_t), 1, pagemap)) {                
                if (e & (1ULL << 63)) { // If bit 63 is set, the page is present (present bit)
                    paddr = e & ((1ULL << 54) - 1); // Extract the PFN (Page Frame Number)
                    paddr = paddr * page_size; // Convert PFN to physical base address
                    paddr = paddr | (vaddr & (page_size - 1)); // Add the page offset
                }
            }
        }
        fclose(pagemap);
    }
    
    return paddr; // Return 0 if not mapped or error
}

int main() {
    // Determine page size dynamically
    const long page_size = sysconf(_SC_PAGESIZE);

    printf("Vetor de %d inteiros:\t\t%d bytes\n", 
           VETOR_SIZE, VETOR_SIZE * (int) sizeof(int));
    printf("Tamanho da Pagina (Frame):\t%ld bytes\n\n", page_size);
    
    // Allocate the vector on the HEAP
    int *vetor = (int *) malloc(VETOR_SIZE * sizeof(int));
    if (vetor == NULL) {
        perror("malloc failed");
        return 1;
    }

    // Fill the vector
    for (int i = 0; i < VETOR_SIZE; i++) {
        vetor[i] = i; 
    }
    
    // Page number variables
    intptr_t current_frame = 0;
    intptr_t last_frame = 0;
    int page_cross_index = -1;
    
    // Iterate through all elements of the array
    for (int i = 0; i < VETOR_SIZE; i++) {

        uintptr_t vaddr = (uintptr_t) &vetor[i];
        intptr_t paddr = vtop(vaddr, page_size);
        
        if (paddr != 0) {
            current_frame = paddr / page_size; // Use division, which is safe for any page_size
        } else {
            current_frame = 0; 
        }

        // Check if the PFN has changed
        if (current_frame != 0 && current_frame != last_frame) {

            printf("Index: %d, \tVirtual: 0x%" PRIxPTR ", \tPhysical: 0x%" PRIxPTR ", \tPFN: 0x%" PRIxPTR "\n", 
                   i, vaddr, paddr, current_frame);
            
            last_frame = current_frame;
            if (page_cross_index == -1) {
                page_cross_index = i;
            }
        }
    }

    printf("\n");

    free(vetor);
    
    return 0;
}