#include "raylib.h"
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define MAX_BOXES 50

typedef enum {
    SHOW_BOXES,
    WAIT_INPUT,
    SHOW_RESULT
} GameState;

Camera3D camera;
Vector3 boxes[MAX_BOXES];

int boxCount = 0;
int roundNum = 1;
int lives = 5;
int score = 0;

float showTime = 2.0f;   // Tempo inicial para ver as caixas
float timer = 0;

int playerAnswer = 0;
bool playerCorrect = false;

GameState state = SHOW_BOXES;


// -------------------- Quantidade aleatória por rodada --------------------
int GetRandomBoxCount(int round) {
    if (round == 1) return GetRandomValue(3, 4);
    if (round == 2) return GetRandomValue(4, 6);
    if (round == 3) return GetRandomValue(5, 7);
    if (round == 4) return GetRandomValue(6, 9);

    return GetRandomValue(7, 12);
}


// -------------------- Gerar posições --------------------
void GenerateBoxes(int count) {
    for (int i = 0; i < count; i++) {
        float minX = -8.0f, maxX = 8.0f;
        float minZ = -6.0f, maxZ = 6.0f;

        boxes[i].x = minX + (float)rand()/RAND_MAX * (maxX - minX);
        boxes[i].y = 1;
        boxes[i].z = minZ + (float)rand()/RAND_MAX * (maxZ - minZ);
    }
}


// -------------------- Desenhar caixas --------------------
void DrawBoxes(Color c) {
    for (int i = 0; i < boxCount; i++) {
        DrawCube(boxes[i], 1, 1, 1, c);
        DrawCubeWires(boxes[i], 1, 1, 1, BLACK);
    }
}


// -------------------- Setup --------------------
void InitGame() {
    camera.position = (Vector3){ 0, 15, 15 };
    camera.target   = (Vector3){ 0, 1, 0 };
    camera.up       = (Vector3){ 0, 1, 0 };
    camera.fovy     = 45;
    camera.projection = CAMERA_PERSPECTIVE;

    srand(time(NULL));

    boxCount = GetRandomBoxCount(roundNum);
    GenerateBoxes(boxCount);
}


// -------------------- Atualização --------------------
void UpdateGame(float dt) {
    timer += dt;

    switch (state) {

        // ---------- 1. Mostrar caixas ----------
        case SHOW_BOXES:
            if (timer >= showTime) {
                state = WAIT_INPUT;
                timer = 0;
                playerAnswer = 0;
            }
            break;

        // ---------- 2. Jogador ajusta resposta ----------
        case WAIT_INPUT:

            if (IsKeyPressed(KEY_UP)) playerAnswer++;
            if (IsKeyPressed(KEY_DOWN)) playerAnswer--;

            if (playerAnswer < 0) playerAnswer = 0;

            if (timer >= 3.0f) {
                state = SHOW_RESULT;
                timer = 0;

                int diff = abs(playerAnswer - boxCount);

                if (diff == 0) {
                    score += 100;
                    playerCorrect = true;
                } else {
                    playerCorrect = false;

                    if (diff == 1)
                        score += 50;

                    lives--;
                }
            }
            break;

        // ---------- 3. Mostrar resultado ----------
        case SHOW_RESULT:
            if (timer >= 2.0f) {

                if (lives <= 0) {
                    // Reset completo
                    roundNum = 1;
                    lives = 5;
                    score = 0;
                    showTime = 2.0f;   // Reinicia o tempo também
                } else {
                    roundNum++;

                    // 🔥 Tempo diminui gradualmente, mínimo = 1.0s
                    showTime -= 0.1f;
                    if (showTime < 1.0f) showTime = 1.0f;
                }

                boxCount = GetRandomBoxCount(roundNum);
                GenerateBoxes(boxCount);

                timer = 0;
                state = SHOW_BOXES;
            }
            break;
    }
}


// -------------------- Desenho --------------------
void DrawGame() {
    BeginDrawing();
    ClearBackground(RAYWHITE);

    BeginMode3D(camera);

    if (state == SHOW_BOXES)
        DrawBoxes(BLUE);

    if (state == SHOW_RESULT) {
        if (playerCorrect)
            DrawBoxes(GREEN);
        else
            DrawBoxes(RED);
    }

    EndMode3D();

    DrawText(TextFormat("Vidas: %d", lives), 20, 20, 30, RED);
    DrawText(TextFormat("Rodada: %d", roundNum), 20, 60, 30, BLACK);
    DrawText(TextFormat("Pontos: %d", score), 20, 100, 30, DARKPURPLE);

    if (state == WAIT_INPUT) {

        DrawText("Use as setas para ajustar sua resposta.",
                 300, 40, 28, BLACK);

        DrawText("Sua resposta:", 400, 180, 30, BLACK);
        DrawText(TextFormat("%d", playerAnswer),
                 400, 230, 80, PURPLE);

        DrawText(TextFormat("Tempo restante: %.1f", 3 - timer),
                 400, 330, 30, MAROON);
    }

    if (state == SHOW_RESULT) {

        if (playerCorrect) {
            DrawText("Correto! +100 pontos", 400, 40, 40, GREEN);
        } else {
            int diff = abs(playerAnswer - boxCount);

            if (diff == 1)
                DrawText("Quase! +50 pontos", 400, 40, 40, ORANGE);
            else
                DrawText("Errado!", 400, 40, 50, RED);

            DrawText(TextFormat("Resposta correta: %d", boxCount),
                     400, 110, 40, BLACK);
        }
    }

    EndDrawing();
}


// -------------------- Main --------------------
int main() {

    InitWindow(1200, 720, "Jogo de Contagem de Caixas");
    SetTargetFPS(60);
    InitGame();

    while (!WindowShouldClose()) {
        UpdateGame(GetFrameTime());
        DrawGame();
    }

    CloseWindow();
    return 0;
}
