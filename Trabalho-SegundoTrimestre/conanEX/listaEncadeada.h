#ifndef LISTAENCADEADA_H
#define LISTAENCADEADA_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct Node {
    int data;
    struct Node *next;
} Node;

Node* criar_lista(void);

bool inserir_inicio(Node *cabecalho, int valor);

bool inserir_fim(Node *cabecalho, int valor);

Node* buscar(Node *cabecalho, int valor);

bool remover(Node *cabecalho, int valor);

void exibir_lista(Node *cabecalho);

void destruir_lista(Node *cabecalho);

#endif