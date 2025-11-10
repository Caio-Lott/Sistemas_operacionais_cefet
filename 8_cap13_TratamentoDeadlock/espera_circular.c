/*
O problema do Jantar dos filósofos, Solução 2:
Atacando a condição de "Espera Circular".

Um filósofo (o último) pega os palitos na ordem inversa
(direita, depois esquerda), quebrando o ciclo de dependências.

Compilar com gcc -Wall filosofos_sol2.c -o filosofos2 -lpthread

Baseado no código de: Carlos Maziero, DINF/UFPR 2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUMFILO 5

pthread_t filosofo [NUMFILO] ; // [cite: 3]
sem_t     hashi    [NUMFILO] ; // [cite: 4]

// espaços para separar as colunas de impressão
char *space[] = {"", "\t", "\t\t", "\t\t\t", "\t\t\t\t" } ; // [cite: 5]

// espera um tempo aleatório
void espera (int n)
{
  sleep (random() % n) ;      // [cite: 6]
  usleep (random() % 100000) ; // [cite: 7] (modificado para 100ms para diferenciar um pouco)
}

// filósofo comendo
void come (int f)
{
  printf ("%sF%d COMENDO\n", space[f], f) ; // [cite: 14]
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
  printf ("%sF%d pegou h%d\n", space[f], f, h) ; // [cite: 11]
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
    // Quebra a espera circular:
    // O último filósofo (i == 4) pega os palitos na ordem inversa.

    if ( i == (NUMFILO - 1) )
    {
      // F4 (Último): Pega DIREITA (h0), depois ESQUERDA (h4)
      pega_hashi (i, (i+1) % NUMFILO) ; // Pega h0
      pega_hashi (i, i) ;                 // Pega h4
    }
    else
    {
      // F0, F1, F2, F3: Pegam ESQUERDA (hi), depois DIREITA (h(i+1))
      pega_hashi (i, i) ;                 // Pega hi
      pega_hashi (i, (i+1) % NUMFILO) ; // Pega h(i+1)
    }
    // <-- FIM DA MUDANÇA
    
    come (i) ; // [cite: 14]

    // A ordem de largar não importa para o deadlock
    larga_hashi (i, i) ;
    larga_hashi (i, (i+1) % NUMFILO) ;
  }
  pthread_exit (NULL) ; // [cite: 15]
}

// programa principal
int main (int argc, char *argv[])
{
  long i, status ;

  setvbuf (stdout, 0, _IONBF, 0) ; // [cite: 16]

  // inicia os hashis
  for(i=0; i<NUMFILO; i++)
    sem_init (&hashi[i], 0, 1) ; // [cite: 17]

  // inicia os filosofos
  for(i=0; i<NUMFILO; i++)
  {
    status = pthread_create (&filosofo[i], NULL, threadFilosofo, (void *) i) ; // [cite: 18]
    if (status) // [cite: 19]
    {
      perror ("pthread_create") ;
      exit (1) ; // [cite: 20]
    }
  }

  // a main encerra aqui
  pthread_exit (NULL) ;
}