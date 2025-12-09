#include "raylib.h"
#include <math.h>


#define largura_tela 900
#define altura_tela  500


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

typedef struct{
    Rectangle rect;
    bool ativa;
}Caixa;

#define NUM_CAIXAS 9
Caixa caixas[NUM_CAIXAS];


bool  emTransicao     = false;
float transicaoAlpha  = 0.0f;
TelasJogo proximaTela;


int menuOpcoes = 0;

void InicializarCaixas(void) {

    Rectangle playArea = (Rectangle){120, 90, largura_tela - 240, altura_tela - 180};

    int linhas  = 3;
    int colunas = 3;

    float cellW = playArea.width  / colunas;
    float cellH = playArea.height / linhas;


    float baseSize = (cellW < cellH) ? cellW : cellH;
    float caixaW   = baseSize * 0.70f;   
    float caixaH   = baseSize * 0.60f;   

    int idx = 0;
    for (int linha = 0; linha < linhas; linha++) {
        for (int col = 0; col < colunas; col++) {
            if (idx >= NUM_CAIXAS) break;


            float centerX = playArea.x + col * cellW + cellW/2.0f;
            float centerY = playArea.y + linha * cellH + cellH/2.0f;

            float x = centerX - caixaW/2.0f;
            float y = centerY - caixaH/2.0f;

            caixas[idx].rect  = (Rectangle){ x, y, caixaW, caixaH };
            caixas[idx].ativa = false;
            idx++;
        }
    }


    if (NUM_CAIXAS >= 3) {
        caixas[0].ativa = true;
        caixas[4].ativa = true;
        caixas[8].ativa = true;
    }
}



void ResetGame(void)
{
    jogo.score       = 0;
    jogo.vidas       = 3;
    jogo.fase        = 1;
    jogo.novoRecorde = false;

    InicializarCaixas();
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
    if (IsKeyPressed(KEY_SPACE)) {
        jogo.score += 100;
        PlaySound(sAcertou);
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        jogo.vidas--;
        PlaySound(sErrou);
    }

  
    if (IsKeyPressed(KEY_ENTER)) {
        if (jogo.score > jogo.melhorScore) {
            jogo.melhorScore = jogo.score;
            jogo.novoRecorde = true;
            PlaySound(sNovoRecorde);
            IniciarTransicao(TELA_NOVORECORDE);
        } else {
            jogo.novoRecorde = false;
            IniciarTransicao(TELA_GAMEOVER);
        }
    }

  
    if (IsKeyPressed(KEY_ESCAPE)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }

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
    }
}

void DesenharCaixa3D(Caixa c)
{
    float t = GetTime();


    float bounce = 0.0f;
    if (c.ativa) {
        bounce = sinf(t * 6.0f) * 3.0f; 
    }

    Rectangle face = c.rect;
    face.y += bounce;

    float offset = 12.0f;  


    Color corFrenteOff = (Color){ 40, 70,130, 255 };
    Color corTopoOff   = (Color){ 60, 95,160, 255 };
    Color corLadoOff   = (Color){ 25, 45,100, 255 };

    Color corFrenteOn  = (Color){  60,190,255, 255 };
    Color corTopoOn    = (Color){ 150,230,255, 255 };
    Color corLadoOn    = (Color){  30,120,210, 255 };

    Color corFrente = c.ativa ? corFrenteOn : corFrenteOff;
    Color corTopo   = c.ativa ? corTopoOn   : corTopoOff;
    Color corLado   = c.ativa ? corLadoOn   : corLadoOff;


    float pulso = 0.08f * sinf(t * 3.0f);
    corFrente = Fade(corFrente, 1.0f + pulso);


    Rectangle frente = face;

    Vector2 p1 = (Vector2){ frente.x,                  frente.y };
    Vector2 p2 = (Vector2){ frente.x + frente.width,   frente.y };
    Vector2 p3 = (Vector2){ p2.x + offset,             p2.y - offset };
    Vector2 p4 = (Vector2){ p1.x + offset,             p1.y - offset };

    Vector2 p5 = (Vector2){ frente.x,                  frente.y + frente.height };
    Vector2 p6 = (Vector2){ frente.x + frente.width,   frente.y + frente.height };
    Vector2 p7 = (Vector2){ p6.x + offset,             p6.y - offset };



    DrawEllipse((int)(frente.x + frente.width/2),
                (int)(frente.y + frente.height + 8),
                (int)(frente.width/2),
                6,
                (Color){0, 0, 0, 80});


    DrawRectangleRec(frente, corFrente);


    DrawTriangle(p1, p2, p4, corTopo);
    DrawTriangle(p2, p3, p4, corTopo);


    DrawTriangle(p2, p6, p3, corLado);
    DrawTriangle(p6, p7, p3, corLado);


    Color linha = (Color){ 210, 230, 255, 220 };
    DrawLineEx(p1, p2, 1.5f, linha);
    DrawLineEx(p2, p3, 1.5f, linha);
    DrawLineEx(p3, p4, 1.5f, linha);
    DrawLineEx(p4, p1, 1.5f, linha);

    DrawLineEx(p2, p6, 1.5f, linha);
    DrawLineEx(p6, p7, 1.5f, linha);
    DrawLineEx(p7, p3, 1.5f, linha);

    DrawLineEx(p1, p5, 1.5f, Fade(linha, 0.6f));
    DrawLineEx(p5, p6, 1.5f, Fade(linha, 0.6f));
}


void DesenharFundoBonito(void)
{

    Color top    = (Color){  5,  10,  35, 255 };   
    Color bottom = (Color){  3,   4,  18, 255 }; 

    DrawRectangleGradientV(0, 0, largura_tela, altura_tela, top, bottom);


    Color vignette = (Color){ 0, 0, 0, 70 };
    DrawRectangle(0, 0, largura_tela, 40, vignette);                        // topo
    DrawRectangle(0, altura_tela-60, largura_tela, 60, vignette);           // baixo
    DrawRectangle(0, 0, 80, altura_tela, vignette);                         // esquerda
    DrawRectangle(largura_tela-80, 0, 80, altura_tela, vignette);          // direita
}


void DesenharGameplay(void)
{
    DesenharFundoBonito();


    DrawText("Rodada de Memoria", 40, 20, 28, RAYWHITE);

    DrawText(TextFormat("Pontos: %d",  jogo.score),       40,  70, 20, LIGHTGRAY);
    DrawText(TextFormat("Vidas: %d",   jogo.vidas),       40, 100, 20, LIGHTGRAY);
    DrawText(TextFormat("Rodada: %d",  jogo.fase),        40, 130, 20, LIGHTGRAY);
    DrawText(TextFormat("Recorde: %d", jogo.melhorScore), largura_tela - 220, 70, 20, GOLD);

    DrawText("ESC = Menu", largura_tela - 180, altura_tela - 40, 18, GRAY);

    DrawText("SPACE: +100 pontos",          150, altura_tela - 90, 18, SKYBLUE);
    DrawText("BACKSPACE: perde 1 vida",     150, altura_tela - 60, 18, ORANGE);
    DrawText("ENTER: finalizar rodada",     150, altura_tela - 30, 18, GREEN);

    for (int i = 0; i < NUM_CAIXAS; i++) {
        DesenharCaixa3D(caixas[i]);
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

    DrawRectangle(0, 40, largura_tela, 70, (Color){0, 0, 0, 40});

    const char *titulo      = "JOGO DE MEMÓRIA";
    int tamFonteTitulo      = 40;
    int tituloLargura       = MeasureText(titulo, tamFonteTitulo);
    int tituloX             = (largura_tela - tituloLargura) / 2;

    float tempo   = GetTime();
    float escala  = 1.0f + 0.03f * sinf(tempo * 2.5f);
    int tamanho   = (int)(tamFonteTitulo * escala);

    DrawText(titulo, tituloX, 50, tamanho, RAYWHITE);
    int lineY = 50 + tamanho + 8;
    DrawRectangle(tituloX, lineY, tituloLargura, 2, (Color){255, 230, 80, 200});


    const char *opcoes[] = { "Novo Jogo", "Instruções", "Placar", "Sair" };
    int opc = 4;

    int iniciarY = 200;
    int espacoY  = 45;

    for (int i = 0; i < opc; i++) {
        Color color  = LIGHTGRAY;
        int fontSize = 24;

        int y = iniciarY + i * espacoY;
        int textoLargura = MeasureText(opcoes[i], fontSize);
        int x = (largura_tela - textoLargura) / 2;

        if (i == menuOpcoes) {
            DrawRectangleRounded((Rectangle){x - 20, y - 6, textoLargura + 40, fontSize + 12}, 0.4f, 8, (Color){40, 80, 160, 140});

            color    = YELLOW;
            fontSize = 28;
        }

        DrawText(opcoes[i], x, y, fontSize, color);

        //int textoLargura = MeasureText(opcoes[i], fontSize);
        //int x = (largura_tela - textoLargura) / 2;
        //int y = iniciarY + i * espacoY;

        //DrawText(opcoes[i], x, y, fontSize, color);

        //if (i == menuOpcoes) {
          //  DrawRectangleLines(x - 10, y - 4,
           //                    textoLargura + 20, fontSize + 8,
           //                    Fade(color, 0.6f));
       // }
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
    DrawText("  No prototipo atual:", x+10, y, fontSize, corTexto); y += 24;
    DrawText("    SPACE      -> +100 pontos",  x+30, y, fontSize, corTexto); y += 22;
    DrawText("    BACKSPACE  -> perde 1 vida", x+30, y, fontSize, corTexto); y += 22;
    DrawText("    ENTER      -> finaliza rodada", x+30, y, fontSize, corTexto); y += 40;

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
    ClearBackground((Color){4, 30, 40, 255});

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
    InitWindow(largura_tela, altura_tela, "Jogo de Memória - Interface");
    InitAudioDevice();

    
    sClick       = LoadSound("assets/click.wav");
    sAcertou     = LoadSound("assets/correct.wav");
    sErrou       = LoadSound("assets/error.wav");
    sNovoRecorde = LoadSound("assets/new_record.wav");
    music        = LoadMusicStream("assets/music.ogg");

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
        ClearBackground((Color){10, 18, 40, 255});

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
