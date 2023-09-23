#include <stdio.h>
#include <stdlib.h>

#include "clientHelpers.h"

int main(int argc, char **argv){
    //create the client
    pokeClient_t *client = createPokeClient();
    //check iniitialization
    if(client == NULL){
        printf("CLIENT: client creation failed\n");
        exit(-1);
    }
    else{
        printf("client creation success\n");
    }
    //loop to ensure menu of choices
    int choice = 0;
    char field[30];
    while(choice != 3){
        printf("Pick an option:\n\t(1) Type Search\n\t(2) Save Results\n\t(3) Exit Program\n");
        if(scanf("%d", &choice)!=0){
            switch(choice){
                case 1: 
                    printf("Please enter query type (note: case sensitive, capitalize fist letter):\n");
                    scanf("%29s", field);
                    queueQuery(client, field);//queue a query
                    break;
                case 2:
                    printf("Please enter file name:\n");
                    scanf("%29s", field);
                    queueSave(client, field);//queue saving the file
                    break;
                case 3:
                    closePokeClient(client);//close client and free all the memory for its components
                    break;
                default:
                    printf("Error: not a valid entry\n");
            }
        }
        else{
            printf("Error: not a valid entry\n");
            fflush(stdin);
        }  
    }
    return 0;
}

