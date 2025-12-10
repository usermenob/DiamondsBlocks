#include "placar.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int carregar_placar(const char *placares_diamonds, RegistroPlacar placar[], int maxRegistros) {
    FILE *arquivo = fopen(placares_diamonds, "r");
    if (arquivo == NULL) {
        return 0;
    }

    char linha[128];
    int count = 0;

    while (fgets(linha, sizeof(linha), arquivo) != NULL && count < maxRegistros) {

        size_t len = strlen(linha);
        if (len > 0 && (linha[len - 1] == '\n' || linha[len - 1] == '\r')) {
            linha[len - 1] = '\0';
        }
        if (linha[0] == '\0') {
            continue;
        }
        char *separador = strchr(linha, ';');
        if (separador == NULL) {
            continue;
        }
        *separador = '\0';
        char *parteNome = linha;
        char *partePontuacao = separador + 1;

        strncpy(placar[count].nome, parteNome, TAM_NOME - 1);
        placar[count].nome[TAM_NOME - 1] = '\0';

        placar[count].pontuacao = atoi(partePontuacao);

        count++;
    }

    fclose(arquivo);

    ordenar_placar(placar, count);

    return count;
}

int salvar_placar(const char *placares_diamonds, RegistroPlacar placar[], int quantidade) {
    FILE *arquivo = fopen(placares_diamonds, "w");
    if (arquivo == NULL) {
        return 0;
    }

    for (int i = 0; i < quantidade; i++) {
        fprintf(arquivo, "%s;%d\n", placar[i].nome, placar[i].pontuacao);
    }

    fclose(arquivo);
    return 1;
}

void ordenar_placar(RegistroPlacar placar[], int quantidade) {

    int trocou;
    do {
        trocou = 0;
        for (int i = 0; i < quantidade - 1; i++) {
            if (placar[i].pontuacao < placar[i + 1].pontuacao) {
                RegistroPlacar temp = placar[i];
                placar[i] = placar[i + 1];
                placar[i + 1] = temp;
                trocou = 1;
            }
        }
    } while (trocou);
}

int atualizar_placar(RegistroPlacar placar[], int *quantidade,const char *nomeJogador, int novaPontuacao) {

    int entrouNoRanking = 0;

    if (*quantidade < MAX_JOGADORES) {

        strncpy(placar[*quantidade].nome, nomeJogador, TAM_NOME - 1);
        placar[*quantidade].nome[TAM_NOME - 1] = '\0';
        placar[*quantidade].pontuacao = novaPontuacao;

        (*quantidade)++;
        entrouNoRanking = 1;

    } else {
        int indiceUltimo = *quantidade - 1;

        if (novaPontuacao >= placar[indiceUltimo].pontuacao) {

            strncpy(placar[indiceUltimo].nome, nomeJogador, TAM_NOME - 1);
            placar[indiceUltimo].nome[TAM_NOME - 1] = '\0';
            placar[indiceUltimo].pontuacao = novaPontuacao;

            entrouNoRanking = 1;
        }
    }

    if (entrouNoRanking) {
        ordenar_placar(placar, *quantidade);
    }

    return entrouNoRanking;
}
