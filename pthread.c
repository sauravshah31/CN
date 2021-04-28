#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>


struct arg{
    int id;
};

void* print_id(void *inp){
    struct arg *args = (struct arg*) inp;
    printf("Thread %d\n",args->id);

    return NULL;
}

int main(){
    int nthreads,i;
    pthread_t *threads;
    struct arg *args;

    printf("No of threads : ");
    scanf("%d",&nthreads);

    /*
    int pthread_create(pthread_t *thread, 
                        const pthread_attr_t *attr,
                        void *(*start_routine) (void *), 
                        void *arg);
    */
    threads = (pthread_t*)malloc(sizeof(pthread_t)*nthreads);
    args = (struct arg*)malloc(sizeof(struct arg)*nthreads);
    for(i=0;i<nthreads;i++){
        struct arg temp;
        temp.id=i+1;
        printf("%p\n",&temp);
        args[i].id = i+1;
        pthread_create(&threads[i], NULL, print_id, &args[i]);
    }

    //join the threads
    /*int pthread_join(pthread_t thread, void **retval);*/
    for(i=0;i<nthreads;i++)
        pthread_join(threads[i],NULL);


}