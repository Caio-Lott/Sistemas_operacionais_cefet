#include <semaphore.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

#define NUM_PROCESSES      100
#define NUM_OPERATIONS  100000

int sum = 0;
sem_t sem;

int main() {

    int id = 0;
    int status = 0;

    // Initialize semaphore
    if (sem_init(&sem, 1, 1) != 0) {
        perror("Semaphore initialization failed");
        return 1;
    }

    // Create processes
    for(int i=0; i < NUM_PROCESSES && id > 0; i++){
        id = fork();
    }

    
    // Child work
    if(id==0){
        for(int i=0; i < NUM_OPERATIONS; i++){
            sem_wait(&sem); // down
            sum++;
            sem_post(&sem); // up
        }
        printf("%i done!\n", id);
        return 0;
    }
    
    // Parent work
    if(id>0){
        printf("Pai vivo");
        for(int i=0; i < NUM_PROCESSES; i++){
            wait(&status);
            if(!WIFEXITED(status)){
                printf("Process terminated with error\n");
            }
        }
        printf("Expected %i, got %i\n", NUM_PROCESSES * NUM_OPERATIONS, sum);
    }
    return 0;
}