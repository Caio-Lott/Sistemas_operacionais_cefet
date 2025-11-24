// clear ; gcc -Wall -static org_mem2.c -o om2 && ./om2

// TP 10 de Laboratório de Sistemas Operacionais - CEFET-MG 2025.2
// Henrique Rodrigues Lima      - 20193009473
// Caio Fernandes Lott Primola  - 20193001742

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE 100000

// Definições de Cores ANSI para Terminal
#define RESET       "\x1b[0m"
#define BRANCO      "\x1b[37m"
#define NEGRITO     "\x1b[1m"
#define VERMELHO    "\x1b[41m"    // TEXT
#define ROXO        "\x1b[45m"    // BSS
#define VERDE       "\x1b[42m"    // HEAP
#define AMARELO     "\x1b[43m"    // STACK
#define CIANO       "\x1b[46m"    // DATA
#define SEP       "|-------------------|-----------------------|--------------------------------------------------------------|\n"

void imprime_mapa_colorido(
    void *stack_addr, 
    void *heap_addr, 
    void *bss_addr, 
    void *data_addr, 
    void *text_addr
) 
{
    printf("\n" NEGRITO "\tMAPA DE MEMORIA CONCEITUAL COLORIDO" RESET "\n");
    printf(SEP);
    printf("|" NEGRITO BRANCO " %-17s " RESET "|" NEGRITO BRANCO " %-21s " RESET "|" NEGRITO BRANCO " %-60s " RESET "|\n", 
        "SECAO", "ENDERECO DE REF.", "CONTEUDO");
    printf(SEP);
    
    // STACK - usa o endereço de 'var_local' - variáveis locais
    printf(AMARELO BRANCO "| %-17s " RESET, "STACK (Pilha)");
    printf("| %-21p |", stack_addr); // Imprime o endereço real
    printf(" %-60s |" RESET "\n", "param, var_local, var_local_big (Alto)");
    printf(SEP);

    // HEAP - usa o endereço de '*var_din' - Memória alocada dinamicamente
    printf(VERDE BRANCO "| %-17s " RESET, "HEAP (Malloc)");
    printf("| %-21p |", heap_addr); // Imprime o endereço real
    printf(" %-60s |" RESET "\n", "*var_din (malloc)");
    printf(SEP);

    // BSS - usa o endereço de 'var_global' - variáveis globais (ou locais estáticas) não inicializadas
    printf(ROXO BRANCO "| %-17s " RESET, "BSS (Nao Init)");
    printf("| %-21p |", bss_addr); // Imprime o endereço real
    printf(" %-60s |" RESET "\n", "var_global, var_global_big, var_local_st, var_din (ponteiro)");
    printf(SEP);

    // DATA - usa o endereço de 'var_global_init' - variáveis globais (ou locais estáticas) inicializadas
    printf(CIANO BRANCO "| %-17s " RESET, "DATA (Inic.)");
    printf("| %-21p |", data_addr); // Imprime o endereço real
    printf(" %-60s |" RESET "\n", "var_global_init, string (ponteiro)");
    printf(SEP);
    
    // TEXT - usa o endereço de 'main'
    printf(VERMELHO BRANCO "| %-17s " RESET, "TEXT (Codigo/RO)");
    printf("| %-21p |", text_addr); // Imprime o endereço real
    printf(" %-60s |" RESET "\n", "main(), func(), const_global, *string (conteudo) (Baixo)");
    printf(SEP);
    
    printf(RESET); 
}

// Variáveis Globais (DATA/BSS)
int var_global ;
int var_global_init = 100 ;
int var_global_big [SIZE] ;
const int const_global = 12345 ;
char *string = "Uma string constante" ;
char *var_din ;

void func (int param, void **local_addr) // Modificada para retornar o endereço local
{
  int var_local ;
  int var_local_big [SIZE] ;
  static int var_local_st ;
  const int const_local = 12345 ;

  // Armazena o endereço de uma variável local para uso na main (STACK)
  *local_addr = &var_local; 
  
  // print variables' addresses (mantido para debugging)
  printf ("  addr param           is %p\n", &param) ;
  printf ("  addr var_local       is %p\n", &var_local) ;
  printf ("  addr var_local_st    is %p\n", &var_local_st) ;
  printf ("  addr var_local_big   is %p\n", &var_local_big) ;
  printf ("  addr const_local     is %p\n", &const_local) ;
}

int main ()
{
  int i ;
  void *stack_ref_addr = NULL; // Ponteiro que irá armazenar o endereço local (STACK)
  
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
  
  // Adiciona o endereço do próprio ponteiro de referência da Stack
  printf ("  addr stack_ref_addr  is %p\n", &stack_ref_addr) ;
  
  // print functions' addresses
  printf ("  addr func main()     is %p\n", main) ;
  printf ("  addr func func()     is %p\n", func) ;

  // Chamada da função para obter o endereço de referência da STACK
  func (12345, &stack_ref_addr) ;

  // Chamada da função com endereços reais de referência para preencher a tabela colorida
  imprime_mapa_colorido(
      stack_ref_addr,          // STACK: Conteúdo do ponteiro (&var_local)
      var_din,                 // HEAP: Endereço da memória alocada dinamicamente (*var_din)
      &var_global,             // BSS: Endereço de uma variável global não inicializada (&var_global)
      &var_global_init,        // DATA: Endereço de uma variável global inicializada (&var_global_init)
      (void *)main             // TEXT: Endereço da função 'main'
  );
  
  return 0;
}