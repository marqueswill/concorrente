#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

sem_t pista;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int equipes;
int N;
int* reservas_pista;
// int pos = 0;

void* piloto(void* meuid);

void main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Uso: %s <vagas_pista> <numero_equipes>\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);  // Tamanho da pista
    equipes = atoi(argv[2]);  // Número de equipes
    int pilotos = equipes * 2;

    if (equipes <= 0) {
        printf("Erro: Deve haver ao menos uma equipe.\n");
        exit(1);
    }

    reservas_pista = (int*)malloc(N * sizeof(int));
    if (reservas_pista == NULL) {
        printf("Erro ao alocar memória para o buffer.\n");
        exit(1);
    }

    int erro;
    int i, n, m;
    int* id;

    sem_init(&pista, 0, N);

    pthread_t tid[pilotos];

    for (i = 0; i < pilotos; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tid[i], NULL, piloto, (void*)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    for (i = 0; i < pilotos; i++) {
        pthread_join(tid[i], NULL);
    }
}

void* piloto(void* pi) {
    int id_piloto = *(int*)(pi);
    int id_time = id_piloto % N;
    free(pi);

    while (1) {
        sem_wait(&pista);

        pthread_mutex_lock(&mutex);  // Mutex para proteger o buffer
        {
            if (reservas_pista[id_time]) {  // Se o time já está na pista
                sem_post(&pista);           // Eu libero a pista paro outro time
                pthread_mutex_unlock(&mutex);
                continue;
            } else {
                reservas_pista[id_time] = 1;
            }
        }
        pthread_mutex_unlock(&mutex);

        printf("[ENTRADA] Equipe %d: Piloto %d \n", id_time, id_piloto);
        sleep(5);

        pthread_mutex_lock(&mutex);  // Mutex para proteger o buffer
        {
            printf("[SAÍDA] Equipe %d: Piloto %d \n", id_time, id_piloto);
            reservas_pista[id_time] = 0;  // Aviso que minha reserva da pista acabou
            sem_post(&pista);             // Libera pista paro o próximo time
        }
        pthread_mutex_unlock(&mutex);
        sleep(2);
    }
    pthread_exit(0);
}
