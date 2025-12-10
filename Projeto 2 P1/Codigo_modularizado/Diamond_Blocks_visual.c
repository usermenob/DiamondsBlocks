#include <math.h>
#include "Diamond_Blocks_visual.h"


static const Color COR_FUNDO = {  5,  10,  30, 255 };


DadosJogo jogo;

RegistroPlacar placar[MAX_JOGADORES];
int qtdPlacar = 0;

char gNomeJogador[TAM_NOME] = "";
int  gTamNomeJogador        = 0;
bool gNomeConfirmado        = false;

Sound sClick;
Sound sAcertou;
Sound sErrou;
Sound sNovoRecorde;
Music music;

float masterVolume = 1.0f;
float musicVolume  = 0.5f;
float sfxVolume    = 0.9f;

TelasJogo tela_atual   = TELA_NOME_JOGADOR;
bool      emTransicao  = false;
float     transicaoAlpha = 0.0f;
TelasJogo proximaTela;

int menuOpcoes = 0;



void IniciarTransicao(TelasJogo tela)
{
    emTransicao    = true;
    proximaTela    = tela;
    transicaoAlpha = 0.0f;
}

static void AtualizarTransicao(void)
{
    transicaoAlpha += 0.08f;

    if (transicaoAlpha >= 1.0f) {
        transicaoAlpha = 0.0f;
        emTransicao    = false;
        tela_atual     = proximaTela;

        if (tela_atual == TELA_GAMEPLAY) {
            Game_Reset(&jogo);
        }
    }
}

static void DesenharTransicao(void)
{
    DrawRectangle(0, 0, largura_tela, altura_tela,
                  (Color){0, 0, 0, (unsigned char)(transicaoAlpha * 255)});
}

static void DesenharFundoBonito(void)
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


static void AtualizarNomeJogador(void)
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

static void DesenharNomeJogador(void)
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


static void AtualizarMenu(void)
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

static void DesenharMenu(void)
{
    DesenharFundoBonito();

    DrawRectangle(0, 40, largura_tela, 70, (Color){0, 0, 0, 40});

    const char *titulo = "Diamonds Blocks";
    int fontSize = 40;
    int len = TextLength(titulo);
    float tempo = GetTime();

    int letterSpacing = 2;
    int widths[64];
    int totalW = 0;

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

        DrawText(ch, x, baseY + (int)offset, fontSize, RAYWHITE);
        x += widths[i] + letterSpacing;
    }

    const char *opcoes[] = { "Novo Jogo", "Instrucoes", "Placar", "Sair" };
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


static void AtualizarInstrucoes(void)
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

static void DesenharInstrucoes(void)
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
    DrawText("Use as teclas para responder o numero.",                x, y, fontSize, corTexto); y += 26;
    DrawText("Acertos rendem pontos, erros tiram vidas.",             x, y, fontSize, corTexto); y += 36;

    DrawText("Teclas principais:", x, y, fontSize+2, RAYWHITE); y += 26;
    DrawText("    SETA CIMA   -> Aumentar seu palpite",  x+30, y, fontSize, corTexto); y += 22;
    DrawText("    SETA BAIXO  -> Diminuir seu palpite",  x+30, y, fontSize, corTexto); y += 22;

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


static void AtualizarPlacar(void)
{
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        PlaySound(sClick);
        IniciarTransicao(TELA_MENU);
    }
}

void DesenharPlacar(void)
{
    DesenharFundoBonito();

    const char *titulo = "PLACAR - TOP 10";
    int sizeTitulo     = 42;
    int larguraTitulo  = MeasureText(titulo, sizeTitulo);
    DrawText(titulo,
             (largura_tela - larguraTitulo)/2,
             40,
             sizeTitulo,
             GOLD);

    int caixaX = 80;
    int caixaY = 110;
    int caixaW = largura_tela - 160;
    int caixaH = altura_tela - 210;

    DrawRectangleRounded(
        (Rectangle){ caixaX, caixaY, caixaW, caixaH },
        0.2f, 12,
        (Color){0, 0, 0, 110}
    );

    if (qtdPlacar == 0) {
        DrawText("Nenhuma partida registrada ainda.",
                 caixaX + 40, caixaY + 40, 24, LIGHTGRAY);

    } else {

        int colunaL_X = caixaX + 40;
        int colunaR_X = caixaX + caixaW/2 + 20;
        int headerY   = caixaY + 30;
        int rowStartY = headerY + 35;
        int rowGap    = 28;

        DrawText("Pos",   colunaL_X,              headerY, 24, YELLOW);
        DrawText("Nome",  colunaL_X + 60,         headerY, 24, YELLOW);
        DrawText("Pontos",colunaL_X + 260,        headerY, 24, YELLOW);

        DrawText("Pos",   colunaR_X,              headerY, 24, YELLOW);
        DrawText("Nome",  colunaR_X + 60,         headerY, 24, YELLOW);
        DrawText("Pontos",colunaR_X + 260,        headerY, 24, YELLOW);

        for (int i = 0; i < qtdPlacar && i < 10; i++) {

            int col    = (i < 5) ? 0 : 1;
            int row    = (i < 5) ? i : (i - 5);

            int baseX  = (col == 0) ? colunaL_X : colunaR_X;
            int y      = rowStartY + row * rowGap;

            Color corLinha = (i == 0 ? GOLD : RAYWHITE);
            int   fontRow  = (i == 0 ? 26  : 22);

            DrawText(TextFormat("%2d", i+1),
                     baseX,
                     y,
                     fontRow,
                     corLinha);

            DrawText(placar[i].nome,
                     baseX + 60,
                     y,
                     fontRow,
                     corLinha);

            DrawText(TextFormat("%5d", placar[i].pontuacao),
                     baseX + 260,
                     y,
                     fontRow,
                     corLinha);
        }
    }

    const char *rodape = "ENTER ou ESC para voltar";
    int tamRodape      = 20;
    int largRodape     = MeasureText(rodape, tamRodape);

    DrawText(rodape,
             (largura_tela - largRodape)/2,
             altura_tela - 40,
             tamRodape,
             LIGHTGRAY);
}



static void AtualizarGameOver(void)
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

static void DesenharGameOver(void)
{
    DesenharFundoBonito();

    const char *titulo = "FIM DE JOGO";
    int size           = 40;
    int largura        = MeasureText(titulo, size);
    DrawText(titulo, (largura_tela - largura)/2, 80, size, RED);

    DrawText(TextFormat("Pontos finais: %d", jogo.score),
             260, 170, 26, RAYWHITE);
    DrawText(TextFormat("Recorde Atual: %d", jogo.melhorScore),
             260, 210, 22, LIGHTGRAY);

    DrawText("ENTER - Jogar Novamente", 260, 280, 20, GREEN);
    DrawText("ESC   - Voltar ao menu",  260, 310, 20, GRAY);
}


static void DesenharNovoRecorde(void)
{
    DesenharFundoBonito();

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
}



void Visual_Init(void)
{
    InitWindow(LARGURA_TELA, ALTURA_TELA, "Diamonds Blocks - Interface");
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


    qtdPlacar = carregar_placar("placares_diamonds.txt",
                                placar,
                                MAX_JOGADORES);

    jogo.melhorScore = (qtdPlacar > 0) ? placar[0].pontuacao : 0;


    Game_Init(&jogo);

    tela_atual = TELA_NOME_JOGADOR;
    gNomeJogador[0] = '\0';
    gTamNomeJogador = 0;

    SetTargetFPS(60);
}

void Visual_Update(void)
{
    UpdateMusicStream(music);

    if (emTransicao) {
        AtualizarTransicao();
        return;
    }

    switch (tela_atual) {
        case TELA_NOME_JOGADOR:
            AtualizarNomeJogador();
            break;
        case TELA_MENU:
            AtualizarMenu();
            break;
        case TELA_INSTRUCOES:
            AtualizarInstrucoes();
            break;
        case TELA_GAMEPLAY:
            Game_Update(&jogo);
            break;
        case TELA_LIDERES:
            AtualizarPlacar();
            break;
        case TELA_GAMEOVER:
            AtualizarGameOver();
            break;
        case TELA_NOVORECORDE:
            AtualizarGameOver();
            break;
    }
}

void Visual_Draw(void)
{
    BeginDrawing();
    ClearBackground(COR_FUNDO);

    switch (tela_atual) {
        case TELA_NOME_JOGADOR:
            DesenharNomeJogador();
            break;
        case TELA_MENU:
            DesenharMenu();
            break;
        case TELA_INSTRUCOES:
            DesenharInstrucoes();
            break;
        case TELA_GAMEPLAY:
            Game_Draw(&jogo);
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
    CloseWindow();
}
