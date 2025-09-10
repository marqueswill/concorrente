#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 10  // macacos que andam de A para B
#define MB 10  // macacos que andam de B para A

pthread_mutex_t lock_corda = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_ab = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_ba = PTHREAD_COND_INITIALIZER;

int num_macacosAB = 0;
int num_macacosBA = 0;
int golira_quer_passar = 0;

char lado_golira = 'A';  // lado em que o golira está

void *macacoAB(void *a) {
    int i = *((int *)a);
    while (1) {
        // Se existem macacos passando de B para A, bloqueia a passagem de macacos de A para B
        // Se o golira está passando, bloqueia a passagem de todos os macacos
        pthread_mutex_lock(&lock_corda);
            while (num_macacosBA || golira_quer_passar) {  // While para garantir que ao acordar da condição, a situação ainda é válida
                pthread_cond_wait(&cond_ab, &lock_corda);
            }
            num_macacosAB++;
        pthread_mutex_unlock(&lock_corda);

        printf("Macaco %02d passado de A para B. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);
        sleep(1);

        // Procedimentos para quando sair da corda
        pthread_mutex_lock(&lock_corda);  // protege a variável num_macacos_corda
            num_macacosAB--;
            printf("Macaco %02d saiu da corda. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);
            if (num_macacosAB == 0) {
                pthread_cond_broadcast(&cond_ba);  // Libera todos os macacos que estão esperando para passar de B para A
            }
        pthread_mutex_unlock(&lock_corda);
        sleep(3); // esperar um pouco do outro lado
    }
    pthread_exit(0);
}

void *macacoBA(void *a) {
    int i = *((int *)a);
    while (1) {
        // Se existem macacos passando de B para A, bloqueia a passagem de macacos de A para B
        // Se o golira está passando, bloqueia a passagem de todos os macacos
        pthread_mutex_lock(&lock_corda);
            while (num_macacosAB || golira_quer_passar) {  // While para garantir que ao acordar da condição, a situação ainda é válida
                pthread_cond_wait(&cond_ba, &lock_corda);
            }
            num_macacosBA++;
        pthread_mutex_unlock(&lock_corda);

        printf("Macaco %02d passado de A para B. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);
        sleep(1);

        // Procedimentos para quando sair da corda
        pthread_mutex_lock(&lock_corda);  // protege a variável num_macacos_corda
            num_macacosBA--;
            printf("Macaco %02d saiu da corda. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);

            if (num_macacosBA == 0) {
                pthread_cond_broadcast(&cond_ab);  // Libera todos os macacos que estão esperando para passar de B para A
            }
        pthread_mutex_unlock(&lock_corda);
        sleep(3); // esperar um pouco do outro lado
    }
    pthread_exit(0);
}

void *golira(void *a) {
    while (1) {
        // Procedimentos para acessar a corda
        
        pthread_mutex_lock(&lock_corda);
            while (num_macacosAB || num_macacosBA) {        // While para garantir que ao acordar da condição, a situação ainda é válida
                golira_quer_passar = 1;      
                printf("GOLIRA QUER PASSAR DE %c \n", lado_golira);

                if (lado_golira == 'A') { // Se o golira está em A, espera os macacos de A para B sairem
                    pthread_cond_wait(&cond_ba, &lock_corda);
                } else {                  // Senao, espera os macacos de B para A sairem
                    pthread_cond_wait(&cond_ab, &lock_corda);   
                }
            }


            if (lado_golira == 'A') {
                printf("GOLIRA PASSANO de A para B \n");
                lado_golira = 'B';
            } else {
                printf("GOLIRA PASSANO de B para A \n");
                lado_golira = 'A';
            }

            golira_quer_passar = 0;

        pthread_mutex_unlock(&lock_corda);

        sleep(5);

    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    pthread_t macacos[MA + MB];
    int *id;
    int i = 0;

    for (i = 0; i < MA + MB; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        if (i % 2 == 0) {
            if (pthread_create(&macacos[i], NULL, &macacoAB, (void *)id)) {
                printf("Não pode criar a thread %02d\n", i);
                return -1;
            }
        } else {
            if (pthread_create(&macacos[i], NULL, &macacoBA, (void *)id)) {
                printf("Não pode criar a thread %02d\n", i);
                return -1;
            }
        }
    }
    pthread_t g;
    pthread_create(&g, NULL, &golira, NULL);

    pthread_join(macacos[0], NULL);
    return 0;
}
