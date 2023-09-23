
#ifndef SERVER_HELPER_HEADER
#define SERVER_HELPER_HEADER

typedef struct pokeServer pokeServer_t;

//pokemon server initializer
pokeServer_t * createPokeServer();
//file opener to check for pokemon.csv title
void openFile(pokeServer_t *server);
//helper function for searching the correct column
int colCheck(char *line, char *check, int num);
//function for the server to accept a client
int acceptClient(pokeServer_t *server);
//receive the query from the client
int receiveQuery(pokeServer_t *server);
//queue a search to the threadpool
void queueSearch(pokeServer_t *server, int num);
//query the file
void queryFile(void *varg1, void * varg2);
//close the server
void closePokeServer(pokeServer_t *server);

#endif