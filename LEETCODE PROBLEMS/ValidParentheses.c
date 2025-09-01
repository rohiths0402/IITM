bool isValid(char* s){
    typedef struct Node{
        char ch;
        struct Node* next;
    }Node;

    Node* stack = NULL;
    for(int i = 0; s[i] != '\0'; i++){
        char c = s[i];
        if(c == '(' || c == '{' || c == '['){
            Node* newNode = (Node*)malloc(sizeof(Node));
            newNode->ch = c;
            newNode->next = stack;
            stack = newNode;
        } 
        else{
            if(!stack) return false;
            char topChar = stack->ch;
            Node* temp = stack;
            stack = stack->next;
            free(temp);
            if((c == ')' && topChar != '(') || (c == '}' && topChar != '{') || (c == ']' && topChar != '[')){
                return false;
            }
        }
    }
    return stack == NULL;
}
