#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef struct Node
{
    char data[10];
    struct Node *left, *right;
}Node;

Node *createNode(char *data)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    strcpy(newNode->data, data);
    newNode->left = newNode->right = NULL;
    return newNode;
}

void inorder(Node *root)
{
    if (!root)
        return;
    if (root->left)
        printf("(");
    inorder(root->left);
    printf(" %s ", root->data);
    inorder(root->right);
    if (root->right)
        printf(")");
}

int evaluate(Node *root)
{
    if (!root)
        return 0;
    if (isdigit(root->data[0]))
        return atoi(root->data);
    int left = evaluate(root->left);
    int right = evaluate(root->right);
    if (strcmp(root->data, "+") == 0)
        return left + right;
    if (strcmp(root->data, "-") == 0)
        return left - right;
    if (strcmp(root->data, "*") == 0)
        return left * right;
    if (strcmp(root->data, "/") == 0)
        return left / right;
    if (strcmp(root->data, ">") == 0)
        return left > right;
    if (strcmp(root->data, "<") == 0)
        return left < right;
    if (strcmp(root->data, "=") == 0)
        return left == right;
    if (strcmp(root->data, "&") == 0)
        return left && right;
    if (strcmp(root->data, "|") == 0)
        return left || right;

    return 0;
}

int main()
{
    Node *root = createNode("&");
    root->left = createNode(">");
    root->right = createNode("<");
    root->left->left = createNode("+");
    root->left->right = createNode("12");
    root->left->left->left = createNode("10");
    root->left->left->right = createNode("5");
    root->right->left = createNode("8");
    root->right->right = createNode("15");
    printf("Inorder (SQL-like): ");
    inorder(root);
    printf("\n");
    int result = evaluate(root);
    printf("Evaluated Result: %d\n", result);

    if (result)
        printf("TRUE\n");
    else
        printf("FALSE\n");

    return 0;
}
