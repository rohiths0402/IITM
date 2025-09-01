int* maxSlidingWindow(int* nums, int numsSize, int k, int* returnSize) {
    if(numsSize == 0 || k == 0){
        *returnSize = 0;
        return NULL;
    }
    int windowCount = numsSize - k + 1;
    int* result = malloc(windowCount * sizeof(int));
    int* deque = malloc(numsSize * sizeof(int));
    int front = 0, back = 0;
    *returnSize = 0;
    for(int i = 0; i < numsSize; i++){
        while (back > front && nums[deque[back - 1]] < nums[i]) {
            back--;
        }
        deque[back++] = i;
        if(deque[front] <= i - k){
            front++;
        }
        if(i >= k - 1){
            result[*returnSize] = nums[deque[front]];
            (*returnSize)++;
        }
    }
    free(deque);
    return result;
}
