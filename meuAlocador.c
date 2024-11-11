#include <unistd.h>
#include <stdio.h>
#include "meuAlocador.h"

void* topoInicialHeap;

// Armazena o endereço do topo corrente da heap                             
void iniciaAlocador() 
{
    topoInicialHeap = sbrk(0);
    return;
}

// Restaura o valor original da heap
void finalizaAlocador() 
{
    brk(topoInicialHeap);
    return;
}

// Indica que
int liberaMem(void* bloco) 
{
    void* p = bloco - 16 * 8;
    p = 0;

    return 0;
}

void* alocaMem(int num_bytes)
{

}

