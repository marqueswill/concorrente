#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PR 10  // número de produtores
#define CN 5    // número de consumidores
#define N 10    // tamanho do buffer

sem_t livres;
sem_t ocupadas;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int buffer[N];
int inserir;  // index de acesso
int remover;
int contador = 0;

void* produtor(void* meuid);
void* consumidor(void* meuid);

void main(int argc, char* argv[]) {
    int erro;
    int i, n, m;
    int* id;

    sem_init(&livres, 0, N);
    sem_init(&ocupadas, 0, 0);

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
    int id = *(int*)(pi);
    free(pi);
    while (1) {
        // printf("[PRODUTOR %d]: vou produzir um item \n", id);
        int item = (int)((1 + drand48()) * 1000.0);
        sleep(2);

        sem_wait(&livres);  // Garante que tenha espaço no buffer

        pthread_mutex_lock(&mutex);
        {
            printf("[PRODUTOR %d]: vou \033[32minserir\033[0m item %d na posição %d\n", id, item, inserir);
            buffer[inserir] = item;  // Inserção sequencial
            inserir = (inserir + 1) % N;
            contador++;
            // printf("Itens no buffer: %d\n", contador);
        }
        pthread_mutex_unlock(&mutex);

        sem_post(&ocupadas);  // Garante que tenha item no buffer
    }
    pthread_exit(0);
}

void* consumidor(void* pi) {
    int id = *(int*)(pi);
    int lido;
    free(pi);

    while (1) {
        sem_wait(&ocupadas);  // Espera até que tenha item no buffer

        pthread_mutex_lock(&mutex);  // Mutex buffer e index
        {
            lido = buffer[remover];  // O mutex garante que a lista funcione como "fila'
            printf("[CONSUMIDOR %d]: vou \033[31mremover\033[0m item %d na posição %d\n", id, lido, remover);
            buffer[remover] = 0;  // Tecnicamente inútil, sempre será substituído
            contador--;           // Inútil também
            remover = (remover + 1) % N;

            // printf("Itens no buffer: %d\n", contador);
        }
        pthread_mutex_unlock(&mutex);

        sem_post(&livres);  // Avisa que pode inserir mais item no buffer
        // printf("[CONSUMIDOR %d]: vou consumir item %d \n", id, lido);

        sleep(1);
    }

    pthread_exit(0);
}
