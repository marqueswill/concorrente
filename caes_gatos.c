#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>

#include "unistd.h"

#define CAES 5
#define GATOS 1

int gato_no_galpao = 0;
int caes_dormindo = CAES;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_gato = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_caes = PTHREAD_COND_INITIALIZER;

void *cao(void *arg);
void *gato(void *arg);

int main(int argc, char **argv) {
    pthread_t c[CAES];
    pthread_t g;

    int i;
    int *id;
    for (i = 0; i < CAES; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        pthread_create(&(c[i]), NULL, cao, (void *)(id));
    }

    pthread_create(&(g), NULL, gato, NULL);
    pthread_join(g, NULL);
}

void *cao(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);

        while (!gato_no_galpao) {
            pthread_cond_wait(&cond_caes, &mutex);
        }
        caes_dormindo--;
        if (caes_dormindo == 0) {
            printf("Todos os cães saíram.\n");
            pthread_cond_signal(&cond_gato);  // Deixa o gato entrar
        }
        pthread_mutex_unlock(&mutex);

        sleep(1);  // Simula o tempo que o cão leva para acordar

        pthread_mutex_lock(&mutex);
        while (gato_no_galpao) {  // Espera o gato comer
            pthread_cond_wait(&cond_caes, &mutex);
        }
        caes_dormindo++;
        pthread_mutex_unlock(&mutex);
    }
}

void *gato(void *arg) {
    while (1) {
        pthread_mutex_lock(&mutex);
        gato_no_galpao = 1;  // Indica que ele quer entrar
        printf("\nGato quer entrar no galpão.\n");

        pthread_cond_broadcast(&cond_caes);  // Manda os cães saírem
        while (caes_dormindo > 0) {
            pthread_cond_wait(&cond_gato, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        printf("Gato está comendo.\n");
        sleep(2);  // Comendo

        pthread_mutex_lock(&mutex);
        gato_no_galpao = 0;  // Indica que ele saiu
        printf("Gato saiu do galpão.\n\n");
        pthread_cond_broadcast(&cond_caes);  // Deixa os cães entrarem
        pthread_mutex_unlock(&mutex);
        sleep(1);  // Simula o tempo que o gato leva para sair
    }
}