#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "unistd.h"

#define N 10  // número de usuários

#define MAX_CARTAS 20  // quantidade de cartas na mochila

int cartas_mochila = 0;
int pombo_disponivel = 1;  // 1 = A; 0 = B

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_pombo = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_usuario = PTHREAD_COND_INITIALIZER;

void *f_usuario(void *arg);
void *f_pombo(void *arg);

int main(int argc, char **argv) {
    int i;

    pthread_t usuario[N];
    int *id;
    for (i = 0; i < N; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&(usuario[i]), NULL, f_usuario, (void *)(id));
    }
    pthread_t pombo;
    id = (int *)malloc(sizeof(int));
    *id = 0;
    pthread_create(&(pombo), NULL, f_pombo, (void *)(id));

    pthread_join(pombo, NULL);
}

void *f_pombo(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Inicialmente está em A, aguardar/dorme a mochila ficar cheia (20 cartas)
        while (cartas_mochila < MAX_CARTAS) {
            pthread_cond_wait(&cond_pombo, &mutex);
        }

        // Leva as cartas para B e volta para A
        pombo_disponivel = 0;
        printf("\nPombo levando cartas para B.\n");
        sleep(5);
        printf("Pombo retornou para A\n\n");
        cartas_mochila = 0;
        pombo_disponivel = 1;

        // Acordar os usuáriosid
        pthread_cond_broadcast(&cond_usuario);

        pthread_mutex_unlock(&mutex);
    }
}

void *f_usuario(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        // Caso o pombo não esteja em A ou a mochila estiver cheia, então dorme
        while (!pombo_disponivel || cartas_mochila == MAX_CARTAS) {
            pthread_cond_wait(&cond_usuario, &mutex);
        }

        // Escreve uma carta
        // Posta sua carta na mochila do pombo
        sleep(1);
        cartas_mochila++;
        printf("Usuário %2d escreveu uma carta. Total de cartas: %d\n", *(int *)arg, cartas_mochila);

        // Caso a mochila fique cheia, acorda o ṕombo
        if (cartas_mochila == MAX_CARTAS) {
            pthread_cond_signal(&cond_pombo);
        }


        pthread_mutex_unlock(&mutex);

        sleep((rand() % 5) + 1);
    }
}
