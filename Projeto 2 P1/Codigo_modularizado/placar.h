#ifndef PLACAR_H
#define PLACAR_H

#define MAX_JOGADORES 10
#define TAM_NOME 32

typedef struct {
    char nome[TAM_NOME];
    int pontuacao;
} RegistroPlacar;

int carregar_placar(const char *placares_diamonds,RegistroPlacar placar[], int maxRegistros);

int salvar_placar(const char *placares_diamonds,RegistroPlacar placar[], int quantidade);

void ordenar_placar(RegistroPlacar placar[], int quantidade);

int atualizar_placar(RegistroPlacar placar[], int *quantidade,const char *nomeJogador, int novaPontuacao);

#endif
