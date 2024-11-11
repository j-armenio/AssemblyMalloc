#include <unistd.h>
#include <stdio.h>
#include "meuAlocador.h"

void* topoInicialHeap;

void iniciaAlocador() {
    topoInicialHeap = sbrk(0);
    return;
}

void finalizaAlocador () {
    brk(topoInicialHeap);
    return;
}

void liberaMem (void* bloco) {
    void* p = bloco - 16 * 8;
    
    p = 0;

    return;
}

