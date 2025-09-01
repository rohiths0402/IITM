#include <stdio.h>
#include <stdlib.h>

#define BLOOM_SIZE 20
int bloom_fliter[BLOOM_SIZE];

void printBloom(){
    for (int i = 0; i < BLOOM_SIZE; i++)
    {
        printf("%d ", bloom_filter[i]);
    }
    printf("\n");
}

int hashFunctionOne(char *word){
    int sum = 0;
    for (int i = 0; word[i] != '\0'; i++)
    {
        sum += word[i];
    }
    return sum % BLOOM_SIZE;
}

int hashFunctionTwo(char *word){
    int sum = 0;
    for (int i = 0; word[i] != '\0'; i++)
    {
        sum += (i + 1) * word[i]; 
    }
    return sum % BLOOM_SIZE;
}

void insert(char *word){
    int firstIndex = hashFunctionOne(word);
    int secondIndex = hashFunctionTwo(word);
    bloom_filter[firstIndex] = 1;
    bloom_filter[secondIndex] = 1;
    printf("Inserted %s at indices %d and %d\n", word, firstIndex, secondIndex);
    printBloom();
}

int search(char *word){
    int firstIndex = hashFunctionOne(word);
    int secondIndex = hashFunctionTwo(word);

    if (bloom_filter[firstIndex] == 1 && bloom_filter[secondIndex] == 1){
        printf("\"%s\" is possibly present at positions %d %d\n",
        word, firstIndex, secondIndex);
        return 1;
    }
    else{
        printf("\"%s\" is definitely not present at positions %d %d\n",
        word, firstIndex, secondIndex);
        return 0; 
    }
}


int main(){
    memset(bloom_fliter, 0,sizeof(bloom_fliter));
    insert("cat");
    insert("dog");
    insert("rat");
    insert("bat");
    search("cat");
    bloom_filter[9] = 1;
    bloom_filter[18] = 1;
    search("cow"); 

    return 0;
}