#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include "listaEncadeada.h"

#define LARGURA_TELA     800
#define ALTURA_TELA      600
#define LARGURA_CANO     80
#define ESPACO_CANO      180
#define DISTANCIA_CANOS  250
#define VELOCIDADE_CANO  4.0f
#define GRAVIDADE        0.45f
#define FORCA_PULO       -8.0f
#define ESCALA_PASSARO   4.0f
#define MAX_HISTORICO    8   /* quantas partidas aparecem na tela */

/* A cabeca do cano e mais larga que o corpo e sobra dos dois lados. */
#define ESCALA_CABECA    1.325f
#define LARGURA_CABECA   (LARGURA_CANO * ESCALA_CABECA)
#define OFFSET_CABECA    ((LARGURA_CABECA - LARGURA_CANO) / 2.0f)

/* Quantidade de canos em circulacao.
   O ciclo (NUM_CANOS * DISTANCIA_CANOS) precisa ser maior que
   LARGURA_TELA + LARGURA_CANO + 2 * OFFSET_CABECA (= 906). Se for menor, o cano
   reciclado reaparece DENTRO da tela em vez de entrar pela direita.
   Com 3 canos o ciclo era 750 e o cano "brotava" em x ~ 670. */
#define NUM_CANOS        4

typedef enum { MENU, JOGANDO, PAUSADO, GAMEOVER } EstadoJogo;
typedef enum { BIRD_PARADO, BIRD_VOANDO, BIRD_MORTO } BirdState;

typedef struct {
    Vector2 posicao;
    float velocidadeY;
    float raio;
    BirdState estado;
    Texture2D spriteParado;
    Texture2D spriteVoando;
    Texture2D spriteMorto;
} Passaro;

typedef struct {
    float x;
    float aberturaY;
    bool passou;
} Cano;

static void reiniciarJogo(Passaro *passaro, Cano canos[], int *pontos)
{
    passaro->posicao = (Vector2){200, ALTURA_TELA / 2.0f};
    passaro->velocidadeY = 0;
    passaro->estado = BIRD_PARADO;

    for (int i = 0; i < NUM_CANOS; i++) {
        canos[i].x = 600 + i * DISTANCIA_CANOS;
        canos[i].aberturaY = (float)GetRandomValue(150, 450);
        canos[i].passou = false;
    }
    *pontos = 0;
}

/* Marca o passaro como morto e registra a pontuacao uma unica vez. */
static void finalizarPartida(Passaro *passaro, EstadoJogo *estado,
                             Node *lista, int pontos, bool *registrada)
{
    passaro->estado = BIRD_MORTO;
    *estado = GAMEOVER;
    if (!*registrada && lista != NULL) {
        inserir_fim(lista, pontos);
        *registrada = true;
    }
}

/* Percorre a lista: devolve a quantidade de partidas e o recorde. */
static int estatisticas(Node *lista, int *recorde)
{
    int total = 0;
    *recorde = 0;
    if (lista == NULL) return 0;

    for (Node *n = lista->next; n != NULL; n = n->next) {
        if (n->data > *recorde) *recorde = n->data;
        total++;
    }
    return total;
}

static void desenharTextoCentradoEm(const char *texto, int centroX, int y, int tamanho, Color cor)
{
    DrawText(texto, centroX - MeasureText(texto, tamanho) / 2, y, tamanho, cor);
}

static void desenharTextoCentralizado(const char *texto, int y, int tamanho, Color cor)
{
    desenharTextoCentradoEm(texto, LARGURA_TELA / 2, y, tamanho, cor);
}

/* Painel com as ultimas MAX_HISTORICO partidas. Tudo e desenhado dentro do retangulo. */
static void desenharHistorico(Node *lista, Rectangle painel)
{
    if (lista == NULL) return;

    int recorde;
    int total = estatisticas(lista, &recorde);
    int x = (int)painel.x + 16;
    int y = (int)painel.y + 14;
    char texto[64];

    DrawRectangleRec(painel, Fade(BLACK, 0.55f));
    DrawRectangleLinesEx(painel, 2, Fade(WHITE, 0.8f));
    DrawText("HISTORICO DE PONTOS", x, y, 22, WHITE);

    if (total == 0) {
        DrawText("Nenhuma partida registrada.", x, y + 36, 18, LIGHTGRAY);
        return;
    }

    snprintf(texto, sizeof(texto), "Recorde: %d", recorde);
    DrawText(texto, x, y + 34, 20, GOLD);

    int pular = total > MAX_HISTORICO ? total - MAX_HISTORICO : 0;
    int indice = 0;
    y += 66;

    for (Node *n = lista->next; n != NULL; n = n->next, indice++) {
        if (indice < pular) continue;
        snprintf(texto, sizeof(texto), "Partida %d: %d pontos", indice + 1, n->data);
        DrawText(texto, x, y, 18, n->data == recorde ? GOLD : LIGHTGRAY);
        y += 24;
    }
}

/* Tela de game over em duas colunas: textos a esquerda, historico a direita. */
static void desenharGameOver(Node *lista, int pontos)
{
    const int centroEsquerda = 240;
    const Rectangle painelHistorico = {480, 150, 300, 300};
    char texto[64];

    DrawRectangle(0, 0, LARGURA_TELA, ALTURA_TELA, Fade(BLACK, 0.45f));

    snprintf(texto, sizeof(texto), "PONTOS: %d", pontos);
    desenharTextoCentradoEm("GAME OVER",         centroEsquerda, 170, 50, RED);
    desenharTextoCentradoEm(texto,               centroEsquerda, 250, 30, WHITE);
    desenharTextoCentradoEm("ENTER - REINICIAR", centroEsquerda, 320, 25, WHITE);
    desenharTextoCentradoEm("ESC - MENU",        centroEsquerda, 365, 25, WHITE);

    desenharHistorico(lista, painelHistorico);
}

static void desenharCanos(const Cano canos[], Texture2D corpo,
                          Texture2D cabecaCima, Texture2D cabecaBaixo)
{
    const float escalaCima  = LARGURA_CABECA / cabecaCima.width;
    const float escalaBaixo = LARGURA_CABECA / cabecaBaixo.width;
    const Rectangle origem = {0, 0, (float)corpo.width, (float)corpo.height};

    for (int i = 0; i < NUM_CANOS; i++) {
        float alturaCima  = canos[i].aberturaY - ESPACO_CANO / 2.0f;
        float yBaixo      = canos[i].aberturaY + ESPACO_CANO / 2.0f;
        float alturaBaixo = ALTURA_TELA - yBaixo;

        /* Cano de cima */
        DrawTexturePro(corpo, origem,
                       (Rectangle){canos[i].x, 0, LARGURA_CANO, alturaCima},
                       (Vector2){0, 0}, 0.0f, WHITE);
        DrawTextureEx(cabecaCima,
                      (Vector2){canos[i].x - OFFSET_CABECA,
                                alturaCima - cabecaCima.height * escalaCima},
                      0.0f, escalaCima, WHITE);

        /* Cano de baixo */
        DrawTexturePro(corpo, origem,
                       (Rectangle){canos[i].x, yBaixo, LARGURA_CANO, alturaBaixo},
                       (Vector2){0, 0}, 0.0f, WHITE);
        DrawTextureEx(cabecaBaixo,
                      (Vector2){canos[i].x - OFFSET_CABECA, yBaixo},
                      0.0f, escalaBaixo, WHITE);
    }
}

static void desenharPassaro(const Passaro *p)
{
    Texture2D sprite = (p->estado == BIRD_PARADO) ? p->spriteParado :
                       (p->estado == BIRD_VOANDO) ? p->spriteVoando :
                                                    p->spriteMorto;
    DrawTextureEx(sprite,
                  (Vector2){p->posicao.x - sprite.width  * ESCALA_PASSARO / 2.0f,
                            p->posicao.y - sprite.height * ESCALA_PASSARO / 2.0f},
                  0.0f, ESCALA_PASSARO, WHITE);
}

static void descarregarTexturas(Passaro *p, Texture2D corpo, Texture2D cima, Texture2D baixo)
{
    UnloadTexture(p->spriteParado);
    UnloadTexture(p->spriteVoando);
    UnloadTexture(p->spriteMorto);
    UnloadTexture(corpo);
    UnloadTexture(cima);
    UnloadTexture(baixo);
}

int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Flappy Bird");
    SetTargetFPS(60);
    SetExitKey(KEY_NULL);   /* ESC nao fecha mais a janela; usamos ESC para voltar ao menu */

    EstadoJogo estadoAtual = MENU;
    Passaro passaro = {0};
    passaro.raio = 20;

    passaro.spriteParado = LoadTexture("../imagens/sprite-1-4.png");
    passaro.spriteVoando = LoadTexture("../imagens/sprite-1-5.png");
    passaro.spriteMorto  = LoadTexture("../imagens/sprite-1-6.png");
    Texture2D corpoCano   = LoadTexture("../imagens/sprite-1-1.png");
    Texture2D cabecaCima  = LoadTexture("../imagens/sprite-1-2.png");
    Texture2D cabecaBaixo = LoadTexture("../imagens/sprite-1-3.png");

    /* Se alguma imagem nao carregou (caminho errado), avisa e encerra. */
    if (passaro.spriteParado.id == 0 || passaro.spriteVoando.id == 0 ||
        passaro.spriteMorto.id == 0 || corpoCano.id == 0 ||
        cabecaCima.id == 0 || cabecaBaixo.id == 0)
    {
        TraceLog(LOG_ERROR, "Falha ao carregar texturas. Confira a pasta ../imagens");
        descarregarTexturas(&passaro, corpoCano, cabecaCima, cabecaBaixo);
        CloseWindow();
        return 1;
    }

    Node *listaPontuacoes = criar_lista();
    if (listaPontuacoes == NULL) {
        descarregarTexturas(&passaro, corpoCano, cabecaCima, cabecaBaixo);
        CloseWindow();
        return 1;
    }

    Cano canos[NUM_CANOS];
    int pontos = 0;
    bool pontuacaoRegistrada = false;
    reiniciarJogo(&passaro, canos, &pontos);

    while (!WindowShouldClose())
    {
        /* ---------- ATUALIZACAO ---------- */
        switch (estadoAtual)
        {
            case MENU:
                if (IsKeyPressed(KEY_ENTER)) {
                    reiniciarJogo(&passaro, canos, &pontos);
                    pontuacaoRegistrada = false;
                    estadoAtual = JOGANDO;
                }
                break;

            case PAUSADO:
                if (IsKeyPressed(KEY_P)) estadoAtual = JOGANDO;
                if (IsKeyPressed(KEY_ESCAPE)) estadoAtual = MENU;
                break;

            case JOGANDO:
            {
                if (IsKeyPressed(KEY_ESCAPE)) { estadoAtual = MENU; break; }
                if (IsKeyPressed(KEY_P))      { estadoAtual = PAUSADO; break; }

                if (IsKeyPressed(KEY_SPACE)) {
                    passaro.velocidadeY = FORCA_PULO;
                }

                passaro.velocidadeY += GRAVIDADE;
                passaro.posicao.y += passaro.velocidadeY;
                passaro.estado = (passaro.velocidadeY < 0) ? BIRD_VOANDO : BIRD_PARADO;

                /* Teto e chao */
                if (passaro.posicao.y - passaro.raio <= 0 ||
                    passaro.posicao.y + passaro.raio >= ALTURA_TELA)
                {
                    finalizarPartida(&passaro, &estadoAtual, listaPontuacoes,
                                     pontos, &pontuacaoRegistrada);
                    break;
                }

                Rectangle birdRect = {
                    passaro.posicao.x - passaro.raio,
                    passaro.posicao.y - passaro.raio,
                    passaro.raio * 2,
                    passaro.raio * 2
                };

                for (int i = 0; i < NUM_CANOS; i++)
                {
                    canos[i].x -= VELOCIDADE_CANO;

                    /* So recicla quando o cano saiu POR COMPLETO da tela
                       (corpo + cabeca) e o recoloca fora da tela, a direita. */
                    if (canos[i].x < -(LARGURA_CANO + OFFSET_CABECA)) {
                        canos[i].x += NUM_CANOS * DISTANCIA_CANOS;
                        canos[i].aberturaY = (float)GetRandomValue(150, 450);
                        canos[i].passou = false;
                    }

                    float yBaixo = canos[i].aberturaY + ESPACO_CANO / 2.0f;
                    Rectangle canoSuperior = {canos[i].x, 0, LARGURA_CANO,
                                              canos[i].aberturaY - ESPACO_CANO / 2.0f};
                    Rectangle canoInferior = {canos[i].x, yBaixo, LARGURA_CANO,
                                              ALTURA_TELA - yBaixo};

                    if (CheckCollisionRecs(birdRect, canoSuperior) ||
                        CheckCollisionRecs(birdRect, canoInferior))
                    {
                        finalizarPartida(&passaro, &estadoAtual, listaPontuacoes,
                                         pontos, &pontuacaoRegistrada);
                        break;
                    }

                    if (!canos[i].passou && passaro.posicao.x > canos[i].x + LARGURA_CANO) {
                        pontos++;
                        canos[i].passou = true;
                    }
                }
                break;
            }

            case GAMEOVER:
                if (IsKeyPressed(KEY_ENTER)) {
                    reiniciarJogo(&passaro, canos, &pontos);
                    pontuacaoRegistrada = false;
                    estadoAtual = JOGANDO;
                }
                if (IsKeyPressed(KEY_ESCAPE)) estadoAtual = MENU;
                break;
        }

        /* ---------- DESENHO ---------- */
        BeginDrawing();
        ClearBackground(SKYBLUE);

        if (estadoAtual == MENU)
        {
            int recorde;
            estatisticas(listaPontuacoes, &recorde);

            desenharTextoCentralizado("FLAPPY BIRD", 180, 50, GOLD);
            desenharTextoCentralizado("ENTER - JOGAR", 300, 30, WHITE);
            desenharTextoCentralizado("ESPACO - PULAR  |  P - PAUSAR", 360, 20, WHITE);

            if (recorde > 0) {
                char texto[48];
                snprintf(texto, sizeof(texto), "Recorde: %d", recorde);
                desenharTextoCentralizado(texto, 420, 24, DARKGREEN);
            }
        }
        else
        {
            desenharCanos(canos, corpoCano, cabecaCima, cabecaBaixo);
            desenharPassaro(&passaro);

            if (estadoAtual == GAMEOVER) {
                desenharGameOver(listaPontuacoes, pontos);
            } else {
                char textoPontos[32];
                snprintf(textoPontos, sizeof(textoPontos), "Pontos: %d", pontos);
                DrawText(textoPontos, 20, 20, 30, BLACK);

                if (estadoAtual == PAUSADO) {
                    desenharTextoCentralizado("PAUSADO", 230, 50, WHITE);
                    desenharTextoCentralizado("P - CONTINUAR", 300, 25, WHITE);
                }
            }
        }
        EndDrawing();
    }

    descarregarTexturas(&passaro, corpoCano, cabecaCima, cabecaBaixo);
    destruir_lista(listaPontuacoes);
    CloseWindow();
    return 0;
}
