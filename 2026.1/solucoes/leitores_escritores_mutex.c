#include "pthread.h"
#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include <windows.h>

#define TRUE 1

#define NE 3   // numero de escritores
#define NL 10  // numero de leitores

pthread_mutex_t lock_bd = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lock_nl = PTHREAD_MUTEX_INITIALIZER;

int num_leitores = 0;

void* reader(void* arg);
void* writer(void* arg);
void read_data_base(int i);
void use_data_read(int i);
void think_up_data(int i);
void write_data_base(int i);

int main() {
	SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    pthread_t r[NL], w[NE];
    int i;
    int* id;
    /* criando leitores */
    for (i = 0; i < NL; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        pthread_create(&r[i], NULL, reader, (void*)(id));
    }
    /* criando escritores */
    for (i = 0; i < NE; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        pthread_create(&w[i], NULL, writer, (void*)(id));
    }
    pthread_join(r[0], NULL);
    return 0;
}

void* reader(void* arg) {
    int i = *((int*)arg);
    while (TRUE) { /* repere para sempre */
        pthread_mutex_lock(&lock_nl);
        num_leitores++;
        if (num_leitores == 1) {
            pthread_mutex_lock(&lock_bd);
        }
        pthread_mutex_unlock(&lock_nl);

        read_data_base(i); /* acesso aos dados */

        pthread_mutex_lock(&lock_nl);
        num_leitores--;
        if (num_leitores == 0) {
            pthread_mutex_unlock(&lock_bd);
        }
        pthread_mutex_unlock(&lock_nl);
        use_data_read(i); /* região não crítica */
    }
    pthread_exit(0);
}

void* writer(void* arg) {
    int i = *((int*)arg);
    while (TRUE) {        /* repete para sempre */
        think_up_data(i); /* região não crítica */
        pthread_mutex_lock(&lock_bd);
        write_data_base(i); /* atualiza os dados */
        pthread_mutex_unlock(&lock_bd);
    }
    pthread_exit(0);
}

void read_data_base(int i) {
    printf("Leitor %d está lendo os dados! Número de leitores: %d\n", i, num_leitores);
    sleep(rand() % 5);
}

void use_data_read(int i) {
    printf("Leitor %d está usando os dados lidos! Número de leitores: %d\n", i, num_leitores);
    sleep(rand() % 5);
}

void think_up_data(int i) {
    printf("Escritor %d está pensando no que escrever!\n", i);
    sleep(rand() % 5);
}

void write_data_base(int i) {
    printf("Escritor %d está escrevendo os dados! Número de leitores: %d\n", i, num_leitores);
    sleep(rand() % 5 + 15);
}
