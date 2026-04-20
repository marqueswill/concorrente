#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MA 5 //macacos que andam de A para B
#define MB 5 //macacos que andam de B para A


pthread_mutex_t mutexAB = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexBA = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t corda = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t turno = PTHREAD_MUTEX_INITIALIZER;

int AB = 0;
int BA = 0;

void * macacoAB(void * a) {
    int i = *((int *) a);    
    while(1){
         sleep(1);
         //Procedimentos para acessar a corda
         pthread_mutex_lock(&turno);
	 pthread_mutex_lock(&mutexAB);
	 AB++;
	 if(AB == 1){
    		pthread_mutex_lock(&corda);
	}
	pthread_mutex_unlock(&mutexAB);
	pthread_mutex_unlock(&turno);

	printf("Macaco %d passado de A para B \n",i);
	sleep(1);
	
        //Procedimentos para quando sair da corda
        pthread_mutex_lock(&mutexAB);
        AB--;
        printf("Macaco %d terminou de passar de A para B. Num: %d\n" ,i,AB);
	if(AB == 0){
    		pthread_mutex_unlock(&corda);
	}
	pthread_mutex_unlock(&mutexAB);
    }
    pthread_exit(0);
}

void * macacoBA(void * a) {
    int i = *((int *) a);    
    while(1){
        sleep(1);
        //Procedimentos para acessar a corda
	pthread_mutex_lock(&turno);
	pthread_mutex_lock(&mutexBA);
	BA++;
	if(BA == 1){
    	     pthread_mutex_lock(&corda);
	}
	pthread_mutex_unlock(&mutexBA);
	pthread_mutex_unlock(&turno);

        printf("Macaco %d passado de B para A \n",i);
	sleep(1);
	
        //Procedimentos para quando sair da corda
        pthread_mutex_lock(&mutexBA);
        BA--;
        printf("Macaco %d terminou de passar de A para B. Num: %d\n" ,i,BA);
	if(BA == 0){
    		pthread_mutex_unlock(&corda);
	}
	pthread_mutex_unlock(&mutexBA);
    }
    pthread_exit(0);
}



void * gorilaAB(void * a){
    while(1){
        sleep(1);  
	//Procedimentos para acessar a corda
        pthread_mutex_lock(&turno);
        pthread_mutex_lock(&corda);
	printf("Gorila passado de A para B \n");
	sleep(5);
        //Procedimentos para quando sair da corda
        pthread_mutex_unlock(&corda);
        pthread_mutex_unlock(&turno);
     }
    pthread_exit(0);
}



void * gorilaBA(void * a){
    while(1){
        sleep(1);
	//Procedimentos para acessar a corda
        pthread_mutex_lock(&turno);
        pthread_mutex_lock(&corda);
	printf("Gorila passado de B para A \n");
	sleep(5);
        //Procedimentos para quando sair da corda
        pthread_mutex_unlock(&corda);
        pthread_mutex_unlock(&turno);
     }
    pthread_exit(0);
}

int main(int argc, char * argv[])
{

    pthread_t macacos[MA+MB];
    int *id;
    int i = 0;

    for(i = 0; i < MA+MB; i++){
        id = (int *) malloc(sizeof(int));
        *id = i;
        if(i%2 == 0){
          if(pthread_create(&macacos[i], NULL, &macacoAB, (void*)id)){
            printf("Não pode criar a thread %d\n", i);
            return -1;
          }
        }else{
          if(pthread_create(&macacos[i], NULL, &macacoBA, (void*)id)){
            printf("Não pode criar a thread %d\n", i);
            return -1;
          }
        }
    }
    pthread_t gAB, gBA;
    pthread_create(&gAB, NULL, &gorilaAB, NULL);
    pthread_create(&gBA, NULL, &gorilaBA, NULL);

  
    pthread_join(macacos[0], NULL);
    return 0;
}
