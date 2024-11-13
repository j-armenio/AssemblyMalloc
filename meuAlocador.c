#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include "meuAlocador.h"

// Bloco:
// | estado(8 bytes) | tamanho(8 bytes) | dados(até 4096 bytes) |

void *topoInicialHeap = NULL;
void *aux = NULL;

// Armazena o endereço do topo corrente da heap e armazena em topoInicialHeap
void iniciaAlocador()
{
    topoInicialHeap = sbrk(0);
    return;
}

// Restaura o valor original da heap
void finalizaAlocador()
{
    if (brk(topoInicialHeap) == 0)
        return;
    else
        printf("Erro ao restaurar heap.\n");
        return;
    return;
}

// Aloca um bloco com num_bytes na heap e retorna o endereço inicial desse bloco
void *alocaMem(int num_bytes)
{
    if (num_bytes <= 0) {
        printf("Número de bytes inválido.\n");
        return NULL;
    }

    void *posAtualHeap = topoInicialHeap;
    void *topoAtualHeap = sbrk(0);

    void *blocoLivre = NULL;
    int64_t menorTamanho = __INT64_MAX__;

    // (best-fit) Percorre toda a heap e seleciona o nó com menor bloco, que é maior do que o solicitado    
    while (posAtualHeap < topoInicialHeap) { // Começa da posição atual da Heap até seu inicio
        int64_t estado = *(int64_t*)posAtualHeap;
        int64_t tamanho = *(int64_t*)(posAtualHeap+8);

        if (estado == 0 && tamanho >= (num_bytes+16) && tamanho < menorTamanho) {
            blocoLivre = posAtualHeap;
            menorTamanho = tamanho;
        }

        posAtualHeap += tamanho;
    }

    // se encontrou um bloco adequado
    if (blocoLivre) {
        *(int64_t*)blocoLivre = 1; // Marca 'estado' como ocupado
        return blocoLivre + 16;
    }
    else { // Não encontrou um bloco adequado
        int64_t novoTamanho = (((num_bytes + 16) + 4095) / 4096) * 4096; // Ajuste para múltiplos de 4096 bytes
        void *novoBloco = sbrk(novoTamanho);

        if (novoBloco == (void*)-1) {
            printf("Erro ao alocar bloco.\n");
            return NULL;
        }

        *(int64_t*)novoBloco = 1;
        *((int64_t*)(novoBloco+8)) = novoTamanho;
        
        return novoBloco + 16;
    }
}

// Devolve para heap o bloco alocado
int liberaMem(void *bloco)
{
    if (!bloco) {
        // Aqui, o certo seria dar um aviso de erro, porém quando eu dou printf, ele altera a heap e o brk é
        // alterado, oq fode com o malloc. Não sei como resolver, mas acho que em assembly n tem esse problema.
        // Alias, em teoria o -1 já seria sinal de erro suficiente, msm sem o printf
        // printf("Erro no ponteiro de bloco.\n");
        return -1;
    }

    void *inicioBloco = bloco-16;
    int64_t estado = *(int64_t*)inicioBloco;

    if (estado == 0) {
        // Msm coisa aqui.
        // printf("Bloco já está livre.\n");
        return -2;
    }

    *(int64_t*)inicioBloco = 0; // Marca como livre

    return 0;
}

// Imprime um mapa da memória da região da heap
void imprimeMapa()
{
    void *topoHeap = sbrk(0);
    void *atual = topoInicialHeap;

    printf("Mapa da heap:\n");

    while (atual < topoHeap) {
        int64_t estado = *(int64_t*)atual;
        int64_t tamanho = *((int64_t*)(atual + 8));

        printf("[");
        for (int i = 0; i < 16; i++) {
            printf("#"); // Representa os metadados
        }
        for (int i = 0; i < tamanho - 16; i++) {
            printf(estado ? "+" : "-"); // + para ocupado, - para livre
        }
        printf("]\n");

        atual += tamanho; // Move para o próximo bloco
    }
    printf("\n");
}

int main()
{   
    iniciaAlocador();

    void *bloco1 = alocaMem(500);
    void *bloco2 = alocaMem(400);

    // PROBLEMA: mesmo problema do printf. Quando o printf é chamado, ele altera a heap, e altera o brk,
    // isso faz com que se você usa o imprimeMapa() AQUI, ele vai imprimir corretamente, porém quando você
    // tentar fazer o liberaMem(), ele vai dar merda, pq o endereço da heap mudou, e ela não está mais organizada
    // bonitinha em bloquinhos, está com um vetor que o printf colocou no meio, e caga tudo. Novamente, eu
    // acho que a impressão do assembly não altera a heap, então talvez la não tenha esse problema, mas em 
    // C, nessa implementação não sei como resolver.

    liberaMem(bloco1);
    liberaMem(bloco2);

    imprimeMapa();

    finalizaAlocador();

    return 0;
}