#include <math.h>
#include <stdlib.h>
#include <time.h>

#include "game.h"
#include "Diamond_Blocks_visual.h"
#include "placar.h"

#define BOARD_COLS    6
#define BOARD_ROWS    4
#define BOARD_SPACING 2.5f
#define MAX_BOXES     50

static const Color COR_TABULEIRO    = { 12,  20,  55, 255 };
static const Color COR_GRID_FRACO   = { 40,  70, 130, 120 };
static const Color COR_GRID_FORTE   = { 70, 110, 180, 180 };
static const Color COR_BLOCO_BORDA  = {200, 235, 255, 255 };
static const Color COR_SOMBRA       = {  0,   0,   0,  70 };
static const Color COR_TEXTO_SUCESSO = {  0, 190, 100, 255 };
static const Color COR_TEXTO_ERRO    = {190,  50,  70, 255 };

static const float BLOCO_SIZE = 0.7f;


Camera3D cameraJogo = {0};

int   boxCount      = 0;
float showTime      = 2.0f;
float timerCaixas   = 0.0f;

int  playerAnswer   = 0;
bool playerCorrect  = false;

GameState state     = SHOW_BOXES;
Vector3 boxes[MAX_BOXES];

extern Sound sClick;
extern Sound sAcertou;
extern Sound sErrou;
extern Sound sNovoRecorde;

extern RegistroPlacar placar[MAX_JOGADORES];
extern int qtdPlacar;
extern char gNomeJogador[TAM_NOME];

// ------------------------------------------------------------------

static int GetRandomBoxCount(int round)
{
    if (round == 1) return GetRandomValue(3, 4);
    if (round == 2) return GetRandomValue(4, 6);
    if (round == 3) return GetRandomValue(5, 7);
    if (round == 4) return GetRandomValue(6, 9);

    int qtd = GetRandomValue(7, 12);
    if (qtd > MAX_BOXES) qtd = MAX_BOXES;
    return qtd;
}

static void GenerateBoxes(int count)
{
    const int cols    = BOARD_COLS;
    const int rows    = BOARD_ROWS;
    const float step  = BOARD_SPACING;

    float boardW = (cols - 1) * step;
    float boardD = (rows - 1) * step;

    float startX = -boardW/2.0f;
    float startZ = -boardD/2.0f;

    Vector3 grid[BOARD_COLS * BOARD_ROWS];
    int totalCells = cols * rows;
    int idx = 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            grid[idx].x = startX + c * step;
            grid[idx].y = 0.5f;
            grid[idx].z = startZ + r * step;
            idx++;
        }
    }

    for (int i = totalCells - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        Vector3 tmp = grid[i];
        grid[i] = grid[j];
        grid[j] = tmp;
    }

    if (count > totalCells) count = totalCells;

    boxCount = count;
    for (int i = 0; i < boxCount; i++) {
        boxes[i] = grid[i];
    }
}

static void DesenharTabuleiro3D(void)
{
    const int cols    = BOARD_COLS;
    const int rows    = BOARD_ROWS;
    const float step  = BOARD_SPACING;

    float boardW = (cols - 1) * step;
    float boardD = (rows - 1) * step;

    float xMin = -boardW/2.0f - step*0.5f;
    float xMax =  boardW/2.0f + step*0.5f;
    float zMin = -boardD/2.0f - step*0.5f;
    float zMax =  boardD/2.0f + step*0.5f;

    Vector3 basePos  = { 0.0f, -0.52f, 0.0f };
    Vector3 baseSize = { (xMax - xMin), 1.0f, (zMax - zMin) };
    DrawCubeV(basePos, baseSize, COR_TABULEIRO);

    for (int r = 0; r <= rows; r++) {
        float z = zMin + r * step;
        Color cor = (r == 0 || r == rows) ? COR_GRID_FORTE : COR_GRID_FRACO;
        DrawLine3D((Vector3){ xMin, 0.0f, z }, (Vector3){ xMax, 0.0f, z }, cor);
    }

    for (int c = 0; c <= cols; c++) {
        float x = xMin + c * step;
        Color cor = (c == 0 || c == cols) ? COR_GRID_FORTE : COR_GRID_FRACO;
        DrawLine3D((Vector3){ x, 0.0f, zMin }, (Vector3){ x, 0.0f, zMax }, cor);
    }
}

static void InitGameplay3D(DadosJogo *jogo)
{
    cameraJogo.position   = (Vector3){ 0.0f, 10.0f, 14.0f };
    cameraJogo.target     = (Vector3){ 0.0f,  0.0f,  0.0f };
    cameraJogo.up         = (Vector3){ 0.0f,  1.0f,  0.0f };
    cameraJogo.fovy       = 35.0f;
    cameraJogo.projection = CAMERA_PERSPECTIVE;

    srand((unsigned int)time(NULL));

    showTime      = 2.0f;
    timerCaixas   = 0.0f;
    state         = SHOW_BOXES;
    playerAnswer  = 0;
    playerCorrect = false;

    boxCount = GetRandomBoxCount(jogo->fase);
    GenerateBoxes(boxCount);
}


void Game_Init(DadosJogo *jogo)
{
    jogo->score       = 0;
    jogo->vidas       = 5;
    jogo->fase        = 1;
    jogo->novoRecorde = false;

    InitGameplay3D(jogo);
}

void Game_Reset(DadosJogo *jogo)
{
    jogo->score       = 0;
    jogo->vidas       = 5;
    jogo->fase        = 1;
    jogo->novoRecorde = false;

    InitGameplay3D(jogo);
}

void Game_Update(DadosJogo *jogo)
{
    float dt = GetFrameTime();
    timerCaixas += dt;

    switch (state) {

        case SHOW_BOXES:
            if (timerCaixas >= showTime) {
                state        = WAIT_INPUT;
                timerCaixas  = 0.0f;
                playerAnswer = 0;
            }
            break;

        case WAIT_INPUT:

            if (IsKeyPressed(KEY_UP)) {
                playerAnswer++;
                PlaySound(sClick);
            }

            if (IsKeyPressed(KEY_DOWN)) {
                if (playerAnswer > 0) playerAnswer--;
                PlaySound(sClick);
            }

            if (timerCaixas >= 3.0f) {
                state       = SHOW_RESULT;
                timerCaixas = 0.0f;

                int diff = abs(playerAnswer - boxCount);

                if (diff == 0) {
                    jogo->score   += 100;
                    playerCorrect  = true;
                    PlaySound(sAcertou);
                } else {
                    playerCorrect = false;

                    if (diff == 1) {
                        jogo->score += 50;
                        PlaySound(sAcertou);
                    } else {
                        PlaySound(sErrou);
                    }

                    jogo->vidas--;
                }
            }
            break;

        case SHOW_RESULT:
            if (timerCaixas >= 2.0f) {

                if (jogo->vidas <= 0) {
                    int entrou = atualizar_placar(placar,
                                                  &qtdPlacar,
                                                  gNomeJogador,
                                                  jogo->score);

                    if (entrou) {
                        salvar_placar("placares_diamonds.txt",
                                      placar,
                                      qtdPlacar);

                        if (qtdPlacar > 0) {
                            jogo->melhorScore = placar[0].pontuacao;
                        }

                        jogo->novoRecorde = true;
                        PlaySound(sNovoRecorde);
                        IniciarTransicao(TELA_NOVORECORDE);
                    } else {
                        jogo->novoRecorde = false;
                        IniciarTransicao(TELA_GAMEOVER);
                    }
                } else {

                    jogo->fase++;

                    showTime -= 0.1f;
                    if (showTime < 1.0f) showTime = 1.0f;

                    boxCount    = GetRandomBoxCount(jogo->fase);
                    GenerateBoxes(boxCount);

                    timerCaixas = 0.0f;
                    state       = SHOW_BOXES;
                }
            }
            break;
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }
}

// Desenha o tabuleiro 3D + HUD
void Game_Draw(const DadosJogo *jogo)
{
    BeginMode3D(cameraJogo);

    DesenharTabuleiro3D();

    Color corCaixa = (Color){  40, 160, 255, 255 };
    if (state == SHOW_RESULT) {
        corCaixa = playerCorrect ? (Color){  50, 200, 120, 255 }
                                 : (Color){ 220,  60,  70, 255 };
    }

    if (state == SHOW_BOXES || state == SHOW_RESULT) {
        for (int i = 0; i < boxCount; i++) {
            Vector3 pos = boxes[i];

            Vector3 tam = { BLOCO_SIZE, BLOCO_SIZE, BLOCO_SIZE };

            Vector3 sombraPos = pos;
            sombraPos.y = -0.49f;
            Vector3 sombraTam = { tam.x * 1.3f, 0.08f, tam.z * 1.3f };
            DrawCubeV(sombraPos, sombraTam, COR_SOMBRA);

            DrawCubeV(pos, tam, corCaixa);
            DrawCubeWiresV(pos, tam, COR_BLOCO_BORDA);

            Vector3 topPos  = { pos.x, pos.y + tam.y * 0.51f, pos.z };
            Vector3 topSize = { tam.x * 0.9f, 0.02f, tam.z * 0.9f };
            Color   topCor  = (Color){ 255, 255, 255, 40 };
            DrawCubeV(topPos, topSize, topCor);
        }
    }

    EndMode3D();

    // HUD 2D
    DrawText("Diamonds Blocks", 20, 10, 28, RAYWHITE);

    DrawText(TextFormat("Pontos: %d", jogo->score),       20,  50, 20, LIGHTGRAY);
    DrawText(TextFormat("Vidas: %d",  jogo->vidas),       20,  80, 20, LIGHTGRAY);
    DrawText(TextFormat("Rodada: %d", jogo->fase),        20, 110, 20, LIGHTGRAY);
    DrawText(TextFormat("Recorde: %d", jogo->melhorScore),
             20, 140, 20, GOLD);

    int centroX = LARGURA_TELA / 2;

    if (state == WAIT_INPUT) {

        DrawText("Use SETA CIMA/BAIXO para responder quantas caixas viu",
                 centroX - MeasureText("Use SETA CIMA/BAIXO para responder quantas caixas viu", 18)/2,
                 ALTURA_TELA - 140, 18, LIGHTGRAY);

        DrawText("Sua resposta:",
                 centroX - 80,
                 ALTURA_TELA - 110, 24, RAYWHITE);

        DrawText(TextFormat("%d", playerAnswer),
                 centroX - 10,
                 ALTURA_TELA - 80, 36, (Color){180, 120, 250, 255});

        DrawText(TextFormat("Tempo restante: %.1f", 3.0f - timerCaixas),
                 centroX - 100,
                 ALTURA_TELA - 40, 20, (Color){255, 230, 120, 255});
    }
    else if (state == SHOW_RESULT) {

        int diff = abs(playerAnswer - boxCount);

        if (playerCorrect) {
            DrawText("Correto! +100",
                     centroX - MeasureText("Correto! +100", 28)/2,
                     ALTURA_TELA - 140, 28, COR_TEXTO_SUCESSO);
        } else if (diff == 1) {
            DrawText("Foi quase! +50",
                     centroX - MeasureText("Foi quase! +50", 28)/2,
                     ALTURA_TELA - 140, 28, ORANGE);
        } else {
            DrawText("Errado!",
                     centroX - MeasureText("Errado!", 30)/2,
                     ALTURA_TELA - 140, 30, COR_TEXTO_ERRO);
        }

        DrawText(TextFormat("Caixas corretas: %d", boxCount),
                 centroX - MeasureText("Caixas corretas: 00", 22)/2,
                 ALTURA_TELA - 100, 22, LIGHTGRAY);

        DrawText(TextFormat("Sua resposta: %d", playerAnswer),
                 centroX - MeasureText("Sua resposta: 00", 22)/2,
                 ALTURA_TELA - 70, 22, LIGHTGRAY);
    }
}

