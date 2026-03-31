#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXCANIBAIS 20

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_canibal = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_cozinheiro = PTHREAD_COND_INITIALIZER;

int porcoes = 0;

void *canibal(void *meuid);
void *cozinheiro(int m);

void main(argc, argv) int argc;
char *argv[];
{
    int erro;
    int i, n, m;
    int *id;

    pthread_t tid[MAXCANIBAIS];

    if (argc != 3) {
        printf("erro na chamada do programa: jantar <#canibais> <#comida>\n");
        exit(1);
    }

    n = atoi(argv[1]);  // número de canibais
    m = atoi(argv[2]);  // quantidade de porções que o cozinheiro consegue preparar por vez
    printf("numero de canibais: %d -- quantidade de comida: %d\n", n, m);

    if (n > MAXCANIBAIS) {
        printf("o numero de canibais e' maior que o maximo permitido: %d\n", MAXCANIBAIS);
        exit(1);
    }

    for (i = 0; i < n; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tid[i], NULL, canibal, (void *)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    cozinheiro(m);
}

void *canibal(void *pi) {
    while (1) {
        pthread_mutex_lock(&mutex);
        while (porcoes == 0) {
            pthread_cond_wait(&cond_canibal, &mutex);
        }

        // pegar uma porção de comida
        porcoes--;
        printf("%d: Peguei uma porção. Porções restantes: %d\n", *(int *)(pi), porcoes);
        
        // acordar o cozinheiro se as porções acabaram
        if (porcoes == 0)
            pthread_cond_signal(&cond_cozinheiro);

        pthread_mutex_unlock(&mutex);

        printf("%d: vou comer a porcao que peguei. \n", *(int *)(pi));
        sleep(1);

    }
    pthread_exit(0);
}

void *cozinheiro(int m) {
    while (1) {
        // dormir enquanto tiver comida
        pthread_mutex_lock(&mutex);
        while (porcoes > 0) {
            pthread_cond_wait(&cond_cozinheiro, &mutex);
        }

        printf("cozinheiro: vou cozinhar\n");
        porcoes = m;
        
        sleep(2);

        // acordar os canibais
        printf("RANGO TÁ PRONTO!\n\n");
        pthread_cond_broadcast(&cond_canibal);

        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}
