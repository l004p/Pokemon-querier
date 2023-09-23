#ifndef THREADPOOL_HEADER
#define THREADPOOL_HEADER

//forward declartions of typedef structs
typedef struct task task_t;
typedef struct threadattr threadattr_t;
typedef struct threadpool threadpool_t;

//initialize threadpool
threadpool_t * createThreadpool();
//initialize attributes of threadpool
threadattr_t * createThreadattr();
//add task to threadpool queue
void queueTask(threadpool_t *pool, void (*taskFunc)(void *, void *), void *varg1, void *varg2);
//function for waiting to do a task
void * threadWait(void * varg);
//exit the queue
void exitQueue(threadpool_t *pool);
//do task in queue
void doTask(task_t * t);
//end the threadpool
void killThreadpool(threadpool_t *threadpool);

#endif
