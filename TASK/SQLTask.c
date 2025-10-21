#include <stdio.h>
#include <string.h>

#define MAX_RECORDS 500
#define NAME_LEN 50
#define DEPT_LEN 30

typedef struct {
    int id;
    char name[NAME_LEN];
    char department[DEPT_LEN];
} Employee;

void populateRecords(Employee records[]) {
    for (int i = 0; i < MAX_RECORDS; i++) {
        records[i].id = i + 1;
        snprintf(records[i].name, NAME_LEN, "Employee_%03d", i + 1);
        if (i % 3 == 0)
            strcpy(records[i].department, "Engineering");
        else if (i % 3 == 1)
            strcpy(records[i].department, "Marketing");
        else
            strcpy(records[i].department, "HR");
    }
}

void searchByDepartment(Employee records[], const char *targetDept) {
    int found = 0;
    printf("Searching for department: %s\n\n", targetDept);
    for (int i = 0; i < MAX_RECORDS; i++) {
        if (strcmp(records[i].department, targetDept) == 0) {
            printf("ID: %d, Name: %s, Department: %s\n",
                   records[i].id, records[i].name, records[i].department);
            found++;
        }
    }
    if (found == 0)
        printf("No records found for department: %s\n", targetDept);
    else
        printf("\nTotal matches: %d\n", found);
}

int main() {
    Employee records[MAX_RECORDS];
    populateRecords(records);
    searchByDepartment(records, "Engineering");

    return 0;
}