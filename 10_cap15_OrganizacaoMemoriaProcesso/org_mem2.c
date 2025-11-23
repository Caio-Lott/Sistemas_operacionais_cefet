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

// Definições de Cores ANSI para Terminal
#define RESET     "\x1b[0m"
#define BRANCO    "\x1b[37m"
#define NEGRITO   "\x1b[1m"
#define STACK_COR "\x1b[44m"    // Azul (STACK)
#define HEAP_COR  "\x1b[45m"    // Roxo (HEAP)
#define BSS_COR   "\x1b[42m"    // Verde (BSS)
#define DATA_COR  "\x1b[43m"    // Amarelo/Marrom (DATA)
#define TEXT_COR  "\x1b[46m"    // Ciano (TEXT)
#define SEP       "|-------------------|-----------------------|---------------------------------------------------|\n"

void imprime_mapa_colorido() {
    printf("\n" NEGRITO "\tMAPA DE MEMORIA CONCEITUAL COLORIDO (ALINHADO)" RESET "\n");
    printf(SEP);
    printf("|" NEGRITO BRANCO " %-19s " RESET "|" NEGRITO BRANCO " %-23s" RESET "|" NEGRITO BRANCO " %-49s " RESET "|\n", 
        "SEÇÃO", "ENDEREÇO INICIAL (EX.)", "CONTEUDO");
    printf(SEP);
    
    // STACK (Azul) - Endereço Alto
    printf(STACK_COR BRANCO "| %-17s " RESET, "STACK (Pilha)");
    printf("| %-21s |", "0x7FFE... (Alto)");
    printf(" %-49s |" RESET "\n", "param, var_local, var_local_big");
    printf(SEP);

    // HEAP (Roxo)
    printf(HEAP_COR BRANCO "| %-17s " RESET, "HEAP (Malloc)");
    printf("| %-21s |", "0x14A1... ou 0x05... "); // Ajustado para refletir 0x050e680 do seu output
    printf(" %-49s |" RESET "\n", "*var_din (malloc)");
    printf(SEP);

    // BSS (Verde)
    printf(BSS_COR BRANCO "| %-17s " RESET, "BSS (Nao Init)");
    printf("| %-21s |", "0x004C... ou 0x05...");
    printf(" %-49s |" RESET "\n", "var_global, var_global_big, var_local_st, var_din");
    printf(SEP);

    // DATA (Amarelo/Marrom)
    printf(DATA_COR BRANCO "| %-17s " RESET, "DATA (Inic.)");
    printf("| %-21s |", "0x004A...");
    printf(" %-49s |" RESET "\n", "var_global_init, string (ponteiro)");
    printf(SEP);
    
    // TEXT (Ciano) - Endereço Baixo
    printf(TEXT_COR BRANCO "| %-17s " RESET, "TEXT (Codigo/RO)");
    printf("| %-21s |", "0x0040... (Baixo)");
    printf(" %-49s |" RESET "\n", "main(), func(), const_global, *string (conteudo)");
    printf(SEP);
    
    printf(RESET); 
}

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

  // Chame esta função no lugar do pmap na função main:
  imprime_mapa_colorido();

  // dump memory map for this process
//   printf ("\nmapa de memória\n") ;
//   sprintf (cmd, "pmap %d", getpid () ) ;
//   system (cmd) ;
}