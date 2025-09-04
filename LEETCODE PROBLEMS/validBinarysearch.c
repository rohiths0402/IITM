/**
 * Return an array of arrays of size *returnSize.
 * The sizes of the arrays are returned as *returnColumnSizes array.
 * Note: Both returned array and *columnSizes array must be malloced, assume caller calls free().
 */

#define TABLE_SIZE 10007

typedef struct Nord{
    char* word;
    struct Nord* next;
}Nord;

typedef struct Entry{
    char* key;
    Nord* words;
    int count;
    struct Entry* next;
}Entry;

Entry* hashTable[TABLE_SIZE];

unsigned int hash(const char* str){
    unsigned int h = 0;
    while (*str){
        h = h * 131 + *str++;
    }
    return h % TABLE_SIZE;
}

char* generateKey(char* str){
    int count[26] = {0};
    for (int i = 0; str[i]; i++){
        count[str[i] - 'a']++;
    }

    char* key = (char*)malloc(100);
    key[0] = '\0';
    char buffer[10];
    for (int i = 0; i < 26; i++){
        if (count[i] > 0) {
            sprintf(buffer, "%c%d", 'a' + i, count[i]);
            strcat(key, buffer);
        }
    }
    return key;
}

void insert(char* word){
    char* key = generateKey(word);
    unsigned int idx = hash(key);
    Entry* curr = hashTable[idx];
    while(curr){
        if(strcmp(curr->key, key) == 0){
            Nord* newWord = (Nord*)malloc(sizeof(Nord));
            newWord->word = word;
            newWord->next = curr->words;
            curr->words = newWord;
            curr->count++;
            free(key);
            return;
        }
        curr = curr->next;
    }
    Entry* newEntry = (Entry*)malloc(sizeof(Entry));
    newEntry->key = key;
    newEntry->words = (Nord*)malloc(sizeof(Nord));
    newEntry->words->word = word;
    newEntry->words->next = NULL;
    newEntry->count = 1;
    newEntry->next = hashTable[idx];
    hashTable[idx] = newEntry;
}

char*** groupAnagrams(char** strs, int strsSize, int* returnSize, int** returnColumnSizes) {
    memset(hashTable, 0, sizeof(hashTable));
    for(int i = 0; i < strsSize; i++){
        insert(strs[i]);
    }
    int groups = 0;
    for(int i = 0; i < TABLE_SIZE; i++){
        Entry* curr = hashTable[i];
        while (curr){
            groups++;
            curr = curr->next;
        }
    }
    *returnSize = groups;
    *returnColumnSizes = (int*)malloc(groups * sizeof(int));
    char*** result = (char***)malloc(groups * sizeof(char**));
    int g = 0;
    for(int i = 0; i < TABLE_SIZE; i++){
        Entry* curr = hashTable[i];
        while (curr) {
            (*returnColumnSizes)[g] = curr->count;
            result[g] = (char**)malloc(curr->count * sizeof(char*));
            Nord* w = curr->words;
            int j = 0;
            while (w) {
                result[g][j++] = w->word;
                w = w->next;
            }
            g++;
            curr = curr->next;
        }
    }
    return result;
}