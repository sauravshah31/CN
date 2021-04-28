/*
    increment value of global variable counter using pthread_mutex
*/
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

#define NTHREADS 12345

int counter;
pthread_mutex_t mut;


void* thread_unsafe_increment(void *arg){
    sleep(*((float*)arg));
    counter++;
}

void* thread_safe_increment(void *arg){
    sleep(*((float*)arg));
    pthread_mutex_lock(&mut);
    counter++;
    pthread_mutex_unlock(&mut);
}

int main(){
    int i;
    pthread_t threads[NTHREADS];
    pthread_mutex_init(&mut, NULL);
    float args[NTHREADS];
    float curr = 0.5;

    //without mutex
    counter = 0;
    for(i=0; i<NTHREADS; i++){
        args[i] = curr;
        pthread_create(&threads[i],NULL,thread_unsafe_increment,&args[i]);
    }
    for(i=0; i<NTHREADS; i++){
        pthread_join(threads[i],NULL);
    }

    printf("Value of counter without mutex\n");
    printf("Expected : %d\n",NTHREADS);
    printf("Got : %d\n\n",counter);

    //with mutex
    counter = 0;
    for(i=0; i<NTHREADS; i++){
        args[i] = curr;
        pthread_create(&threads[i],NULL,thread_safe_increment,&args[i]);
    }
    for(i=0; i<NTHREADS; i++){
        pthread_join(threads[i],NULL);
    }

    printf("Value of counter with mutex\n");
    printf("Expected : %d\n",NTHREADS);
    printf("Got : %d\n\n",counter);
}