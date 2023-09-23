#ifndef CLIENT_HELPER_HEADER
#define CLIENT_HELPER_HEADER

typedef struct pokeClient pokeClient_t;

//initialize client
pokeClient_t * createPokeClient();
void saveFile(void * varg1, void *varg2);//save
void query(void * varg1, void *varg2);//query
void printQueries(pokeClient_t *client);//print completed queries
void queueQuery(pokeClient_t *client, char inStr[30]);//add query to queue
void queueSave(pokeClient_t *client, char inStr[30]);//add save to queue
void closePokeClient(pokeClient_t *client);//close client and free associated memory etc

#endif