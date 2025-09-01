#include <stdio.h>
#include <stdlib.h>

struct node {
    int val;
    int height;
    struct node* leftChild;
    struct node* rightChild;
};

int max(int a, int b){
    return (a > b) ? a : b;
}

int getHeight(struct node* n){
    return (n == NULL) ? 0 : n->height;
}

int getBalance(struct node* n){
    return (n == NULL) ? 0 : getHeight(n->leftChild) - getHeight(n->rightChild);
}

struct node* newNode(int val){
    struct node* n = (struct node*)malloc(sizeof(struct node));
    n->val = val;
    n->leftChild = n->rightChild = NULL;
    n->height = 1;
    return n;
}

/* -------- Rotations -------- */
struct node* rotateRight(struct node* y) {
    struct node* x = y->leftChild;
    struct node* T2 = x->rightChild;

    x->rightChild = y;
    y->leftChild = T2;

    y->height = max(getHeight(y->leftChild), getHeight(y->rightChild)) + 1;
    x->height = max(getHeight(x->leftChild), getHeight(x->rightChild)) + 1;

    return x;
}

struct node* rotateLeft(struct node* x) {
    struct node* y = x->rightChild;
    struct node* T2 = y->leftChild;  

    y->leftChild = x;
    x->rightChild = T2;

    x->height = max(getHeight(x->leftChild), getHeight(x->rightChild)) + 1;
    y->height = max(getHeight(y->leftChild), getHeight(y->rightChild)) + 1;

    return y;
}

/* -------- Insert -------- */
struct node* insert(struct node* root, int val) {
    if (root == NULL) return newNode(val);

    if (val < root->val)
        root->leftChild = insert(root->leftChild, val);
    else if (val > root->val)
        root->rightChild = insert(root->rightChild, val);
    else {
        // Duplicate value
        return root;
    }

    // Update height
    root->height = 1 + max(getHeight(root->leftChild), getHeight(root->rightChild));

    // Check balance
    int balance = getBalance(root);

    // LL
    if (balance > 1 && val < root->leftChild->val)
        return rotateRight(root);

    // RR
    if (balance < -1 && val > root->rightChild->val)
        return rotateLeft(root);

    // LR
    if (balance > 1 && val > root->leftChild->val) {
        root->leftChild = rotateLeft(root->leftChild);
        return rotateRight(root);
    }

    // RL
    if (balance < -1 && val < root->rightChild->val) {
        root->rightChild = rotateRight(root->rightChild);
        return rotateLeft(root);
    }

    printf("Inserted %d, Balance at %d = %d\n", val, root->val, balance);
    return root;
}

/* -------- Min Value Node -------- */
struct node* findMin(struct node* n) {
    struct node* cur = n;
    while (cur && cur->leftChild) cur = cur->leftChild;
    return cur;
}

/* -------- Delete -------- */
struct node* deleteNode(struct node* root, int val) {
    if (root == NULL) return root;

    if (val < root->val)
        root->leftChild = deleteNode(root->leftChild, val);
    else if (val > root->val)
        root->rightChild = deleteNode(root->rightChild, val);
    else {
        if (root->leftChild == NULL || root->rightChild == NULL) {
            struct node* temp = root->leftChild ? root->leftChild : root->rightChild;
            free(root);
            return temp;
        }
        struct node* temp = findMin(root->rightChild);
        root->val = temp->val;
        root->rightChild = deleteNode(root->rightChild, temp->val);
    }

    if (root == NULL) return root;

    root->height = 1 + max(getHeight(root->leftChild), getHeight(root->rightChild));
    int balance = getBalance(root);

    // LL
    if (balance > 1 && getBalance(root->leftChild) >= 0)
        return rotateRight(root);

    // LR
    if (balance > 1 && getBalance(root->leftChild) < 0) {
        root->leftChild = rotateLeft(root->leftChild);
        return rotateRight(root);
    }

    // RR
    if (balance < -1 && getBalance(root->rightChild) <= 0)
        return rotateLeft(root);

    // RL
    if (balance < -1 && getBalance(root->rightChild) > 0) {
        root->rightChild = rotateRight(root->rightChild);
        return rotateLeft(root);
    }

    return root;
}

/* -------- Display -------- */
void displayTree(struct node* root, int space) {
    if (root == NULL) return;
    space += 5;
    displayTree(root->rightChild, space);
    for (int i = 0; i < space; i++) printf(" ");
    printf("%d\n", root->val);
    displayTree(root->leftChild, space);
}

/* -------- Cleanup -------- */
void freeTree(struct node* root) {
    if (root == NULL) return;
    freeTree(root->leftChild);
    freeTree(root->rightChild);
    free(root);
}

/* -------- Main Menu -------- */
int main() {
    struct node* root = NULL;
    int option, data, n, *arr;

    while (1) {
        printf("\n1. Insert Single Value\n");
        printf("2. Insert Array of Values\n");
        printf("3. Display AVL Tree\n");
        printf("4. Delete\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &option);

        switch (option) {
        case 1:
            printf("Enter value to insert: ");
            scanf("%d", &data);
            root = insert(root, data);
            break;
        case 2:
            printf("Enter number of elements: ");
            scanf("%d", &n);
            arr = (int*)malloc(n * sizeof(int));
            printf("Enter %d values: ", n);
            for (int i = 0; i < n; i++) scanf("%d", &arr[i]);
            for (int i = 0; i < n; i++) root = insert(root, arr[i]);
            free(arr);
            break;
        case 3:
            printf("\nAVL Tree Structure:\n");
            displayTree(root, 0);
            break;
        case 4:
            printf("Enter value to delete: ");
            scanf("%d", &data);
            root = deleteNode(root, data);
            break;
        case 5:
            freeTree(root);
            return 0;
        default:
            printf("Invalid option.\n");
        }
    }
}