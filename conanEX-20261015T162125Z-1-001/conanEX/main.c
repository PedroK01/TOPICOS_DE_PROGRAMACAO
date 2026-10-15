#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include "listaEncadeada.h"
#define LARGURA_TELA 800
#define ALTURA_TELA 600
#define NUM_CANOS 3
#define LARGURA_CANO 80
#define ESPACO_CANO 180
typedef enum EstadoJogo {
    MENU,
    JOGANDO,
    GAMEOVER
} EstadoJogo;
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
void reiniciarjogo(Passaro *passaro, Cano canos[], int *pontos)
{
    passaro->posicao = (Vector2){200, ALTURA_TELA / 2.0f};
    passaro->velocidadeY = 0;
    passaro->estado = BIRD_PARADO;
    for (int i = 0; i < NUM_CANOS; i++) {
        canos[i].x = 600 + i * 250;
        canos[i].aberturaY = (float)GetRandomValue(150, 450);
        canos[i].passou = false;
    }
    *pontos = 0;
}
void registrar_pontuacao(Node *listaPontuacoes, int pontos)
{
    if (listaPontuacoes != NULL) {
        inserir_fim(listaPontuacoes, pontos);
    }
}


int main(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Flappy Bird");
    SetTargetFPS(60);

    EstadoJogo estadoAtual = MENU;
    Passaro passaro;
    passaro.raio = 20;
    passaro.estado = BIRD_PARADO;
    // Carregando Texturas
    passaro.spriteParado = LoadTexture("../imagens/sprite-1-4.png");
    passaro.spriteVoando = LoadTexture("../imagens/sprite-1-5.png");
    passaro.spriteMorto = LoadTexture("../imagens/sprite-1-6.png");
    Cano canos[NUM_CANOS];
    Texture2D corpoCano = LoadTexture("../imagens/sprite-1-1.png");
    Texture2D cabecaCima = LoadTexture("../imagens/sprite-1-2.png");
    Texture2D cabecaBaixo = LoadTexture("../imagens/sprite-1-3.png");
    int pontos = 0;
    reiniciarjogo(&passaro, canos, &pontos);
    while (!WindowShouldClose())
    {
     
        switch (estadoAtual)
        {
            case MENU:
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    reiniciarjogo(&passaro, canos, &pontos);
                    estadoAtual = JOGANDO;
                }
                break;
            }
            case JOGANDO:
            {
                if (IsKeyPressed(KEY_ESCAPE))
                    estadoAtual = MENU;
                if (IsKeyPressed(KEY_SPACE)) {
                    passaro.velocidadeY = -8.0f;
                    passaro.estado = BIRD_VOANDO;
                }
                passaro.velocidadeY += 0.45f;
                passaro.posicao.y += passaro.velocidadeY;
                if (passaro.estado != BIRD_MORTO) {
                    if (passaro.velocidadeY < 0) passaro.estado = BIRD_VOANDO;
                    else passaro.estado = BIRD_PARADO;
                }
                // Colisão com teto e chão
                if (passaro.posicao.y - passaro.raio <= 0 ||
                    passaro.posicao.y + passaro.raio >= ALTURA_TELA)
                {
                    passaro.estado = BIRD_MORTO;
                    estadoAtual = GAMEOVER;
                }
                Rectangle birdRect = {
                    passaro.posicao.x - passaro.raio,
                    passaro.posicao.y - passaro.raio,
                    passaro.raio * 2,
                    passaro.raio * 2
                };
                for (int i = 0; i < NUM_CANOS; i++)
                {
                    canos[i].x -= 4;
                    // Recicla o cano quando sai da tela
                    if (canos[i].x < -LARGURA_CANO)
                    {
                        canos[i].x += NUM_CANOS * 250; // Mantém espaçamento uniforme
                        canos[i].aberturaY = (float)GetRandomValue(150, 450);
                        canos[i].passou = false;
                    }
                    Rectangle canoSuperior = {
                        canos[i].x,
                        0,
                        LARGURA_CANO,
                        canos[i].aberturaY - ESPACO_CANO / 2.0f
                    };
                    Rectangle canoInferior = {
                        canos[i].x,
                        canos[i].aberturaY + ESPACO_CANO / 2.0f,
                        LARGURA_CANO,
                        ALTURA_TELA - (canos[i].aberturaY + ESPACO_CANO / 2.0f)
                    };
                    if (CheckCollisionRecs(birdRect, canoSuperior) ||
                        CheckCollisionRecs(birdRect, canoInferior))
                    {
                        passaro.estado = BIRD_MORTO;
                        estadoAtual = GAMEOVER;
                    }
                    if (!canos[i].passou &&
                        passaro.posicao.x > canos[i].x + LARGURA_CANO)
                    {
                        pontos++;
                        canos[i].passou = true;
                    }
                }
                break;
            }
            case GAMEOVER:
            {
                if (IsKeyPressed(KEY_ENTER))
                {
                    reiniciarjogo(&passaro, canos, &pontos);
                    estadoAtual = JOGANDO;
                }
                if (IsKeyPressed(KEY_ESCAPE))
                    estadoAtual = MENU;
                break;
            }
        }
        BeginDrawing();
        ClearBackground(SKYBLUE);
        switch (estadoAtual)
        {
            case MENU:
            {
                DrawText("FLAPPY BIRD",
                         LARGURA_TELA / 2 - MeasureText("FLAPPY BIRD", 50) / 2,
                         180, 50, GOLD);
                DrawText("ENTER - JOGAR",
                         LARGURA_TELA / 2 - MeasureText("ENTER - JOGAR", 30) / 2,
                         300, 30, WHITE);
                DrawText("ESPACO - PULAR",
                         LARGURA_TELA / 2 - MeasureText("ESPACO - PULAR", 20) / 2,
                         360, 20, WHITE);
                break;
            }
            case JOGANDO:
            case GAMEOVER:
            {
                // Desenha os canos
                for (int i = 0; i < NUM_CANOS; i++)
                {
                    float alturaCima = canos[i].aberturaY - ESPACO_CANO / 2.0f;
                    float alturaBaixo = ALTURA_TELA - (canos[i].aberturaY + ESPACO_CANO / 2.0f);
                    // Escalas separadas para cada cabeça
                    float escalaCabecaCima = (LARGURA_CANO / cabecaCima.width)*1.325;
                    float escalaCabecaBaixo =(LARGURA_CANO / cabecaBaixo.width)*1.325;
                    // --- Cano de cima ---
                    DrawTexturePro(
                        corpoCano,
                        (Rectangle){0, 0, (float)corpoCano.width, (float)corpoCano.height},
                        (Rectangle){canos[i].x, 0, LARGURA_CANO, alturaCima},
                        (Vector2){0, 0}, 0.0f, WHITE
                    );
                    DrawTextureEx(
                        cabecaCima,
                        (Vector2){
                            canos[i].x-13,
                            alturaCima - cabecaCima.height * escalaCabecaCima
                        },
                        0.0f,
                        escalaCabecaCima,
                        WHITE
                    );
                    // --- Cano de baixo ---
                    DrawTexturePro(
                        corpoCano,
                        (Rectangle){0, 0, (float)corpoCano.width, (float)corpoCano.height},
                        (Rectangle){
                            canos[i].x,
                            canos[i].aberturaY + ESPACO_CANO / 2.0f,
                            LARGURA_CANO,
                            alturaBaixo
                        },
                        (Vector2){0, 0}, 0.0f, WHITE
                    );
                    DrawTextureEx(
                        cabecaBaixo,
                        (Vector2){
                            canos[i].x-13,
                            canos[i].aberturaY + ESPACO_CANO / 2.0f
                        },
                        0.0f,
                        escalaCabecaBaixo,
                        WHITE
                    );
                }
                // Desenha o pássaro
                Texture2D spriteAtual =
                    (passaro.estado == BIRD_PARADO) ? passaro.spriteParado :
                    (passaro.estado == BIRD_VOANDO) ? passaro.spriteVoando :
                                                      passaro.spriteMorto;
                float escalaPassaro = 4.0f;
                DrawTextureEx(
                    spriteAtual,
                    (Vector2){
                        passaro.posicao.x - (spriteAtual.width * escalaPassaro) / 2.0f,
                        passaro.posicao.y - (spriteAtual.height * escalaPassaro) / 2.0f
                    },
                    0.0f,
                    escalaPassaro,
                    WHITE
                );
                // Pontuação
                char textoPontos[32];
                sprintf(textoPontos, "Pontos: %d", pontos);
                DrawText(textoPontos, 20, 20, 30, BLACK);
                // Textos do Game Over
                if (estadoAtual == GAMEOVER)
                {
                    DrawText("GAME OVER",
                             LARGURA_TELA / 2 - MeasureText("GAME OVER", 50) / 2,
                             180, 50, RED);
                    char texto[64];
                    sprintf(texto, "PONTOS: %d", pontos);
                    DrawText(texto,
                             LARGURA_TELA / 2 - MeasureText(texto, 30) / 2,
                             260, 30, BLACK);
                    DrawText("ENTER - REINICIAR",
                             LARGURA_TELA / 2 - MeasureText("ENTER - REINICIAR", 25) / 2,
                             330, 25, WHITE);
                    DrawText("ESC - MENU",
                             LARGURA_TELA / 2 - MeasureText("ESC - MENU", 25) / 2,
                             380, 25, WHITE);

                    
                }
                break;
            }
        }
        EndDrawing();
    }
    // Desalocando Texturas
    UnloadTexture(passaro.spriteParado);
    UnloadTexture(passaro.spriteVoando);
    UnloadTexture(passaro.spriteMorto);
    UnloadTexture(corpoCano);
    UnloadTexture(cabecaCima);
    UnloadTexture(cabecaBaixo);

    /* Libera todos os nos da lista encadeada. */

    CloseWindow();
    return 0;
}