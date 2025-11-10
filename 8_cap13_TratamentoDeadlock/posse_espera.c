/*
O problema do Jantar dos filósofos, Solução 1:
Atacando a condição de "Posse e Espera".

Um filósofo só pode pegar os palitos se ambos estiverem
disponíveis (operação atômica). Isso é feito usando um
mutex global para proteger a seção crítica de "pegar palitos".

Compilar com gcc -Wall filosofos_sol1.c -o filosofos1 -lpthread

Baseado no código de: Carlos Maziero, DINF/UFPR 2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUMFILO 5

pthread_t filosofo [NUMFILO] ;
sem_t     hashi    [NUMFILO] ; // um semaforo para cada palito (iniciam em 1) [cite: 3, 4]
sem_t     mutex ;              // <-- MUDANÇA: Mutex para garantir aquisição atômica

// espaços para separar as colunas de impressão
char *space[] = {"", "\t", "\t\t", "\t\t\t", "\t\t\t\t" } ;

// espera um tempo aleatório
void espera (int n)
{
  // sleep (random() % n) ;      // [cite: 6]
  // usleep (random() % 1000000) ; // [cite: 7]
}

// filósofo comendo
void come (int f)
{
  printf ("%sF%d COMENDO\n", space[f], f) ;
  espera (1) ; // [cite: 8]
}

// filósofo meditando
void medita (int f)
{
  printf ("%sF%d meditando\n", space[f], f) ;
  espera (1) ; // [cite: 9]
}

// pega o hashi
void pega_hashi (int f, int h)
{
  printf ("%sF%d quer  h%d\n", space[f], f, h) ;
  sem_wait (&hashi [h]) ; // [cite: 10]
  printf ("%sF%d pegou h%d\n", space[f], f, h) ;
}

// larga o hashi
void larga_hashi (int f, int h)
{
  printf ("%sF%d larga h%d\n", space[f], f, h) ;
  sem_post (&hashi [h]) ; // [cite: 12]
}

// corpo da thread filosofo
void *threadFilosofo (void *arg)
{
  int i = (long int) arg ;
  while (1) // [cite: 13]
  {
    medita (i) ;

    // <-- INÍCIO DA MUDANÇA
    // Garante que a aquisição dos dois hashis seja atômica
    printf ("%sF%d tenta pegar os 2 hashis\n", space[i], i) ;
    sem_wait (&mutex) ; // Entra na seção crítica
      pega_hashi (i, i) ;
      pega_hashi (i, (i+1) % NUMFILO) ;
    sem_post (&mutex) ; // Sai da seção crítica
    // <-- FIM DA MUDANÇA

    come (i) ; // [cite: 14]

    larga_hashi (i, i) ;
    larga_hashi (i, (i+1) % NUMFILO) ;
  }
  pthread_exit (NULL) ;
}

// programa principal
int main (int argc, char *argv[])
{
  long i, status ;

  setvbuf (stdout, 0, _IONBF, 0) ; // [cite: 16]

  // <-- MUDANÇA: Inicia o mutex
  sem_init (&mutex, 0, 1) ;

  // inicia os hashis [cite: 17]
  for(i=0; i<NUMFILO; i++)
    sem_init (&hashi[i], 0, 1) ;

  // inicia os filosofos [cite: 18]
  for(i=0; i<NUMFILO; i++)
  {
    status = pthread_create (&filosofo[i], NULL, threadFilosofo, (void *) i) ;
    if (status) // [cite: 19]
    {
      perror ("pthread_create") ;
      exit (1) ; // [cite: 20]
    }
  }

  // a main encerra aqui
  pthread_exit (NULL) ; //
}