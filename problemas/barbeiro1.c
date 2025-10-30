/*
 * Problema do barbeiro dorminhoco.
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define N_CLIENTES 15
#define N_CADEIRAS 5

#define COLOR_GREEN "\x1b[32m"
#define COLOR_RESET "\x1b[0m"
#define COLOR_RED "\x1b[31m"

sem_t sem_cadeiras;
sem_t sem_cadeira_barbeiro;
sem_t sem_barbeiro;
sem_t sem_cliente;

void* f_barbeiro(void* v) {
    while (1) {
        //...Esperar/dormindo algum cliente sentar na cadeira do barbeiro (e acordar o barbeiro)
        printf("\nBarbeiro está esperando um cliente...\n\n");
        sem_wait(&sem_barbeiro);

        sleep(1);  // Cortar o cabelo do cliente
        printf("Barbeiro cortou o cabelo de um cliente\n");

        //...Liberar/desbloquear o cliente
        sem_post(&sem_cliente);
    }
    pthread_exit(0);
}

void* f_cliente(void* v) {
    int id = *(int*)v;

    sleep(id % 5);
    if (sem_trywait(&sem_cadeiras) == 0) {  // conseguiu pegar uma cadeira de espera
        printf("Cliente %d entrou na barbearia \n", id);

        //... pegar/sentar a cadeira do barbeiro
        sem_wait(&sem_cadeira_barbeiro);

        //... liberar a sua cadeira de espera
        sem_post(&sem_cadeiras);

        //... acordar o barbeiro para cortar seu cabelo
        sem_post(&sem_barbeiro);

        //... aguardar o corte do seu cabelo
        sem_wait(&sem_cliente);

        //... liberar a cadeira do barbeiro
        sem_post(&sem_cadeira_barbeiro);

        printf(COLOR_GREEN "Cliente %d cortou o cabelo e foi embora \n" COLOR_RESET, id);
    } else {  // barbearia cheia
        printf(COLOR_RED "Barbearia cheia, cliente %d indo embora\n" COLOR_RESET, id);
    }

    pthread_exit(0);
}

int main() {
    pthread_t thr_clientes[N_CLIENTES], thr_barbeiro;
    int i, id[N_CLIENTES];

    sem_init(&sem_cadeiras, 0, N_CADEIRAS - 1);
    sem_init(&sem_barbeiro, 0, 0);
    sem_init(&sem_cliente, 0, 0);
    sem_init(&sem_cadeira_barbeiro, 0, 1);

    // Threads clientes
    for (i = 0; i < N_CLIENTES; i++) {
        id[i] = i;
        pthread_create(&thr_clientes[i], NULL, f_cliente, (void*)&id[i]);
    }

    // Thread barbeiro
    pthread_create(&thr_barbeiro, NULL, f_barbeiro, NULL);

    for (i = 0; i < N_CLIENTES; i++)
        pthread_join(thr_clientes[i], NULL);

    /* Barbeiro assassinado */

    return 0;
}
