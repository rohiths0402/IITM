#include <stdio.h>
#include <stdlib.h>

struct node{
    int data;
    struct node* left;
    struct node* right;
};

struct node* createNode(int value){
    struct node* n= (struct node*)malloc(sizeof(struct node));
    n->data = value;
    n->left = n->right = NULL;
    return n;
}

struct node* insert(struct node* root, int value){
    if (root == NULL)
        return createNode(value);

    if(value < root->data){
        root->left = insert(root->left, value);
    }
    else if ( value > root->data){
        root->right = insert(root->right, value);
    }
    return root;
}

void inorder(struct node* root){
    if (root == NULL)
        return;

    inorder(root->left);
    printf("%d ", root->data);
    inorder(root->right);
}

struct node* search(struct node* root, int value){
    if(root == NULL || root->data == value){
        return root;
    }
    if(value < root->data){
        return search(root->left, value);
    }
    else if (value > root->data){
        return search(root->right, value);
    }
}

int main(){
    struct node* root = NULL;
    int value, choice;

    while(1){
        printf("\n\n---- Binary Search Tree Menu----\n");
        printf("1. Insert\n");
        printf("2. Search\n");
        printf("3. Exit\n");
        printf("Enter your choice: ");
        scanf("%d ", &choice);

        switch(choice){
            case 1:
                printf("Enter the Value to Enter\n");
                scanf("%d ", &value);
                root = insert(root, value);
                printf("\n Inorder traversal");
                inorder(root);
                printf("\n");
                break;
            case 2:
                printf("Enter the value for search");
                scanf("%d ", value);
                if(search(root, value)){
                    printf("%d is found \n", value);
                }
                else{
                    printf("%d not found\n", value);
                }
                break;
            case 3:
                printf("Exiting...\n");
                exit(0);
            default:
                printf("Invalid choice! Try again.\n");
        }
    }


    return 0;
}