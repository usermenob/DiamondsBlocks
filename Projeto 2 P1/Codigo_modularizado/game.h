#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>

#define LARGURA_TELA 900
#define ALTURA_TELA  500

typedef struct {
    int score;
    int melhorScore;
    int vidas;
    int fase;
    bool novoRecorde;
} DadosJogo;

typedef enum {
    SHOW_BOXES,
    WAIT_INPUT,
    SHOW_RESULT
} GameState;


void Game_Init(DadosJogo *jogo);
void Game_Reset(DadosJogo *jogo);
void Game_Update(DadosJogo *jogo);
void Game_Draw(const DadosJogo *jogo);

#endif

