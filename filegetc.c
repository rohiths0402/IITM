#include <stdio.h>

int main(){

    FILE* fptr = fopen("file.txt","r");

    if(fptr == NULL){
        printf("file cant be access");
    }
    else{
        int data = fgetc(fptr);
        if(data != EOF){
            putchar(data);
        }
        fclose(fptr);
      
    }

    return 0;
}