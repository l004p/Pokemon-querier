#ifndef LLIST_HEADER
#define LLIST_HEADER

//data structure for the data in the list
typedef struct data{
    char entryLine[100];
} data_t;

//data structure for the node
typedef struct node{
    data_t *entry;
    struct node *next;
} node_t;

//init link list
node_t * initLList();
//print link list
void printLList(node_t *head);
//save the list to a file
void saveLList(node_t *head, FILE *p);
//add entry
void addEntry(node_t **llist, data_t *data);
//put data into node
data_t * initData(char *line);
//cleanup
void destroyLList(node_t *head);

#endif