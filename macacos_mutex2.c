#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 10  // macacos que andam de A para B
#define MB 10  // macacos que andam de B para A

pthread_mutex_t lock_corda = PTHREAD_MUTEX_INITIALIZER;

int num_macacosAB = 0;
int num_macacosBA = 0;
int golira_quer_passar = 0;
int ultimo_lado_aberto = 'Y';  // Variável para controlar o lado que passou por último

// Desnecessário já que já estamos contando os macacos passando de um lado pra outro
// int BA_esperando = 1;
// int AB_esperando = 0;

char lado_golira = 'X';  // lado em que o golira está

char lado_macacosAB = 'X';  // lado que os macacos AB estão
char lado_macacosBA = 'Y';  // lado que os macacos BA estão

void *macacoAB(void *a) {
    int i = *((int *)a);
    while (1) {  // Loop principal do macaco

        while (1) {
            pthread_mutex_lock(&lock_corda);                          // Loop de espera para entrar na corda, as threads ficam bloqueando e desbloqueando em loop
                if (num_macacosBA == 0 && golira_quer_passar == 0) {  // Se não há macacos indo de B para A e o gorila não quer passar, eu deixo as threads passarem
                    num_macacosAB++;                                  // O controle do último lado aberto faz com que as threads AB e BA se alternem, evitando que um lado fique preso esperando o outro acabar. Funciona por conta da contagem de macacos
                    pthread_mutex_unlock(&lock_corda);                // Libero a corda para os macacos passarem
                    break;                                            // Sai do loop, agora as trheads AB podem continuar
                }
            pthread_mutex_unlock(&lock_corda);
            sleep(1);
        }

        if (lado_macacosAB == 'Y')
            printf("Macaco %02d passado de Y para X. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);
        else
            printf("Macaco %02d passado de X para Y. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);

        sleep(1);

        // Sai da corda
        pthread_mutex_lock(&lock_corda);
            num_macacosAB--;
            ultimo_lado_aberto = lado_macacosAB;  // Salva o último lado que passou

            printf("Macaco %02d saiu da corda. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);

            if (num_macacosAB == 0) {
                if (lado_macacosAB == 'X')  // Lógica para alternar o lado dos macacos AB
                    lado_macacosAB = 'Y';
                else
                    lado_macacosAB = 'X';
                printf("Todos os macacos AB passaram. Agora eles estão do lado %c\n\n", lado_macacosAB);
            }

        pthread_mutex_unlock(&lock_corda);

        sleep(1);  // Esperar um pouco do outro lado pra não dar ruim
    }
    pthread_exit(0);
}

void *macacoBA(void *a) {
    int i = *((int *)a);
    while (1) {
        while (1) {  // Loop de espera para entrar na corda
            pthread_mutex_lock(&lock_corda);
                if (num_macacosAB == 0 && golira_quer_passar == 0) {
                    num_macacosBA++;
                    pthread_mutex_unlock(&lock_corda);
                    break;  // Sai do loop
                }
            pthread_mutex_unlock(&lock_corda);
            sleep(1);
        }

        if (lado_macacosBA == 'Y')
            printf("Macaco %02d passado de Y para X. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);
        else
            printf("Macaco %02d passado de X para Y. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);

        sleep(1);

        // Sai da corda
        pthread_mutex_lock(&lock_corda);
            num_macacosBA--;
            ultimo_lado_aberto = lado_macacosBA;  // Salva o último lado que passou

            printf("Macaco %02d saiu da corda. Macacos AB: %02d. Macacos BA: %02d\n", i, num_macacosAB, num_macacosBA);

            if (num_macacosBA == 0) {
                if (lado_macacosBA == 'X')  // Lógica para alternar o lado dos macacos BA
                    lado_macacosBA = 'Y';
                else
                    lado_macacosBA = 'X';

                printf("Todos os macacos BA passaram. Agora eles estão do lado %c\n\n", lado_macacosBA);
            }
        pthread_mutex_unlock(&lock_corda);

        sleep(1);  // Esperar um pouco do outro lado
    }
    pthread_exit(0);
}

void *golira(void *a) {
    while (1) {
        while (1) {  // Loop de espera para o gorila atravessar
            golira_quer_passar = 1;
            pthread_mutex_lock(&lock_corda);
                if (num_macacosAB == 0 && num_macacosBA == 0) {
                    pthread_mutex_unlock(&lock_corda);
                    break;  // Sai do loop
                }
            pthread_mutex_unlock(&lock_corda);
            sleep(1);
        }

        pthread_mutex_lock(&lock_corda);
            if (lado_golira == 'X') {
                printf("\n\nGOLIRA PASSANO de X para Y\n\n");
                lado_golira = 'Y';
            } else {
                printf("\n\nGOLIRA PASSANO de Y para X\n\n");
                lado_golira = 'X';
            }
            golira_quer_passar = 0;
        pthread_mutex_unlock(&lock_corda);

        sleep(5);  // Tempo que o gorila leva para atravessar e descansar
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
