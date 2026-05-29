#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PR 1  // número de produtores
#define CN 1  // número de consumidores

// #define N 5   // tamanho do buffer

sem_t leitura;
sem_t impressao;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int N;
int* buffer;
int pos = 0;

// int buffer[N];

void* leitor(void* meuid);
void* impressor(void* meuid);

void main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Uso: %s <tamanho_do_buffer>\n", argv[0]);
        exit(1);
    }

    N = atoi(argv[1]);

    if (N <= 0) {
        printf("Erro: O tamanho do buffer deve ser maior que 0.\n");
        exit(1);
    }

    buffer = (int*)malloc(N * sizeof(int));
    if (buffer == NULL) {
        printf("Erro ao alocar memória para o buffer.\n");
        exit(1);
    }

        int erro;
    int i, n, m;
    int* id;

    sem_init(&leitura, 0, 1);
    sem_init(&impressao, 0, 0);

    pthread_t tid[PR];

    for (i = 0; i < PR; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tid[i], NULL, leitor, (void*)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_t tCid[CN];

    for (i = 0; i < CN; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tCid[i], NULL, impressor, (void*)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_join(tid[0], NULL);
}

void* leitor(void* pi) {
    int id = *(int*)(pi);
    free(pi);
    while (1) {
        sem_wait(&leitura);
        printf("Digite os caracteres para impressão.\n");
        while (pos < N) {
            char c = getchar();

            if (c == '\n' || c == '\r') {
                continue;
            }

            buffer[pos] = c;
            printf("[LEITOR]: caractere %c \033[32minserido\033[0m na fila de impressão.\n", c);
            pos++;
        }

        sem_post(&impressao);
        sleep(2);
    }
    pthread_exit(0);
}

void* impressor(void* pi) {
    int id = *(int*)(pi);
    int lido;
    free(pi);

    while (1) {
        sem_wait(&impressao);  // Espera até que tenha item no buffer

        printf("[IMPRESSOR]: vou \033[31mimprimir\033[0m os caracteres da lista.\n");
        while (pos > 0) {
            char c = buffer[N - pos];
            printf("%c", c);
            pos--;
        }
        printf("\n");

        sem_post(&leitura);

        sleep(1);
    }

    pthread_exit(0);
}
