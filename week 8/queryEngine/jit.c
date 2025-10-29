#include <stdio.h>
#include <time.h>
#include <string.h>

int interpreted_eval(int marks, const char* city) {
    return (marks > 80 && strcmp(city, "Pune") == 0);
}

// Pretend JIT — pre-compiled logic (no interpretation)
int compiled_eval(int marks, const char* city) {
    // Machine-level equivalent logic already known
    if (marks > 80 && city[0] == 'P') return 1;
    return 0;
}

int main() {
    clock_t start, end;
    int count = 0;

    // Simulate 10 million evaluations
    start = clock();
    for (int i = 0; i < 10000000; i++) {
        count += interpreted_eval(85, "Pune");
    }
    end = clock();
    printf("Interpreted time: %.2f seconds\n",
           (double)(end - start) / CLOCKS_PER_SEC);

    count = 0;
    start = clock();
    for (int i = 0; i < 10000000; i++) {
        count += compiled_eval(85, "Pune");
    }
    end = clock();
    printf("Pre-compiled (JIT-style) time: %.2f seconds\n",
           (double)(end - start) / CLOCKS_PER_SEC);
}
