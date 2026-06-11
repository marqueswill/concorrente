#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define N 5
#define ESQ(id) (id - 1 + N) % N
#define DIR(id) (id + 1) % N

sem_t garfos[N];

void* filosofos(void* arg);

void pega_talher(int n);
void devolve_talher(int n);

int main() {
    int i;
    int* id;

    for (i = 0; i < N; i++) {
        sem_init(&garfos[i], 0, 1);
    }

    pthread_t r[N];

    // criacao das threads de filosofos
    for (i = 0; i < N; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        pthread_create(&r[i], NULL, filosofos, (void*)(id));
    }

    for (i = 0; i < N; i++) {
        pthread_join(r[i], NULL);
    }

    return 0;
}

void* filosofos(void* arg) {
    int n = *((int*)arg);
    while (1) {
        // pensar
        printf("Filosofo %d pensando ...\n", n);
        sleep(3);

        pega_talher(n);

        // comer
        printf("\tFilosofo %d comendo ...\n", n);
        sleep(3);
        printf("\tFilosofo %d acabou de comer ...\n", n);

        devolve_talher(n);
    }
}

void pega_talher(int n) {
    if (n % 2 == 0) {  // Evita dependência circular
        sem_wait(&garfos[DIR(n)]);
        sem_wait(&garfos[ESQ(n)]);
    } else {
        sem_wait(&garfos[ESQ(n)]);
        sem_wait(&garfos[DIR(n)]);
    }
}

void devolve_talher(int n) {
    sem_post(&garfos[ESQ(n)]);  // Devolve o garfo e acorda o filósofo se ele estiver esperando
    sem_post(&garfos[DIR(n)]);  // Devolve o garfo e acorda o filósofo se ele estiver esperando
}
