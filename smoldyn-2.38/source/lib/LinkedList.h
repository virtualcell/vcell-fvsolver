//
// Created by Logan Drescher on 2/12/25.
//

#ifndef LINKEDLIST_H
#define LINKEDLIST_H
#include "LinkedListNode.h"

struct Struct_LinkedList;
typedef struct Struct_LinkedList LinkedList;
// Define linked list "methods" here
// Because you can have dangling pointers, we must *always* copy incoming data
// Please maintain the paradigm that there is one owner to the data in a Node of a Linked List!
typedef void (*appendDataToLL)(LinkedList*, void* data);

typedef int (*insertDataIntoLL)(LinkedList*, int index, void* data);

typedef void* (*getDataFromLL)(LinkedList*, int index);

typedef void* (*replaceDataInLL)(LinkedList*, int index, void* data);

typedef int (*removeDataFromLL)(LinkedList*, int index);

/*
 * This function is meant to serve as an analog to the modern "for each" loop.
 * The function pointer parameter must perform all operations as side effects.
 * The programmer must ensure that the function pointed to by the function-pointer can safely handle the data
 */
typedef void (*performOpOnAllDataInLL)(LinkedList*, void (*)(void*)); // Note the function pointer parameter!

typedef int (*emptyDataFromLL)(LinkedList*);

typedef void (*freeLL)(LinkedList*);

// LinkedList Definition
struct Struct_LinkedList {
    LinkedList_Node* _head;
    LinkedList_Node* _tail;
    int _size;
    //-----------
    appendDataToLL append;
    insertDataIntoLL insert;
    getDataFromLL get;
    replaceDataInLL replace;
    removeDataFromLL remove;
    performOpOnAllDataInLL applyToAll;

    freeLL _freeThis;
};

// Creation functions
LinkedList* create_String_LinkedList();

// Utility functions
void display_String_LinkedList(LinkedList*);

#endif //LINKEDLIST_H
