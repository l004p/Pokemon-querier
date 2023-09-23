#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "llist.h"


//init link list
node_t * initLList(){
    node_t *head = NULL;
    return head;
}

//print link list
void printLList(node_t *head){
    node_t *currNode = head;
    while(currNode != NULL){
        printf("%s\n", currNode->entry->entryLine);
        currNode = currNode->next;
    }
}

//save to file, takes a file pointer to put the list into
void saveLList(node_t *head, FILE *p){
    node_t *currNode = head;
    while(currNode != NULL){
        fputs(currNode->entry->entryLine, p);
        currNode = currNode->next;
    }
}

//add entry
void addEntry(node_t **list, data_t *data){
    node_t *newNode;
    node_t *currNode, *prevNode;
    newNode = malloc(sizeof(node_t));
    if(newNode == NULL){
        printf("error allocating node\n");
    }
    newNode->entry = data;
    newNode->next = NULL;
    prevNode = NULL;
    currNode = *list;
    while(currNode != NULL){
        prevNode = currNode;
        currNode = currNode->next;
    }
    if(prevNode == NULL){
        *list = newNode;
    }
    else{
        prevNode->next = newNode;
    }
    newNode->next = currNode;
}

//put finished query into the data
data_t * initData(char *line){
    data_t *dataValue = malloc(sizeof(data_t));
    if(dataValue == NULL){
        printf("error allocating data\n");
        return NULL;
    }
    strcpy(dataValue->entryLine, line);
    return dataValue;
}

//cleanup
void destroyLList(node_t *head){
    node_t *currNode, *nextNode;
    currNode = head;
    while (currNode != NULL) {
        free(currNode->entry);
        nextNode = currNode->next;
        free(currNode);
        currNode = nextNode;
    }
}