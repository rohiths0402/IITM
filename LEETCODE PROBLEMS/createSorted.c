#include <stdlib.h>

#define MOD 1000000007

int lowbit(int x) { return x & (-x); }

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
 * Note: The returned value is modulo 10^9 + 7
 */
int createSortedArray(int* instructions, int instructionsSize) {
    if (instructionsSize == 0) return 0;

    int* sorted = malloc(sizeof(int) * instructionsSize);
    for (int i = 0; i < instructionsSize; i++) sorted[i] = instructions[i];
    qsort(sorted, instructionsSize, sizeof(int), cmp);

    int* rank = malloc(sizeof(int) * instructionsSize);
    for (int i = 0; i < instructionsSize; i++) {
        int left = 0, right = instructionsSize - 1, r = 0;
        while (left <= right) {
            int mid = (left + right) / 2;
            if (sorted[mid] < instructions[i]) {
                r = mid + 1;
                left = mid + 1;
            } else {
                right = mid - 1;
            }
        }
        rank[i] = r + 1; 
    }

    int maxRank = instructionsSize + 2;
    int* bit = calloc(maxRank, sizeof(int));
    long cost = 0;

    for (int i = 0; i < instructionsSize; i++) {
        int leftCount = query(bit, rank[i] - 1);
        int rightCount = i - query(bit, rank[i]);
        cost = (cost + (leftCount < rightCount ? leftCount : rightCount)) % MOD;
        update(bit, rank[i], 1, maxRank - 1);
    }

    free(sorted);
    free(rank);
    free(bit);

    return (int)cost;
}