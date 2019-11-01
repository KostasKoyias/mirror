#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "list.h"
#define NAME 20
#define USERS 4

// a list holding data of type 'struct Person' will be created'
struct Person{
    int id;
    char name[NAME];
};

// the list's member methods are defined below
void *compare(void *p1, const void *vid){
    struct Person *pp1 = (struct Person*)p1;
    int *id = (int*)vid;
    return (pp1->id == *id) ? p1 : NULL;
}

int assign(void *p1, const void *p2){
    if(p1 == NULL || p2 == NULL)
        return -1;

    struct Person *pp1 = (struct Person*)p1, *pp2 = (struct Person *)p2;
    pp1->id = pp2->id;
    strcpy(pp1->name, pp2->name);
    return 0;
}

int print(void *p1){
    if(p1 == NULL)
        return -1;

    struct Person *pp1 = (struct Person*)p1;
    fprintf(stdout, "\nId: %d\nName: %s\n", pp1->id, pp1->name);
    return 0;
}

int main(){
    struct Person ps[USERS]={{0,"Thomas"}, {2, "Kyrie"}, {15, "Kemba"}, {23, "Gardner"}};
    list_t list = {NULL, sizeof(struct Person), 0, compare, assign, print, NULL, NULL}; // initialize list
    int i;

    // insert records
    for(i = 0; i < USERS; i++)
        listInsert(&list, ps + i);
    
    // print list
    listPrint(&list);

    // delete first, a random and the last record of the list
    for(i = 0; i < USERS; i++){
        if(ps[i].id != 2)
            listDelete(&list, &(ps[i].id));
    }

    
    // print again
    listPrint(&list);

    // free resources
    listFree(&list);
    return 0;
}