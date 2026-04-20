#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 2  // macacos que andam de A para B
#define MB 2  // macacos que andam de B para A

pthread_mutex_t corda = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t turno = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexAB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexBA = PTHREAD_MUTEX_INITIALIZER;

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
        sleep(1);

        pthread_mutex_lock(&turno);  // Alterna os lados -> ambos tentam começar acessar a corda
        pthread_mutex_lock(&mutexAB);
        cont_a++;
        if (cont_a == 1) {
            pthread_mutex_lock(&corda);
        }
        pthread_mutex_unlock(&mutexAB);

        if (lado_atual == 'A')
            printf("Macaco %d passado de A para B.\n", i);
        else
            printf("Macaco %d passado de B para A.\n", i);

        pthread_mutex_lock(&mutexAB);
        cont_a--;
        if (cont_a == 0) {
            pthread_mutex_unlock(&corda);
        }
        pthread_mutex_unlock(&mutexAB);

        // Variável interna, não precisa de lock
        if (lado_atual == 'A') {
            lado_atual = 'B';
        } else {
            lado_atual = 'A';
        }

        pthread_mutex_unlock(&turno);
        sleep(1);
    }
    pthread_exit(0);
}

// macacos que começam do lado B
void* macacoBA(void* a) {
    char lado_atual = 'B';
    int i = *((int*)a);
    while (1) {
        sleep(1);

        pthread_mutex_lock(&turno);  // Alterna os lados -> ambos tentam começar acessar a corda
        pthread_mutex_lock(&mutexBA);
        cont_b++;
        if (cont_b == 1) {
            pthread_mutex_lock(&corda);
        }
        pthread_mutex_unlock(&mutexBA);

        if (lado_atual == 'A')
            printf("Macaco %d passado de A para B.\n", i);
        else
            printf("Macaco %d passado de B para A.\n", i);

        pthread_mutex_lock(&mutexBA);
        cont_b--;
        if (cont_b == 0) {
            pthread_mutex_unlock(&corda);
        }
        pthread_mutex_unlock(&mutexBA);

        // Variável interna, não precisa de lock
        if (lado_atual == 'A') {
            lado_atual = 'B';
        } else {
            lado_atual = 'A';
        }

        pthread_mutex_unlock(&turno);
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

    for (i = 0; i < MA + MB; i++) { // Essa lógica assume mesmo número incial de cada lado
        id = (int*)malloc(sizeof(int));
        *id = i;
        if (i % 2 == 0) {
            if (pthread_create(&macacos[i], NULL, &macacoAB, (void*)id)) {
                printf("Não pode criar a thread %d\n", i);
                return -1;
            } else {
                printf("Macaco %d entrou na fila do lado A.\n", i);
            }
        } else {
            if (pthread_create(&macacos[i], NULL, &macacoBA, (void*)id)) {
                printf("Não pode criar a thread %d\n", i);
                return -1;

            } else {
                printf("Macaco %d entrou na fila do lado B.\n", i);
            }
        }
    }
    // pthread_t g;
    // pthread_create(&g, NULL, &gorila, NULL);

    pthread_join(macacos[0], NULL);
    return 0;
}
