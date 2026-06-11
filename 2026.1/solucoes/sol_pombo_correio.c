#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "unistd.h"

#define N 12 //número de usuários

#define CAPACIDADE 20 //capacidade da mochila

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t pombo_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t usuarios_cond = PTHREAD_COND_INITIALIZER;

int cartas = 0;

void * f_usuario(void *arg);
void * f_pombo(void *arg);

int main(int argc, char **argv){
    int i;

    pthread_t usuario[N];
    int *id;
    for(i = 0; i < N; i++){
        id = (int *) malloc(sizeof(int));
        *id = i;
	pthread_create(&(usuario[i]),NULL,f_usuario,  (void *) (id));
    }
    pthread_t pombo;
    pthread_create(&(pombo),NULL,f_pombo, NULL);
    pthread_join(pombo,NULL);
}


void * f_pombo(void *arg){
  
    while(1){
        //Inicialmente está em A, aguardar/dorme a mochila ficar cheia (20 cartas)
        pthread_mutex_lock(&mutex);
		while(cartas < CAPACIDADE){
		    pthread_cond_wait(&pombo_cond,&mutex);
		}
		printf("Pombo vai levar as cartas para B: %d \n",cartas);
                sleep(5);
                cartas = 0;
                printf("Pombo voltou para A\n");
                pthread_cond_broadcast(&usuarios_cond);
       pthread_mutex_unlock(&mutex); 
    }
}

void * f_usuario(void *arg){
    int id = *((int *) arg);
    while(1){
	//Escreve uma carta
        sleep(2);
        //Caso o pombo não esteja em A ou a mochila estiver cheia, então dorme
        
        pthread_mutex_lock(&mutex);
            while(cartas == CAPACIDADE){
                pthread_cond_wait(&usuarios_cond,&mutex);
            }
            cartas++; 
            printf("Usuario %d postou uma carta: %d\n",id,cartas);
            
            sleep(1);
            if (cartas == CAPACIDADE){
                pthread_cond_signal(&pombo_cond);
             }
        pthread_mutex_unlock(&mutex);	
        
    }
}
