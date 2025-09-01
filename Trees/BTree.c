#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_KEYS 10

typedef struct BTreeNode {
    int *keys;
    int numKeys;
    bool isLeaf;
} BTreeNode;

BTreeNode* createNode(int n) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    node->keys = (int*)malloc(n * sizeof(int));
    node->numKeys = n;
    node->isLeaf = true;
    printf("Enter %d values: ", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &node->keys[i]);
    }
    return node;
}

bool search(BTreeNode* node, int key) {
    for (int i = 0; i < node->numKeys; i++) {
        if (node->keys[i] == key) {
            return true;
        }
    }
    return false;
}

int main(){
    int n;
    printf("How many values to store in the node (max %d)? ", MAX_KEYS);
    scanf("%d", &n);
    if (n > MAX_KEYS){
        printf("Too many values! Max allowed is %d\n", MAX_KEYS);
        return 1;
    }
    BTreeNode* root = createNode(n);
    int key;
    printf("Enter a key to search: ");
    scanf("%d", &key);
    if (search(root, key)) {
        printf("Key %d found!\n", key);
    } else {
        printf("Key %d not found.\n", key);
    }
    free(root->keys);
    free(root);

    return 0;
}
