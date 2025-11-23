// Programa escrito para estudar como os diferentes tipos de variáveis
// são alocados nas diferentes áreas de memória de um processo.

// compilar estaticamente para ter um mapa de memória mais simples:
// gcc -Wall -static org_mem.c -o om

// Carlos Maziero, Set/2014.

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

#define SIZE 100000

int var_global ;
int var_global_init = 100 ;
int var_global_big [SIZE] ;
const int const_global = 12345 ;
char *string = "Uma string constante" ;
char *var_din ;

void func (int param)
{
  int var_local ;
  int var_local_big [SIZE] ;
  static int var_local_st ;
  const int const_local = 12345 ;

  // print variables' addresses
  printf ("  addr param           is %p\n", &param) ;
  printf ("  addr var_local       is %p\n", &var_local) ;
  printf ("  addr var_local_st    is %p\n", &var_local_st) ;
  printf ("  addr var_local_big   is %p\n", &var_local_big) ;
  printf ("  addr const_local     is %p\n", &const_local) ;
}

int main ()
{
  int i ;
  char cmd[128] ;

  var_din = malloc (SIZE) ;
  for (i=0; i< SIZE; i++)
    var_din[i] = random() % 256 ;

  // print variables' addresses
  printf ("My variables are at:\n") ;
  printf ("  addr var_global      is %p\n", &var_global) ;
  printf ("  addr var_global_init is %p\n", &var_global_init) ;
  printf ("  addr var_global_big  is %p\n", &var_global_big) ;
  printf ("  addr const_global    is %p\n", &const_global) ;
  printf ("  addr string          is %p\n", &string) ;
  printf ("  addr *string         is %p\n", string) ;
  printf ("  addr var_din         is %p\n", &var_din) ;
  printf ("  addr *var_din        is %p\n", var_din) ;

  // print functions' addresses
  printf ("  addr func main()     is %p\n", main) ;
  printf ("  addr func func()     is %p\n", func) ;

  func (12345) ;

  // dump memory map for this process
  printf ("\nmapa de memória\n") ;
  sprintf (cmd, "pmap %d", getpid () ) ;
  system (cmd) ;
}