#include "listaEncadeada.h"



Node* criar_lista(void) {
    Node *cabecalho = (Node*) malloc(sizeof(Node));
    if (cabecalho == NULL) {
        printf("Erro crítico: Falha na alocação de memória para o nó cabeçalho.\n");
        return NULL;
    }
    cabecalho->data = 0;
    cabecalho->next = NULL;
    return cabecalho;
}


bool inserir_inicio(Node *cabecalho, int valor) {
    if (cabecalho == NULL) return false;

    Node *novo_no = (Node*) malloc(sizeof(Node));
    if (novo_no == NULL) {
        printf("Erro: Falha na alocação de memória para novo nó.\n");
        return false;
    }

    novo_no->data = valor;
    novo_no->next = cabecalho->next;
    cabecalho->next = novo_no;

    return true;
}


bool inserir_fim(Node *cabecalho, int valor) {
    if (cabecalho == NULL) return false;

    Node *novo_no = (Node*) malloc(sizeof(Node));
    if (novo_no == NULL) {
        printf("Erro: Falha na alocação de memória para novo nó.\n");
        return false;
    }

    novo_no->data = valor;
    novo_no->next = NULL;

    Node *atual = cabecalho;
    while (atual->next != NULL) {
        atual = atual->next;
    }

    atual->next = novo_no;
    return true;
}


Node* buscar(Node *cabecalho, int valor) {
    if (cabecalho == NULL) return NULL;

    Node *atual = cabecalho->next;
    while (atual != NULL) {
        if (atual->data == valor) {
            return atual;
        }
        atual = atual->next;
    }
    return NULL;
}


bool remover(Node *cabecalho, int valor) {
    if (cabecalho == NULL || cabecalho->next == NULL) {
        return false;
    }

    Node *anterior = cabecalho;
    Node *atual = cabecalho->next;

    while (atual != NULL && atual->data != valor) {
        anterior = atual;
        atual = atual->next;
    }

    if (atual == NULL) {
        return false;
    }

    anterior->next = atual->next;
    free(atual);
    return true;
}


void exibir_lista(Node *cabecalho) {
    if (cabecalho == NULL) return;

    Node *atual = cabecalho->next;
    printf("Cabeçalho -> ");
    while (atual != NULL) {
        printf("[%d] -> ", atual->data);
        atual = atual->next;
    }
    printf("NULL\n");
}


void destruir_lista(Node *cabecalho) {
    if (cabecalho == NULL) return;

    Node *atual = cabecalho;
    while (atual != NULL) {
        Node *proximo = atual->next;
        free(atual);
        atual = proximo;
    }
}

// int main(void) {
//     Node *lista = criar_lista();

//     printf("--- Operações de Inserção ---\n");
//     inserir_fim(lista, 100);
//     inserir_fim(lista, 200);
//     inserir_fim(lista, 300);
//     inserir_inicio(lista, 50);
//     exibir_lista(lista);

//     printf("\n--- Operações de Busca ---\n");
//     int chave = 200;
//     Node *no_encontrado = buscar(lista, chave);
//     if (no_encontrado != NULL) {
//         printf("Elemento %d localizado no endereço de memória %p.\n", chave, (void*)no_encontrado);
//     } else {
//         printf("Elemento %d não localizado na lista.\n", chave);
//     }

//     printf("\n--- Operações de Remoção ---\n");
//     printf("Removendo o primeiro nó de dados (50)...\n");
//     remover(lista, 50);
//     exibir_lista(lista);

//     printf("Removendo nó intermediário (200)...\n");
//     remover(lista, 200);
//     exibir_lista(lista);

//     printf("Tentativa de remoção de elemento inexistente (999)...\n");
//     if (!remover(lista, 999)) {
//         printf("Elemento 999 não encontrado para remoção.\n");
//     }

//     printf("\n--- Desalocação de Recursos ---\n");
//     destruir_lista(lista);
//     lista = NULL;
//     printf("Memória liberada com sucesso.\n");

//     return 0;
// }