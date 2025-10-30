#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MA 10
#define MB 10
#define GA 1
#define GB 1

void *macacoA(void *meuid);
void *macacoB(void *meuid);
void *gorilaA(void *meuid);
void *gorilaB(void *meuid);

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ma_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t mb_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t ga_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t gb_cond = PTHREAD_COND_INITIALIZER;

int mA, mB, gA, gB = 0;

int gaQuer = 0;
int gbQuer = 0;
int maQuer = 0;
int mbQuer = 0;

int turno = 0;  // 0 para macacosA e 1 para macacosB

void main(argc, argv) int argc;
char *argv[];
{
    int erro;
    int i, n, m;
    int *id;

    pthread_t tP[MA];
    for (i = 0; i < MA; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tP[i], NULL, macacoA, (void *)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_t tA[MB];
    for (i = 0; i < MB; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tA[i], NULL, macacoB, (void *)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }
    pthread_t tF[GA];
    for (i = 0; i < GA; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tF[i], NULL, gorilaA, (void *)(id));
        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_t tGB[GB];
    for (i = 0; i < GB; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tGB[i], NULL, gorilaB, (void *)(id));
        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }
    pthread_join(tP[0], NULL);
}

void *macacoA(void *a) {
    int i = *((int *)a);
    int lado_atual = 0;
    while (1) {
        sleep(rand() % (i + 1));
        pthread_mutex_lock(&mutex);

        if (lado_atual == 0)
            maQuer++;
        else
            mbQuer++;

        // Espero se não for o turno do lado em que ele está e se tem macaco do lado B ou gorila na corda
        // Ou se algum gorila quer passar, pq eles tem prioridade
        while (turno != lado_atual || (lado_atual == 0 && mB > 0) || (lado_atual == 1 && mA > 0) || gA > 0 || gB > 0 || gaQuer || gbQuer) {
            pthread_cond_wait(&ma_cond, &mutex);
        }

        if (lado_atual == 0) {
            maQuer--;          // Paro de avisar que quero passar pro outro lado
            mA++;              // Macaco passando de A para B
            if (mbQuer > 0) {  // Se os macacos do lado B querem passar, o próximo turno é deles
                turno = 1;
            }
        } else {
            mbQuer--;
            mB++;
            if (maQuer > 0) {  // Se os macacos do lado A querem passar, o próximo turno é deles
                turno = 0;
            }
        }

        pthread_mutex_unlock(&mutex);

        if (lado_atual == 0)
            printf("Macaco %dA passado de A para B \n", i);
        else
            printf("Macaco %dA passado de B para A \n", i);

        sleep(1);

        pthread_mutex_lock(&mutex);

        if (lado_atual == 0) {
            printf("Macaco %dA terminou de passar de A para B; num: %d\n", i, mA);
            mA--;
        } else {
            printf("Macaco %dA terminou de passar de B para A; num: %d\n", i, mA);
            mB--;
        }

        lado_atual = !lado_atual;  // Inverte o lado que o macaco está (variável local)

        // Se ele passou de A para B e não há macacos indo de A para B
        // Se ele passou de B para A e não há macacos indo de B para A
        if ((lado_atual == 1 && mA == 0) || (lado_atual == 0 && mB == 0)) {
            printf("\nCORDA LIVRE\n\n");
            pthread_cond_broadcast(&mb_cond);
            pthread_cond_signal(&ga_cond);
            pthread_cond_signal(&gb_cond);
        }

        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void *macacoB(void *a) {
    int i = *((int *)a);
    int lado_atual = 1;
    while (1) {
        sleep(rand() % (i + 1));
        pthread_mutex_lock(&mutex);

        if (lado_atual == 0)
            maQuer++;
        else
            mbQuer++;

        while (turno != lado_atual || (lado_atual == 0 && mB > 0) || (lado_atual == 1 && mA > 0) || gA > 0 || gB > 0 || gaQuer || gbQuer) {
            pthread_cond_wait(&mb_cond, &mutex);
        }

        if (lado_atual == 0) {
            maQuer--;          // Paro de avisar que quero passar pro outro lado
            mA++;              // Macaco passando de A para B
            if (mbQuer > 0) {  // Se os macacos do lado B querem passar, o próximo turno é deles
                turno = 1;
            }
        } else {
            mbQuer--;
            mB++;
            if (maQuer > 0) {  // Se os macacos do lado A querem passar, o próximo turno é deles
                turno = 0;
            }
        }

        pthread_mutex_unlock(&mutex);

        if (lado_atual == 0)
            printf("Macaco %dB passado de A para B \n", i);
        else
            printf("Macaco %dB passado de B para A \n", i);

        sleep(1);

        pthread_mutex_lock(&mutex);

        if (lado_atual == 0) {
            printf("Macaco %dB terminou de passar de A para B; num: %d\n", i, mA);
            mA--;
        } else {
            printf("Macaco %dB terminou de passar de B para A; num: %d\n", i, mA);
            mB--;
        }

        lado_atual = !lado_atual;

        if ((lado_atual == 1 && mA == 0) || (lado_atual == 0 && mB == 0)) {
            printf("\nCORDA LIVRE\n\n");
            pthread_cond_broadcast(&ma_cond);
            pthread_cond_signal(&ga_cond);
            pthread_cond_signal(&gb_cond);
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void *gorilaA(void *a) {
    int i = *((int *)a);
    int lado_atual = 0;
    while (1) {
        sleep(rand() % (i + 1) + 10);
        pthread_mutex_lock(&mutex);

        if (lado_atual == 0)
            gaQuer++;
        else
            gbQuer++;

        if (gaQuer || gbQuer)
            printf("EU GORILA %dA PASSAR!!!!!\n", i);
        while (mA > 0 || mB > 0 || gA > 0 || gB > 0) {
            pthread_cond_wait(&ga_cond, &mutex);
        }

        if (lado_atual == 0) {
            gaQuer--;
            gA++;
        } else {
            gbQuer--;
            gB++;
        }

        pthread_mutex_unlock(&mutex);

        if (lado_atual == 0)
            printf("GORILA %dA passado de A para B \n", i);
        else
            printf("GORILA %dA passado de B para A \n", i);

        sleep(5);

        pthread_mutex_lock(&mutex);

        if (lado_atual == 0) {
            gA--;
            printf("GORILA %dA terminou de passar de A para B; num: %d\n", i, gA);
        } else {
            gB--;
            printf("GORILA %dA terminou de passar de B para A; num: %d\n", i, gA);
        }
        lado_atual = !lado_atual;
        printf("\nCORDA LIVRE\n\n");
        pthread_cond_broadcast(&ma_cond);
        pthread_cond_broadcast(&mb_cond);
        pthread_cond_signal(&ga_cond);
        pthread_cond_signal(&gb_cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}

void *gorilaB(void *a) {
    int i = *((int *)a);
    int lado_atual = 1;

    while (1) {
        sleep(rand() % (i + 1) + 10);
        pthread_mutex_lock(&mutex);

        if (lado_atual == 0)
            gaQuer++;
        else
            gbQuer++;

        if (gaQuer || gbQuer)
            printf("EU GORILA %dB PASSAR!!!!!\n", i);
        while (mA > 0 || mB > 0 || gA > 0 || gB > 0) {
            pthread_cond_wait(&gb_cond, &mutex);
        }

        if (lado_atual == 0) {
            gaQuer--;
            gA++;
        } else {
            gbQuer--;
            gB++;
        }

        pthread_mutex_unlock(&mutex);

        if (lado_atual == 0)
            printf("GORILA %dB passado de A para B \n", i);
        else
            printf("GORILA %dB passado de B para A \n", i);

        sleep(5);

        pthread_mutex_lock(&mutex);


        if (lado_atual == 0) {
            gA--;
            printf("GORILA %dB terminou de passar de A para B; num: %d\n", i, gA);
        } else {
            gB--;
            printf("GORILA %dB terminou de passar de B para A; num: %d\n", i, gA);
        }
        lado_atual = !lado_atual;


        printf("\nCORDA LIVRE\n\n");
        pthread_cond_broadcast(&ma_cond);
        pthread_cond_broadcast(&mb_cond);
        pthread_cond_signal(&ga_cond);
        pthread_cond_signal(&gb_cond);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(0);
}
