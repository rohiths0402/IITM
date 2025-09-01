#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include "./hashTable.h"


struct Node* hashTable[TABLE_SIZE];

//hash function
unsigned int hash(const char *key)
{
    unsigned int hashValue = 0;

    while (*key)
    {
        hashValue = (hashValue * 31) + *key++;
    }
    return hashValue % TABLE_SIZE;
}

//insert
void insert(const char *key, int value)
{
    unsigned int index = hash(key);

    struct Node *newNode = (struct Node*)malloc(sizeof(struct Node));
    strcpy(newNode->key, key);
    newNode->value = value;
    newNode->next = NULL;

    if(hashTable[index] == NULL)
    {
        hashTable[index] = newNode;
    }
    else
    {
        struct Node *temp = hashTable[index];
        while(temp->next != NULL)
        {
            if(strcmp(temp->key, key) == 0)
            {
                temp->value = value;
                free(newNode);
            }
            temp = temp->next;
        }
    
        if(strcmp(temp->key, key) == 0)
        {
            temp->value = value;
            free(newNode);
            return;
        }
        temp->next = newNode;
    }  
    printf("(%s, %d) is successfully inserted at the index %u \n", key, value, index); 
}

//search
int search(const char *key)
{
    unsigned int index = hash(key);
    struct Node *temp = hashTable[index];

    while (temp != NULL)
    {
        if(strcmp(temp->key, key) == 0)
        {
            return temp->value;
        }
        temp = temp->next;
    }
    return -1;
}

//delete
void delete(const char *key)
{
    unsigned int index = hash(key);
    struct Node *temp = hashTable[index];
    struct Node *prev = NULL;

    while (temp != NULL)
    {
        if(strcmp(temp->key, key) == 0)
        {
            if(prev == NULL)
            {
                hashTable[index] = temp->next;
            }
            else
            {
                prev->next = temp->next;
            }
            free(temp);
            printf("Deleted the data with the key %s \n", key);
            return;
        }
        prev = temp;
        temp = temp->next;
    }
    printf("The key %s is not found \n", key);
}

//display
void display() {
    printf("Hash Table Contents:\n");
    for (int i = 0; i < TABLE_SIZE; i++) {
        struct Node *temp = hashTable[i];
        if (temp != NULL) {
            printf("Index %d: ", i);
            while (temp != NULL) {
                printf("(%s, %d) -> ", temp->key, temp->value);
                temp = temp->next;
            }
            printf("NULL\n");
        }
    }
}

int main(){
    for(int i=0; i<TABLE_SIZE;i++){
        hashTable [i] = NULL;
    }

    insert("orange",200);
    insert("banana", 500);
    insert("orange", 300);
    insert("grape", 400);
    insert("apple", 150);

    display();

    printf("\nSearching for 'banana': %d\n", search("banana"));
    // display();
    printf("Searching for 'mango': %d\n", search("mango")); 
    display();
    delete("orange");
    printf("\nAfter deletion:\n");
    display();
    delete("mango");
    printf("\nAfter deletion:\n");
    display();
    return 0;

}
