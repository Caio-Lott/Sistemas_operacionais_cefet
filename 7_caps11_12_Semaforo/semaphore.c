// gcc -lrt -lpthread semaphore.c -o s && ./s

#include <semaphore.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

#define SHM_NAME        "sum"
#define NUM_PROCESSES   100
#define NUM_OPERATIONS  100000

// Both the sum variable and the semaphore should be on shared memory
typedef struct {
    double sum;
    sem_t sem;
} shared_data;


shared_data * createSharedMemory(){
    // Configuration parameters
    int shm_fd;
    size_t shm_size = sizeof(shared_data); // Tamanho exato para N resultados
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
    // Pointer to the mapping
    shared_data *sm_pointer;
    sm_pointer = mmap(0, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (sm_pointer == MAP_FAILED) {
        perror("mmap failed");
        shm_unlink(SHM_NAME); // Try to clear memory
        exit(1);
    }

    // Close file descriptor, but the mapping stays
    close(shm_fd); 
    printf("Created shared memory successfully!\n");
    return sm_pointer;
}

void createProcesses(int * f_val, int * proc_idx){
    // Process creation
    *f_val = 1;
    *proc_idx = 0;
    
    while(*f_val > 0 && *proc_idx < NUM_PROCESSES){
        *f_val = fork();
        if(*f_val < 0){
            perror ("Error while creating process") ;
            exit(1); 
        }
        *proc_idx = *proc_idx + 1;
        if(*f_val == 0){
            printf("Child %i created\n", *proc_idx);
        }
    }
}

void childWork(shared_data * sd, int * proc_idx){
    for(int i=0; i < NUM_OPERATIONS; i++){
        sem_wait(&sd->sem); // down
        sd->sum = sd->sum + 1;
        sem_post(&sd->sem); // up
    }
    // printf("%i done! last value: %.0f\n", *proc_idx, *sum);
    printf("%i done\n", *proc_idx);
    exit(0);
}

void parentWork(shared_data * sd){
    for(int i=0; i < NUM_PROCESSES; i++){
        wait(0);
    }
    printf("Expected %i, got %.0f\n", NUM_PROCESSES * NUM_OPERATIONS, sd->sum);
}

int main() {

    // Create shared memory
    shared_data *sd = createSharedMemory();
    sd->sum = 0;

    // Initialize semaphore
    if(sem_init(&(sd->sem), 1, 1) != 0){
        perror("Semaphore initialization failed");
        exit(1);
    }

    // Create processes
    int proc_idx;
    int f_val;
    createProcesses(&f_val, &proc_idx);
    
    // Child work
    if(f_val==0){
        childWork(sd, &proc_idx);
    }
    
    // Parent work
    if(f_val>0){
        parentWork(sd);
    }
    
    return 0;
}