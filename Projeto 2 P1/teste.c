#include "diamonds_Blocks_Visual.h"
#include <math.h>

#define largura_tela LARGURA_TELA
#define altura_tela  ALTURA_TELA

RegistroPlacar gPlacar[MAX_JOGADORES];
int gQuantidadePlacar = 0;


char gNomeJogador[TAM_NOME] = "";
int  gTamNomeJogador       = 0;
bool gNomeConfirmado       = false;


TelasJogo tela_atual = TELA_MENU;
DadosJogo jogo;


Sound sClick;
Sound sAcertou;
Sound sErrou;
Sound sNovoRecorde;
Music music;


#define NUM_CAIXAS 9
Caixa caixas[NUM_CAIXAS];

bool  emTransicao    = false;
float transicaoAlpha = 0.0f;
TelasJogo proximaTela;

// Menu
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

    // Apenas um padrão fictício para ficar bonitinho
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
            InitGame();
        }
    }
}

void FimPartida(void)
{
    int entrou = atualizar_placar(gPlacar, &gQuantidadePlacar, gNomeJogador, jogo.score);

    if (entrou) {
        salvar_placar(ARQUIVO_PLACAR, gPlacar, gQuantidadePlacar);

        if (gQuantidadePlacar > 0) {
            jogo.melhorScore = gPlacar[0].pontuacao;
        }

        jogo.novoRecorde = true;
        PlaySound(sNovoRecorde);
        IniciarTransicao(TELA_NOVORECORDE);
    } else {
        jogo.novoRecorde = false;
        IniciarTransicao(TELA_GAMEOVER);
    }
}


void AtualizarNomeJogador(void)
{
    int key = GetCharPressed();

    while (key > 0) {
        if (key >= 32 && key <= 125 && gTamNomeJogador < TAM_NOME - 1) {
            gNomeJogador[gTamNomeJogador] = (char)key;
            gTamNomeJogador++;
            gNomeJogador[gTamNomeJogador] = '\0';
        }
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_BACKSPACE) && gTamNomeJogador > 0) {
        gTamNomeJogador--;
        gNomeJogador[gTamNomeJogador] = '\0';
    }

    if (IsKeyPressed(KEY_ENTER) && gTamNomeJogador > 0) {
        gNomeConfirmado = true;
        IniciarTransicao(TELA_MENU);
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        CloseWindow();
    }
}

void DesenharFundoBonito(void);

void DesenharNomeJogador(void)
{
    DesenharFundoBonito();

    const char* titulo = "Digite seu nome";
    int tamanhoTitulo  = 36;
    int largura        = MeasureText(titulo, tamanhoTitulo);
    DrawText(titulo, (largura_tela - largura) / 2, 80, tamanhoTitulo, RAYWHITE);

    DrawText("Use o teclado para digitar.", 220, 150, 20, LIGHTGRAY);
    DrawText("ENTER para confirmar, BACKSPACE para apagar.", 140, 180, 20, LIGHTGRAY);

    int caixaLargura = 400;
    int caixaAltura  = 50;
    int cx = (largura_tela - caixaLargura) / 2;
    int cy = 250;

    DrawRectangleLines(cx, cy, caixaLargura, caixaAltura, RAYWHITE);

    DrawText(gNomeJogador, cx + 10, cy + 12, 24, YELLOW);

    if (((int)(GetTime() * 2)) % 2 == 0 && gTamNomeJogador < TAM_NOME - 1) {
        int textoLarg = MeasureText(gNomeJogador, 24);
        DrawText("_", cx + 10 + textoLarg, cy + 12, 24, YELLOW);
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

    UpdateGame(dt);

    jogo.score = Game_GetScore();
    jogo.vidas = Game_GetLives();
    jogo.fase  = Game_GetRound();

    if (IsKeyPressed(KEY_ESCAPE)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }

    if (IsKeyPressed(KEY_ENTER)) {
        FimPartida();
    }

    if (jogo.vidas <= 0) {
        FimPartida();
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
    DrawRectangle(0, 0, largura_tela, 40, vignette);
    DrawRectangle(0, altura_tela-60, largura_tela, 60, vignette);
    DrawRectangle(0, 0, 80, altura_tela, vignette);
    DrawRectangle(largura_tela-80, 0, 80, altura_tela, vignette);
}

void DesenharGameplay_Interface2D(void)
{
    DesenharFundoBonito();

    DrawText("Rodada de Memoria", 40, 20, 28, RAYWHITE);

    DrawText(TextFormat("Pontos: %d",  jogo.score),       40,  70, 20, LIGHTGRAY);
    DrawText(TextFormat("Vidas: %d",   jogo.vidas),       40, 100, 20, LIGHTGRAY);
    DrawText(TextFormat("Rodada: %d",  jogo.fase),        40, 130, 20, LIGHTGRAY);
    DrawText(TextFormat("Recorde: %d", jogo.melhorScore), largura_tela - 220, 70, 20, GOLD);

    DrawText("ESC = Menu", largura_tela - 180, altura_tela - 40, 18, GRAY);

    DrawText("No jogo final, a contagem sera feita pela logica 3D.", 150, altura_tela - 60, 18, SKYBLUE);

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

    const char *titulo      = "JOGO DE MEMORIA";
    int tamFonteTitulo      = 40;
    int tituloLargura       = MeasureText(titulo, tamFonteTitulo);
    int tituloX             = (largura_tela - tituloLargura) / 2;

    float tempo   = GetTime();
    float escala  = 1.0f + 0.03f * sinf(tempo * 2.5f);
    int tamanho   = (int)(tamFonteTitulo * escala);

    DrawText(titulo, tituloX, 50, tamanho, RAYWHITE);
    int lineY = 50 + tamanho + 8;
    DrawRectangle(tituloX, lineY, tituloLargura, 2, (Color){255, 230, 80, 200});

    const char *opcoes[] = { "Novo Jogo", "Instrucoes", "Placar", "Sair" };
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
            DrawRectangleRounded(
                (Rectangle){x - 20, y - 6, textoLargura + 40, fontSize + 12},
                0.4f, 8, (Color){40, 80, 160, 140}
            );

            color    = YELLOW;
            fontSize = 28;
        }

        DrawText(opcoes[i], x, y, fontSize, color);
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
    DrawText("Elas somem e voce deve lembrar QUANTAS eram.",         x, y, fontSize, corTexto); y += 26;
    DrawText("Use as teclas (no jogo real) para responder o numero.", x, y, fontSize, corTexto); y += 26;
    DrawText("Acertos rendem pontos, erros tiram vidas.",             x, y, fontSize, corTexto); y += 36;

    DrawText("No prototipo atual, a logica 3D da Pessoa 1 faz a contagem.", x, y, fontSize, corTexto); y += 26;

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

    const char *titulo = "Placar - Top 10";
    int size           = 34;
    int largura        = MeasureText(titulo, size);
    DrawText(titulo, (largura_tela - largura)/2, 40, size, RAYWHITE);

    if (gQuantidadePlacar == 0) {
        DrawText("Nenhuma partida registrada ainda.", 150, 150, 22, LIGHTGRAY);
    } else {
        DrawText("Pos  Nome                 Pontos", 150, 120, 20, GRAY);

        for (int i = 0; i < gQuantidadePlacar; i++) {
            DrawText(
                TextFormat("%2d. %-15s %6d",
                           i + 1,
                           gPlacar[i].nome,
                           gPlacar[i].pontuacao),
                150, 150 + (i * 26), 22, RAYWHITE
            );
        }

        DrawText(TextFormat("Melhor pontuacao: %d pontos", gPlacar[0].pontuacao),
                 150, 150 + gQuantidadePlacar * 26 + 20, 22, GOLD);
    }

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

    DrawText(TextFormat("Pontuacao final: %d", jogo.score),
             260, 170, 26, RAYWHITE);
    DrawText(TextFormat("Recorde Atual: %d", jogo.melhorScore),
             260, 210, 22, LIGHTGRAY);

    DrawText("ENTER - Jogar novamente", 260, 280, 20, GREEN);
    DrawText("ESC   - Voltar ao menu",  260, 310, 20, GRAY);
}



void DesenharNovoRecorde(void)
{
    ClearBackground((Color){4, 30, 40, 255});

    const char *titulo = "NOVO RECORDE!";
    int size           = 42;
    int largura        = MeasureText(titulo, size);
    DrawText(titulo, (largura_tela - largura)/2, 80, size, GOLD);

    DrawText(TextFormat("Sua pontuacao: %d", jogo.score),
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



void Visual_Init(void)
{

    InitAudioDevice();

    sClick       = LoadSound("assets/click.wav");
    sAcertou     = LoadSound("assets/correct.wav");
    sErrou       = LoadSound("assets/error.wav");
    sNovoRecorde = LoadSound("assets/new_record.wav");
    music        = LoadMusicStream("assets/music.ogg");

    PlayMusicStream(music);


    gQuantidadePlacar = carregar_placar(ARQUIVO_PLACAR, gPlacar, MAX_JOGADORES);

    if (gQuantidadePlacar > 0) {
        jogo.melhorScore = gPlacar[0].pontuacao;
    } else {
        jogo.melhorScore = 0;
    }


    tela_atual = TELA_NOME_JOGADOR;
    ResetGame();
}

void Visual_Update(void)
{
    UpdateMusicStream(music);

    if (emTransicao) {
        AtualizarTransicao();
        return;
    }

    switch (tela_atual) {
        case TELA_MENU:
            AtualizarMenu();
            break;
        case TELA_INSTRUCOES:
            AtualizarInstrucoes();
            break;
        case TELA_GAMEPLAY:
            AtualizarGame();
            break;
        case TELA_LIDERES:
            AtualizarPlacar();
            break;
        case TELA_GAMEOVER:
            AtualizarGameOver();
            break;
        case TELA_NOVORECORDE:

            break;
        case TELA_NOME_JOGADOR:
            AtualizarNomeJogador();
            break;
    }
}

void Visual_Draw(void)
{

    if (tela_atual == TELA_GAMEPLAY) {

        DrawGame();
        return;
    }

    BeginDrawing();
    ClearBackground((Color){10, 18, 40, 255});

    switch (tela_atual) {
        case TELA_MENU:
            DesenharMenu();
            break;
        case TELA_INSTRUCOES:
            DesenharInstrucoes();
            break;
        case TELA_LIDERES:
            DesenharPlacar();
            break;
        case TELA_GAMEOVER:
            DesenharGameOver();
            break;
        case TELA_NOVORECORDE:
            DesenharNovoRecorde();
            break;
        case TELA_NOME_JOGADOR:
            DesenharNomeJogador();
            break;
        default:
            break;
    }

    if (emTransicao) DesenharTransicao();

    EndDrawing();
}

void Visual_Unload(void)
{
    UnloadSound(sClick);
    UnloadSound(sAcertou);
    UnloadSound(sErrou);
    UnloadSound(sNovoRecorde);
    UnloadMusicStream(music);
    CloseAudioDevice();
}
