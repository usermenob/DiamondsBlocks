#ifndef DIAMOND_BLOCKS_VISUAL_H
#define DIAMOND_BLOCKS_VISUAL_H

#include "raylib.h"
#include <stdbool.h>
#include "game.h"
#include "placar.h"


#define largura_tela LARGURA_TELA
#define altura_tela  ALTURA_TELA


typedef enum {
    TELA_NOME_JOGADOR = 0,
    TELA_MENU,
    TELA_INSTRUCOES,
    TELA_GAMEPLAY,
    TELA_LIDERES,
    TELA_GAMEOVER,
    TELA_NOVORECORDE
} TelasJogo;

extern DadosJogo jogo;


void Visual_Init(void);
void Visual_Update(void);
void Visual_Draw(void);
void Visual_Unload(void);


void IniciarTransicao(TelasJogo tela);

extern Sound sClick;
extern Sound sAcertou;
extern Sound sErrou;
extern Sound sNovoRecorde;

extern RegistroPlacar placar[MAX_JOGADORES];
extern int qtdPlacar;

extern char gNomeJogador[TAM_NOME];

#endif

