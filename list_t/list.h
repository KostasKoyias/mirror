/**********************************************************************************************\
** a generic doubly linked list implementation, appropriate for any data type of your choice, **
** all you need to do is define it's member methods based on your data type's characteristics **
** Author: Kostas Koyias  https://github.com/KostasKoyias                                     **
\**********************************************************************************************/  
#ifndef list_H_
#define list_H_                           

    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdint.h>

    typedef struct G_node{
        void *data;
        struct G_node *next;
        struct G_node *prev;
    }node_t;

    typedef struct G_list{
        // fields
        node_t* head;
        size_t type_size; 
        uint8_t length;   

        // member methods
        void* (*comp)(void*, const void*);
        int (*assign)(void*, const void*);
        int (*print)(void*);
        void (*free_data)(void *);
        double (*value)(const void *);
    }list_t;

    /* doubly linked list methods*/
    void* listSearch(const list_t*, const void* data);
    int listInsert(list_t*, const void*);
    int listPrint(const list_t*);
    int listDelete(list_t*, const void*);
    int listFree(list_t*);
    double listSum(const list_t*);
    int listMap(list_t*, int (*)(void *));

#endif