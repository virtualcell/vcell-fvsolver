//
// Created by Logan Drescher on 2/12/25.
//

#include "LinkedList.h"
#include <stdlib.h>
#include <stdio.h>

static LinkedList_Node* getNodeAtIndexFromLL(LinkedList* ll, int index) {
    if (index < 0 || index >= ll->_size) return NULL;
    if (index == ll->_size - 1) return ll->_tail;
    LinkedList_Node* currentNode = ll->_head;
    for (int i = 0; i < index; i++) currentNode = currentNode->next;
    return currentNode;
}

static void performOperationsOnAllEntriesInLL(LinkedList* ll, void (*operation)(void*)){
    if (ll->_size == 0) return;
    LinkedList_Node* currentNode = ll->_head;
    while (currentNode != NULL) {
        operation(currentNode->data);
        currentNode = currentNode->next;
    }
}

//=================================================================================================================//
// String Implementation
//=================================================================================================================//
static int insertStringDataIntoLL(LinkedList* ll, int index, void* data) {
    char* stringData = (char*) data;
    printf("index: %d; size: %d; data: %s\n", index, ll->_size, stringData);
    printf("head: %p\n", ll->_head);
    printf("tail: %p\n", ll->_tail);
    if (index < 0 || index > ll->_size) return 0;
    LinkedList_Node* newNode = createStringLinkedList_Node(0); // if this is NULL, we have BIG problems!
    newNode->data = data;
    ll->_size++; // remember: we increased size! all calculations below must account for that!
    if (index == 0 && ll->_head == NULL) { ll->_head = newNode; ll->_tail = ll->_head; return 1; } // Only a value of NULL for `newNode` would cause a "false" value
    if (index == ll->_size - 1) { ll->_tail->next = newNode; ll->_tail = newNode; return 1;} // Only a value of NULL for `newNode` would cause a "false" value
    if (index == 0) { newNode->next = ll->_head; ll->_head = newNode; return 1; } // Only a value of NULL for `newNode` would cause a "false" value

    LinkedList_Node* targetNode = getNodeAtIndexFromLL(ll, index - 1);
    newNode->next = targetNode->next;
    targetNode->next = newNode; // Only a value of NULL for `newNode` would cause a "false" value
    return 1;
}

static void appendStringDataToLL(LinkedList* ll, void* data) {
    insertStringDataIntoLL(ll, ll->_size, data);
}

static void* getStringDataFromLL(LinkedList* ll, int index) {
    if (index < 0 || index >= ll->_size) return NULL;
    return getNodeAtIndexFromLL(ll, index)->data;
}

static void* replaceStringInLL(LinkedList* ll, int index, void* data) {
    if (index < 0 || index >= ll->_size) return NULL;
    LinkedList_Node* currentNode = getNodeAtIndexFromLL(ll, index);
    return currentNode->replace(currentNode, data);
}

static int removeStringFromLL(LinkedList* ll, int index) {
    if (index < 0 || index >= ll->_size) return 0;
    LinkedList_Node* nodeToBeRemoved;
    if (index == 0) {
        nodeToBeRemoved = ll->_head;
        ll->_head = ll->_head->next;
    } else {
        LinkedList_Node* oneNodeBefore = getNodeAtIndexFromLL(ll, index - 1);
        if (index == ll->_size - 1) ll->_tail = oneNodeBefore;
        nodeToBeRemoved = oneNodeBefore->next;
        oneNodeBefore->next = nodeToBeRemoved->next;
    }
    char* oldData = (char*)nodeToBeRemoved->replace(nodeToBeRemoved, NULL);
    if (oldData != NULL) free(oldData);
    free(nodeToBeRemoved);
    ll->_size--;
    if (ll->_size != 0) return 1;
    ll->_head = NULL;
    ll->_tail = NULL;
    return 1;
}

static int emptyStringLinkedList(LinkedList* ll) {
    int thingsHaveBeenRemoved = 0;
    while (ll->_head != NULL) {
        int result = ll->remove(ll, 0);
        //int result = removeStringFromLL(ll, 0);
        thingsHaveBeenRemoved |= result;
    }

    return thingsHaveBeenRemoved;
}

static void freeStringLinkedList(LinkedList* ll) {
    emptyStringLinkedList(ll);
    free(ll);
}

LinkedList* create_String_LinkedList() {
    LinkedList* stringLinkedList = (LinkedList*) malloc(sizeof(LinkedList));
    if (stringLinkedList == NULL) return NULL;
    stringLinkedList->_head = NULL;
    stringLinkedList->_tail = stringLinkedList->_head;
    stringLinkedList->_size = 0;
    stringLinkedList->append = appendStringDataToLL;
    stringLinkedList->insert = insertStringDataIntoLL;
    stringLinkedList->get = getStringDataFromLL;
    stringLinkedList->replace = replaceStringInLL;
    stringLinkedList->remove = removeStringFromLL;
    stringLinkedList->applyToAll = performOperationsOnAllEntriesInLL;
    stringLinkedList->_freeThis = freeStringLinkedList;
    return stringLinkedList;
}

void display_String_LinkedList(LinkedList* ll) {
    LinkedList_Node* currentNode = ll->_head;
    while (currentNode != NULL) {
        printf("(%p)\"%s\"->", currentNode, (char*)currentNode->data);
        currentNode = currentNode->next;
    }
    printf("0\n");
}
//=================================================================================================================//