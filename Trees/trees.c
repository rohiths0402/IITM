#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int data;
    struct Node* left;
    struct Node* right;
} Node;

Node* createNode(int value){
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode){
        printf("Memory allocation failed!\n");
        exit(1);
    }
    newNode->data = value;
    newNode->left = NULL;
    newNode->right = NULL;
    return newNode;
}
typedef struct Queue {
    Node** arr;
    int front, rear, size;
} Queue;

Queue* createQueue(int size){
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->arr = (Node**)malloc(size * sizeof(Node*));
    q->front = q->rear = 0;
    q->size = size;
    return q;
}

int isEmpty(Queue* q){
    return q->front == q->rear;
}
void enqueue(Queue* q, Node* node){
    q->arr[q->rear++] = node;
}
Node* dequeue(Queue* q){
    return q->arr[q->front++];
}
void insertLevelOrder(Node** root, int value) {
    if (*root == NULL){
        *root = createNode(value);
        return;
    }
    Queue* q = createQueue(100); 
    enqueue(q, *root);
    while (!isEmpty(q)) {
        Node* temp = dequeue(q);
        if(temp->left == NULL){
            temp->left = createNode(value);
            break;
        } 
        else{
            enqueue(q, temp->left);
        }
        if (temp->right == NULL){
            temp->right = createNode(value);
            break;
        } else {
            enqueue(q, temp->right);
        }
    }
    free(q->arr);
    free(q);
}

void printLevelOrder(Node* root){
    if (root == NULL) return;
    Queue* q = createQueue(100);
    enqueue(q, root);
    while (!isEmpty(q)){
        Node* temp = dequeue(q);
        printf("%d ", temp->data);
        if (temp->left) enqueue(q, temp->left);
        if (temp->right) enqueue(q, temp->right);
    }
    free(q->arr);
    free(q);
}

int main(){
    Node* root = NULL;
    int n, value;
    printf("Enter number of nodes to insert: ");
    scanf("%d", &n);
    for (int i = 0; i < n; i++){
        printf("Enter value for node %d: ", i + 1);
        scanf("%d", &value);
        insertLevelOrder(&root, value);
    }
    printf("\nLevel order traversal: ");
    printLevelOrder(root);
    printf("\n");
    return 0;
}
