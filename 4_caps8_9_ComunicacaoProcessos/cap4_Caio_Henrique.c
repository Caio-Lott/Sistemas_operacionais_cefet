/*

TP Capítulo 4 - Laboratório de Sistemas Operacionais - CEFET-MG 2025.2

Henrique Rodrigues Lima         20193009473
Caio Fernandes lott Primola     20193001742

Este arquivo corresponde aos TPs 07, 08 e 09

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

#include <errno.h>
#include "check.h"
#include <mqueue.h>

// Define the valid options for the second argument
#define OPTION_SHARED_MEMORY    "sm"
#define DESC_SHARED_MEMORY      "Shared Memory"
#define OPTION_MESSAGE_QUEUE    "mq"
#define DESC_MESSAGE_QUEUE      "Message Queue"
#define OPTION_PIPES            "p"
#define DESC_PIPES              "Pipes"

// Shared memory parameters
#define BILLION 1000000000.0
#define SHM_NAME "pi_shm"
#define SIZE 4096 // Tamanho do segmento de memória compartilhada (4KB é suficiente)

// Queue parameters
#define QUEUE_NAME  "/test_queue1"
#define MAX_SIZE    1024
#define MSG_STOP    "exit"

// Pipe parameters
#define BUFFER_SIZE 25
#define READ_END	0
#define WRITE_END	1

unsigned long long int passos = 11 * BILLION; // Total de passos

// Function to print the usage message
void print_usage(char *program_name) {
    printf("Usage: %s <number of processes> <operation>\n", program_name);
    printf("  <number of processes>: Any integer value greater than 0.\n");
    printf("  <operation>: One of the following options:\n");
    printf("    - %s\t(%s)\n", OPTION_SHARED_MEMORY, DESC_SHARED_MEMORY);
    printf("    - %s\t(%s)\n", OPTION_MESSAGE_QUEUE, DESC_MESSAGE_QUEUE);
    printf("    - %s\t\t(%s)\n", OPTION_PIPES, DESC_PIPES);
}

void createProcesses(int num_proc, int *f_val, int *proc_idx){
    // Process creation
    *f_val = 1;
    *proc_idx = 0;
    
    while(*f_val > 0 && *proc_idx < num_proc){
        *f_val = fork();
        if(*f_val < 0){
            perror ("Error while creating process") ;
            exit(1); 
        }
        *proc_idx = *proc_idx + 1;
        // printf("f_val: %i proc_idx: %i num_proc: %i\n", *f_val, *proc_idx, num_proc);
        // if(*f_val == 0){
        //     printf("Processo filho: %4ld - PID: %5d Pai:%5d\n", *proc_idx, getpid(), getppid());
        // }
    }
}

double * createSharedMemory(int num_proc){
    // Configuration parameters
    int shm_fd;
    size_t shm_size = num_proc * sizeof(double); // Tamanho exato para N resultados
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
    double *sm_pointer;
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

double proc_pi_calc(int num_proc, int proc_idx){
            
    double sum = 0.0, x;
    unsigned long long int i;
    double h = 1.0 / passos;

    // Each child calculates its share of the pi value
    for (i = proc_idx; i <= passos; i += num_proc) {
        x = h * ((double)i - 0.5); 
        sum += 4.0 / (1.0 + x * x); 
    }
    return h*sum;
}

int sm_calc(int num_proc){
    
    // Create shared memory
    double *sm_pointer = createSharedMemory(num_proc);

    // Create processes
    int proc_idx;
    int f_val;
    createProcesses(num_proc, &f_val, &proc_idx);

    // Child productor
    if(f_val==0){
        
        printf("\tSubprocess %4ld - PID: %5d Parent:%5d\n", proc_idx-1, getpid(), getppid());
        
        // Writes to the correspondent memory field
        sm_pointer[proc_idx-1] = proc_pi_calc(num_proc, proc_idx);
        
        exit(0); // Child quits and notifies the parent
    }

    // Parent consumer
    if(f_val>0){
        for (int i = 0; i < num_proc; i++) {
            wait(NULL); // Wait for any child process
        }

        double pi = 0.0;
        for (int i = 0; i < num_proc; i++)		
            pi = pi + sm_pointer[i];

        printf("\nValor de pi %.8f\n", pi);
    }
}

int mq_calc(int num_proc){
    // Create processes
    int proc_idx;
    int f_val;
    createProcesses(num_proc, &f_val, &proc_idx);

	mqd_t mq;
	struct mq_attr attr;

    /* Inicializa os atributos da fila*/
	/*"0": o processo será bloqueado caso não haja 
	mensagens na fila. "O_NONBLOCK": a função de 
	leitura retorna código de erro e não é bloqueada.*/	
	attr.mq_flags = 0; 
	//Número máximo de mensagens na fila. 
	attr.mq_maxmsg = num_proc;
	//Tamanho máximo de cada mensagem.
	attr.mq_msgsize = MAX_SIZE;
	//Número de mensagens que estão atualmente na fila.
	attr.mq_curmsgs = 0;

    // Children
    if(f_val==0){
        char buffer[MAX_SIZE];

        double val = proc_pi_calc(num_proc, proc_idx);
        sprintf(buffer, "%lf", val);

        /* Abre a fila de mensagens.*/
        mq = mq_open(QUEUE_NAME, O_WRONLY);
        CHECK((mqd_t)-1 != mq);

        CHECK(mq_send(mq, buffer, MAX_SIZE, 0) >= 0);
        printf("Proc %i sent %f\n", proc_idx, val);
        CHECK(mq_getattr(mq, &attr) != -1);
        printf("%ld messages on queue.\n",attr.mq_curmsgs);

        /* Encerra a conexão com a fila */
        CHECK((mqd_t)-1 != mq_close(mq));
        
        exit(0);

    }

    // Parent
    if(f_val>0){

        double pi_val = 0;
        char buffer[MAX_SIZE + 1];

        /* Abre a fila de mensagens.*/
        mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);

        printf("Pai esperando por filhos\n");

        // Espera por filhos
        // for (int i = 0; i < num_proc; i++) {
        //     wait(NULL); // Wait for any child process
        // }

        //Continua a execução do programa se a condição 
        //abaixo for verdadeira.
        CHECK(mq !=(mqd_t)-1); //se (mq != -1)

        for(int i=0; i<num_proc; i++){
            ssize_t bytes_read;
            bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
            CHECK(bytes_read >= 0);
            //Adiciona um '\0' ao final da mensagem lida.
            buffer[bytes_read] = '\0';

            //Exibe a mensagem recebida.
            printf("Received: %s\n", buffer);
            char *endptr;
            errno = 0; 
            double result_value = strtod(buffer, &endptr);
            pi_val += result_value;

            sleep(1); 
        }

        printf("\nValor de pi %.8f\n", pi_val);
        
        //Fecha a fila. 
        CHECK((mqd_t)-1 != mq_close(mq));

        if (mq_unlink(QUEUE_NAME) == -1) {
            perror("mq_unlink falhou");
            // Se falhar, a fila pode permanecer em /dev/mqueue
        }
    }
}

int p_calc(int num_proc){

    int fd[num_proc][2];

	/* create the pipe */
    for(int i=0; i<num_proc; i++){
        if (pipe(fd[i]) == -1) {
            fprintf(stderr,"Pipe failed");
            return 1;
        }
        printf("Created pipe %i\n", i);
    }

    // Create processes
    int proc_idx;
    int f_val;
    createProcesses(num_proc, &f_val, &proc_idx);

    // Filho
    if(f_val==0){
        
        char write_msg[BUFFER_SIZE];
		printf("Child %d working\n", proc_idx);

        double val = proc_pi_calc(num_proc, proc_idx);
        sprintf(write_msg, "%lf", val);

		/* close the unused end of the pipe */
		close(fd[proc_idx-1][READ_END]);

		/* write to the pipe */
		// printf("Child writing to the pipe %i: %s\n", proc_idx, write_msg);
		write(fd[proc_idx-1][WRITE_END], write_msg, strlen(write_msg)+1);

		/* close the write end of the pipe */
		close(fd[proc_idx-1][WRITE_END]);
        exit(0);
    }

    // Pai
    if(f_val>0){

        double pi_val = 0;

        // Espera por filhos
        for (int i = 0; i < num_proc; i++) {
            // wait(NULL); // Wait for any child process

            char read_msg[BUFFER_SIZE];
            /* close the unused end of the pipe */
            close(fd[i][WRITE_END]);
            //read blocks client as long as there is a writer.
            read(fd[i][READ_END], read_msg, BUFFER_SIZE);
            printf("Parent - content read from the pipe %i: %s\n", i+1, read_msg);

            char *endptr;
            errno = 0; 
            double result_value = strtod(read_msg, &endptr);
            pi_val += result_value;

            close(fd[i][READ_END]);

        }

        printf("\nValor de pi %.8f\n", pi_val);

    }    
}

int pi_calc(int num_proc, char * operation){
    // Error by default
    int success = 1;

    // Time counter
    struct timespec start, end;
    clock_gettime(CLOCK_REALTIME, &start);

    if (strcmp(operation, OPTION_SHARED_MEMORY) == 0) {
        printf("Pi calculation via: %s by %i processes\n", DESC_SHARED_MEMORY, num_proc);
        sm_calc(num_proc);
    } else if (strcmp(operation, OPTION_MESSAGE_QUEUE) == 0) {
        printf("Pi calculation via: %s by %i processes\n", DESC_MESSAGE_QUEUE, num_proc);
        mq_calc(num_proc);
    } else if (strcmp(operation, OPTION_PIPES) == 0) {
        printf("Pi calculation via: %s by %i processes\n", DESC_PIPES, num_proc);
        p_calc(num_proc);
    } else {
        // Validation failed: The operation is not recognized
        printf("Error: Invalid operation '%s'.\n", operation);
        print_usage("");
        return 1;
    }

    clock_gettime(CLOCK_REALTIME, &end);
    double time_spent = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / BILLION;

    printf("Tempo de execução: %f segundos\n", time_spent);

    return success;
}

int main(int argc, char *argv[]) {

    // Arguments number validation
    if (argc != 3) {
        printf("Error: Incorrect number of arguments.\n");
        print_usage(argv[0]);
        return 1;
    }

    // Argumento processing
    int num_processes = atoi(argv[1]);
    char *operation = argv[2];

    return pi_calc(num_processes, operation);
}

