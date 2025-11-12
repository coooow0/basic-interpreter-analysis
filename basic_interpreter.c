/* Basic Interpreter by H?eyin Uslu raistlinthewiz@hotmail.com */
/* Code licenced under GPL */

#include <stdio.h>
// #include <conio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef _WIN32
#define CLEAR() system("cls")
#else
#define CLEAR() system("clear")
#endif

struct node {
    int type; /* 1 var, 2 function, 3 function call, 4 begin, 5 end */
    char exp_data;
    int val;
    int line;
    struct node* next;
};
typedef struct node Node;

struct stack { Node* top; };
typedef struct stack Stack;

struct opnode { char op; struct opnode* next; };
typedef struct opnode opNode;

struct opstack { opNode* top; };
typedef struct opstack OpStack;

struct postfixnode { int val; struct postfixnode* next; };
typedef struct postfixnode Postfixnode;

struct postfixstack { Postfixnode* top; };
typedef struct postfixstack PostfixStack;

static int GetVal(char, int*, Stack*);
static int GetLastFunctionCall(Stack*);
static Stack* FreeAll(Stack*);
static int my_stricmp(const char* a, const char* b);
static void rstrip(char* s);

static Stack* Push(Node sNode, Stack* stck)
{
    Node* newnode = (Node*)malloc(sizeof(Node));
    if (!newnode) { printf("ERROR, Couldn't allocate memory..."); return NULL; }
    newnode->type = sNode.type;
    newnode->val = sNode.val;
    newnode->exp_data = sNode.exp_data;
    newnode->line = sNode.line;
    newnode->next = stck->top;
    stck->top = newnode;
    return stck;
}

static OpStack* PushOp(char op, OpStack* opstck)
{
    opNode* newnode = (opNode*)malloc(sizeof(opNode));
    if (!newnode) { printf("ERROR, Couldn't allocate memory..."); return NULL; }
    newnode->op = op;
    newnode->next = opstck->top;
    opstck->top = newnode;
    return opstck;
}

static char PopOp(OpStack* opstck)
{
    opNode* temp;
    char op;
    if (opstck->top == NULL)
    {
        return 0;
    }
    op = opstck->top->op;
    temp = opstck->top;
    opstck->top = opstck->top->next;
    free(temp);
    return op;
}

static PostfixStack* PushPostfix(int val, PostfixStack* poststck)
{
    Postfixnode* newnode = (Postfixnode*)malloc(sizeof(Postfixnode));
    if (!newnode) { printf("ERROR, Couldn't allocate memory..."); return NULL; }
    newnode->val = val;
    newnode->next = poststck->top;
    poststck->top = newnode;
    return poststck;
}

static int PopPostfix(PostfixStack* poststck)
{
    Postfixnode* temp;
    int val;
    if (poststck->top == NULL)
    {
        return 0;
    }
    val = poststck->top->val;
    temp = poststck->top;
    poststck->top = poststck->top->next;
    free(temp);
    return val;
}

static void Pop(Node* sNode, Stack* stck)
{
    Node* temp;
    if (stck->top == NULL) return;
    sNode->exp_data = stck->top->exp_data;
    sNode->type = stck->top->type;
    sNode->line = stck->top->line;
    sNode->val = stck->top->val;
    temp = stck->top;
    stck->top = stck->top->next;
    free(temp);
}

static int isStackEmpty(OpStack* stck)
{
    return stck->top == 0;
}

static int Priotry(char operator)
{
    if ((operator=='+') || (operator=='-')) return 1;
    else if ((operator=='/') || (operator=='*')) return 2;
    return 0;
}

int main(int argc, char** argv)
{
    char line[4096];
    char dummy[4096];
    char lineyedek[4096];
    char postfix[4096];
    char* firstword;

    int val1;
    int val2;

    int LastExpReturn = 0;
    int LastFunctionReturn = -999;
    int CalingFunctionArgVal = 0;

    Node tempNode;

    OpStack* MathStack = (OpStack*)malloc(sizeof(OpStack));
    FILE* filePtr;
    PostfixStack* CalcStack = (PostfixStack*)malloc(sizeof(PostfixStack));
    int resultVal = 0;
    Stack* STACK = (Stack*)malloc(sizeof(Stack));

    int curLine = 0;
    int foundMain = 0;
    int WillBreak = 0;

    if (!MathStack || !CalcStack || !STACK) {
        printf("Memory alloc failed\n");
        return 1;
    }
    MathStack->top = NULL;
    CalcStack->top = NULL;
    STACK->top = NULL;

    CLEAR();

    if (argc != 2)
    {
        printf("Incorrect arguments!\n");
        printf("Usage: %s <inputfile.spl>", argv[0]);
        return 1;
    }

    filePtr = fopen(argv[1], "r");
    if (filePtr == NULL)
    {
        printf("Can't open %s. Check the file please", argv[1]);
        return 2;
    }

    while (fgets(line, 4096, filePtr))
    {
        int k = 0;

        while (line[k] != '\0')
        {
            if (line[k] == '\t') line[k] = ' ';
            k++;
        }

        rstrip(line);
        strcpy(lineyedek, line);

        curLine++;
        tempNode.val = -999;
        tempNode.exp_data = ' ';
        tempNode.line = -999;
        tempNode.type = -999;

        if (my_stricmp("begin", line) == 0)
        {
            if (foundMain)
            {
                tempNode.type = 4;
                STACK = Push(tempNode, STACK);
            }
        }
        else if (my_stricmp("end", line) == 0)
        {
            if (foundMain)
            {
                int sline;
                tempNode.type = 5;
                STACK = Push(tempNode, STACK);

                sline = GetLastFunctionCall(STACK);
                if (sline == 0)
                {
                    printf("Output=%d", LastExpReturn);
                }
                else
                {
                    int j;
                    int foundCall = 0;
                    LastFunctionReturn = LastExpReturn;

                    fclose(filePtr);
                    filePtr = fopen(argv[1], "r");
                    curLine = 0;
                    for (j = 1; j < sline; j++)
                    {
                        fgets(dummy, 4096, filePtr);
                        curLine++;
                    }

                    while (foundCall == 0)
                    {
                        Pop(&tempNode, STACK);
                        if (tempNode.type == 3) foundCall = 1;
                    }
                }
            }
        }
        else
        {
            firstword = strtok(line, " ");
            if (!firstword) continue;

            if (my_stricmp("int", firstword) == 0)
            {
                if (foundMain)
                {
                    tempNode.type = 1;
                    firstword = strtok(NULL, " ");
                    if (!firstword) continue;
                    tempNode.exp_data = firstword[0];

                    firstword = strtok(NULL, " ");
                    if (!firstword) continue;

                    if (my_stricmp("=", firstword) == 0)
                    {
                        firstword = strtok(NULL, " ");
                        if (!firstword) continue;
                    }

                    tempNode.val = atoi(firstword);
                    tempNode.line = 0;
                    STACK = Push(tempNode, STACK);
                }
            }
            else if (my_stricmp("function", firstword) == 0)
            {
                firstword = strtok(NULL, " ");
                if (!firstword) continue;

                tempNode.type = 2;
                tempNode.exp_data = firstword[0];
                tempNode.line = curLine;
                tempNode.val = 0;
                STACK = Push(tempNode, STACK);

                if (firstword[0] == 'm' && firstword[1] == 'a' && firstword[2] == 'i' && firstword[3] == 'n')
                {
                    foundMain = 1;
                }
                else
                {
                    if (foundMain)
                    {
                        firstword = strtok(NULL, " ");
                        if (!firstword) continue;
                        tempNode.type = 1;
                        tempNode.exp_data = firstword[0];
                        tempNode.val = CalingFunctionArgVal;
                        tempNode.line = 0;
                        STACK = Push(tempNode, STACK);
                    }
                }
            }
            else if (firstword[0] == '(')
            {
                if (foundMain)
                {
                    int i = 0;
                    int y = 0;

                    MathStack->top = NULL;

                    while (lineyedek[i] != '\0')
                    {
                        if (isdigit((unsigned char)lineyedek[i]))
                        {
                            postfix[y] = lineyedek[i];
                            y++;
                        }
                        else if (lineyedek[i] == ')')
                        {
                            if (!isStackEmpty(MathStack))
                            {
                                postfix[y] = PopOp(MathStack);
                                y++;
                            }
                        }
                        else if (lineyedek[i] == '+' || lineyedek[i] == '-' || lineyedek[i] == '*' || lineyedek[i] == '/')
                        {
                            if (isStackEmpty(MathStack))
                            {
                                MathStack = PushOp(lineyedek[i], MathStack);
                            }
                            else
                            {
                                if (Priotry(lineyedek[i]) <= Priotry(MathStack->top->op))
                                {
                                    postfix[y] = PopOp(MathStack);
                                    y++;
                                    MathStack = PushOp(lineyedek[i], MathStack);
                                }
                                else
                                {
                                    MathStack = PushOp(lineyedek[i], MathStack);
                                }
                            }
                        }
                        else if (isalpha((unsigned char)lineyedek[i]) > 0)
                        {
                            int codeline = 0;
                            int dummyint = 0;
                            int retVal = GetVal(lineyedek[i], &codeline, STACK);

                            if ((retVal != -1) && (retVal != -999))
                            {
                                postfix[y] = (char)(retVal + 48);
                                y++;
                            }
                            else
                            {
                                if (LastFunctionReturn == -999)
                                {
                                    int j;
                                    tempNode.type = 3;
                                    tempNode.line = curLine;
                                    STACK = Push(tempNode, STACK);

                                    CalingFunctionArgVal = GetVal(lineyedek[i + 2], &dummyint, STACK);

                                    fclose(filePtr);
                                    filePtr = fopen(argv[1], "r");
                                    curLine = 0;

                                    for (j = 1; j < codeline; j++)
                                    {
                                        fgets(dummy, 4096, filePtr);
                                        curLine++;
                                    }

                                    WillBreak = 1;
                                    break;
                                }
                                else
                                {
                                    postfix[y] = (char)(LastFunctionReturn + 48);
                                    y++;
                                    i = i + 3;
                                    LastFunctionReturn = -999;
                                }
                            }
                        }
                        i++;
                    }

                    if (WillBreak == 0)
                    {
                        while (!isStackEmpty(MathStack))
                        {
                            postfix[y] = PopOp(MathStack);
                            y++;
                        }

                        postfix[y] = '\0';

                        i = 0;
                        CalcStack->top = NULL;
                        while (postfix[i] != '\0')
                        {
                            if (isdigit((unsigned char)postfix[i]))
                            {
                                CalcStack = PushPostfix(postfix[i] - '0', CalcStack);
                            }
                            else if (postfix[i] == '+' || postfix[i] == '-' || postfix[i] == '*' || postfix[i] == '/')
                            {
                                val1 = PopPostfix(CalcStack);
                                val2 = PopPostfix(CalcStack);

                                switch (postfix[i])
                                {
                                case '+': resultVal = val2 + val1; break;
                                case '-': resultVal = val2 - val1; break;
                                case '/': resultVal = val2 / val1; break;
                                case '*': resultVal = val2 * val1; break;
                                }
                                CalcStack = PushPostfix(resultVal, CalcStack);
                            }
                            i++;
                        }

                        LastExpReturn = CalcStack->top->val;
                    }
                    WillBreak = 0;
                }
            }
        }
    }

    fclose(filePtr);
    STACK = FreeAll(STACK);
    // STACK = FreeAll(&STACK);
    printf("\nPress a key to exit...");
    // getch();
    getchar();
    return 0;
}

static Stack* FreeAll(Stack* stck)
{
    Node* head = stck->top;
    while (head) {
        Node* temp = head;
        head = head->next;
        free(temp);
    }
    stck->top = NULL;
    return NULL;
}

static int GetLastFunctionCall(Stack* stck)
{
    Node* head = stck->top;
    while (head) {
        if (head->type == 3) return head->line;
        head = head->next;
    }
    return 0;
}

static int GetVal(char exp_name, int* line, Stack* stck)
{
    Node* head;
    *line = 0;
    if (stck->top == NULL) return -999;
    head = stck->top;
    while (head) {
        if (head->exp_data == exp_name)
        {
            if (head->type == 1) return head->val;
            else if (head->type == 2) { *line = head->line; return -1; }
        }
        head = head->next;
    }
    return -999;
}

static int my_stricmp(const char* a, const char* b)
{
    unsigned char ca, cb;
    while (*a || *b) {
        ca = (unsigned char)tolower((unsigned char)*a);
        cb = (unsigned char)tolower((unsigned char)*b);
        if (ca != cb) return (int)ca - (int)cb;
        if (*a) a++;
        if (*b) b++;
    }
    return 0;
}

static void rstrip(char* s)
{
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r' || s[n - 1] == ' ')) s[--n] = '\0';
}
