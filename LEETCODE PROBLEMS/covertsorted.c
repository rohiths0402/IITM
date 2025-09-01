/**
 * Definition for singly-linked list.
 * function ListNode(val, next) {
 *     this.val = (val===undefined ? 0 : val)
 *     this.next = (next===undefined ? null : next)
 * }
 */
/**
 * Definition for a binary tree node.
 * function TreeNode(val, left, right) {
 *     this.val = (val===undefined ? 0 : val)
 *     this.left = (left===undefined ? null : left)
 *     this.right = (right===undefined ? null : right)
 * }
 */
/**
 * @param {ListNode} head
 * @return {TreeNode}
 */
var sortedListToBST = function(head) {
    function findMiddle(start, end) {
        let slow = start;
        let fast = start;
        while (fast !== end && fast.next !== end) {
            slow = slow.next;
            fast = fast.next.next;
        }
        return slow;
    }
    function buildBST(start, end) {
        if (start === end) return null; 
        let mid = findMiddle(start, end);
        let root = new TreeNode(mid.val);
        root.left = buildBST(start, mid);
        root.right = buildBST(mid.next, end);
        return root;
    }

    return buildBST(head, null);
};