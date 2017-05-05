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


// To keep track of the highest severity of error, 0 = no errors, 1 = warning, 2 = error, 3 = fatal
int hel = 0;

// This should be kept up-to-date if more keywords are added to the set.
enum { KEYWORD_COUNT = 9 };

enum { TABLE_SIZE = 9997};

char* C_KEYWORD_ARRAY[KEYWORD_COUNT] = { "cin", "cout", "else", "endl", "float",
                        "if", "int", "return", "while" };

struct scope_node* global_scope;
struct scope_node* current_scope;
STRLIT_LIST str_list_head;
struct ast* ast_root;

struct strlit_node* appendToStrList(char* str)
{
    struct strlit_node* ptr = str_list_head;

    if(ptr != NULL && lookup(str) != NULL)
    {
        while(ptr != NULL && ptr->str != str) 
        {
            if (strcmp(ptr->str, str) == 0)
            {
                printf("\tappend: string already in list, ptr->str = %s\tptr->loc = %d\n", ptr->str, ptr->loc);
                return ptr;
            }
            else
                ptr = ptr->next;
        }
    }
    if (ptr == NULL)
    {
        if(LEX_DEBUG) printf("\tAdding %s to STRLIT_LIST\n", str);
        
        ptr = str_list_head;

        if (ptr == NULL)
        {
            if(LEX_DEBUG) printf("\tStarting new STRLIT_LIST...");
            ptr = (struct strlit_node*)malloc(sizeof(struct strlit_node));
            ptr->str = strdup(str);
            ptr->loc = 0;            // This assumes no global variables!
            ptr->next = NULL;
            str_list_head = ptr;
            if(LEX_DEBUG) printf("\tNew node allocated and initialized\n");
            fflush(stdout);
        }
        else
        {
            while (ptr->next != NULL) ptr = ptr->next;

            struct strlit_node* prev = ptr;
            ptr = ptr->next = (struct strlit_node*)malloc(sizeof(struct strlit_node));
    
            ptr->str = strdup(str);
            ptr->loc = prev->loc + strlen(prev->str) - 1;
            ptr->next = NULL;
        }
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

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
        {
            if (strcmp(argv[i], "-d") == 0) DEBUG = 1;
            if (strcmp(argv[i], "-pd") == 0) PAR_DEBUG = 1;
            if (strcmp(argv[i], "-ld") == 0) LEX_DEBUG = 1;
        }
    }
    #if YYDEBUG
        yydebug = 1;
    #endif

    global_scope = current_scope = NULL;
    str_list_head = NULL;
    ast_root = NULL;

    global_scope = pushScope();

    yyparse();

    if(DEBUG || LEX_DEBUG) 
    {
        printf("\n!! Dumping string literal list !!\n");
        struct strlit_node* ptr = str_list_head;
        while(ptr != NULL)
        {
            printf("\tString = %s\t\t\t| VMQ Loc = %d\n", ptr->str, ptr->loc);
            ptr = ptr->next;
        }
    }

    // ast_root now points to root of AST tree

    // Set up VMQ program's global memory using strlit_nodes in str_list_head linked list
    
    // Evaluate the AST, print results to VMQ file.

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
                /* cases here will be based on parser */
        //}
    }


void printAST(struct ast *a)
{
    struct ast *ptr = a;
    if (ptr == NULL) return;

    if(ptr->nodetype == STR_LITERAL)
    {
        printf("\tSTRINGVAL NODE");
        printf("\tstr = %s\tVMQ Loc = %d\n", ((struct stringval*)ptr)->str->str, ((struct stringval*)ptr)->str->loc);
        fflush(stdout);
    }
    else if(ptr->nodetype == ID)
    {
        printf("\tSYMREF NODE\n");
        printSymbolData(((struct symref*)ptr)->s);
    }
    else
    {
        //printf("\n\t!!ptr->nodetype != STR_LITERAL\n");
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
        
        printAST(ptr->l);
        printAST(ptr->r);
    }
}

