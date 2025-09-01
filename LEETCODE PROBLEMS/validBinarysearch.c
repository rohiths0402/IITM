/**
 * Definition for a binary tree node.
 * struct TreeNode {
 *     int val;
 *     struct TreeNode *left;
 *     struct TreeNode *right;
 * };
 */


bool validate(struct TreeNode* root, long minVal, long maxVal) {
    if(root == NULL){
        return true;
    } 
    if(root->val <= minVal || root->val >= maxVal){
        return false;
    }
    return validate(root->left, minVal, root->val) && validate(root->right, root->val, maxVal);
}
bool isValidBST(struct TreeNode* root) {
    return validate(root, LONG_MIN, LONG_MAX);
}
