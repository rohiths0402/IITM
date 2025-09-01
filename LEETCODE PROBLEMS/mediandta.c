#include <stdio.h>
#include <stdlib.h>

typedef struct AVLNode {
    int val;
    int height;
    int count;    // number of duplicates
    int size;     // total nodes in subtree
    struct AVLNode* left;
    struct AVLNode* right;
} AVLNode;

// Utility functions
int height(AVLNode* node) {
    return node ? node->height : 0;
}

int size(AVLNode* node) {
    return node ? node->size : 0;
}

int max(int a, int b) {
    return (a > b) ? a : b;
}

// Update node info
void update(AVLNode* node) {
    if (node) {
        node->height = 1 + max(height(node->left), height(node->right));
        node->size = node->count + size(node->left) + size(node->right);
    }
}

// Right rotation
AVLNode* rightRotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    update(y);
    update(x);

    return x;
}

// Left rotation
AVLNode* leftRotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    update(x);
    update(y);

    return y;
}

int getBalance(AVLNode* node) {
    return node ? height(node->left) - height(node->right) : 0;
}

// AVL insert
AVLNode* insert(AVLNode* root, int val) {
    if (!root) {
        AVLNode* node = malloc(sizeof(AVLNode));
        node->val = val;
        node->height = 1;
        node->count = 1;
        node->size = 1;
        node->left = node->right = NULL;
        return node;
    }

    if (val < root->val)
        root->left = insert(root->left, val);
    else if (val > root->val)
        root->right = insert(root->right, val);
    else
        root->count++; // duplicate

    update(root);

    int balance = getBalance(root);

    // Left Left
    if (balance > 1 && val < root->left->val)
        return rightRotate(root);

    // Right Right
    if (balance < -1 && val > root->right->val)
        return leftRotate(root);

    // Left Right
    if (balance > 1 && val > root->left->val) {
        root->left = leftRotate(root->left);
        return rightRotate(root);
    }

    // Right Left
    if (balance < -1 && val < root->right->val) {
        root->right = rightRotate(root->right);
        return leftRotate(root);
    }

    return root;
}

// Find k-th smallest in AVL tree
int kth(AVLNode* root, int k) {
    if (!root) return -1;

    int leftSize = size(root->left);

    if (k <= leftSize)
        return kth(root->left, k);
    else if (k <= leftSize + root->count)
        return root->val;
    else
        return kth(root->right, k - leftSize - root->count);
}

// MedianFinder
typedef struct {
    AVLNode* root;
    int total;
} MedianFinder;

MedianFinder* medianFinderCreate() {
    MedianFinder* mf = malloc(sizeof(MedianFinder));
    mf->root = NULL;
    mf->total = 0;
    return mf;
}

void medianFinderAddNum(MedianFinder* obj, int num) {
    obj->root = insert(obj->root, num);
    obj->total++;
}

double medianFinderFindMedian(MedianFinder* obj) {
    if (obj->total % 2 == 1) {
        int k = obj->total / 2 + 1;
        return kth(obj->root, k);
    } else {
        int k1 = obj->total / 2;
        int k2 = obj->total / 2 + 1;
        return (kth(obj->root, k1) + kth(obj->root, k2)) / 2.0;
    }
}

void freeAVL(AVLNode* root) {
    if (!root) return;
    freeAVL(root->left);
    freeAVL(root->right);
    free(root);
}

void medianFinderFree(MedianFinder* obj) {
    freeAVL(obj->root);
    free(obj);
}