#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PR 1  // número de produtores
#define CN 1  // número de consumidores
#define N 5   // tamanho do buffer

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
pthread_cond_t cons_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t prod_cond = PTHREAD_COND_INITIALIZER;

void* produtor(void* meuid);
void* consumidor(void* meuid);

void main(argc, argv) int argc;
char* argv[];
{
    int erro;
    int i, n, m;
    int* id;

    pthread_t tid[PR];

    for (i = 0; i < PR; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tid[i], NULL, produtor, (void*)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_t tCid[CN];

    for (i = 0; i < CN; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tCid[i], NULL, consumidor, (void*)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_join(tid[0], NULL);
}

void* produtor(void* pi) {
    while (1) {
    }
    pthread_exit(0);
}

void* consumidor(void* pi) {
    while (1) {
    }
    pthread_exit(0);
}
