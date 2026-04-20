#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 1   // macacos que andam de A para B
#define MB 11  // macacos que andam de B para A

pthread_mutex_t corda = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t lado_a = PTHREAD_MUTEX_INITIALIZER;      // atualizar variáveis do lado A
pthread_mutex_t lado_b = PTHREAD_MUTEX_INITIALIZER;      // atualizar variáveis do lado B
pthread_mutex_t prioridade = PTHREAD_MUTEX_INITIALIZER;  // usado par o gorila

// inicializa contadores
int cont_a = MA;
int cont_b = MB;

// Inicio da travessia: subtrai do lado atual, final da travessia: soma do lado oposto
// Só pode atravessar 1 por vez

// macacos que começam do lado A
void* macacoAB(void* a) {
    char lado_atual = 'A';
    int i = *((int*)a);
    while (1) {
        pthread_mutex_lock(&prioridade);  // Alterna os lados -> ambos tentam começar acessar a corda
        pthread_mutex_lock(&corda);
        if (lado_atual == 'A') {  // Garante que os contadores não fica negativo, já que eu sei a posição atual do macaco
          cont_a--;
          cont_b++;
            lado_atual = 'B';
        } else {
            cont_b--;
            cont_a++;
            lado_atual = 'A';
        }
        pthread_mutex_unlock(&corda);

        if (lado_atual == 'A')
            printf("Macaco %d passado de A para B.\n", i);
        else
            printf("Macaco %d passado de B para A.\n", i);

        sleep(1);

        pthread_mutex_unlock(&prioridade);
        sleep(1);
    }
    pthread_exit(0);
}

// macacos que começam do lado B
void* macacoBA(void* a) {
    char lado_atual = 'B';
    int i = *((int*)a);
    while (1) {
        pthread_mutex_lock(&prioridade);
        pthread_mutex_lock(&corda);
        if (lado_atual == 'A') {
            cont_a--;
            cont_b++;
            lado_atual = 'B';
        } else {
            cont_b--;
            cont_a++;
            lado_atual = 'A';
        }
        pthread_mutex_unlock(&corda);

        if (lado_atual == 'A')
            printf("Macaco %d passado de A para B.\n", i);
        else
            printf("Macaco %d passado de B para A.\n", i);

        sleep(1);

        pthread_mutex_unlock(&prioridade);
        sleep(1);
    }
    pthread_exit(0);
}

void* gorila(void* a) {
    while (1) {
        // Procedimentos para acessar a corda
        printf("Gorila passado de A para B \n");
        sleep(5);
        // Procedimentos para quando sair da corda
    }
    pthread_exit(0);
}

int main(int argc, char* argv[]) {
    pthread_t macacos[MA + MB];
    int* id;
    int i = 0;

    for (i = 0; i < MA + MB; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        if (i % 2 == 0) {
            if (pthread_create(&macacos[i], NULL, &macacoAB, (void*)id)) {
                printf("Não pode criar a thread %d\n", i);
                return -1;
            }
        } else {
            if (pthread_create(&macacos[i], NULL, &macacoBA, (void*)id)) {
                printf("Não pode criar a thread %d\n", i);
                return -1;
            }
        }
    }
    // pthread_t g;
    // pthread_create(&g, NULL, &gorila, NULL);

    pthread_join(macacos[0], NULL);
    return 0;
}
