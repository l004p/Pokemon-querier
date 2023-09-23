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
#include "serverHelpers.h"

//port number
#define PORT 6000
#define NUMTYPES 18

//pokemon server struct
struct pokeServer{
    FILE *fp;//file pointer
    pthread_mutex_t queryLock;//lock for reading and sending (to prevent two queries from sending out of order)
    threadpool_t *pool;//threads
    int clientSocket, serverSocket;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t addressSize;
    int bytesRcv;
    const char *types[NUMTYPES];
};


//create the server
pokeServer_t * createPokeServer(){
    pokeServer_t *server;
    //allocate and check allocation
    server = malloc(sizeof(*server));
    if(server==NULL){
        printf("*** SERVER: error allocating self");
        return NULL;
    }
    server->fp=NULL;//make file pointer null
    pthread_mutex_init(&server->queryLock, NULL);//initialize mutex
    server->pool = createThreadpool();//make the threadpool
    if(server->pool == NULL){
        printf("Error making server threadpool\n");
        return NULL;
    }
    //set up the server
    memset(&server->serverAddress, 0, sizeof(server->serverAddress));
    if((server->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        printf("*** SERVER ERROR: Could not open socket.\n");
        return NULL;
    }
    server->serverAddress.sin_family = AF_INET;
    server->serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    server->serverAddress.sin_port = htons((unsigned short) PORT);
    if(bind(server->serverSocket, (struct sockaddr*) &server->serverAddress, sizeof(server->serverAddress)) < 0){
        printf("*** SERVER ERROR: Could not bind socket.\n");
        return NULL;
    }
    if(listen(server->serverSocket, 1)<0){
        printf("*** SERVER ERROR: Could not listen on socket.\n");
        return NULL;
    }
    server->bytesRcv = 0;
    server->types[0]= "Normal";
    server->types[1]= "Fire";
    server->types[2]= "Water";
    server->types[3]= "Grass";
    server->types[4]= "Electric";
    server->types[5]= "Ice";
    server->types[6]= "Fighting";
    server->types[7]= "Poison";
    server->types[8]= "Ground";
    server->types[9]= "Flying";
    server->types[10]= "Psychic";
    server->types[11]= "Bug";
    server->types[12]= "Rock";
    server->types[13]= "Ghost";
    server->types[14]= "Dark";
    server->types[15]= "Dragon";
    server->types[16]= "Steel";
    server->types[17]= "Fair";
    return server;
}

//check for pokemon.csv
void openFile(pokeServer_t *server){
    char strName[30] ="\0";
    printf("enter file:\n");
    scanf("%29s", strName);
    //prompt for rentering name until correct
    while((server->fp = fopen(strName, "r")) == NULL){
        printf("Pokemon file not found. Please enter name again:\n");
        scanf("%29s", strName);
    }
}

//accept a client to the server
int acceptClient(pokeServer_t *server){
    server->addressSize = sizeof(server->clientAddress);
    server->clientSocket =accept(server->serverSocket, (struct sockaddr*) &server->clientAddress, &server->addressSize);
    if(server->clientSocket < 0){
        printf("*** SERVER ERROR: Could not accept incoming client connection.\n");
        exit(-1);
    }
    printf("SERVER: Accepted client\n");
    return 1;
}

//receive query from client
int receiveQuery(pokeServer_t *server){
    char f[30];
    memset(f, 0, 30);
    server->bytesRcv = recv(server->clientSocket, f, sizeof(f), 0);
    int num = -1;
    for(int i = 0; i < NUMTYPES; i++){
        if(strcmp(f, server->types[i])==0){
            num = i;
        }
    }
    //queue a search with the received term
    //when the client exits, it automatically sends "done" to the server so the server can exit
    if(strcmp(f,"done")!=0){
        if(num >= 0){
            printf("SERVER: received %s, querying\n", f);
            queueSearch(server, num);//queue the search
            return 1;
        }
        else{
            printf("SERVER: not a valid query\n");
            return 1;
        }
        
    }
    else{
        return 0;
    }
}

//queuing the search: it calls the queuetask function
void queueSearch(pokeServer_t *server, int num){
    queueTask(server->pool, &queryFile, num, server);
}

//query the file
void queryFile(void *varg1, void *varg2){
    pokeServer_t *s = (pokeServer_t*)varg2;//set the server
    if(varg1 ==NULL){
        printf("varg null\n");
    }
    int num = (int)varg1;
    pthread_mutex_lock(&s->queryLock);
    char done[5] = "done";
    FILE *p = s->fp;
    if(p==NULL){
        printf("null file\n");
    }
    char line[150] = "\0";
    char *found = NULL;
    while((fgets(line, 150, p)) != NULL){
        if((found = strstr(line, s->types[num])) && colCheck(line, found, 3)){
            send(s->clientSocket, line, 150, 0);
        }
    }
    //when theres no more lines send done so the client knows theres nothing else
    send(s->clientSocket, done, 150, 0);
    fseek(p, 0, SEEK_SET);//reset file pointer to beginning of file
    pthread_mutex_unlock(&s->queryLock);
}

//make sure the search is searching correct column
int colCheck(char *line, char *check, int num){
    char *compare = line;
    char *compare2 = line;
    int commaNum = 0;
    while(commaNum < num && *compare!='\n'){
        if(*compare == ','){
            ++commaNum;
            if(commaNum == num-1){
                compare2 = compare;
            }
        }
        compare++;
    }
    return ((compare > check) && (check > compare2));
}

//close the server and make sure to free all necessary memory etc
void closePokeServer(pokeServer_t *server){
    killThreadpool(server->pool);
    printf("Shutting down Server...\n");
    fclose(server->fp);
    pthread_mutex_destroy(&server->queryLock);
    close(server->clientSocket);
    close(server->serverSocket);
    free(server);
}