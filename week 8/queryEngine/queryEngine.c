#include <stdio.h>
#include <string.h>

#define MAX_CHAR    10
#define TABLESIZE   5
#define LIMIT   1

//Query: SELECT name, age FROM students WHERE city="chennai" LIMIT 1;

typedef struct Student{
    char name[MAX_CHAR];
    int age;
    char city[MAX_CHAR];
}Student_t;

Student_t students[] = {{"Rohith", 23, "chennai"},
                      {"Naveen", 10, "bangalore"},
                      {"Dinesh", 18, "Coimbatore"},
                      {"Vignesh", 24, "chennai"},
                      {"Praveen", 26, "Mumbai"}};

int currentIndex = 0;
int currentLimit = 0;
Student_t* current = NULL;

void table_scan_open(){
    currentIndex = 0;
    printf("Table scan opened!\n");
}

Student_t* table_scan_next(){
    if(currentIndex >= TABLESIZE)
        return NULL;
    return &students[currentIndex++];
}

void table_scan_close(){
    printf("Table scan closed!\n");
}

void filter_open(){
    printf("Filter opened!(city = chennai)\n");
    table_scan_open();
}

Student_t* filter_next(){
    while((current = table_scan_next()) != NULL){
        if(strcmp(current->city, "chennai") == 0){
            return current;
        }
    }
    return NULL;
}

void filter_close(){
    printf("Filter closed!\n");
    table_scan_close();
}

void limit_open(){
    printf("Limit opened!(1)\n");
    filter_open();
}

Student_t* limit_next(){
    if(currentLimit >= LIMIT)
        return NULL;
    current = filter_next();
    if(current != NULL){
        currentLimit++;
        return current;
    }
    return NULL;
}

void limit_close(){
    printf("Limit closed!\n");
    filter_close();
}

void projection_open(){
    printf("Projection opened!(Name and Age)\n");
    limit_open();

}

void projection_next(){
    printf("--------------------\n");
    while((current = limit_next()) != NULL){
        printf("%s %d", current->name, current->age);
        printf("\n");
    }
    printf("--------------------\n");
}

void projection_close(){
    printf("Projection closed!\n");
    limit_close();
}

int main(){
    projection_open();
    projection_next();
    projection_close();
    return 0;
}