#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX 3

typedef struct BPlusTreeNode
{
   int keys[MAX];
   int count;
   struct BPlusTreeNode* child[MAX+1];
   bool isLeaf;
}BplusTreeNode;

BPlusTreeNode* createNode(bool isLeaf)
{
    BTreeNode* node = (BPlusTreeNode*)malloc(sizeof(BPlusTreeNode));
    node->isLeaf = isLeaf;
    node->count = 0;
    node->next = NULL;

    for (int iterator = 0; iterator <= MAX; iterator++)
    {
        node->child[iterator] = NULL;
    }

    return node;
}

void insertLeadf(BplusTreeNode* leaf,int keys){
    int i=0;
    while(i>=0&& keys< leaf->keys[i]){
        leaf->keys[i+1] = leaf->keys[i];
        i--;
    }
    leaf->keys[i+1] = keys;
    leaf->count++;
}

bool searchKey(int keyToBeSearched, BTreeNode* root)
{
    if(root == NULL)
    {
        return false;
    }
    int iterator = 0;

    while (iterator < root->count && keyToBeSearched > root->keys[iterator])
    {
        iterator++;
    }

    if(iterator < root->count && keyToBeSearched == root->keys[iterator])
    {
        return true;
    }

    if(root->isLeaf)
    {
        return false;
    }

    return searchKey(keyToBeSearched, root->child[iterator]);
}

void printRange(BplusTreeNode* root,int start,int end){
    while(!root->isLeaf){
        int i=0;
        while(i<root->count&& start >root->keys[i]){
            i++;
        }
        root= root->child[i];
    }
    while(root){
        for(int i=0;i<root;i++){
            if(root->keys[i]>= stand && root->keys[i] <= end){
                printf("%d ", root->keys[i]);
            }
            if(root->keys[i] >end){
                return;
            }
        }
        root = root->next;
    }
}


int main(){
    

    return 0;
}