/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */

// Utility: Inorder traversal to store nodes in array
void inorder(struct TreeNode* root, int* arr, int* size) {
    if (!root) return;
    inorder(root->left, arr, size);
    arr[(*size)++] = root->val;
    inorder(root->right, arr, size);
}

struct TreeNode* buildBalanced(int* arr, int start, int end) {
    if (start > end) return NULL;
    int mid = start + (end - start) / 2;
    struct TreeNode* node = (struct TreeNode*)malloc(sizeof(struct TreeNode));
    node->val = arr[mid];
    node->left = buildBalanced(arr, start, mid - 1);
    node->right = buildBalanced(arr, mid + 1, end);

    return node;
}

struct TreeNode* balanceBST(struct TreeNode* root){
    int arr[10000];
    int size = 0;
    inorder(root, arr, &size);
    return buildBalanced(arr, 0, size - 1);
}