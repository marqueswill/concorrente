#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


void * routine(){
    printf("1, 2, 3... Testing thread %ld\n", pthread_self());
    pthread_exit(0);
    
}

int main(int argc, char*argv[]){
    pthread_t t1;

    pthread_create(&t1, NULL, &routine, NULL);

    pthread_join(t1, NULL);

    return 0;

}