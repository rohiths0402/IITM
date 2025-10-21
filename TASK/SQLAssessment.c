#include <stdio.h>
#include <string.h>
#include <time.h>

#define MAX_RECORDS 500
#define NAME_LEN 50
#define DEPT_LEN 30

typedef struct{
    int id;
    char name[NAME_LEN];
    char department[DEPT_LEN];
} Employee;

void populateRecords(Employee records[]){
    for(int i = 0; i < MAX_RECORDS; i++){
        records[i].id = i + 1;
        snprintf(records[i].name, NAME_LEN, "Employee_%03d", i + 1);
        if(i % 3 == 0){
            strcpy(records[i].department, "Engineering");
        }
        else if(i % 3 == 1){
            strcpy(records[i].department, "Marketing");
        }
        else{
            strcpy(records[i].department, "HR");
        }
    }
}

int linearSearch(Employee records[], const char *targetDept){
    int found = 0;
    for(int i = 0; i < MAX_RECORDS; i++){
        if(strcmp(records[i].department, targetDept) == 0){
            found++;
        }
    }
    return found;
}

int binarySearch(Employee records[], int targetId){
    int low = 0, high = MAX_RECORDS - 1;
    while(low <= high){
        int mid =(low + high) / 2;
        if(records[mid].id == targetId){
            return mid;
        }
        else if(records[mid].id < targetId){
            low = mid + 1;
        }
        else{
            high = mid - 1;
        }
    }
    return -1;
}

int main(){
    Employee records[MAX_RECORDS];
    populateRecords(records);
    const char *targetDept = "Engineering";
    int targetId = 333;
    clock_t start = clock();
    int linearMatches = linearSearch(records, targetDept);
    clock_t end = clock();
    double linearTime =(double)(end - start) / CLOCKS_PER_SEC;
    start = clock();
    int index = binarySearch(records, targetId);
    end = clock();
    double binaryTime =(double)(end - start) / CLOCKS_PER_SEC;
    printf("Linear Search:\n");
    printf("Matches for department '%s': %d\n", targetDept, linearMatches);
    printf("Time taken: %.6f seconds\n\n", linearTime);
    printf("Binary Search:\n");
    if(index != -1){
        printf("Record found at index %d: ID=%d, Name=%s, Department=%s\n", index, records[index].id, records[index].name, records[index].department);
    } 
    else{
        printf("Record with ID %d not found.\n", targetId);
    }
    printf("Time taken: %.6f seconds\n", binaryTime);

    return 0;
}