#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <semaphore.h>
#include <unistd.h>

#define NUMCONS 5
#define NUMPROD 2
#define TAM 10

pthread_t cons[NUMCONS];
pthread_t prod[NUMPROD];
int buffer[TAM];
int prod_pos=0;
int cons_pos = 0;
pthread_mutex_t buffer_mutex;
sem_t posicoes_livres;
sem_t posicoes_ocupadas;

void * produtor(void *arg);
void * consumidor(void *arg);

int main(int argc, char **argv){
    int i;

    srand48(time(NULL));
    pthread_mutex_init(&buffer_mutex,NULL);
    sem_init(&posicoes_livres,0,TAM);    
    sem_init(&posicoes_ocupadas,0,0);

    int *id;
    for(i = 0; i < NUMCONS;i++){
        id = (int *) malloc(sizeof(int));
        *id = i;
	pthread_create(&(cons[i]),NULL,consumidor, (void*) id);
    }

    for(i = 0; i < NUMPROD;i++){
        id = (int *) malloc(sizeof(int));
        *id = i;
	pthread_create(&(prod[i]),NULL,produtor, (void*) id);
    }

    for(i = 0; i < NUMCONS;i++){
	pthread_join(cons[i],NULL);
    }
 
    for(i = 0; i < NUMPROD;i++){
	pthread_join(prod[i],NULL);
    }

}


void * produtor(void *arg){
    int id = *((int *) arg);
    int item;
    while(1){
		item = (int) (drand48() * 1000.0);
		sleep(2);

		sem_wait(&posicoes_livres);
		        pthread_mutex_lock(&buffer_mutex); 		
				buffer[prod_pos] = item;
				printf("Produtor %d, Produzindo numero %d na posicao %d\n",id, item,prod_pos);
				prod_pos = (prod_pos+1)%TAM;
			pthread_mutex_unlock(&buffer_mutex);
		sem_post(&posicoes_ocupadas);
    }
}

void * consumidor(void *arg){
     int id = *((int *) arg);
     //printf("%d \n",id);
    int item;
    while(1){
                sem_wait(&posicoes_ocupadas);
			pthread_mutex_lock(&buffer_mutex);
				item = buffer[cons_pos];
				printf("               Consumidor %d, Consumindo numero %d na posicao %d \n",id,item,cons_pos);
				cons_pos = (cons_pos+1)%TAM;
			pthread_mutex_unlock(&buffer_mutex);
		sem_post(&posicoes_livres);

		sleep(10);
    }
}





















