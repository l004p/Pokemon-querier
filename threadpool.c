#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "threadpool.h"

//these numbers can be adjusted.
#define THREADNUM 2
#define TOTALTASK 25

struct task{
    void (*jobFunc)(void *, void *);
    void *taskArg1, *taskArg2;
};

struct threadattr{
    int num;
    int keepRunning;
    volatile int taskCount;
    task_t taskQueue[TOTALTASK];
    pthread_mutex_t mutexQueue;
    pthread_cond_t cond;
    
};

struct threadpool{
    pthread_t threads[THREADNUM];
    struct threadattr *attrs;
};


threadpool_t *createThreadpool(){
    threadpool_t *pool;
    //allocate memory
    pool = malloc(sizeof(*pool));
    if(pool == NULL){
        printf("error allocating threadpool\n");
        return NULL;
    }
    //set attributes and make threads
    pool->attrs = createThreadattr();
    for(int i = 0; i < THREADNUM; i++){
        pthread_create(&pool->threads[i], NULL, &threadWait, pool->attrs);
    }
    return pool;
}

//initialize threadpool
threadattr_t *createThreadattr(){
    struct threadattr *attrs;
    //allocate memory
    attrs = malloc(sizeof(struct threadattr));
    if(attrs == NULL){
        printf("error allocating attributes\n");
        return NULL;
    }
    //set attributes
    attrs->num = THREADNUM;
    attrs->keepRunning = 1;
    attrs->taskCount = 0;
    pthread_mutex_init(&attrs->mutexQueue, NULL);
    pthread_cond_init(&attrs->cond, NULL);
    return attrs;
}


void queueTask(threadpool_t *pool, void (*taskFunc)(void *, void *), void *varg1, void *varg2){
    task_t newTask = {
        .jobFunc = taskFunc,
        .taskArg1 = varg1,
        .taskArg2 = varg2,
    };
    pthread_mutex_lock(&pool->attrs->mutexQueue);
    //add task to queue
    if(pool->attrs->taskCount < TOTALTASK){
        pool->attrs->taskQueue[pool->attrs->taskCount] = newTask;
        pool->attrs->taskCount++;
    }
    else{
        //highly unlikely the user would queue a high number of tasks in this circumstance so this is the only simple error handling implemented
        printf("queued max tasks... wait and try again");
    }
    pthread_mutex_unlock(&pool->attrs->mutexQueue);
    pthread_cond_signal(&pool->attrs->cond);//signal threads, will cause threads waiting in threadwait to do stuff again
}

//main threadpool loop
//here is where they wait for threads
void * threadWait(void * varg){
    threadattr_t *attr = (threadattr_t *)varg;//attributes of the threads... ie mutex etc
    while(attr->keepRunning == 1){
        task_t temp;
        //lock the queue
        pthread_mutex_lock(&attr->mutexQueue);
        //while there is no task and it is still running
        while(attr->taskCount == 0 && attr->keepRunning == 1){
            //threads wait for some signal to do the code after this part (signal comes when task added to queue)
            pthread_cond_wait(&attr->cond, &attr->mutexQueue);
        }
        //put the task in the temp and then adjust the queue and decrement the task counter
        if(attr->keepRunning == 1){
            temp = attr->taskQueue[0];
            for(int i = 0; i < attr->taskCount-1; i++){
                attr->taskQueue[i] = attr->taskQueue[i+1];
            }
            attr->taskCount--;
            pthread_mutex_unlock(&attr->mutexQueue);
            doTask(&temp);//do the task
        }
        else{
            pthread_mutex_unlock(&attr->mutexQueue);
            break;
        }
    }
}

void exitQueue(threadpool_t *pool){
    pthread_mutex_lock(&pool->attrs->mutexQueue);
    pool->attrs->keepRunning = 0;
    pthread_mutex_unlock(&pool->attrs->mutexQueue);
    pthread_cond_broadcast(&pool->attrs->cond);//signal all threads to unblock
}

//do the task in the queue
void doTask(task_t * t){
    t->jobFunc(t->taskArg1, t->taskArg2);
}

//end the threadpool
void killThreadpool(threadpool_t *pool){
    exitQueue(pool);
    //finish the threads
    for(int i = 0; i < THREADNUM; i++){
        pthread_join(pool->threads[i], NULL);
    }
    pthread_mutex_destroy(&pool->attrs->mutexQueue);
    pthread_cond_destroy(&pool->attrs->cond);
    free(pool->attrs);
    free(pool);
}

