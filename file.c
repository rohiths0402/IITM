#include <stdio.h>

int main(){

    char data[] = "qwertyuiop \n hello world";

    FILE* fptr = fopen("file.txt","w");

    if(fptr == NULL){
        printf("file cant be access");
    }
    else{
        fputs(data, fptr);
        fputs("\n", fptr);
        fclose(fptr);
    }

    return 0;
}