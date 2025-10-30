#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 10 // macacos que andam de A para B
#define MB 10 // macacos que andam de B para A

pthread_mutex_t lock_corda = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_macacosAB = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_macacosBA = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond_gorila = PTHREAD_COND_INITIALIZER;

int num_macacosAB = 0;
int num_macacosBA = 0;
int gorila_presente = 0; // 0: ausente, 1: quer passar

char direcao_corda = 'N'; // 'A': A->B, 'B': B->A, 'N': nenhum

void *macacoAB(void *a) {
    int id = *((int *)a);
    while (1) {
        pthread_mutex_lock(&lock_corda);

            // Esperar enquanto a direção da corda for B->A ou o gorila estiver presente
            while ((direcao_corda == 'B' && num_macacosBA > 0) || gorila_presente == 1) {
                pthread_cond_wait(&cond_macacosAB, &lock_corda);
            }

            direcao_corda = 'A'; // Muda a direção da corda para A->B
            num_macacosAB++;
            
            printf("Macaco %02d (AB) entrou na corda. Macacos AB: %02d. Macacos BA: %02d\n", id, num_macacosAB, num_macacosBA);
            
        pthread_mutex_unlock(&lock_corda);
        
        sleep(1); // Tempo de travessia
        
        pthread_mutex_lock(&lock_corda);
            num_macacosAB--;
            printf("Macaco %02d (AB) saiu da corda. Macacos AB: %02d. Macacos BA: %02d\n", id, num_macacosAB, num_macacosBA);
            
            if (num_macacosAB == 0) {
                direcao_corda = 'N';
                if (gorila_presente == 1) {
                    pthread_cond_signal(&cond_gorila);
                } else {
                    pthread_cond_broadcast(&cond_macacosBA);
                }
            }
        pthread_mutex_unlock(&lock_corda);
        sleep(1);
    }
    pthread_exit(0);
}

void *macacoBA(void *a) {
    int id = *((int *)a);
    while (1) {
        pthread_mutex_lock(&lock_corda);

        while ((direcao_corda == 'A' && num_macacosAB > 0) || gorila_presente == 1) {
            pthread_cond_wait(&cond_macacosBA, &lock_corda);
        }

        direcao_corda = 'B';
        num_macacosBA++;
        
        printf("Macaco %02d (BA) entrou na corda. Macacos AB: %02d. Macacos BA: %02d\n", id, num_macacosAB, num_macacosBA);
        
        pthread_mutex_unlock(&lock_corda);
        
        sleep(1); // Tempo de travessia
        
        pthread_mutex_lock(&lock_corda);
        
        num_macacosBA--;
        printf("Macaco %02d (BA) saiu da corda. Macacos AB: %02d. Macacos BA: %02d\n", id, num_macacosBA, num_macacosBA);
        
        if (num_macacosBA == 0) {
            direcao_corda = 'N';
            if (gorila_presente == 1) {
                pthread_cond_signal(&cond_gorila);
            } else {
                pthread_cond_broadcast(&cond_macacosAB);
            }
        }
        
        pthread_mutex_unlock(&lock_corda);
        sleep(1);
    }
    pthread_exit(0);
}

void *gorila(void *a) {
    while (1) {
        sleep(5); // Tempo para o gorila chegar
        pthread_mutex_lock(&lock_corda);
        
        gorila_presente = 1;
        printf("\nO gorila quer atravessar! Esperando a corda ficar livre...\n");
        
        while (num_macacosAB > 0 || num_macacosBA > 0) {
            pthread_cond_wait(&cond_gorila, &lock_corda);
        }
        
        printf("\n\nGORILA ATRAVESSANDO!\n\n");
        
        gorila_presente = 0;
        
        pthread_mutex_unlock(&lock_corda);
        
        sleep(5); // Tempo de travessia do gorila
        
        pthread_mutex_lock(&lock_corda);
        printf("\nO gorila terminou. Liberando a corda para os macacos.\n\n");
        
        // Acorda macacos que podem estar esperando
        pthread_cond_broadcast(&cond_macacosAB);
        pthread_cond_broadcast(&cond_macacosBA);
        
        pthread_mutex_unlock(&lock_corda);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    pthread_t macacos[MA + MB];
    int *id;

    // Cria as threads de macacos
    for (int i = 0; i < MA + MB; i++) {
        id = (int *)malloc(sizeof(int));
        *id = i;
        if (i % 2 == 0) {
            pthread_create(&macacos[i], NULL, macacoAB, (void *)id);
        } else {
            pthread_create(&macacos[i], NULL, macacoBA, (void *)id);
        }
    }
    
    // Cria a thread do gorila
    pthread_t g;
    pthread_create(&g, NULL, gorila, NULL);
    
    pthread_join(macacos[0], NULL); // Espera por uma thread para manter o main vivo
    
    // Limpeza de recursos (na prática, não será alcançado)
    for (int i = 0; i < MA + MB; i++) {
        pthread_cancel(macacos[i]);
    }
    pthread_cancel(g);

    return 0;
}