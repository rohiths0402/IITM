long countWhileMergeSort(long* sums, int left, int right, int lower, int upper) {
    if (right - left <= 1) return 0;

    int mid = (left + right) / 2;
    long count = countWhileMergeSort(sums, left, mid, lower, upper) +
                 countWhileMergeSort(sums, mid, right, lower, upper);

    int j = mid, k = mid, t = mid;
    long* cache = (long*)malloc((right - left) * sizeof(long));
    int r = 0;

    for (int i = left; i < mid; ++i) {
        while (k < right && sums[k] - sums[i] < lower) k++;
        while (j < right && sums[j] - sums[i] <= upper) j++;
        count += j - k;

        while (t < right && sums[t] < sums[i]) cache[r++] = sums[t++];
        cache[r++] = sums[i];
    }

    for (int i = 0; i < t - left; ++i)
        sums[left + i] = cache[i];

    free(cache);
    return count;
}

int countRangeSum(int* nums, int numsSize, int lower, int upper) {
    long* sums = (long*)malloc((numsSize + 1) * sizeof(long));
    sums[0] = 0;
    for (int i = 0; i < numsSize; ++i)
        sums[i + 1] = sums[i] + nums[i];

    int result = countWhileMergeSort(sums, 0, numsSize + 1, lower, upper);
    free(sums);
    return result;
}