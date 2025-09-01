#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_LEVEL 6
#define P 0.5

typedef struct skiplevelNode{
    int data;
    struct skiplevelNode **forward ;
}skiplevelNode;

typedef struct skiplevel{
    int level;
    skiplevelNode *header;
} skiplevel;

skiplevelNode* createNode(int level, int data){
    skiplevelNode* node = (skiplevelNode*)malloc(sizeof(skiplevelNode));
    node->data = data;
    node->forward = (skiplevelNode**)malloc(sizeof(skiplevelNode*) * (level + 1));
    for (int i = 0; i <= level; i++){
        node->forward[i] = NULL;
    }
    return node;
}

skiplevel* createSkipList(){
    skiplevel* list = (skiplevel*)malloc(sizeof(skiplevel));
    list->level = 0;
    list->header = createNode(MAX_LEVEL, -1);
    return list;
}

int randomLevel(){
    int level = 0;
    while (((double)rand() / RAND_MAX) < P && level < MAX_LEVEL){
        level++;
    }
    return level;
}

void insertElement(skiplevel* list, int data){
    skiplevelNode *update[MAX_LEVEL + 1];
    skiplevelNode *current = list->header;
    for (int i = list->level; i >= 0; i--){
        while (current->forward[i] != NULL && current->forward[i]->data < data) {
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    if (current == NULL || current->data != data){
        int newLevel = randomLevel();
        if (newLevel > list->level) {
            for (int i = list->level + 1; i <= newLevel; i++){
                update[i] = list->header;
            }
            list->level = newLevel;
        }
        skiplevelNode* newNode = createNode(newLevel, data);
        for (int i = 0; i <= newLevel; i++) {
            newNode->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = newNode;
        }
    }
}

int searchElement(skiplevel* list, int data){
    skiplevelNode* current = list->header;
    for (int i = list->level; i >= 0; i--){
        while (current->forward[i] && current->forward[i]->data < data){
            current = current->forward[i];
        }
    }
    current = current->forward[0];
    if (current && current->data == data){
        return 1; 
    } else {
        return 0; 
    }
}

void deleteElement(skiplevel* list, int data){
    skiplevelNode *update[MAX_LEVEL + 1];
    skiplevelNode *current = list->header;
    for (int i = list->level; i >= 0; i--){
        while (current->forward[i] && current->forward[i]->data < data){
            current = current->forward[i];
        }
        update[i] = current;
    }
    current = current->forward[0];
    if (current && current->data == data){
        for (int i = 0; i <= list->level; i++) {
            if (update[i]->forward[i] != current)
                break;
            update[i]->forward[i] = current->forward[i];
        }
        free(current->forward);
        free(current);
        while (list->level > 0 && list->header->forward[list->level] == NULL){
            list->level--;
        }
    }
}


void displaySkipList(skiplevel* list){
    printf("\n-----Skip List----\n");
    for (int i = list->level; i >= 0; i--){
        skiplevelNode* node = list->header->forward[i];
        printf("Level %d: ", i);
        while (node != NULL) {
            printf("%d ", node->data);
            node = node->forward[i];
        }
        printf("\n");
    }
}

int main() {
    srand(time(NULL));
    skiplevel* list = createSkipList();
    insertElement(list, 3);
    insertElement(list, 6);
    insertElement(list, 7);

    insertElement(list, 12);
    insertElement(list, 19);
    insertElement(list, 18);
    insertElement(list, 21);
    insertElement(list, 25);
    displaySkipList(list);
    printf("seraching the value of 18 %s",searchElement(list,18) ? "Found" : "Notfound" );
    printf("\nDeleting ...\n" );
    deleteElement(list, 18);
    printf("seraching the value of 18 %s",searchElement(list,18) ? "Found" : "Notfound" );

    // printf("seraching the value of 18 %s",searchElement(list,18) ? 'Found' : 'Notfound' )

    displaySkipList(list);
    return 0;
}