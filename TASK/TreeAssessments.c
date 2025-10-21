#include <stdio.h>
#include <stdlib.h>

struct node{
    int data;
    struct node* left;
    struct node* right;
};

struct node* createnode(int value){
    struct node* n= (struct node*)malloc(sizeof(struct node));
    n->data = value;
    n->left = n->right = NULL;
    return n;
}

struct node* insert(struct node* root, int value){
    if (root == NULL)
        return createnode(value);

    if(value < root->data){
        root->left = insert(root->left, value);
    }
    else if ( value > root->data){
        root->right = insert(root->right, value);
    }
    return root;
}

int height(struct node* root) {
    if (root == NULL)
        return -1;
    int leftHeight = height(root->left);
    int rightHeight = height(root->right);
    return (leftHeight > rightHeight ? leftHeight : rightHeight) + 1;
}

void printDepths(struct node* root, int depth) {
    if (root == NULL)
        return;
    printf("node %d -> Depth %d\n", root->data, depth);
    printDepths(root->left, depth + 1);
    printDepths(root->right, depth + 1);
}

int main(){
    struct node* root = NULL;
    int nums;
    printf("Enter integers to insert into the BST: \n");
    while (scanf("%d", &nums) == 1) {
        root = insert(root, nums);
    }
    int h = height(root);
    printf("\nHeight of the BST: %d\n", h);
    printf("\nDepth of each node:\n");
    printDepths(root, 0);

    return 0;
}