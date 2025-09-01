/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */
/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
void inorderHelper(struct TreeNode* root, int* arr, int* index){
    if (root == NULL) return;
    inorderHelper(root->left, arr, index);
    arr[(*index)++] = root->val;
    inorderHelper(root->right, arr, index);
}

int* inorderTraversal(struct TreeNode* root, int* returnSize) {
    int* result = (int*)malloc(100 * sizeof(int));
    int index = 0;
    inorderHelper(root, result, &index);
    *returnSize = index;
    return result;
}
