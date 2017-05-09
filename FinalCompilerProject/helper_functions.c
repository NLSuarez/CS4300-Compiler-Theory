

#include "helper_functions.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>    // need it for va_list, va_start in the error functions

// Useful for debugging and populating the symbol table - need a better, non-hack solution for this in the future.
#define INT_LITERAL 258
#define FLT_LITERAL 259
#define STR_LITERAL 260
#define CIN 261
#define COUT 262
#define ELSE 263
#define ENDL 264
#define FLOAT 265
#define IF 266
#define INT 267
#define RETURN 268
#define WHILE 269
#define ID 270
#define FUNC 271
#define EOL 272
#define ASSIGNOP 273
#define MULOP 274
#define ADDOP 275
#define INCOP 276
#define RELOP 277
#define NOT 278
#define OR 279
#define AND 280
#define STREAMIN 281
#define STREAMOUT 282
#define UNARY 283


    /* GLOBAL VARIABLES */
int hel = 0;

enum { KEYWORD_COUNT = 9 };

enum { TABLE_SIZE = 9997};

char* C_KEYWORD_ARRAY[KEYWORD_COUNT] = { "cin", "cout", "else", "endl", "float",
                        "if", "int", "return", "while" };

struct scope_node* global_scope;
struct scope_node* current_scope;

STRLIT_LIST str_list_head;
STRLIT_LIST VMQ_list;

struct ast* ast_root;

struct eval_data* val;

FILE* source_file;

    /* FUNCTION DEFINITIONS */

struct strlit_node* appendToStrList(STRLIT_LIST* head, char* str, int eval_state)
{
    STRLIT_LIST ptr = *head;

    if(*head != NULL && !eval_state)
    {
        while(ptr != NULL && ptr->str != str)
        {
            if (strcmp(ptr->str, str) == 0)
                return ptr;
            else
                ptr = ptr->next;
        }
    }

    if(LEX_DEBUG) printf("\tAdding %s to STRLIT_LIST\n", str);

    if (*head == NULL)
    {
        if(LEX_DEBUG) printf("\tStarting new STRLIT_LIST...");
        *head = (struct strlit_node*)malloc(sizeof(struct strlit_node));
        (*head)->str = strdup(str);
        (*head)->loc = 0;            // This assumes no global variables!
        (*head)->next = NULL;
        if(LEX_DEBUG) printf("\tNew node allocated and initialized\n");
        fflush(stdout);
        ptr = *head;
    }
    else
    {
        ptr = *head;
        while(ptr->next != NULL) ptr = ptr->next;
        STRLIT_LIST prev = ptr;
        ptr = ptr->next = (struct strlit_node*)malloc(sizeof(struct strlit_node));
        ptr->str = strdup(str);
        if (!eval_state) ptr->loc = prev->loc + strlen(prev->str) - 1;
        else             ptr->loc = 0;
        if (strcmp(prev->str, "\\n") == 0) ptr->loc++;
        ptr->next = NULL;
    }

    if(LEX_DEBUG) printf("append: ptr->str = %s\tptr->loc = %d\n", ptr->str, ptr->loc);

    return ptr;
}

struct scope_node* pushScope()
{
    if(DEBUG) printf("\n\tPushing scope...");
    struct scope_node* newScope = (struct scope_node*) malloc(sizeof(struct scope_node));
    newScope->symTab = generateSymbolTable(TABLE_SIZE);
    newScope->next = NULL;
    newScope->is_new_scope = 1;

    fflush(stdout);
    // If global_scope is uninitialized, then set global_scope to equal the new scope and return.
    if (global_scope == NULL)
        global_scope = current_scope = newScope;

    current_scope->next = newScope;

    current_scope = newScope;

    if(DEBUG) printf("\tDone!\n");
    if(DEBUG) printf("current_scope = %llX; global_scope = %llX\n", (unsigned long long)current_scope, (unsigned long long)global_scope);
    return current_scope;
}

void popScope()
{
    if(DEBUG) printf("Popping scope... freeing memory...");
    fflush(stdout);

    struct scope_node* ptr = global_scope;

    while(ptr->next != current_scope) ptr = ptr->next;

    ptr->next = NULL;

    //for(int i = 0; i < TABLE_SIZE; i++)
    //    free(current_scope->symTab[i].symbol);
    free(current_scope->symTab);
    free(current_scope);

    if(DEBUG) printf("\tDone!\n");
    fflush(stdout);

    current_scope = global_scope;
    while(current_scope->next != NULL)
        current_scope = current_scope->next;

    if(DEBUG) printf("current_scope = %llX; global_scope = %llX\n", (unsigned long long)current_scope, (unsigned long long)global_scope);
}

unsigned int hash(const char* symbol)
{
    unsigned int hashKey = 0;
    int i;
    for (i = 0; symbol[i] != 0; ++i)
        hashKey += symbol[i];

    hashKey = hashKey % TABLE_SIZE;

    return hashKey;
}

struct symbol_node* lookup(const char* symbol)
{
    if(DEBUG) printf("ENTERED lookup: symbol = %s\n", symbol);
    // Hash the key.
    unsigned int hashKey = hash(symbol);

    // Current scope starts at global, works towards deepest scope.
    struct scope_node* scopePtr = global_scope;
    SYMBOL_TABLE tablePtr = scopePtr->symTab;
    struct symbol_node* symNode = NULL;

    while(symNode == NULL)
    {
            struct symbol_node* ptr = &tablePtr[hashKey];

            /* Check for symbols in linked list of hash table entry */
            while(ptr != NULL)
            {
                        if(ptr->symbol != NULL && strcmp(symbol, ptr->symbol) == 0)
                        {
                                        if(DEBUG && scopePtr == global_scope) printf("\n\tFound symbol %s in global scope!\n", symbol);
                                        return ptr;
                        }
                        else
                            ptr = ptr->next;
            }
            /* Symbol not found, check deeper scopes */
            if (ptr == NULL)
            {
                        scopePtr = scopePtr->next;

                        /* Symbol not found - ran out of scope */
                        if (scopePtr == NULL)
                            return (struct symbol_node*)NULL;

                        tablePtr = scopePtr->symTab;
            }

    }

    fflush(stdout);

    return symNode;
}

struct symbol_node* addSymbol(SYMBOL_TABLE symTab, char* str, int kind)
{
    unsigned int hashKey = hash(str);

    if(DEBUG) printf("\n\tAdding symbol \"%s\"\n", str);

    // Case 1: No entry at hash table index.
    if (symTab[hashKey].symbol == NULL && symTab[hashKey].next == NULL)
    {
            symTab[hashKey].symbol = strdup(str);
            symTab[hashKey].kind = kind;
            return &(symTab[hashKey]);
    }

    // Case 2: Existing entry at hash table index.  Find end of list and append.
    struct symbol_node* ptr = &symTab[hashKey];

    while(ptr->next != NULL)
        ptr = ptr->next;

    ptr = ptr->next = (struct symbol_node*) malloc(sizeof(struct symbol_node));

    ptr->symbol = strdup(str); ptr->kind = kind; ptr-> next = NULL;

    return ptr;
}

SYMBOL_TABLE generateSymbolTable(unsigned int tableSize)
{
    // Allocate memory for symbol table
    SYMBOL_TABLE symTab = (SYMBOL_TABLE) malloc(tableSize * sizeof(struct symbol_node));

    if (symTab == NULL)
        return NULL;    // Couldn't allocate memory

    // Initialize all elements of new table to NULL
    for (unsigned int i = 0; i < tableSize; ++i)
    {
                    symTab[i].symbol = NULL;
                    symTab[i].next = NULL;
                    symTab[i].kind = 0;
    }

    // Only populate global_scope with C++ keywords (might not even need to do this anymore...)
    if (global_scope == NULL)    populateSymbolTable(symTab);

    return symTab;
}

void populateSymbolTable(SYMBOL_TABLE symTab)
{
    if(symTab == NULL)    return;

    for (int i = 0; i < KEYWORD_COUNT; ++i)    addSymbol(symTab, C_KEYWORD_ARRAY[i], CIN + i);

    return;
}


void printSymbolData(struct symbol_node* sym)
{
    printf("Address of symbol:\t0x%llX\n", (unsigned long long)sym);
    printf("Contained string:\t%s\n", sym->symbol);
    printf("Symbol kind:\t\t%s\n", kindToString(sym->kind));
    if(sym->kind == INT) printf("Symbol value:\t\t%d\n", sym->val.d);
    if(sym->kind == FLOAT) printf("Symbol value:\t\t%f\n", sym->val.f);
    printf("\n");
}


int getKind(char *str)
{
    for(int i = 0; i < KEYWORD_COUNT; ++i)
    {
                    if (strcmp(str, C_KEYWORD_ARRAY[i]) == 0)
                        return (CIN + i);
    }
    extern unsigned int DEBUG;

    if(DEBUG) printf("!!getKind: -1 return on str = %s\n\n", str);
    return -1;
}



char* kindToString(const int kind)
{
    // Stupid solution, but it works.
    if (kind == CIN)            return "CIN";
    if (kind == COUT)            return "COUT";
    if (kind == ELSE)            return "ELSE";
    if (kind == ENDL)            return "ENDL";
    if (kind == FLOAT)            return "FLOAT";
    if (kind == IF)                return "IF";
    if (kind == INT)            return "INT";
    if (kind == RETURN)            return "RETURN";
    if (kind == WHILE)            return "WHILE";
    if (kind == STR_LITERAL)    return "STR_LITERAL";
    if (kind == ID)                return "ID";

    return "UNKNOWN";
}


int main(int argc, char **argv)
{
    extern unsigned int DEBUG;
    extern unsigned int PAR_DEBUG;
    extern unsigned int LEX_DEBUG;
    extern int yyparse();
    DEBUG = PAR_DEBUG = LEX_DEBUG = 0;

    extern FILE* yyin;

    if (argc > 1)
    {
        // Assuming file is last argument in command line, to allow for DEBUG flags.
        char* ptr = argv[argc-1]; ptr = ptr+strlen(ptr) - 4;
        if(strcmp(ptr, ".cpp") == 0) yyin = fopen(argv[argc-1], "r");
        for (int i = 2; i < argc; ++i)
        {
            if (strcmp(argv[i], "-d") == 0)        DEBUG = 1;
            if (strcmp(argv[i], "-pd") == 0)    PAR_DEBUG = 1;
            if (strcmp(argv[i], "-ld") == 0)    LEX_DEBUG = 1;
        }
    }
    #if YYDEBUG
        yydebug = 1;
    #endif

    global_scope = current_scope = NULL;
    str_list_head = NULL;
    VMQ_list = NULL;
    ast_root = NULL;
    val = NULL;
    global_scope = pushScope();

    yyparse();

    fclose(yyin);

    eval(ast_root);

    STRLIT_LIST list_ptr = str_list_head;

    char* filename = strdup(argv[argc-1]);

    char* str_ptr = filename + strlen(filename) - 3;
    *str_ptr = 'q'; str_ptr++;
    *str_ptr = '\0';

    FILE* VMQ_file = fopen(filename, "w");

    while(list_ptr != NULL)
    {
        if(strcmp(list_ptr->str, "\\n") == 0)
        {
            sprintf(str_ptr, "%d \"%s\"\n", list_ptr->loc, list_ptr->str);
            fputs(str_ptr, VMQ_file);
        }
        else
        {
            sprintf(str_ptr, "%d %s\n", list_ptr->loc, list_ptr->str);
            fputs(str_ptr, VMQ_file);
        }

        if(list_ptr->next == NULL)
        {
            // It's a hack, but it works for now.
            sprintf(str_ptr, "$ 1 %lu\n", list_ptr->loc + strlen(list_ptr->str) - 2);
            fputs(str_ptr, VMQ_file);
        }

        list_ptr = list_ptr->next;
    }

    list_ptr = VMQ_list;
    while(list_ptr != NULL)
    {
        fputs(list_ptr->str, VMQ_file);
        fputs("\n", VMQ_file);
        list_ptr = list_ptr->next;
    }

    fclose(VMQ_file);

    if(DEBUG || LEX_DEBUG || PAR_DEBUG) printf("\n\nPROGRAM EXIT\n\n");

    return 0;
}


void yyerror(char* s, ...)
{
    if(DEBUG) printf("\n\nENTERED yyerror\n\n");
    va_list ap;
    va_start(ap, s);

    // print out where the error occurs
    if(yylloc.first_line)
    {
                    fprintf(stderr, "%d.%d-%d.%d: ", yylloc.first_line, yylloc.first_column,
                                                                    yylloc.last_line, yylloc.last_column);
    }
    vfprintf(stderr, s, ap);    // print out the error decription
    fprintf(stderr, "\n");
    //printf("%s\n", s);
}

void pError(errorLevel el, char* s, ...)
{
    if(DEBUG) printf("\n\nENTERED pError\n\n");
    va_list ap;
    va_start(ap, s);
    char *els[3] = {"Warning", "Error", "Fatal"};

    // update the severity level countered
    if (el > hel)
    {
                    hel = el;
    }

    fprintf(stderr, "%s: %d.%d-%d.%d: ", els[el - 1], yylloc.first_line, yylloc.first_column,
                                yylloc.last_line, yylloc.last_column);
    vfprintf(stderr, s, ap);
    fprintf(stderr, "\n");
    //printf("%s\n", s);
}

/*
 * functions to build the AST
 */
 struct ast *
 newast(int nodetype, struct ast *l, struct ast *r)
 {
   struct ast *a = malloc(sizeof(struct ast));

    a->l = a->r = NULL;
   if(!a) {
     pError(fatal, "out of space");
     exit(-1);
   }
   //printf("\n\n\tAllocated new AST, initializing...");
   a->nodetype = nodetype;
   a->l = l;
   a->r = r;
   //printf("Done!\n");

   return a;
 }

 struct ast *
 newrel(int reltype, struct ast *l, struct ast *r)
 {
   struct ast *a = malloc(sizeof(struct ast));

   if(!a) {
     pError(fatal, "out of space");
     exit(-1);
   }

   a->nodetype = reltype;
   a->l = l;
   a->r = r;
 }

 struct ast *
 newstr(struct strlit_node* strliteral) {
     struct stringval *a = malloc(sizeof(struct stringval));
     if(!a) {
             pError(fatal, "out of space");
             exit(-1);
     }
     a->nodetype = STR_LITERAL;
     a->str = strliteral;
    return (struct ast*)a;
 }

 struct ast *
 newint(int num)
 {
     struct intval *a = malloc(sizeof(struct intval));
     if(!a) {
                     pError(fatal, "out of space");
                     exit(-1);
     }
     a->nodetype = INT; //VMQ defines an int using f
     a->number = num;
     return (struct ast *)a;
 }

 struct ast *
 newfloat(float num)
 {
     struct floatval *a = malloc(sizeof(struct floatval));
     if(!a) {
                     pError(fatal, "out of space");
                     exit(-1);
     }
     a->nodetype = FLOAT; //VMQ defines a float using F
     a->number = num;
     return (struct ast *)a;
 }

/*
 struct ast *
 newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el) {
     struct flow *a = malloc(sizeof(struct flow));

     if(!a) {
             pError(fatal, "out of space");
             exit(-1);
     }
     a->nodetype = nodetype;
     a->cond = cond;
     a->tl = tl;
     a->el = el;
     return (struct ast *)a;
 }
*/
 struct ast *
 newref(struct symbol_node *s) {
    struct symref *a = malloc(sizeof(struct symref));

    if(!a) {
            pError(fatal, "out of space");
            exit(-1);
    }
    a->nodetype = ID;
    a->s = s;

    return (struct ast*)a;
 }
// struct ast *newasgn(struct symbol_node *s, struct ast *v);

// Function to delete and free AST
  void
    treefree(struct ast *a)
    {
        //switch(a->nodetype) {
            /* cases here will be basod on parser */
        //}
    }


void eval(struct ast *a)
{
    if(a == NULL) { return; }

    switch (a->nodetype)
    {
        /* TERMINAL CASES */

        case COUT:            break;
        case RETURN:        break;
        case STR_LITERAL:    (void)0; char* VMQ_push_stmt = (char*)malloc(20);
                            STRLIT_LIST ptr = str_list_head; char* str = strdup(((struct stringval*)a)->str->str);
                            while(ptr != NULL) { if(strcmp(ptr->str, str)== 0) break; else ptr = ptr->next; }
                            sprintf(VMQ_push_stmt, "p #%d", ptr->loc);
                            appendToStrList(&VMQ_list, VMQ_push_stmt, 1);
                            appendToStrList(&VMQ_list, "c 0 -11", 1);
                            appendToStrList(&VMQ_list, "^ 2", 1);
                            break;

        // output_statement
        case 'o':                    if(a->l != NULL) eval(a->l);
                                    if(a->r->nodetype == STR_LITERAL) eval(a->r); break;

        // statement
        case ('s'+'t'+'m'+'t'):         eval(a->l); break;

        // statements
        case ('s'+'t'+'m'+'t'+'s'):     if(a->l != NULL) eval(a->l);
                                     if(a->r != NULL) eval(a->r); break;

        // block
        case ('b'+'l'+'k'):            /* eval(a->l) eventually for var_defs */ eval(a->r); break;

        case ('f'+'d'):                /* eval(a->l) eventually for func_head */ eval(a->r); break;

        case 'h':                    eval(a->r); appendToStrList(&VMQ_list, "h", 1); break;

        default:                    if(a->l != NULL) eval(a->l); if(a->r != NULL) eval(a->r);
    }

}
/*
void transferStack(VMQ_STACK dest, VMQ_STACK src)
{
    if (src == NULL) return;

    VMQ_STACK temp = (VMQ_STACK)malloc(sizeof(struct strlit_node));

    printf("\nSTACK TRANSFER IN PROCESS!\n");
    while(src != NULL)
    {
        pushToStrStack(temp, strdup(src->str));
        popStrStack(src);
    }

    while(temp != NULL)
    {
        pushToStrStack(dest, strdup(temp->str));
        printf("\tString transfered = %s\n", temp->str);
        popStrStack(temp);
    }

    return;
}

void pushToStrStack(VMQ_STACK stk, char* str)
{
    if(val->stack_head == NULL)
    {
        printf("\n\tStack is empty, pushing %s...", str);
        val->stack_head = (struct strlit_node*)malloc(sizeof(struct strlit_node));
        val->stack_head->str = strdup(str);
        val->stack_head->next = NULL;
        printf("Done!\n");
    }
    else
    {
        printf("\n\tStack is not empty, pushing %s...", str);
        struct strlit_node* ptr = (struct strlit_node*)malloc(sizeof(struct strlit_node));
        ptr->str = strdup(str);
        ptr->next = stk;
        val->stack_head = ptr;
        printf("Done!\n");
    }
}

void popStrStack(VMQ_STACK stk)
{
    if(stk == NULL) return;

    VMQ_STACK ptr = stk->next;
    free(stk);
    stk = ptr;
}
*/
void printAST(struct ast *a)
{
    struct ast *ptr = a;
    if (ptr == NULL) return;

    if(ptr->nodetype == STR_LITERAL)
    {
        printf("\tSTRINGVAL NODE\n");
        //printf("\tstr = %s\tVMQ Loc = %d\n", ((struct stringval*)ptr)->str->str, ((struct stringval*)ptr)->str->loc);
        //fflush(stdout);
    }
    else if(ptr->nodetype == ID)
    {
        printf("\tSYMREF NODE\n");
        //printSymbolData(((struct symref*)ptr)->s);
    }
    else
    {
        //printf("\n\t!!ptr->nodetype != STR_LITERAL\n");
        printAST(ptr->l);
        if (ptr->nodetype == 's'+'t'+'m'+'t')   printf("\tSTATEMENT NODE\n");
        else if (ptr->nodetype == 'o')          printf("\tOUTSTMT NODE (o)\n");
        else if (ptr->nodetype == COUT)            printf("\tCOUT NODE\n");
        else if (ptr->nodetype == 'h')            printf("\tROOT NODE (PROGRAM)\n");
        else if (ptr->nodetype == 'f'+'d')        printf("\tFUNC DEF NODE\n");
        else if (ptr->nodetype == 'f'+'h')        printf("\tFUNC HEAD NODE\n");
        else if (ptr->nodetype == 'a'+'r'+'g')    printf("\tARG NODE\n");
        else if (ptr->nodetype == 'p'+'l')        printf("\tPARAM LIST NODE\n");
        else if (ptr->nodetype == 'p'+'a'+'r')    printf("\tPARAMETERS NODE\n");
        else if (ptr->nodetype == 'b'+'l'+'k')    printf("\tBLOCK NODE\n");
        else if (ptr->nodetype == 'v'+'a'+'r')    printf("\tVAR DEF NODE\n");
        else if (ptr->nodetype == 's'+'t'+'m'+'t'+'s') printf("\tSTATEMENTS NODE\n");
        else if (ptr->nodetype == RETURN)        printf("\tRETURN NODE\n");
        else                                    printf("\tOTHER NODE (%d)\n", ptr->nodetype);

        //printAST(ptr->l);
        printAST(ptr->r);
    }
}
