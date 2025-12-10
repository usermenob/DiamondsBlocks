#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>


typedef enum {
    SHOW_BOXES,
    WAIT_INPUT,
    SHOW_RESULT
} GameState;


void InitGame(void);
void UpdateGame(float deltaTime);
void DrawGame(void);

#endif

