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

    struct G_node{
        void *data;
        struct G_node *next;
        struct G_node *prev;
    };

    struct G_list{
        // fields
        struct G_node* head;
        size_t type_size; 
        uint8_t length;   

        // member methods
        void* (*comp)(void*, const void*);
        int (*assign)(void*, const void*);
        int (*print)(void*);
        void (*free_data)(void *);
        double (*value)(const void *);
    };

    /* doubly linked list methods*/
    void* listSearch(const struct G_list*, const void* data);
    int listInsert(struct G_list*, const void*);
    int listPrint(const struct G_list*);
    int listDelete(struct G_list*, const void*);
    int listFree(struct G_list*);

#endif