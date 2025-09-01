#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 3  // Max keys per node

typedef struct BPlusTreeNode {
    int keys[MAX];
    int count;
    struct BPlusTreeNode* child[MAX + 1];
    struct BPlusTreeNode* next;  // For leaf node chaining
    bool isLeaf;
} BPlusTreeNode;

// Create a new node
BPlusTreeNode* createNode(bool isLeaf) {
    BPlusTreeNode* node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    node->isLeaf = isLeaf;
    node->count = 0;
    node->next = NULL;
    for (int i = 0; i <= MAX; i++) {
        node->child[i] = NULL;
    }
    return node;
}

// Insert key into leaf node (no split)
void insertLeaf(BPlusTreeNode* leaf, int key) {
    int i = leaf->count - 1;
    while (i >= 0 && leaf->keys[i] > key) {
        leaf->keys[i + 1] = leaf->keys[i];
        i--;
    }
    leaf->keys[i + 1] = key;
    leaf->count++;
}

// Split leaf node and return new leaf
BPlusTreeNode* splitLeaf(BPlusTreeNode* leaf, int key, int* upKey) {
    int tempKeys[MAX + 1];
    for (int i = 0; i < MAX; i++) {
        tempKeys[i] = leaf->keys[i];
    }
    tempKeys[MAX] = key;

    // Sort keys
    for (int i = MAX; i > 0 && tempKeys[i] < tempKeys[i - 1]; i--) {
        int temp = tempKeys[i];
        tempKeys[i] = tempKeys[i - 1];
        tempKeys[i - 1] = temp;
    }

    int mid = (MAX + 1) / 2;
    leaf->count = mid;
    BPlusTreeNode* newLeaf = createNode(true);
    newLeaf->count = MAX + 1 - mid;

    for (int i = 0; i < mid; i++) {
        leaf->keys[i] = tempKeys[i];
    }
    for (int i = 0; i < newLeaf->count; i++) {
        newLeaf->keys[i] = tempKeys[mid + i];
    }

    newLeaf->next = leaf->next;
    leaf->next = newLeaf;

    *upKey = newLeaf->keys[0];
    return newLeaf;
}

// Recursive insert with split handling
BPlusTreeNode* insert(BPlusTreeNode* root, int key, int* upKey, BPlusTreeNode** newChild) {
    if (root->isLeaf) {
        if (root->count < MAX) {
            insertLeaf(root, key);
            return NULL;
        } else {
            *newChild = splitLeaf(root, key, upKey);
            return root;
        }
    }

    int i = 0;
    while (i < root->count && key > root->keys[i]) i++;

    int tempKey;
    BPlusTreeNode* tempChild = NULL;
    BPlusTreeNode* child = insert(root->child[i], key, &tempKey, &tempChild);

    if (child == NULL) return NULL;

    if (root->count < MAX) {
        for (int j = root->count; j > i; j--) {
            root->keys[j] = root->keys[j - 1];
            root->child[j + 1] = root->child[j];
        }
        root->keys[i] = tempKey;
        root->child[i + 1] = tempChild;
        root->count++;
        return NULL;
    } else {
        // Split internal node
        int tempKeys[MAX + 1];
        BPlusTreeNode* tempChildren[MAX + 2];

        for (int j = 0; j < MAX; j++) tempKeys[j] = root->keys[j];
        for (int j = 0; j <= MAX; j++) tempChildren[j] = root->child[j];

        for (int j = MAX; j > i; j--) {
            tempKeys[j] = tempKeys[j - 1];
            tempChildren[j + 1] = tempChildren[j];
        }
        tempKeys[i] = tempKey;
        tempChildren[i + 1] = tempChild;

        int mid = (MAX + 1) / 2;
        root->count = mid;
        for (int j = 0; j < mid; j++) {
            root->keys[j] = tempKeys[j];
            root->child[j] = tempChildren[j];
        }
        root->child[mid] = tempChildren[mid];

        *upKey = tempKeys[mid];
        BPlusTreeNode* newInternal = createNode(false);
        newInternal->count = MAX - mid;
        for (int j = 0; j < newInternal->count; j++) {
            newInternal->keys[j] = tempKeys[mid + 1 + j];
            newInternal->child[j] = tempChildren[mid + 1 + j];
        }
        newInternal->child[newInternal->count] = tempChildren[MAX + 1];

        *newChild = newInternal;
        return root;
    }
}

// Print keys in range [start, end]
void printRange(BPlusTreeNode* root, int start, int end) {
    while (!root->isLeaf) {
        int i = 0;
        while (i < root->count && start > root->keys[i]) i++;
        root = root->child[i];
    }

    while (root) {
        for (int i = 0; i < root->count; i++) {
            if (root->keys[i] >= start && root->keys[i] <= end) {
                printf("%d ", root->keys[i]);
            }
            if (root->keys[i] > end) return;
        }
        root = root->next;
    }
    printf("\n");
}

// Simple tree traversal for debugging
void printTree(BPlusTreeNode* root, int level) {
    if (root == NULL) return;

    printf("Level %d [", level);
    for (int i = 0; i < root->count; i++) {
        printf("%d ", root->keys[i]);
    }
    printf("]\n");

    if (!root->isLeaf) {
        for (int i = 0; i <= root->count; i++) {
            printTree(root->child[i], level + 1);
        }
    }
}

int main() {
    BPlusTreeNode* root = createNode(true);
    int upKey;
    BPlusTreeNode* newChild;

    int* keys;
    int n;

    printf("Enter number of keys: ");
    scanf("%d", &n);

    keys = (int*)malloc(n * sizeof(int));
    if (!keys) {
        printf("Memory allocation failed\n");
        return 1;
    }

    printf("Enter %d keys:\n", n);
    for (int i = 0; i < n; i++) {
        scanf("%d", &keys[i]);
    }

    for (int i = 0; i < n; i++) {
        BPlusTreeNode* result = insert(root, keys[i], &upKey, &newChild);
        if (result != NULL) {
            BPlusTreeNode* newRoot = createNode(false);
            newRoot->keys[0] = upKey;
            newRoot->child[0] = root;
            newRoot->child[1] = newChild;
            newRoot->count = 1;
            root = newRoot;
        }
    }

    printf("\nB+ Tree Structure:\n");
    printTree(root, 0);

    int start, end;
    printf("\nEnter range to print (start end): ");
    scanf("%d %d", &start, &end);
    printf("Keys in range [%d, %d]: ", start, end);
    printRange(root, start, end);

    free(keys);
    return 0;
}