/*
Acesso concorrente a uma variável por muitas threads, solução com alternância.
*/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define NUM_THREADS  30
#define NUM_STEPS  1000

int sum = 0 ; //Recurso compartilhado que será atualizado pelas threads

int waiting[NUM_THREADS]; //Threads que estão aguardando para utilizar o recurso compartilhado

int lock; //Utilizada pelas threads para controlar o acesso à seção crítica

void *threadBody(void *id) {
	int contador = 0; //Controla a quantidade de vezes que a thread
	//atualizará o valor da variável compartilhada.

	long turn = (long) id; //id da thread
	long other; //utilizado para se referir aos ids as outras threads
	int key;//utilizado localmente para armazenar o "status" da seção crítica  
   	for (contador=0; contador< NUM_STEPS; contador++){
		waiting[turn] = 1; //Indica que o processo i está esperando
		key = 1; //Assume que a seção crítica está ocupada
		while (waiting[turn] && key) 
           key = __sync_lock_test_and_set(&lock, 1);
		waiting[turn] = 0;
        if (sum % 500 == 0)
           printf ("Sum: %d\n", sum) ;
		/* Executa a seção crítica */		
		sum++;
		/*Procura na lista pelo o próximo processo que está esperando*/
		other = (turn + 1) % NUM_THREADS; //Inicia a busca no próximo
		while ((other != turn) && !waiting[other]) //Enquanto não encontrar
			 other = (other + 1) % NUM_THREADS; //Passa para o próximo
		if (other == turn) //Se não há processos esperando
			lock = 0; //Libera o recurso
		else 
			waiting[other] = 0; //Libera a thread "other" do “while”
	} 
    pthread_exit(NULL);
}

int main (int argc, char *argv[]){
   	pthread_t thread [NUM_THREADS] ;
   	long i;  
   	for (i=0; i<NUM_THREADS; i++) //cria threads
      	pthread_create (&thread[i], NULL, threadBody, (void *) i) ;

	for (i=0; i<NUM_THREADS; i++)
	    pthread_join (thread[i], NULL) ;

   printf ("Sum should be %d and is %d\n", NUM_THREADS*NUM_STEPS, sum) ;
   pthread_exit (NULL) ;
}

