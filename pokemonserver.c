#include <stdio.h>
#include <stdlib.h>

#include "serverHelpers.h"

int main(int argc, char **argv){
    //create the server
    pokeServer_t *server = createPokeServer();
    if(server == NULL){
        printf("SERVER: Error creating server\n");
        exit(-1);
    }
    //check pokemon file name and open the file
    openFile(server);
    acceptClient(server);
    while(1){
        if(receiveQuery(server)){
            continue;
        }
        else{
            break;
        }
    }
    closePokeServer(server);
    return 0;
}
