#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <unistd.h>
#include "meuAlocador.h"

// Bloco:
// | estado(8 bytes) | tamanho(8 bytes) | dados(até 4096 bytes) |

void *topoInicialHeap = NULL;

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
        printf("Heap restaurada com sucesso.\n");
    else
        printf("Erro ao restaurar heap.\n");
    return;
}

// Aloca um bloco com num_bytes na heap e retorna o endereço inicial desse bloco
void *alocaMem(int num_bytes)
{
    printf("alocaMem CHAMADO! num_bytes: %d\n", num_bytes);

    if (num_bytes <= 0) {
        printf("num_bytes inválido.\n");
        return NULL;
    }

    void *blocoLivre = NULL;
    void *posAtual = topoInicialHeap;
    void *topoHeap = sbrk(0);
    int64_t menorTamanho = __INT64_MAX__;

    // (best-fit) Percorre toda a lista e seleciona o nó com menor bloco, que é maior do que o solicitado
    while (posAtual < topoHeap) {
        printf("ENTROU NO WHILE\n");
        
        int64_t estado = *(int64_t*)posAtual;
        int64_t tamanho = *((int64_t*)(posAtual + 8));

        if ((estado == 0) && (tamanho >= (num_bytes + 16)) && (tamanho < menorTamanho)) {
            blocoLivre = posAtual;
            menorTamanho = tamanho;
        }

        // Move para próximo bloco
        posAtual += tamanho;
    }

    // Se encontrou bloco adequado
    if (blocoLivre) {
        *(int64_t*)blocoLivre = 1; // Marca como ocupado
        return blocoLivre + 16;
    } 
    else { // Não encontrou bloco adequado
        int64_t novoTamanho = (((num_bytes + 16) + 4095) / 4096) * 4096; // Ajuste para múltiplos de 4096 bytes
        printf("novo tamanho: %" PRId64 "\n", novoTamanho);
        void *novoBloco = sbrk(novoTamanho);

        if (novoBloco == (void*)-1) {
            printf("Erro ao alocar bloco.\n");
            return NULL;
        }

        *(int64_t*)novoBloco = 1;                   // Marca como ocupado
        *((int64_t*)(novoBloco + 8)) = novoTamanho; // Armazena tamanho do bloco

        return novoBloco + 16;
    }
}

// Devolve para heap o bloco alocado
int liberaMem(void *bloco) 
{
    if (!bloco) {
        printf("Erro.\n");
        return -1;
    }

    void *inicioBloco = bloco - 16;
    int64_t estado = *(int64_t*)inicioBloco;

    if (estado == 0) {
        printf("Bloco já está livre.\n");
        return -1;
    }

    *(int64_t*)inicioBloco = 0; // Marca como livre

    printf("Bloco liberado no endereço: %p\n", bloco);
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

        printf("endereço: %p, estado: %" PRId64 ", tamanho: %" PRId64 "", atual, estado, tamanho);

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

    printf("topoInicialHeap: %p\n", topoInicialHeap);

    void *bloco1 = alocaMem(500);
    printf("bloco1: %p\n", bloco1);

    // finalizaAlocador();
    return 0;
}