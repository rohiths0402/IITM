#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 100
#define MAX_SUBJECTS 20
#define PASS_MARKS 35  // Threshold for pass in each subject

typedef struct {
    char *name;
    int marks;
} Subject;

typedef struct {
    char *name;
    int roll_no;
    Subject *subjects;
    float total;
    float average;
} Student;

// Function to add a new student
int add_student(Student *students, int *student_count, int subject_count, char **subject_names) {
    Student *s = &students[*student_count];

    s->name = (char *)malloc(MAX_NAME_LEN);
    printf("Enter Student Name: ");
    scanf(" %[^\n]", s->name);

    printf("Enter Roll Number: ");
    scanf("%d", &s->roll_no);

    s->subjects = (Subject *)malloc(sizeof(Subject) * subject_count);
    s->total = 0;

    for (int i = 0; i < subject_count; i++) {
        s->subjects[i].name = subject_names[i];
        printf("Enter Marks for %s: ", subject_names[i]);
        scanf("%d", &s->subjects[i].marks);
        s->total += s->subjects[i].marks;
    }

    s->average = s->total / subject_count;
    (*student_count)++;
    return 1;
}

// Function to display a single student's details
void display_student(Student s, int subject_count) {
    printf("\nName: %s\n", s.name);
    printf("Roll No: %d\n", s.roll_no);
    printf("Subjects and Marks:\n");

    for (int i = 0; i < subject_count; i++) {
        printf("%s: %d\n", s.subjects[i].name, s.subjects[i].marks);
    }

    printf("Total: %.2f\n", s.total);
    printf("Average: %.2f\n", s.average);
}

// Function to export student data to a file
void export_student(Student s, int subject_count) {
    char filename[50];
    sprintf(filename, "student_%d.txt", s.roll_no);
    FILE *fptr = fopen(filename, "w");

    if (!fptr) {
        printf("Error writing to file!\n");
        return;
    }

    fprintf(fptr, "Name: %s\n", s.name);
    fprintf(fptr, "Roll No: %d\n", s.roll_no);
    fprintf(fptr, "Subjects and Marks:\n");

    for (int i = 0; i < subject_count; i++) {
        fprintf(fptr, "%s: %d\n", s.subjects[i].name, s.subjects[i].marks);
    }

    fprintf(fptr, "Total: %.2f\n", s.total);
    fprintf(fptr, "Average: %.2f\n", s.average);
    fclose(fptr);

    printf("Exported to %s successfully.\n", filename);
}

// Function to compute class stats
void class_stats(Student *students, int student_count, int subject_count) {
    float class_total = 0;
    int total_marks_possible = student_count * subject_count * 100;
    int pass_count = 0;

    for (int i = 0; i < student_count; i++) {
        class_total += students[i].total;

        int passed = 1;
        for (int j = 0; j < subject_count; j++) {
            if (students[i].subjects[j].marks < PASS_MARKS) {
                passed = 0;
                break;
            }
        }
        if (passed) pass_count++;
    }

    float class_percentage = (class_total / total_marks_possible) * 100;
    float pass_percentage = ((float)pass_count / student_count) * 100;

    printf("\nClass Percentage: %.2f%%\n", class_percentage);
    printf("Pass Percentage: %.2f%% (%d out of %d students passed)\n", pass_percentage, pass_count, student_count);
}

int main() {
    int NumberOfStudents;
    int SubjectCount;
    char **subjectNames;

    printf("Enter the number of subjects: ");
    scanf("%d", &SubjectCount);

    if (SubjectCount < 3) {
        printf("Minimum 3 subjects required.\n");
        return 1;
    }

    // Allocate memory for subject names
    subjectNames = (char **)malloc(sizeof(char *) * SubjectCount);
    for (int i = 0; i < SubjectCount; i++) {
        subjectNames[i] = (char *)malloc(MAX_NAME_LEN);
        printf("Enter name of subject %d: ", i + 1);
        scanf(" %[^\n]", subjectNames[i]);
    }

    Student students[100];  // can dynamically allocate if needed
    int student_count = 0;

    int choice;
    do {
        printf("\n1. Add Student\n2. Display All\n3. Export\n4. Class Stats\n5. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        if (choice == 1) {
            add_student(students, &student_count, SubjectCount, subjectNames);
        } else if (choice == 2) {
            if (student_count == 0) {
                printf("No students added yet.\n");
            } else {
                for (int i = 0; i < student_count; i++)
                    display_student(students[i], SubjectCount);
            }
        } else if (choice == 3) {
            if (student_count == 0) {
                printf("No students to export.\n");
            } else {
                for (int i = 0; i < student_count; i++)
                    export_student(students[i], SubjectCount);
            }
        } else if (choice == 4) {
            if (student_count == 0) {
                printf("No student data available for statistics.\n");
            } else {
                class_stats(students, student_count, SubjectCount);
            }
        } else if (choice != 5) {
            printf("Invalid choice.\n");
        }
    } while (choice != 5);

    // Free allocated memory
    for (int i = 0; i < SubjectCount; i++)
        free(subjectNames[i]);
    free(subjectNames);

    for (int i = 0; i < student_count; i++) {
        free(students[i].name);
        free(students[i].subjects);
    }

    printf("Program terminated.\n");
    return 0;
}
