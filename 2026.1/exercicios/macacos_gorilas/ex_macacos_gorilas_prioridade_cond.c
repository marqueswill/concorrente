#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#define MA 10
#define MB 10
#define GA 1
#define GB 1

#define direcaoAB 0
#define direcaoBA 1

void* macacoA(void* meuid);  // Tem prioridade 2
void* macacoB(void* meuid);  // Tem prioridade 2
void* gorilaA(void* meuid);  // Tem prioridade 1
void* gorilaB(void* meuid);  // Tem prioridade 1

pthread_mutex_t turno = PTHREAD_MUTEX_INITIALIZER;  // Controla entrada e saída da corda
pthread_cond_t ma_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t mb_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t ga_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t gb_cond = PTHREAD_COND_INITIALIZER;

int maCorda, mbCorda, gaCorda, gbCorda = 0;
int gaQuer, gbQuer, maQuer, mbQuer = 0;
int direcao = 0;  // Controla a direção da travessia

int main(int argc, char* argv[]) {
    int erro;
    int i, n, m;
    int* id;

    pthread_t tP[MA];
    for (i = 0; i < MA; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tP[i], NULL, macacoA, (void*)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_t tA[MB];
    for (i = 0; i < MB; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tA[i], NULL, macacoB, (void*)(id));

        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }
    pthread_t tF[GA];
    for (i = 0; i < GA; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tF[i], NULL, gorilaA, (void*)(id));
        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }

    pthread_t tGB[GB];
    for (i = 0; i < GB; i++) {
        id = (int*)malloc(sizeof(int));
        *id = i;
        erro = pthread_create(&tGB[i], NULL, gorilaB, (void*)(id));
        if (erro) {
            printf("erro na criacao do thread %d\n", i);
            exit(1);
        }
    }
    pthread_join(tP[0], NULL);
}

void* macacoA(void* a) {
    int i = *((int*)a);
    while (1) {
        sleep(rand() % (i + 1));
        pthread_mutex_lock(&turno);
        {  // 1) Macaco A tenta atravessar
            // Primeiro verifica se deve esperar
            // While para evitar dessincronização após o signal -> Verifica estado das variáveis antes de prosseguir
            maQuer++;
            while (                                             // Enquanto
                (direcao == direcaoBA) ||                       // a direção é B -> A
                (mbCorda > 0 || gaCorda > 0 || gbCorda > 0) ||  // ou ainda há macacos de outro tipo na corda
                (gbQuer > 0 || gaQuer > 0)) {                   // ou gorilas querem passar (prioridade)
                pthread_cond_wait(&ma_cond, &turno);            // os macacos A esperam por permissão e deixam outro primata tentar atravessar
            }
            maQuer--;

            // 2) Verifica se o lado B quer atravessar
            // Usado para evitar starvation
            if (mbQuer > 1) {         // Se macacos querem ir B para A
                direcao = direcaoBA;  // Ele indica que o proximo turno é do lado B
            }

            // 3) Macaco A entra na corda
            maCorda++;
        }
        pthread_mutex_unlock(&turno);

        printf("Macaco %dA atravessando.\n", i);
        sleep(1);

        pthread_mutex_lock(&turno);
        {
            // 1) Macaco sai da corda, um por vez
            maCorda--;
            printf("Macaco %dA terminou de atravessar.\n", i);

            // 2) Se o fluxo de um lado pro outro acabar, eu libero para os demais
            if (maCorda == 0) {
                pthread_cond_signal(&ga_cond);
                pthread_cond_signal(&gb_cond);
                pthread_cond_broadcast(&mb_cond);
            }
        }
        pthread_mutex_unlock(&turno);
    }
    pthread_exit(0);
}

void* macacoB(void* a) {
    int i = *((int*)a);
    while (1) {
        sleep(rand() % (i + 1));
        pthread_mutex_lock(&turno);
        {  // 1) Macaco B tenta atravessar
           // Primeiro verifica se a corda tá disponível
            mbQuer++;
            while (                                             // Enquanto
                (direcao == direcaoAB) ||                       // a direção é A -> B
                (maCorda > 0 || gaCorda > 0 || gbCorda > 0) ||  // ou ainda há primatas na corda
                (gbQuer > 0 || gaQuer > 0)) {                   // ou gorilas querem passar
                pthread_cond_wait(&mb_cond, &turno);            // os macacos A esperam por permissão e deixam outro primata tentar atravessar
            }
            mbQuer--;

            // 2) Verifica se o lado A quer atravessar
            if (mbQuer > 1) {         // Se macacos querem ir A para B
                direcao = direcaoAB;  // Ele indica que o proximo turno é do lado A
            }

            // 3) Macaco A entra na corda
            mbCorda++;
        }
        pthread_mutex_unlock(&turno);

        printf("Macaco %dB atravessando.\n", i);
        sleep(1);

        pthread_mutex_lock(&turno);
        {
            mbCorda--;
            printf("Macaco %dB terminou de atravessar.\n", i);
            if (mbCorda == 0) {
                pthread_cond_signal(&ga_cond);
                pthread_cond_signal(&gb_cond);
                pthread_cond_broadcast(&ma_cond);
            }
        }
        pthread_mutex_unlock(&turno);
    }
    pthread_exit(0);
}

void* gorilaA(void* a) {
    int i = *((int*)a);
    while (1) {
        sleep(rand() % (i + 1) + 5);

        pthread_mutex_lock(&turno);
        {
            gaQuer++;
            printf("GORILA A QUER PASSAR!!!\n");
            while ((direcao == direcaoBA) ||
                   (maCorda > 0 || mbCorda > 0) ||  // Se ainda tem macaco passando
                   (gaCorda > 0 || gbCorda > 0)) {  // Se ainda tem gorila passando (1 por vez)
                pthread_cond_wait(&ga_cond, &turno);
            }
            gaQuer--;

            // if (mbQuer || gbQuer) {
            // direcao = direcaoBA;
            // }

            gbCorda++;
        }
        pthread_mutex_unlock(&turno);

        printf("\nGorila A atravessando.\n", i);
        sleep(1);

        pthread_mutex_lock(&turno);
        {
            gbCorda--;
            printf("Gorila A terminou de atravessar.\n\n");
            pthread_cond_signal(&ga_cond);
            pthread_cond_signal(&gb_cond);
            pthread_cond_broadcast(&ma_cond);
            pthread_cond_broadcast(&mb_cond);
        }
        pthread_mutex_unlock(&turno);
    }
    pthread_exit(0);
}

void* gorilaB(void* a) {
    int i = *((int*)a);
    while (1) {
        sleep(rand() % (i + 1) + 5);

        pthread_mutex_lock(&turno);
        {
            gbQuer++;
            printf("GORILA B QUER PASSAR!!!\n");
            while ((direcao == direcaoAB) ||
                   (maCorda > 0 || mbCorda > 0 || gaCorda > 0 || gbCorda > 0)) {
                pthread_cond_wait(&gb_cond, &turno);
            }
            gbQuer--;

            // if (maQuer || gaQuer) {
            // direcao = direcaoAB;
            // }

            gbCorda++;
        }
        pthread_mutex_unlock(&turno);

        printf("\nGorila B atravessando.\n", i);
        sleep(1);

        pthread_mutex_lock(&turno);
        {
            gbCorda--;
            printf("Gorila B terminou de atravessar.\n\n");
            pthread_cond_signal(&ga_cond);
            pthread_cond_signal(&gb_cond);
            pthread_cond_broadcast(&ma_cond);
            pthread_cond_broadcast(&mb_cond);
        }
        pthread_mutex_unlock(&turno);
    }
    pthread_exit(0);
}