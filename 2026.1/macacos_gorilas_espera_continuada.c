#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 5  // macacos que andam de A para B
#define MB 5  // macacos que andam de B para A

// TODO: prioridade com laços de espera contiinuada

pthread_mutex_t turno = PTHREAD_MUTEX_INITIALIZER;  // Um primata por vez verifica a condição atual da corda

int mA, mB, gA, gB = 0;                  // Contadores
int mAQuer, mBQuer, gAQuer, gBQuer = 0;  // Sinalizadores

void* macacoAB(void* a) {
    int i = *((int*)a);
    while (1) {
        sleep(1);
        while (1) {  // Loop de espera para entrar na corda
            pthread_mutex_lock(&turno);
            mAQuer++;                                                               // QUERO PASSARRRRR!!
            if ((mB == 0 && gA == 0 && gB == 0) && (gBQuer == 0 && gAQuer == 0)) {  // Nenhum macaco corda e nenhum gorila quer passar
                mA++;                                                               // mA começa a passar
                mAQuer--;                                                           // Eba tô passando!
                pthread_mutex_unlock(&turno);
                break;
            }
            mAQuer--;  // Ok, vou esperar um pouco...
            pthread_mutex_unlock(&turno);
            sleep(1);
        }

        printf("Macaco %d passado de A para B \n", i);
        sleep(1);

        pthread_mutex_lock(&turno);
        mA--;  // Macaco termina de passar
        printf("Macaco %d terminou de passar de A para B. Num: %d\n", i, mA);
        pthread_mutex_unlock(&turno);
    }
    pthread_exit(0);
}

void* macacoBA(void* a) {
    int i = *((int*)a);
    while (1) {
        sleep(1);
        while (1) {  // Loop de espera para entrar na corda
            mBQuer++;
            pthread_mutex_lock(&turno);                                             // Um macaco tenta entrar por vez
            if ((mA == 0 && gA == 0 && gB == 0) && (gBQuer == 0 && gAQuer == 0)) {  // Nenhum macaco corda e nenhum gorila quer passar
                mB++;
                mBQuer--;  // mB começa a passar
                pthread_mutex_unlock(&turno);
                break;
            }
            mBQuer--;
            pthread_mutex_unlock(&turno);
            sleep(1);
        }

        printf("Macaco %d passado de B para A \n", i);
        sleep(1);

        pthread_mutex_lock(&turno);  // Um macaco sai por vez
        mB--;
        printf("Macaco %d terminou de passar de B para A. Num: %d\n", i, mB);
        pthread_mutex_unlock(&turno);
    }
    pthread_exit(0);
}

void* gorilaAB(void* a) {
    while (1) {
        sleep(1);
        // Procedimentos para acessar a corda
        printf("GORILA AB QUER PASSAR!\n");
        while (1) {
            pthread_mutex_lock(&turno);
            gBQuer++;
            if (mA == 0 && mB == 0 && gB == 0) {  // Gorila fica em espera até a corda esvaziar
                gB++;
                gBQuer--;

                pthread_mutex_unlock(&turno);
                break;
            }
            gBQuer--;
            pthread_mutex_unlock(&turno);
        }

        pthread_mutex_lock(&turno);
        printf("Gorila passado de A para B \n");
        sleep(5);
        // Procedimentos para quando sair da corda
        pthread_mutex_unlock(&corda);
        gorila_esperando = 0;
        pthread_mutex_unlock(&turno);
    }
    pthread_exit(0);
}

void* gorilaBA(void* a) {
    while (1) {
        sleep(1);
        gorila_esperando = 1;  // Faz macacos entrar em loop de espera
        printf("GORILA BA QUER PASSAR!\n");
        while (1) {
            pthread_mutex_lock(&corda);
            if (AB == 0 && BA == 0) {  // Gorila fica em espera até a corda esvaziar
                pthread_mutex_unlock(&corda);
                break;
            }
            pthread_mutex_unlock(&corda);
        }
        // Procedimentos para acessar a corda
        pthread_mutex_lock(&turno);
        pthread_mutex_lock(&corda);
        printf("Gorila passado de B para A \n");
        sleep(5);
        // Procedimentos para quando sair da corda
        pthread_mutex_unlock(&corda);
        gorila_esperando = 0;
        pthread_mutex_unlock(&turno);
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
    pthread_t gAB, gBA;
    pthread_create(&gAB, NULL, &gorilaAB, NULL);
    pthread_create(&gBA, NULL, &gorilaBA, NULL);

    pthread_join(macacos[0], NULL);
    return 0;
}
