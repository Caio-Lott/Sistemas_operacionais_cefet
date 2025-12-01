#include <stdlib.h>
#include <stdio.h>
#include <string.h>
/*Recebe a quantidade de memória RAM (em MB) que será alocada*/
/*./a.out 100  -> aloca 100 MB de memória*/
/*
gcc eat_RAM.c -o er && ./er 31796
*/
int main(int argc, char** argv) {
    int max = -1;
    int mb = 1;
    char* buffer;
    if(argc != 2)
		return 1;

    max = atoi(argv[1]);
    while((buffer=malloc(1024*1024)) != NULL && mb != max) {
        memset(buffer, rand()%1000, 1024*1024);
        mb++;
 	    printf("%d MB alocados\n", mb);
    }
    printf("%d MB alocados\n", mb);
    return 0;
}
