#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "threadpool.h"
#include "llist.h"
#include "clientHelpers.h"

//ip and port
#define SERV_IP "127.0.0.1"
#define SERV_PORT 6000


//pokemon client
struct pokeClient{
    volatile int completedQueries;//number of completed queries
    pthread_mutex_t listLock;//mutex for query related data (number of queries, query list)
    pthread_mutex_t saveLock;//mutex for save related stuff (preventing two threads from trying to save to the same file)
    threadpool_t *savePool;//threads for saving
    threadpool_t *listPool;//threads for sending and recieving queries
    node_t *queryList;
    node_t *titleList;
    int clientSocket;
    struct sockaddr_in serverAddress;
    int status, bytesRcv;

};

//initialize client
pokeClient_t * createPokeClient(){
    pokeClient_t *client;
    client = malloc(sizeof(*client));
    if(client==NULL){
        printf("CLIENT: error allocating self\n");
        return NULL;
    }
    client->completedQueries = 0;
    pthread_mutex_init(&client->listLock, NULL);//initialize mutexes
    pthread_mutex_init(&client->saveLock, NULL);
    client->savePool = createThreadpool();
    if(client->savePool==NULL){
        printf("Error making client savePool\n");
        return NULL;
    }
    client->listPool = createThreadpool();
    if(client->listPool==NULL){
        printf("Error making client listPool\n");
        return NULL;
    }
    client->queryList = initLList();
    client->titleList = initLList();
    memset(&client->serverAddress, 0, sizeof(client->serverAddress));
    if((client->clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("*** CLIENT ERROR: Could not open socket.\n");
        return NULL;
    }
    client->serverAddress.sin_family = AF_INET;
    client->serverAddress.sin_addr.s_addr = inet_addr(SERV_IP);
    client->serverAddress.sin_port = htons((unsigned short) SERV_PORT);
    if(connect(client->clientSocket, (struct sockaddr*)&client->serverAddress, sizeof(client->serverAddress))<0){
        printf("*** CLIENT ERROR: Could not connect.\n");
        return NULL;
    }
    printf("CLIENT: succesful created and connected to server\n");
    return client;
}

//queue query in threadpool
void queueQuery(pokeClient_t *client, char inStr[30]){
    queueTask(client->listPool, &query, inStr, client);
}

//queue saving in threadpool
void queueSave(pokeClient_t *client, char inStr[30]){
    queueTask(client->savePool, &saveFile, inStr, client);
}

//query function
void query(void *varg1, void * varg2){
    char f[30] = "\0";
    strcpy(f, (char*)varg1);
    pokeClient_t *c = (pokeClient_t*)varg2;
    char rec[150];
    char inLine[150]="\0";
    printf("CLIENT: sending query to server: %s\n", f);
    send(c->clientSocket, f, 30, 0);
    //locks the list and receives data to add to query list until done, unlocks the list
    pthread_mutex_lock(&c->listLock);
    while(strcmp(inLine,"done")!=0){
        recv(c->clientSocket, rec, 150, 0);
        strcpy(inLine,rec);
        if(strcmp(inLine,"done")!=0){
            addEntry(&c->queryList, initData(inLine));
        }
    }
    c->completedQueries++;//increase number of completed queries
    pthread_mutex_unlock(&c->listLock);
}

//print the queries
void printQueries(pokeClient_t *client){
    pthread_mutex_lock(&client->listLock);
    printf("printing:\n");
    printLList(client->queryList);
    pthread_mutex_unlock(&client->listLock);
}

//save function takes three parameters
void saveFile(void * varg1, void * varg2){
    pokeClient_t *c = (pokeClient_t*)varg2;
    //mutex here is to protect multiple files of the same name to be written to simultaneously
    pthread_mutex_lock(&c->saveLock);
    char title[30]= {};
    strcpy(title, (char*)varg1);
    addEntry(&c->titleList, initData(title));
    FILE *t = fopen(title, "w");
    if(t==NULL){
        printf("CLIENT ERROR: could not open file\n");
    }
    //this mutex is to ensure the linked list of queries is not being used by another thread while it is trying to be saved
    pthread_mutex_lock(&c->listLock);
    saveLList(c->queryList, t);
    pthread_mutex_unlock(&c->listLock);
    fclose(t);
    pthread_mutex_unlock(&c->saveLock);
}

//close the client
void closePokeClient(pokeClient_t *client){
    printf("CLIENT: exiting...\n");
    char done[5];
    strcpy(done, "done");
    send(client->clientSocket, done, 30, 0);//tell the server the client is done so it can shut down
    printf("server Shutdown\n");
    killThreadpool(client->listPool);//close the threadpools
    killThreadpool(client->savePool);
    printf("CLIENT: total number of queries: %d\n", client->completedQueries);
    printf("CLIENT: all titles of files made:\n");
    printLList(client->titleList);
    //finish destroying and freeing memory etc
    pthread_mutex_destroy(&client->listLock);
    pthread_mutex_destroy(&client->saveLock);
    destroyLList(client->queryList);
    destroyLList(client->titleList);
    close(client->clientSocket);
    free(client);
}

