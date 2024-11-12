#include <unistd.h>
#include <stdio.h>
#include "meuAlocador.h"

void* topoInicialHeap = NULL;
void* topoAtualHeap = NULL;

// Armazena o endereço do topo corrente da heap e armazena em topoInicialHeap
void iniciaAlocador()
{
    topoInicialHeap = sbrk(0);
    topoAtualHeap = topoInicialHeap;
    return;
}

// Restaura o valor original da heap
void finalizaAlocador() 
{
    if (brk(topoInicialHeap) == 0)
        printf("Heap restaurada com sucesso.\n");
    else
        printf("Erro ao restaurar heap.\n");
    return;
}

// Devolve para heap o bloco alocado
int liberaMem(void* bloco) 
{
    void* p = bloco - 16 * 8;
    p = 0;

    return 0;
}

// Bloco:
// | estado(8 bytes) | tamanho(8 bytes) | dados(até 4096 bytes) |

// Aloca um bloco com num_bytes na heap e retorna o endereço inicial desse bloco
void* alocaMem(int num_bytes)
{
    if (num_bytes <= 0) {
        printf("num_bytes inválido.\n");
        return NULL;
    }

    // Ajusta o tamanho solicitado para incluir metadados
    int tamAlocar = num_bytes + 16;

    // Procura melhor bloco livre (best-fit)
    void* melhorBloco = NULL;
    void* atual = topoInicialHeap;
    int menorTamanho = __INT_MAX__;

    while (atual < topoAtualHeap) {
        int estado = *(int*)atual;        // Estado
        int tamanho = *((int*)(atual+8)); // Tamanho

        if (estado == 0 && tamanho >= tamAlocar && tamanho < menorTamanho) {
            melhorBloco = atual;
            menorTamanho = tamanho;
        }

        // Move para o proximo bloco
        atual += tamanho;
    }

    // Se encontrou um bloco adequado
    if (melhorBloco) {
        *(int*)melhorBloco = 1; // Marca como ocupado
        return melhorBloco + 16; // REtorna endereço útil do bloco
    }
    
    // Se não encontrou expande a heap
    int tamanhoNovoBloco = ((tamAlocar + 4095) / 4096) * 4096;
    void *novoBloco = sbrk(tamanhoNovoBloco);
    if (novoBloco == (void*)-1) return NULL;

    *(int*)novoBloco = 1; // Marca como ocupado
    *((int*)(novoBloco + 8)) = tamanhoNovoBloco; // Tamanho do bloco

    // Atualiza topo da heap
    topoAtualHeap = novoBloco + tamanhoNovoBloco;

    return novoBloco + 16;
}