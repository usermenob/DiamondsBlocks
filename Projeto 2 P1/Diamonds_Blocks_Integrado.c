#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>


#define largura_tela 900
#define altura_tela  500
#define BOARD_COLS    6
#define BOARD_ROWS    4
#define BOARD_SPACING 2.5f 

// ----------------- BLOCO DE CORES E DIMENSÕES DO TABULEIRO -----------------

static const Color COR_FUNDO        = {  5,  10,  30, 255 };  
static const Color COR_TABULEIRO    = { 12,  20,  55, 255 };  
static const Color COR_GRID_FRACO   = { 40,  70, 130, 120 }; 
static const Color COR_GRID_FORTE   = { 70, 110, 180, 180 };  
static const Color COR_BLOCO_BORDA  = {200, 235, 255, 255 };  
static const Color COR_SOMBRA       = {  0,   0,   0,  70 };
static const Color COR_TEXTO_SUCESSO = {  0, 190, 100, 255 }; 
static const Color COR_TEXTO_ERRO    = {190,  50,  70, 255 };  


static const float BLOCO_SIZE   = 0.7f;

// -------------------------------------------------------------------------------
typedef enum {
    TELA_MENU = 0,
    TELA_INSTRUCOES,
    TELA_GAMEPLAY,
    TELA_LIDERES,
    TELA_GAMEOVER,
    TELA_NOVORECORDE
} TelasJogo;


typedef struct {
    int score;
    int melhorScore;
    int vidas;
    int fase;
    bool novoRecorde;
} DadosJogo;

TelasJogo tela_atual = TELA_MENU;
DadosJogo jogo;

Sound sClick;
Sound sAcertou;
Sound sErrou;
Sound sNovoRecorde;
Music music;

float masterVolume = 1.0f;
float musicVolume  = 0.5f;
float sfxVolume    = 0.9f;

typedef enum{
    SHOW_BOXES,
    WAIT_INPUT,
    SHOW_RESULT
} GameState;

#define MAX_BOXES 50

Camera3D cameraJogo = {0};

int boxCount = 0;
float showTime = 2.0f;
float timerCaixas = 0.0f;

int playerAnswer = 0;
bool playerCorrect = false;

GameState state = SHOW_BOXES;
Vector3 boxes[MAX_BOXES];

bool  emTransicao     = false;
float transicaoAlpha  = 0.0f;
TelasJogo proximaTela;


int menuOpcoes = 0;

int GetRandomBoxCount(int round)
{
    if (round == 1) return GetRandomValue(3, 4);
    if (round == 2) return GetRandomValue(4, 6);
    if (round == 3) return GetRandomValue(5, 7);
    if (round == 4) return GetRandomValue(6, 9);


    int qtd = GetRandomValue(7, 12);


    if (qtd > MAX_BOXES) qtd = MAX_BOXES;

    return qtd;
}

void GenerateBoxes(int count)
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



void InitGameplay3D(void)
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

    
    boxCount = GetRandomBoxCount(jogo.fase);
    GenerateBoxes(boxCount);
}


void ResetGame(void)
{
    jogo.score       = 0;
    jogo.vidas       = 5;
    jogo.fase        = 1;
    jogo.novoRecorde = false;

    InitGameplay3D();
}


void IniciarTransicao(TelasJogo tela)
{
    emTransicao    = true;
    proximaTela    = tela;
    transicaoAlpha = 0.0f;
}

void AtualizarTransicao(void)
{
    transicaoAlpha += 0.08f;

    if (transicaoAlpha >= 1.0f) {
        transicaoAlpha = 0.0f;
        emTransicao    = false;
        tela_atual     = proximaTela;

        if (tela_atual == TELA_GAMEPLAY) {
            ResetGame();
        }
    }
}

void DesenharTransicao(void)
{
    DrawRectangle(0, 0, largura_tela, altura_tela,
                  (Color){0, 0, 0, (unsigned char)(transicaoAlpha * 255)});
}


void AtualizarGame(void)
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
                    jogo.score    += 100;
                    playerCorrect  = true;
                    PlaySound(sAcertou);
                } else {
                    playerCorrect = false;

                    if (diff == 1) {
                        jogo.score += 50;
                        PlaySound(sAcertou);
                    } else {
                        PlaySound(sErrou);
                    }

                    jogo.vidas--;
                }
            }
            break;

        
        case SHOW_RESULT:
            if (timerCaixas >= 2.0f) {

                if (jogo.vidas <= 0) {
                    
                    if (jogo.score > jogo.melhorScore) {
                        jogo.melhorScore = jogo.score;
                        jogo.novoRecorde = true;
                        PlaySound(sNovoRecorde);
                        IniciarTransicao(TELA_NOVORECORDE);
                    } else {
                        jogo.novoRecorde = false;
                        IniciarTransicao(TELA_GAMEOVER);
                    }
                } else {
                    
                    jogo.fase++;

                    showTime -= 0.1f;
                    if (showTime < 1.0f) showTime = 1.0f;

                    boxCount    = GetRandomBoxCount(jogo.fase);
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

void DesenharFundoBonito(void)
{

    Color top    = (Color){  5,  10,  35, 255 };   
    Color bottom = (Color){  3,   4,  18, 255 }; 

    DrawRectangleGradientV(0, 0, largura_tela, altura_tela, top, bottom);


    Color vignette = (Color){ 0, 0, 0, 70 };
    DrawRectangle(0, 0, largura_tela, 40, vignette);                      
    DrawRectangle(0, altura_tela-60, largura_tela, 60, vignette);          
    DrawRectangle(0, 0, 80, altura_tela, vignette);                        
    DrawRectangle(largura_tela-80, 0, 80, altura_tela, vignette);          
}

void DesenharGameplay(void)
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

    // ---------------- HUD 2D ----------------
    DrawText("Diamonds Blocks", 20, 10, 28, RAYWHITE);

    DrawText(TextFormat("Pontos: %d", jogo.score),       20, 50, 20, LIGHTGRAY);
    DrawText(TextFormat("Vidas: %d",  jogo.vidas),       20, 80, 20, LIGHTGRAY);
    DrawText(TextFormat("Rodada: %d", jogo.fase),        20,110, 20, LIGHTGRAY);
    DrawText(TextFormat("Recorde: %d", jogo.melhorScore),
             20, 140, 20, GOLD);

    int centroX = largura_tela / 2;

    if (state == WAIT_INPUT) {
    DrawText("Use SETA CIMA/BAIXO para responder quantas caixas viu",
             centroX - MeasureText("Use SETA CIMA/BAIXO para responder quantas caixas viu", 18)/2,
             altura_tela - 140, 18, LIGHTGRAY);

    DrawText("Sua resposta:",
             centroX - 80,
             altura_tela - 110, 24, RAYWHITE);

    DrawText(TextFormat("%d", playerAnswer),
             centroX - 10,
             altura_tela - 80, 36, (Color){180, 120, 250, 255});

    DrawText(TextFormat("Tempo restante: %.1f", 3.0f - timerCaixas),
             centroX - 100,
             altura_tela - 40, 20, (Color){255, 230, 120, 255});
}
else if (state == SHOW_RESULT) {

    int diff = abs(playerAnswer - boxCount);

    if (playerCorrect) {

        DrawText("Você acertou! +100",
                 centroX - MeasureText("Voce acertou! +100", 28)/2,
                 altura_tela - 140, 28, COR_TEXTO_SUCESSO);
    } else if (diff == 1) {

        DrawText("Foi quase! +50",
                 centroX - MeasureText("Foi quase! +50", 28)/2,
                 altura_tela - 140, 28, ORANGE);
    } else {

        DrawText("Você errou!",
                 centroX - MeasureText("Voce errou!", 30)/2,
                 altura_tela - 140, 30, COR_TEXTO_ERRO);
    }


    DrawText(TextFormat("Caixas corretas: %d", boxCount),
             centroX - MeasureText("Caixas corretas: 00", 22)/2,
             altura_tela - 100, 22, LIGHTGRAY);

    DrawText(TextFormat("Sua resposta: %d", playerAnswer),
             centroX - MeasureText("Sua resposta: 00", 22)/2,
             altura_tela - 70, 22, LIGHTGRAY);
}

}


void AtualizarMenu(void)
{
    int opc = 4;

    if (IsKeyPressed(KEY_UP)) {
        menuOpcoes--;
        if (menuOpcoes < 0) menuOpcoes = opc - 1;
        PlaySound(sClick);
    }

    if (IsKeyPressed(KEY_DOWN)) {
        menuOpcoes++;
        if (menuOpcoes >= opc) menuOpcoes = 0;
        PlaySound(sClick);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(sClick);
        switch (menuOpcoes) {
            case 0: IniciarTransicao(TELA_GAMEPLAY);   break;
            case 1: IniciarTransicao(TELA_INSTRUCOES); break;
            case 2: IniciarTransicao(TELA_LIDERES);    break;
            case 3: CloseWindow();                     break;
        }
    }
}

void DesenharMenu(void)
{
    DesenharFundoBonito();

    // Faixa escura no topo (se não quiser, pode remover)
    DrawRectangle(0, 40, largura_tela, 70, (Color){0, 0, 0, 40});

    // -------- TÍTULO COM LETRAS PULANDO --------
    const char *titulo = "Diamonds Blocks";
    int fontSize = 40;
    int len = TextLength(titulo);
    float tempo = GetTime();

    int letterSpacing = 2;           // espaço extra entre letras (ajusta se quiser)
    int widths[64];                  // tamanho suficiente pro título
    int totalW = 0;

    // calcula largura de cada caractere e a largura total
    for (int i = 0; i < len; i++) {
        char ch[2] = { titulo[i], '\0' };
        widths[i] = MeasureText(ch, fontSize);
        totalW += widths[i];
        if (i < len - 1) totalW += letterSpacing;
    }

    int startX = (largura_tela - totalW) / 2;
    int baseY  = 60;
    int x      = startX;

    for (int i = 0; i < len; i++) {
        char ch[2] = { titulo[i], '\0' };

        float offset = 4.0f * sinf(tempo * 3.0f + i * 0.4f);

        DrawText(ch,
                 x,
                 baseY + (int)offset,
                 fontSize,
                 RAYWHITE);

        x += widths[i] + letterSpacing;
    }


    // -------- MENU DE OPÇÕES --------
    const char *opcoes[] = { "Novo Jogo", "Instruções", "Placar", "Sair" };
    int opc = 4;

    int iniciarY = 200;
    int espacoY  = 45;

    for (int i = 0; i < opc; i++) {
        Color color  = LIGHTGRAY;
        int fontSizeOp = 24;

        int y = iniciarY + i * espacoY;
        int textoLargura = MeasureText(opcoes[i], fontSizeOp);
        int x = (largura_tela - textoLargura) / 2;

        if (i == menuOpcoes) {
            DrawRectangleRounded(
                (Rectangle){ x - 20, y - 6, textoLargura + 40, fontSizeOp + 12 },
                0.4f, 8,
                (Color){40, 80, 160, 140}
            );

            color      = YELLOW;
            fontSizeOp = 28;
            textoLargura = MeasureText(opcoes[i], fontSizeOp);
            x = (largura_tela - textoLargura) / 2;
        }

        DrawText(opcoes[i], x, y, fontSizeOp, color);
    }

    DrawText("Use SETA CIMA/BAIXO e ENTER",
             (largura_tela - MeasureText("Use SETA CIMA/BAIXO e ENTER", 18))/2,
             altura_tela - 40, 18, GRAY);
}



void AtualizarInstrucoes(void)
{
    if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_GAMEPLAY);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }
}

void DesenharInstrucoes(void)
{
    DesenharFundoBonito();

    int w = largura_tela;
    int h = altura_tela;

    const char *titulo    = "Como Jogar";
    int tituloSize        = 36;
    int larguraTitulo     = MeasureText(titulo, tituloSize);
    int tituloX           = (w - larguraTitulo) / 2;
    int tituloY           = 40;

    DrawText(titulo, tituloX, tituloY, tituloSize, RAYWHITE);

    int x = 80;
    int y = 110;
    int fontSize   = 20;
    Color corTexto = LIGHTGRAY;

    DrawText("Algumas caixas aparecem na tela por alguns segundos:", x, y, fontSize, corTexto); y += 26;
    DrawText("Elas somem e você deve lembrar QUANTAS eram.",         x, y, fontSize, corTexto); y += 26;
    DrawText("Use as teclas (no jogo real) para responder o número.", x, y, fontSize, corTexto); y += 26;
    DrawText("Acertos rendem pontos, erros tiram vidas.",             x, y, fontSize, corTexto); y += 36;

    DrawText("Teclas principais:", x, y, fontSize+2, RAYWHITE); y += 26;
    DrawText("    SETA CIMA      -> Aumentar seu palpite",  x+30, y, fontSize, corTexto); y += 22;
    DrawText("    SETA BAIXO  -> Diminuir seu palpite", x+30, y, fontSize, corTexto); y += 22;

    const char *enterTxt   = "ENTER para comecar";
    int enterSize          = 22;
    int enterLargura       = MeasureText(enterTxt, enterSize);
    int enterX             = (w - enterLargura) / 2;
    int enterY             = h - 80;
    DrawText(enterTxt, enterX, enterY, enterSize, YELLOW);

    const char *escTxt     = "ESC para voltar ao menu";
    int escSize            = 18;
    int escLargura         = MeasureText(escTxt, escSize);
    int escX               = (w - escLargura) / 2;
    int escY               = h - 45;
    DrawText(escTxt, escX, escY, escSize, GRAY);
}


void AtualizarPlacar(void)
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }
}

void DesenharPlacar(void)
{
    DesenharFundoBonito();

    const char *titulo = "Placar";
    int size           = 34;
    int largura        = MeasureText(titulo, size);
    DrawText(titulo, (largura_tela - largura)/2, 40, size, RAYWHITE);

    DrawText("Melhor pontuação até agora:", 150, 150, 22, LIGHTGRAY);
    DrawText(TextFormat("%d pontos", jogo.melhorScore), 150, 190, 26, GOLD);

    DrawText("No futuro tera o TOP 10 jogadores", 150, 240, 18, GRAY);

    DrawText("ESC ou ENTER para voltar", 260, altura_tela - 50, 18, GRAY);
}


void AtualizarGameOver(void)
{
    if (IsKeyPressed(KEY_ENTER)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_GAMEPLAY);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }
}

void DesenharGameOver(void)
{
    DesenharFundoBonito();

    const char *titulo = "FIM DE JOGO";
    int size           = 40;
    int largura        = MeasureText(titulo, size);
    DrawText(titulo, (largura_tela - largura)/2, 80, size, RED);

    DrawText(TextFormat("Pontuação final: %d", jogo.score),
             260, 170, 26, RAYWHITE);
    DrawText(TextFormat("Recorde Atual: %d", jogo.melhorScore),
             260, 210, 22, LIGHTGRAY);

    DrawText("ENTER - Jogar Novamente", 260, 280, 20, GREEN);
    DrawText("ESC   - Voltar ao menu",  260, 310, 20, GRAY);
}


void DesenharNovoRecorde(void)
{


    const char *titulo = "NOVO RECORDE!";
    int size           = 42;
    int largura        = MeasureText(titulo, size);
    DrawText(titulo, (largura_tela - largura)/2, 80, size, GOLD);

    DrawText(TextFormat("Sua pontuação: %d", jogo.score),
             260, 170, 26, RAYWHITE);
    DrawText(TextFormat("Novo recorde do jogo: %d", jogo.melhorScore),
             260, 210, 22, LIGHTGRAY);

    DrawText("Aperte ENTER ou ESC para voltar ao menu.",
             220, 280, 20, LIGHTGRAY);

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }
}


int main(void)
{
    InitWindow(largura_tela, altura_tela, "Diamonds Blocks - Interface");
    InitAudioDevice();

    SetMasterVolume(1.0f);
    
    sClick       = LoadSound("assets/click.wav");
    sAcertou     = LoadSound("assets/correct.wav");
    sErrou       = LoadSound("assets/error.wav");
    sNovoRecorde = LoadSound("assets/new_record.wav");
    music        = LoadMusicStream("assets/music.ogg");

    SetMasterVolume(masterVolume);
    SetMusicVolume(music, musicVolume);
    SetSoundVolume(sClick,       sfxVolume);
    SetSoundVolume(sAcertou,     sfxVolume);
    SetSoundVolume(sErrou,       sfxVolume);
    SetSoundVolume(sNovoRecorde, sfxVolume);

    PlayMusicStream(music);

    jogo.melhorScore = 0;
    ResetGame();
    SetTargetFPS(60);

    while (!WindowShouldClose())
    {
        UpdateMusicStream(music);

        if (emTransicao) {
            AtualizarTransicao();
        } else {
            switch (tela_atual) {
                case TELA_MENU:        AtualizarMenu();       break;
                case TELA_INSTRUCOES:  AtualizarInstrucoes(); break;
                case TELA_GAMEPLAY:    AtualizarGame();       break;
                case TELA_LIDERES:     AtualizarPlacar();     break;
                case TELA_GAMEOVER:    AtualizarGameOver();   break;
                case TELA_NOVORECORDE: /* só desenha */       break;
            }
        }

        BeginDrawing();
        ClearBackground(COR_FUNDO);

        switch (tela_atual) {
            case TELA_MENU:        DesenharMenu();        break;
            case TELA_INSTRUCOES:  DesenharInstrucoes();  break;
            case TELA_GAMEPLAY:    DesenharGameplay();    break;
            case TELA_LIDERES:     DesenharPlacar();      break;
            case TELA_GAMEOVER:    DesenharGameOver();    break;
            case TELA_NOVORECORDE: DesenharNovoRecorde(); break;
        }

        if (emTransicao) DesenharTransicao();

        EndDrawing();
    }

    UnloadSound(sClick);
    UnloadSound(sAcertou);
    UnloadSound(sErrou);
    UnloadSound(sNovoRecorde);
    UnloadMusicStream(music);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}
