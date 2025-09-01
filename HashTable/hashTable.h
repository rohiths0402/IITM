#ifndef HASH_TABLE
#define HASH_tABLE

#define TABLE_SIZE 10

struct Node {
    char key[50];
    int value;
    struct Node *next;
};

unsigned int hash(const char *key);
void insert(const char *key, int value);
int search(const char *key);
void delete(const char *key);
void display();


#endif