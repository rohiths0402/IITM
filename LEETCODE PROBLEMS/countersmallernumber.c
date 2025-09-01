#include <stdlib.h>

int lowbit(int x) { return x & -x; }

void update(int* bit, int idx, int val, int size) {
    while (idx <= size) {
        bit[idx] += val;
        idx += lowbit(idx);
    }
}

int query(int* bit, int idx) {
    int sum = 0;
    while (idx > 0) {
        sum += bit[idx];
        idx -= lowbit(idx);
    }
    return sum;
}

int cmp(const void* a, const void* b) {
    return *(int*)a - *(int*)b;
}

/**
 * Note: The returned array must be malloced, assume caller calls free().
 */
int* countSmaller(int* nums, int numsSize, int* returnSize) {
    int* result = malloc(sizeof(int) * numsSize);
    if (numsSize == 0) {
        *returnSize = 0;
        return result;
    }

    // Coordinate compression
    int* sorted = malloc(sizeof(int) * numsSize);
    for (int i = 0; i < numsSize; i++) sorted[i] = nums[i];
    qsort(sorted, numsSize, sizeof(int), cmp);
    int* rank = malloc(sizeof(int) * numsSize);
    for (int i = 0; i < numsSize; i++){
        int left = 0, right = numsSize - 1, r = 0;
        while (left <= right) {
            int mid = (left + right) / 2;
            if (sorted[mid] < nums[i]) {
                r = mid + 1;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        rank[i] = r + 1; 
    }

    int maxRank = numsSize + 2;
    int* bit = calloc(maxRank, sizeof(int));

    for (int i = numsSize - 1; i >= 0; i--) {
        result[i] = query(bit, rank[i] - 1);
        update(bit, rank[i], 1, maxRank - 1);
    }

    free(sorted);
    free(rank);
    free(bit);

    *returnSize = numsSize;
    return result;
}