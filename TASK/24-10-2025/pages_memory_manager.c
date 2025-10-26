#include <stdio.h>
#include <stdlib.h>

#define TOTAL_PAGES 16  
#define FREE 0
#define USED 1

int page_table[TOTAL_PAGES];

void init_pages();
void print_page_table();
void allocate_pages(int n);
void free_pages(int start, int n);
int find_free_block(int n);
void show_menu();

int main() {
    int choice, n, start;

    init_pages();
    printf("*******Simple Memory Page Manager*****\n");
    print_page_table();

    while (1) {
        show_menu();
        printf("Enter choice: ");
        if (scanf("%d", &choice) != 1) break;

        switch (choice) {
            case 1:
                printf("Enter number of pages to allocate: ");
                scanf("%d", &n);
                allocate_pages(n);
                break;
            case 2:
                printf("Enter starting page index to free: ");
                scanf("%d", &start);
                printf("Enter number of pages to free: ");
                scanf("%d", &n);
                free_pages(start, n);
                break;
            case 3:
                print_page_table();
                break;
            case 0:
                printf("Exiting memory manager\n");
                exit(0);
            default:
                printf("Invalid choice\n");
        }
    }

    return 0;
}

void init_pages() {
    for (int i = 0; i < TOTAL_PAGES; i++) {
        page_table[i] = FREE;
    }
}

void print_page_table() {
    int used = 0;
    printf("\nPage Table:\n");
    printf("------------------------\n");
    printf("Page | Status\n");
    printf("------------------------\n");
    for (int i = 0; i < TOTAL_PAGES; i++) {
        printf("  %2d  | %s\n", i, page_table[i] == USED ? "USED" : "FREE");
        if (page_table[i] == USED) used++;
    }
    printf("Memory Usage: %d/%d pages used (%.1f%%)\n\n",used, TOTAL_PAGES, (used * 100.0) / TOTAL_PAGES);
}

void allocate_pages(int n) {
    if (n <= 0 || n > TOTAL_PAGES) {
        printf("Invalid number of pages.\n");
        return;
    }

    int start = find_free_block(n);
    if (start == -1) {
        printf("Error: Not enough contiguous free pages.\n");
        return;
    }

    for (int i = start; i < start + n; i++) {
        page_table[i] = USED;
    }

    printf("Allocated %d pages starting from page %d.\n", n, start);
    print_page_table();
}

void free_pages(int start, int n) {
    if (start < 0 || start >= TOTAL_PAGES || n <= 0 || start + n > TOTAL_PAGES) {
        printf("Invalid range.\n");
        return;
    }

    for (int i = start; i < start + n; i++) {
        if (page_table[i] == FREE) {
            printf("Warning: Page %d is already free.\n", i);
        }
        page_table[i] = FREE;
    }

    printf("Freed %d pages starting from page %d.\n", n, start);
    print_page_table();
}

int find_free_block(int n) {
    int count = 0;
    for (int i = 0; i < TOTAL_PAGES; i++) {
        if (page_table[i] == FREE) {
            count++;
            if (count == n) {
                return i - n + 1;
            }
        } else {
            count = 0;
        }
    }
    return -1;
}

void show_menu() {
    printf("1. Allocate pages\n");
    printf("2. Free pages\n");
    printf("3. Show page table\n");
    printf("0. Exit\n");
}
