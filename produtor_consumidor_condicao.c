#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PR 5   // número de produtores
#define CN 10  // número de consumidores
#define N 100  // tamanho do buffer

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t produtor_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumidor_cond = PTHREAD_COND_INITIALIZER;

#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"
#define ANSI_COLOR_RED "\x1b[31m"

void *produtor(void *meuid);
void *consumidor(void *meuid);

void main(argc, argv) int argc;
char *argv[];
{
    int erro;
    int i, n, m;
    int *id;

    pthread_t tid[PR];
    for (i = 0; i < PR; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tid[i], NULL, produtor, (void *)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_t tCid[CN];
    for (i = 0; i < CN; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tCid[i], NULL, consumidor, (void *)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_join(tid[0], NULL);
}

int buffer[N];
int count = 0;  // Quantidade de itens no buffer
int i = 0;      // Posição para o proximo item a ser adicionado
int o = 0;      // Posição para o proximo item a ser removido

void *produtor(void *pi) {
    int item;

    while (1) {
        pthread_mutex_lock(&mutex);
        while (count == N) {
            printf("\nProdutor %d: Buffer cheio. Esperando...\n\n", *((int *)pi));
            pthread_cond_wait(&produtor_cond, &mutex);  // Se estiver esperando, quando alguém consumir vai liberar
        }

        item = rand() % 100;
        buffer[i] = item;
        i = (i + 1) % N;  // Lógica circular
        count++;

        printf(ANSI_COLOR_GREEN "Produtor %d produziu o item: %d\n" ANSI_COLOR_RESET, *((int *)pi), item);
        printf("Estoque atual: %d\n\n", count);

        if (count >= 1)
            pthread_cond_signal(&consumidor_cond);  // Indica que pode consumir

        sleep(2);

        pthread_mutex_unlock(&mutex);

        // Pausa para simulação
        sleep(1);
    }

    pthread_exit(0);
}

void *consumidor(void *pi) {
    int item;

    while (1) {
        pthread_mutex_lock(&mutex);
        while (count == 0) {
            printf("Consumidor %d: Buffer vazio. Esperando...\n", *((int *)pi));
            pthread_cond_wait(&consumidor_cond, &mutex);
        }

        item = buffer[o];
        buffer[o] = -1;
        o = (o + 1) % N;  // Lógica circular
        count--;

        printf(ANSI_COLOR_RED "Consumidor %d consumiu o item: %d\n" ANSI_COLOR_RESET, *((int *)pi), item);
        printf("Estoque atual: %d\n\n", count);

        if (count < N)                            // Meio inútil por conta dos loops de espera no inicio
            pthread_cond_signal(&produtor_cond);  // Indica que pode produzir

        sleep(1);

        pthread_mutex_unlock(&mutex);

        sleep(1);
    }

    pthread_exit(0);
}
