#include "LinkedList.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static void printStringWithSpace(void* str){
    printf("%s ", (char*) str);
}

static void displayLinkedListAsSingleString(LinkedList* list){
    list->applyToAll(list, printStringWithSpace);
    printf("\n");
}

int main(){
    LinkedList* list = create_String_LinkedList();
    char hello[] = "Hello";
    char world[] = "World";
    char virt[] = "Virtual";
    char cell[] = "Cell";
    char exclaim[] = "!";
    char huh[] = "HUH";
    char* dynHello = (char*)malloc(sizeof(char) * (strlen(hello) + 1));
    char* dynWorld = (char*)malloc(sizeof(char) * (strlen(world) + 1));
    char* dynVirt = (char*)malloc(sizeof(char) * (strlen(virt) + 1));
    char* dynCell = (char*)malloc(sizeof(char) * (strlen(cell) + 1));
    char* dynExclaim = (char*)malloc(sizeof(char) * (strlen(exclaim) + 1));
    char* dynHuh = (char*)malloc(sizeof(char) * (strlen(huh) + 1));
    strcpy(dynHello, hello);
    printf("%s\n", dynHello);
    strcpy(dynWorld, world);
    printf("%s\n", dynWorld);
    strcpy(dynVirt, virt);
    printf("%s\n", dynVirt);
    strcpy(dynCell, cell);
    printf("%s\n", dynCell);
    strcpy(dynExclaim, exclaim);
    printf("%s\n", dynExclaim);
    strcpy(dynHuh, huh);
    printf("%s\n", dynHuh);

    displayLinkedListAsSingleString(list);

    list->append(list, dynHello);
    display_String_LinkedList(list);
    displayLinkedListAsSingleString(list);

    list->append(list, dynWorld);
    display_String_LinkedList(list);
    displayLinkedListAsSingleString(list);

    list->append(list, dynExclaim);
    display_String_LinkedList(list);
    displayLinkedListAsSingleString(list);

    char* old_value = list->replace(list, 1, dynVirt);
    if (strcmp(dynWorld, old_value) != 0) {
        fprintf(stderr, "Old value isn't correct\n");
        exit(1);
    }

    list->insert(list, 2, dynCell);
    display_String_LinkedList(list);
    displayLinkedListAsSingleString(list);

    list->insert(list, list->_size, dynHuh);
    display_String_LinkedList(list);
    displayLinkedListAsSingleString(list);

    list->remove(list, list->_size - 1);
    display_String_LinkedList(list);
    displayLinkedListAsSingleString(list);

    printf("Goodbye, %s %s...\n", (char*)list->get(list, 1), (char*)list->get(list, 2));

    list->_freeThis(list);
    if (list != NULL) exit(1);
    return 0;
}