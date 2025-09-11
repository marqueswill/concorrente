#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MX 10  // macacos que andam de A para B
#define MY 10  // macacos que andam de B para A

#define A 0
#define B 1

pthread_mutex_t lock_corda = PTHREAD_MUTEX_INITIALIZER;

int num_macacosX = 0;
int num_macacosY = 0;
int golira_quer_passar = 0;
int jare_na_ponte = 0;
// 0: A->B tá liberado, 1: B->A tá liberado
int lado_aberto = A;  // A->B tá liberado, B->A tá bloqueado
int lado_golira = A;  // lado em que o golira está
int lado_macacosX = A;
int lado_macacosY = B;
char ultimo_a_passar = 'Y';

void *macacoX(void *a) {
    int i = *((int *)a);
    while (1) {  // Loop principal do macaco

        while (1) {
            pthread_mutex_lock(&lock_corda);  // Loop de espera para entrar na corda, as threads ficam bloqueando e desbloqueando em loop
            // Macaco X pode passar se:
            // Ponte não tem macacos Y, lado oposto está bloqueado, gorila não quer passar, quem passou por último foi o grupo Y
            // Eu não preciso controlar quem passa, só quem passou por último
            if (num_macacosY == 0 && ultimo_a_passar == 'Y' && golira_quer_passar == 0 && lado_macacosX == lado_aberto && jare_na_ponte == 0) {
                num_macacosX++;                     // O controle do último lado aberto faz com que as threads X e Y se alternem, evitando que um lado fique preso esperando o outro acabar. Funciona por conta da contagem de macacos
                pthread_mutex_unlock(&lock_corda);  // Libera pra geral que tava esperando
                break;

                // while (1) {
                //     if (num_macacosX == 10)  // Espera todos os macacos X começarem passar, chance de condição de corrida aqui
                //         break;
                //     pthread_mutex_unlock(&lock_corda);  // Libero a corda para os macacos passarem
                // }

                // ultimo_a_passar = 'X';
                // break;  // Sai do loop, agora as trheads X podem continuar
            }
            pthread_mutex_unlock(&lock_corda);
            sleep(1);
        }

        if (lado_macacosX == A)
            printf("Macaco %02dX passado de A para B. Macacos X: %02d. Macacos Y: %02d\n", i, num_macacosX, num_macacosY);
        else
            printf("Macaco %02dX passado de B para A. Macacos X: %02d. Macacos Y: %02d\n", i, num_macacosX, num_macacosY);

        sleep(1);

        // Sai da corda
        pthread_mutex_lock(&lock_corda);  // protege num_macacosX
        num_macacosX--;
        printf("Macaco %02dX saiu da corda. Macacos X: %02d. Macacos Y: %02d\n", i, num_macacosX, num_macacosY);
        if (num_macacosX == 0) {
            lado_aberto = !lado_aberto;      // Depois que todos passarem, lado A é bloqueado
            lado_macacosX = !lado_macacosX;  // Inverte o lado
            ultimo_a_passar = 'X';
            printf("Quem passou por último: %c, X:%s, Y:%s, Aberto:%s\n",
                   ultimo_a_passar,
                   lado_macacosX == 0 ? "A" : "B",
                   lado_macacosY == 0 ? "A" : "B",
                   lado_aberto == 0 ? "A" : "B");
        } else {
            // printf("Esperando os X\n");
            pthread_mutex_unlock(&lock_corda);
            while (num_macacosX > 0) {
                sleep(1);
            }
            ultimo_a_passar = 'X';  // Só pra garantir
        }

        pthread_mutex_unlock(&lock_corda);

        sleep(1);  // Esperar um pouco do outro lado pra não dar ruim
    }
    pthread_exit(0);
}

void *macacoY(void *a) {
    int i = *((int *)a);
    while (1) {  // Loop principal do macaco

        while (1) {
            pthread_mutex_lock(&lock_corda);  // Loop de espera para entrar na corda, as threads ficam bloqueando e desbloqueando em loop
            // Macaco X pode passar se:
            // Ponte não tem macacos Y, lado oposto está bloqueado, gorila não quer passar, quem passou por último foi o grupo Y
            // Eu não preciso controlar quem passa, só quem passou por último

            if (num_macacosX == 0 && ultimo_a_passar == 'X' && golira_quer_passar == 0 && lado_macacosY == lado_aberto && jare_na_ponte == 0) {
                num_macacosY++;                     // O controle do último lado aberto faz com que as threads X e Y se alternem, evitando que um lado fique preso esperando o outro acabar. Funciona por conta da contagem de macacos
                pthread_mutex_unlock(&lock_corda);  // Libera pra geral que tava esperando
                break;

                // while (1) {
                //     if (num_macacosY == 10)  // Espera todos os macacos X começarem passar, chance de condição de corrida aqui
                //         break;
                //     pthread_mutex_unlock(&lock_corda);  // Libero a corda para os macacos passarem
                // }

                // ultimo_a_passar = 'Y';
                // break;  // Sai do loop, agora as trheads X podem continuar
            }
            pthread_mutex_unlock(&lock_corda);
            sleep(1);
        }

        if (lado_macacosY == A)
            printf("Macaco %02dY passado de A para B. Macacos X: %02d. Macacos Y: %02d\n", i, num_macacosX, num_macacosY);
        else
            printf("Macaco %02dY passado de B para A. Macacos X: %02d. Macacos Y: %02d\n", i, num_macacosX, num_macacosY);

        sleep(1);

        // Sai da corda
        pthread_mutex_lock(&lock_corda);
        num_macacosY--;
        printf("Macaco %02dY saiu da corda. Macacos X: %02d. Macacos Y: %02d\n", i, num_macacosX, num_macacosY);
        if (num_macacosY == 0) {
            lado_aberto = !lado_aberto;  // Depois que todos passarem, lado A é bloqueado
            lado_macacosY = !lado_macacosY;
            ultimo_a_passar = 'Y';
            printf("Quem passou por último: %c, X:%s, Y:%s, Aberto:%s\n",
                   ultimo_a_passar,
                   lado_macacosX == 0 ? "A" : "B",
                   lado_macacosY == 0 ? "A" : "B",
                   lado_aberto == 0 ? "A" : "B");
        } else {
            // printf("Esperando os Y\n");
            pthread_mutex_unlock(&lock_corda);  // Deixa os resto continuar na corda
            while (num_macacosY > 0) {
                sleep(1);
            }
            ultimo_a_passar = 'Y';  // Só pra garantir
        }

        pthread_mutex_unlock(&lock_corda);

        sleep(1);  // Esperar um pouco do outro lado pra não dar ruim
    }
    pthread_exit(0);
}

void *golira(void *a) {
    while (1) {
        while (1) {  // Loop de espera para o gorila atravessar
            golira_quer_passar = 1;
            pthread_mutex_lock(&lock_corda);
            if (num_macacosX == 0 && num_macacosY == 0 && jare_na_ponte == 0) {
                pthread_mutex_unlock(&lock_corda);
                break;  // Sai do loop
            }
            pthread_mutex_unlock(&lock_corda);
            sleep(1);
        }

        // O golira consegue desfazer deadlocks
        pthread_mutex_lock(&lock_corda);
        if (lado_golira == A) {
            printf("GOLIRA PASSANO de A para B\n");
            lado_golira = B;
            lado_aberto = B;
        } else {
            printf("GOLIRA PASSANO de B para A\n");
            lado_golira = A;
            lado_aberto = A;
        }
        golira_quer_passar = 0;
        pthread_mutex_unlock(&lock_corda);

        sleep(5);  // Tempo que o gorila leva para atravessar e descansar
    }
    pthread_exit(0);
}

void *jagaré(void *a) {
    while (1) {
        if (jare_na_ponte)
            printf("JARÉ NA PONTE!!!! \n");
        else
            printf("PODE PASSAR!!!\n");
        sleep(7);
        jare_na_ponte = !jare_na_ponte;
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    pthread_t j;
    pthread_create(&j, NULL, &jagaré, NULL);

    pthread_t g;
    pthread_create(&g, NULL, &golira, NULL);

    pthread_t macacos[MX + MY];
    int *id;
    int i = 0;

    for (i = 0; i < MX + MY; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        if (i % 2 == 0) {
            if (pthread_create(&macacos[i], NULL, &macacoX, (void *)id)) {
                printf("Não pode criar a thread %02d\n", i);
                return -1;
            }
        } else {
            if (pthread_create(&macacos[i], NULL, &macacoY, (void *)id)) {
                printf("Não pode criar a thread %02d\n", i);
                return -1;
            }
        }
    }

    pthread_join(macacos[0], NULL);
    return 0;
}
