#include "helper_functions.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>    // need it for va_list, va_start in the error functions

// Useful for debugging and populating the symbol table - may need a better, non-hack solution for this in the future.
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
#define INCOP 275
#define RELOP 276
#define NOT 277
#define OR 278
#define AND 279
#define STREAMIN 280
#define STREAMOUT 281
#define ADDOP 282
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

INTLIT_LIST int_list_head;

FUNC_LIST func_list_head;
FUNC_LIST func_list_tail;

struct ast* ast_root;

FILE* source_file;

unsigned int func_max_temp_vars = 0;

    /* FUNCTION DEFINITIONS */

/* 
    This function is used to append to a list of symbolnode pointers, which 
    can also serve as just plain strings for tracking VMQ statement strings
*/
struct strlit_node* appendToStrList(STRLIT_LIST* head, char* str, int eval_state)
{
    STRLIT_LIST ptr = *head;

    // Look for existing string, so we don't make duplicates.
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
    
    // List is empty, initialize with new string
    if (*head == NULL)
    {
        *head = (struct strlit_node*)malloc(sizeof(struct strlit_node));
        (*head)->str = strdup(str);
        (*head)->loc = 0;
        (*head)->next = NULL;
        ptr = *head;
	}
    else    // List is not empty, append string to end of list.
    {
        ptr = *head;
        while(ptr->next != NULL) ptr = ptr->next;
        STRLIT_LIST prev = ptr;
        ptr = ptr->next = (struct strlit_node*)malloc(sizeof(struct strlit_node));
        
            ptr->str = strdup(str);
        ptr->loc = 0;            // VMQ Location will be handled later.
        ptr->next = NULL;
	}
    
    return ptr;
}

// Used to append intlit structs to the end of a intlit_node linked list.
// The linked list is used to track INT_LITERALs encountered by the lexer
// so that we can add them to the VMQ global memory space later.
struct intlit_node* appendToIntList(INTLIT_LIST* head, int val)
{
    INTLIT_LIST ptr = *head;

    // Look for existing int val, so we don't make duplicates.
    if(ptr != NULL)
    {
        while(ptr != NULL)
        {
            if (ptr->val == val)
                return ptr;
                    else
                ptr = ptr->next;
		}
	}

    ptr = *head;
    
    // List is empty, initialize with new int node
    if (ptr == NULL)
    {
        ptr = (struct intlit_node*)malloc(sizeof(struct intlit_node));
        
            ptr->val = val;
        ptr->loc = 0;
        ptr->next = NULL;
            *head = ptr;
	}
    else    // List is not empty, append node to end of list.
    {
            ptr = *head;
        while(ptr->next != NULL) ptr = ptr->next;
        INTLIT_LIST prev = ptr;
        ptr = ptr->next = (struct intlit_node*)malloc(sizeof(struct intlit_node));
       
            ptr->val = val;
        ptr->loc = 0;                // VMQ Location will be handled later.
        ptr->next = NULL;
	}

    return ptr;
}

struct var_node* appendToFuncVars(struct symbol_node* var)
{
    if(!func_list_tail) return (struct var_node*)NULL;
    
    struct var_node* vars_ptr = func_list_tail->vars;

    struct var_node* new_var = vars_ptr;
    if(!vars_ptr)
    {
        new_var = malloc(sizeof(struct var_node));
        new_var->symbol = strdup(var->symbol);
        new_var->kind = var->kind;
        new_var->loc = 2;
        new_var->next = NULL;

        new_var->isParam = 0;

        if(new_var->kind == FLOAT)
            func_list_tail->end_var_addr = 4;
        else
            func_list_tail->end_var_addr = 2;

        func_list_tail->vars = new_var;
	}
    else
    {
        while(vars_ptr->next != NULL) vars_ptr = vars_ptr->next;

        new_var = vars_ptr->next = malloc(sizeof(struct var_node));

        new_var->symbol = strdup(var->symbol);
        new_var->kind = var->kind;

        if(vars_ptr->kind == FLOAT)
            new_var->loc = vars_ptr->loc + 4;
        else
            new_var->loc = vars_ptr->loc + 2;
        
        // Floats are 4 bytes and must be aligned on a four-byte boundary.
        if(new_var->kind == FLOAT && new_var->loc % 4 != 0) new_var->loc += 2;
        
        new_var->next = NULL;

        new_var->isParam = 0;

        if(new_var->kind == FLOAT)
            func_list_tail->end_var_addr += 4;
        else
            func_list_tail->end_var_addr += 2;
	}

    func_list_tail->var_count++;

    return new_var;
}

struct var_node* appendToFuncParams(struct symbol_node* param)
{
    if(!func_list_tail) return (struct var_node*)NULL;

    struct var_node* params_ptr = func_list_tail->params;

    struct var_node* new_param = params_ptr;
    if(!params_ptr)
    {
        new_param = malloc(sizeof(struct var_node));
        new_param->symbol = strdup(param->symbol);
        new_param->kind = param->kind;
        new_param->loc = 6;            // Parameters start at /6.
        new_param->next = NULL;
        new_param->isParam = 1;

        if(new_param->kind == FLOAT)
            func_list_tail->end_param_addr = new_param->loc + 4;
        else
            func_list_tail->end_param_addr = new_param->loc + 2;

        func_list_tail->params = params_ptr = new_param;
	}
    else
    {
        while(params_ptr->next != NULL) params_ptr = params_ptr->next;

        new_param = params_ptr->next = malloc(sizeof(struct var_node));

        new_param->symbol = strdup(param->symbol);
        new_param->kind = param->kind;

        if(params_ptr->kind == FLOAT)
            new_param->loc = params_ptr->loc + 4;
        else
            new_param->loc = params_ptr->loc + 2;

        // Floats are 4 bytes and must be aligned on a four-byte boundary.
        if(new_param->kind == FLOAT && new_param->loc % 4 != 0) new_param->loc += 2;

        new_param->next = NULL;

        new_param->isParam = 1;

        if(new_param->kind == FLOAT)
           func_list_tail->end_param_addr += 4;
        else
           func_list_tail->end_param_addr += 2;
	}

    func_list_tail->param_count++;

    return new_param;

}

struct funcvars_node* appendNewFunc(struct symbol_node* func)
{
    if(func_list_head == NULL)
    {
        func_list_head = func_list_tail = malloc(sizeof(struct funcvars_node));

        func_list_head->func = func;
        func_list_head->params = NULL;
        func_list_head->vars = NULL;
        func_list_head->param_count = func_list_head->var_count = 0;
        func_list_head->next = NULL;
	}    
    else
    {
        func_list_tail = func_list_tail->next = malloc(sizeof(struct funcvars_node));

        func_list_tail->func = func;
        func_list_tail->params = NULL;
        func_list_tail->vars = NULL;
        func_list_tail->param_count = func_list_tail->var_count = 0;
        func_list_tail->next = NULL;
	}

    return func_list_tail;
}

struct var_node* getFuncVar(char* str)
{
    struct var_node* list_ptr = func_list_tail->params;

    // Search param list
    while(list_ptr != NULL)
    {
        if(strcmp(list_ptr->symbol, str) == 0)
            return list_ptr;
        else
            list_ptr = list_ptr->next;
	}

    list_ptr = func_list_tail->vars;

    // Search var list
    while(list_ptr != NULL)
    {
        if(strcmp(list_ptr->symbol, str) == 0)
            return list_ptr;
        else
            list_ptr = list_ptr->next;
	}

    // Not found, return NULL.
    return list_ptr;
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
    else if (argc == 1)
    {
            pError(fatal, "No file provided.");
            exit(-1);
	}
    #if YYDEBUG
        yydebug = 1;
    #endif

    global_scope = current_scope = NULL;
    str_list_head = NULL;
    int_list_head = NULL;
    VMQ_list = NULL;
    ast_root = NULL;
    global_scope = pushScope();
    

    // Enter parser
    yyparse();
    
    fclose(yyin);

    STRLIT_LIST list_ptr = str_list_head;
    

    // Assign VMQ memory locations to string literals
    while(list_ptr->next != NULL) 
    { 
            if(strcmp(list_ptr->str, "\\n") == 0)
                list_ptr->next->loc = list_ptr->loc + 2;
            else
                list_ptr->next->loc = list_ptr->loc + strlen(list_ptr->str) - 1;

            list_ptr = list_ptr->next;
	}

    INTLIT_LIST int_list_ptr = int_list_head;

    // Start ints after strings.
    int first_int_loc = list_ptr->loc + strlen(list_ptr->str);

    // Memory bounds must start on even number for integers.
    if (first_int_loc % 2 == 1) first_int_loc++;
    int_list_ptr->loc = first_int_loc;

    // Assign VMQ memory locations to int literals
    while(int_list_ptr->next != NULL)
    {
            int_list_ptr->next->loc = int_list_ptr->loc + 2;

            int_list_ptr = int_list_ptr->next;
	}


    eval(ast_root);

    char* filename = strdup(argv[argc-1]);
    
    char* str_ptr = filename + strlen(filename) - 3;
    *str_ptr = 'q'; str_ptr++;
    *str_ptr = '\0';

    FILE* VMQ_file = fopen(filename, "w");

    list_ptr = str_list_head;

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

            if (list_ptr->next == NULL) break;

        list_ptr = list_ptr->next;
	}

    int_list_ptr = int_list_head;

    while(int_list_ptr != NULL)
    {
            sprintf(str_ptr, "%d %d\n", int_list_ptr->loc, int_list_ptr->val);
            fputs(str_ptr, VMQ_file);

            if(int_list_ptr->next == NULL) break;
            
            int_list_ptr = int_list_ptr->next;
	}

    
           
    // Write runtime initialization op to file.
    if(int_list_head == NULL)
        sprintf(str_ptr, "$ 1 %lu\n", list_ptr->loc + strlen(list_ptr->str) - 2);
    else
        sprintf(str_ptr, "$ 1 %u\n", int_list_ptr->loc + 2);
    fputs(str_ptr, VMQ_file);
    
    // Write create stack frame op to file
    sprintf(str_ptr, "# %d\n", 2*(func_list_head->var_count + func_max_temp_vars));
    fputs(str_ptr, VMQ_file);

    list_ptr = VMQ_list;
    while(list_ptr != NULL)
    {
        fputs(list_ptr->str, VMQ_file);
        fputs("\n", VMQ_file);
        list_ptr = list_ptr->next;
	}

    //fclose(VMQ_file);

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
  // printf("\n\t\tNew AST built with nodetype == %d\n", nodetype);
   a->nodetype = nodetype;
   a->l = l;
   a->r = r;
    
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
    //printf("\n\t\tNew rel built with nodetype == %d\n", reltype);

    a->nodetype = reltype;
    a->l = l;
    a->r = r;

    return (struct ast*)a;
 }

 struct ast *
 newstr(struct strlit_node* strliteral) {
     struct stringval *a = malloc(sizeof(struct stringval));
     if(!a) {
             pError(fatal, "out of space");
             exit(-1);
	 }
    //printf("\n\t\tNew str built with nodetype == %d\n", STR_LITERAL);

     a->nodetype = STR_LITERAL;
     a->str = strliteral;
    return (struct ast*)a;
 }

 struct ast *
 newint(struct intlit_node* intliteral)
 {
     struct intval *a = malloc(sizeof(struct intval));
     if(!a) {
                     pError(fatal, "out of space");
                     exit(-1);
	 }
    //printf("\n\t\tNew int built with nodetype == %d\n", INT_LITERAL);

     a->nodetype = INT_LITERAL; //VMQ defines an int using f
     a->number = intliteral;
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
    //printf("\n\t\tNew float built with nodetype == %d\n", FLT_LITERAL);

     a->nodetype = FLT_LITERAL; //VMQ defines a float using F
     a->number = num;
     return (struct ast *)a;
 }

 struct ast *
 newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *el) {
     struct flow *a = malloc(sizeof(struct flow));

     if(!a) {
             pError(fatal, "out of space");
             exit(-1);
	 }
    //printf("\n\t\tNew flow built with nodetype == %d\n", nodetype);

     a->nodetype = nodetype;
     a->cond = cond;
     a->tl = tl;
     a->el = el;
     return (struct ast *)a;
 }

 struct ast *
 newref(struct var_node *vn) {
    struct symref *a = malloc(sizeof(struct symref));
    
    if(!a) {
            pError(fatal, "out of space");
            exit(-1);
	}
    //printf("\n\t\tNew ref built with nodetype == %d\n", ID);

    a->nodetype = ID;
    a->vn = vn;

    return (struct ast*)a;
 }
// struct ast *newasgn(struct symbol_node *s, struct ast *v);

// Function to delete and free AST
//    Note - This may not even be needed.  Program will exit and OS will take care of deallocation.
  void
    treefree(struct ast *a)
    {
        //switch(a->nodetype) {
            /* cases here will be based on parser */
        //}
	}

// Some flags and counters for eval that could be useful.
unsigned int temp_vars = 0;
unsigned int expr_max_temp_vars = 0;

FUNC_LIST current_func = NULL;

struct ast* eval(struct ast *a)
{
	struct ast* cond = NULL;
    struct ast* lnode = NULL;
    struct ast* rnode = NULL;
    
    if(a == NULL) return a;
    
    char* VMQ_add_stmt = malloc(20);

    switch (a->nodetype)
    {
        case COUT:            eval(a->l); break;
        case CIN:            eval(a->l); break;
        case RETURN:        break;
    
        case STR_LITERAL:   return a; break;

        case INT_LITERAL:    return a; break;
    
        case ID:            return a; break;

        case ASSIGNOP:      lnode = a->l;
                            rnode = a->r;
								
							// Start at the bottom of the ASSIGNOP AST subtree.
                            if(rnode && rnode->nodetype == ID) { eval(a->r); return a; }

                            /* 
                                lnode is always a variable (ID)
                                rnode is either a variable or an expression
                            */
                            if(rnode && rnode->nodetype == ID)
                            {
                                struct var_node* l_val = ((struct symref*)lnode)->vn;
                                struct var_node* r_val = ((struct symref*)rnode)->vn;

                                char* l_val_ref = malloc(3);
                                char* r_val_ref = malloc(3);
                                
                                if(l_val->isParam)    l_val_ref = "@/";
                                else                l_val_ref = "/-";
                                
                                if(r_val->isParam)    r_val_ref = "@/";
                                else                r_val_ref = "/-";

                                sprintf(VMQ_add_stmt, "i %s%d %s%d", r_val_ref, r_val->loc, l_val_ref, l_val->loc);
                                appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
							}
							else if (rnode && rnode->nodetype == INT_LITERAL)
							{
								struct var_node* l_val = ((struct symref*)lnode)->vn;
								struct intlit_node* r_val = ((struct intval*)rnode)->number;

								char* l_val_ref = malloc(3);

							    if(l_val->isParam)	l_val_ref = "@/";
								else				l_val_ref = "/-";

							    sprintf(VMQ_add_stmt, "i %d %s%d", r_val->loc, l_val_ref, l_val->loc);
								appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
							}
                            else if (rnode && 
									 (rnode->nodetype == '+' || rnode->nodetype == '-' || rnode->nodetype == '*' ||
                                      rnode->nodetype == '/' || rnode->nodetype == '%' ))
							{
								struct var_node* l_val = ((struct symref*)lnode)->vn;

								eval(a->r);
								
								char* l_val_ref = malloc(3);

								if(l_val->isParam)	l_val_ref = "@/";
								else				l_val_ref = "/-";

								sprintf(VMQ_add_stmt, "i /-%d %s%d", 2*(current_func->var_count + 1), l_val_ref, l_val->loc);
								appendToStrList(&VMQ_list, VMQ_add_stmt, 1);

								// Reset any temp variables that were used.
                                while(expr_max_temp_vars != 0)
                                {                          /* "s tmp tmp tmp" will set tmp to 0. */
                                    sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", 2*(expr_max_temp_vars + current_func->var_count), 2*(expr_max_temp_vars + current_func->var_count), 2*(expr_max_temp_vars + current_func->var_count));
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                    expr_max_temp_vars--;
								}

								temp_vars = 0;

							}
							else if (rnode && rnode->nodetype == ASSIGNOP)
                            {
                                eval(a->r);
                                struct var_node* l_val = ((struct symref*)lnode)->vn;
								struct var_node* r_val = ((struct symref*)rnode->l)->vn;

                                char* l_val_ref = malloc(3);
                                if (l_val->isParam)    l_val_ref = "@/";
                                else                l_val_ref = "/-";

                                sprintf(VMQ_add_stmt, "i /-%d %s%d", r_val->loc, l_val_ref, l_val->loc);
                                appendToStrList(&VMQ_list, VMQ_add_stmt, 1);

                                // Reset any temp variables that were used.
                                while(expr_max_temp_vars != 0)
                                {                         /* "s tmp tmp tmp" will set tmp to 0 */
                                    sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", 2*(expr_max_temp_vars + current_func->var_count), 2*(expr_max_temp_vars + current_func->var_count), 2*(expr_max_temp_vars + current_func->var_count));
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                    expr_max_temp_vars--;
								}
                                temp_vars = 0;
							}
                            break;

        case '+':            lnode = a->l;
                            rnode = a->r;
                            if (lnode && lnode->nodetype != INT_LITERAL) { eval(a->l); /*temp_vars--;*/ }
                            if (rnode && rnode->nodetype != INT_LITERAL) { /*temp_vars++;*/ eval(a->r); }
                            if(lnode && rnode)
                            {
                                if(lnode->nodetype == INT_LITERAL)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "a %d %d /-%d", ((struct intval*)lnode)->number->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
                                    else if (rnode->nodetype == ID)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "a %d /-%d /-%d", ((struct intval*)lnode)->number->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "a %d /-%d /-%d", ((struct intval*)lnode)->number->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else if (lnode->nodetype == ID)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "a /-%d %d /-%d", ((struct symref*)lnode)->vn->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
									else if(rnode->nodetype == ID)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;
                                        sprintf(VMQ_add_stmt, "a /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "a /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                        sprintf(VMQ_add_stmt, "a /-%d %d /-%d", 2*(temp_vars + current_func->var_count), ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
                                    else if(rnode->nodetype == ID)
                                        sprintf(VMQ_add_stmt, "a /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count), ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
                                    else
                                    {
                                        sprintf(VMQ_add_stmt, "a /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count - 1), 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count - 1));
										temp_vars--;
									}
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
							}
                            break;

        case '-':            lnode = a->l;
                            rnode = a->r;
                            if (lnode && lnode->nodetype != INT_LITERAL) { eval(a->l); /*temp_vars--;*/ }
                            if (rnode && rnode->nodetype != INT_LITERAL) { /*temp_vars++;*/ eval(a->r); }
                            if(lnode && rnode)
                            {
                                if(lnode->nodetype == INT_LITERAL)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "s %d %d /-%d", ((struct intval*)lnode)->number->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
									else if (rnode->nodetype == ID)
                                    {
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "s %d /-%d /-%d", ((struct intval*)lnode)->number->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "s %d /-%d /-%d", ((struct intval*)lnode)->number->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else if (lnode->nodetype == ID)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "s /-%d %d /-%d", ((struct symref*)lnode)->vn->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
									else if(rnode->nodetype == ID)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                        sprintf(VMQ_add_stmt, "s /-%d %d /-%d", 2*(temp_vars + current_func->var_count), ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
                                    else if(rnode->nodetype == ID)
                                        sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count), ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
                                    else
                                    {
                                        sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count - 1), 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count - 1));
																			temp_vars--;
									}
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
							}
								else if (rnode) // UNARY MINUS
								{
																if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;
																
																if(rnode->nodetype == INT_LITERAL)
																    sprintf(VMQ_add_stmt, "n %d /-%d", ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
																else if(rnode->nodetype == ID)
																	sprintf(VMQ_add_stmt, "n /-%d /-%d", ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
																
                                appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                            break;

        case '*':           lnode = a->l;
                            rnode = a->r;
                            if (lnode && lnode->nodetype != INT_LITERAL) { eval(a->l); /*temp_vars--;*/ }
                            if (rnode && rnode->nodetype != INT_LITERAL) { /*temp_vars++;*/ eval(a->r); }
                            if(lnode && rnode)
                            {
                                if(lnode->nodetype == INT_LITERAL)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;
                                        sprintf(VMQ_add_stmt, "m %d %d /-%d", ((struct intval*)lnode)->number->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
                                    else if (rnode->nodetype == ID)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "m %d /-%d /-%d", ((struct intval*)lnode)->number->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "m %d /-%d /-%d", ((struct intval*)lnode)->number->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else if (lnode->nodetype == ID)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "m /-%d %d /-%d", ((struct symref*)lnode)->vn->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
									else if(rnode->nodetype == ID)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;
                                        sprintf(VMQ_add_stmt, "m /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
    									sprintf(VMQ_add_stmt, "m /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                        sprintf(VMQ_add_stmt, "m /-%d %d /-%d", 2*(temp_vars + current_func->var_count), ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
                                    else if(rnode->nodetype == ID)
                                        sprintf(VMQ_add_stmt, "m /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count), ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
                                    else
                                    {
                                        sprintf(VMQ_add_stmt, "m /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count - 1), 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count - 1));
										temp_vars--;
									}
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
							}
                            break;

        case '/':            lnode = a->l;
                            rnode = a->r;
                            if (lnode && lnode->nodetype != INT_LITERAL) { eval(a->l); /*temp_vars--;*/ }
                            if (rnode && rnode->nodetype != INT_LITERAL) { /*temp_vars++;*/ eval(a->r); }
                            if(lnode && rnode)
                            {
                                if(lnode->nodetype == INT_LITERAL)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "d %d %d /-%d", ((struct intval*)lnode)->number->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
                                    else if (rnode->nodetype == ID)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "d %d /-%d /-%d", ((struct intval*)lnode)->number->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "d %d /-%d /-%d", ((struct intval*)lnode)->number->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else if (lnode->nodetype == ID)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "d /-%d %d /-%d", ((struct symref*)lnode)->vn->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
									else if(rnode->nodetype == ID)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "d /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "d /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                        sprintf(VMQ_add_stmt, "d /-%d %d /-%d", 2*(temp_vars + current_func->var_count), ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
                                    else if(rnode->nodetype == ID)
                                        sprintf(VMQ_add_stmt, "d /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count), ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
                                    else
                                    {
                                        sprintf(VMQ_add_stmt, "d /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count - 1), 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count - 1));
																			temp_vars--;
									}
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
							}
                            break;
        
        case '%':           lnode = a->l;
                            rnode = a->r;
                            if (lnode && lnode->nodetype != INT_LITERAL) { eval(a->l); /*temp_vars--;*/ }
                            if (rnode && rnode->nodetype != INT_LITERAL) { /*temp_vars++;*/ eval(a->r); }
                            if(lnode && rnode)
                            {
                                if(lnode->nodetype == INT_LITERAL)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "r %d %d /-%d", ((struct intval*)lnode)->number->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
                                    else if (rnode->nodetype == ID)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "r %d /-%d /-%d", ((struct intval*)lnode)->number->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
									else
										sprintf(VMQ_add_stmt, "r %d /-%d /-%d", ((struct intval*)lnode)->number->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else if (lnode->nodetype == ID)
                                {
                                    if(rnode->nodetype == INT_LITERAL)
									{
										if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;

                                        sprintf(VMQ_add_stmt, "r /-%d %d /-%d", ((struct symref*)lnode)->vn->loc, ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
									}
									else if(rnode->nodetype == ID)
                                    {
                                        if(++temp_vars > func_max_temp_vars) func_max_temp_vars = temp_vars;
                                        if(temp_vars > expr_max_temp_vars) expr_max_temp_vars = temp_vars;
                                        sprintf(VMQ_add_stmt, "r /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
									}
																	else
																		sprintf(VMQ_add_stmt, "r /-%d /-%d /-%d", ((struct symref*)lnode)->vn->loc, 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count));

                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
                                else
                                {
                                    if(rnode->nodetype == INT_LITERAL)
                                        sprintf(VMQ_add_stmt, "r /-%d %d /-%d", 2*(temp_vars + current_func->var_count), ((struct intval*)rnode)->number->loc, 2*(temp_vars + current_func->var_count));
                                    else if(rnode->nodetype == ID)
                                        sprintf(VMQ_add_stmt, "r /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count), ((struct symref*)rnode)->vn->loc, 2*(temp_vars + current_func->var_count));
                                    else
                                    {
                                        sprintf(VMQ_add_stmt, "r /-%d /-%d /-%d", 2*(temp_vars + current_func->var_count - 1), 2*(temp_vars + current_func->var_count), 2*(temp_vars + current_func->var_count - 1));
																			temp_vars--;
									}
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
								}
							}
                            break;

		case ('+' + '='):	lnode = a->l; // lnode is always a variable
							rnode = a->r; // rnode is a variable, int_literal, or expression.

							if(rnode != NULL)
							{
								struct var_node* l_val = ((struct symref*)lnode)->vn;
								
								if(rnode->nodetype == ID)
								{
									struct var_node* r_val = ((struct symref*)rnode)->vn;
									sprintf(VMQ_add_stmt, "a /-%d /-%d /-%d", l_val->loc, r_val->loc, l_val->loc);
								}
								else if(rnode->nodetype == INT_LITERAL)
								{
									struct intlit_node* r_val = ((struct intval*)rnode)->number;
									sprintf(VMQ_add_stmt, "a /-%d %d /-%d", l_val->loc, r_val->loc, l_val->loc);
								}
								else
								{
									eval(a->r);
									sprintf(VMQ_add_stmt, "a /-%d /-%d /-%d", l_val->loc, 2*(temp_vars + current_func->var_count), l_val->loc);
								}
								
								appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
							}
							else
								pError(error, "Missing r_value for (+=) operator");
							
							break;

		case ('-' + '='):	lnode = a->l;
							rnode = a->r;

							if(rnode != NULL)
							{
								struct var_node* l_val = ((struct symref*)lnode)->vn;

								if(rnode->nodetype == ID)
								{
									struct var_node* r_val = ((struct symref*)rnode)->vn;
									sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", l_val->loc, r_val->loc, l_val->loc);
								}
								else if(rnode->nodetype == INT_LITERAL)
								{
									struct intlit_node* r_val = ((struct intval*)rnode)->number;
									sprintf(VMQ_add_stmt, "s /-%d %d /-%d", l_val->loc, r_val->loc, l_val->loc);
								}
								else
								{
									eval(a->r);
									sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", l_val->loc, 2*(temp_vars + current_func->var_count), l_val->loc);
								}

								appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
							}
							else
								pError(error, "Missing r_value for (-=) operator");
							
							break;

        case STREAMOUT:    eval(a->l);        // Takes us to bottom of the output tree (first output statement to execute).
                        rnode = a->r;    // Either a STR_LITERAL or some kind of expression
                        if(rnode != NULL)
                            switch(rnode->nodetype)
                            {
                                case STR_LITERAL:   ;char* VMQ_push_stmt; VMQ_push_stmt = (char*)malloc(20);
                                                    STRLIT_LIST ptr = str_list_head; char* str = strdup(((struct stringval*)rnode)->str->str);
                                                    while(ptr != NULL) { if(strcmp(ptr->str, str)== 0) break; else ptr = ptr->next; }
                                                    sprintf(VMQ_push_stmt, "p #%d", ptr->loc);
                                                    appendToStrList(&VMQ_list, VMQ_push_stmt, 1);
                                                    appendToStrList(&VMQ_list, "c 0 -11", 1);
                                                    appendToStrList(&VMQ_list, "^ 2", 1);
                                                    break;

                                case INT_LITERAL:    ;char* VMQ_intlit_stmt = malloc(20);
                                                    sprintf(VMQ_intlit_stmt, "p #%d", ((struct intval*)rnode)->number->loc);
                                                    appendToStrList(&VMQ_list, VMQ_intlit_stmt, 1);
                                                    appendToStrList(&VMQ_list, "c 0 -9", 1);
                                                    appendToStrList(&VMQ_list, "^ 2", 1);
                                                    break;

                                case ID:            if(((struct symref*)rnode)->vn->isParam)
                                                        sprintf(VMQ_add_stmt, "p /%d", ((struct symref*)rnode)->vn->loc);
                                                    else
                                                        sprintf(VMQ_add_stmt, "p #/-%d", ((struct symref*)rnode)->vn->loc);

                                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                                    appendToStrList(&VMQ_list, "c 0 -9", 1);
                                                    appendToStrList(&VMQ_list, "^ 2", 1);
                                                    break;
                                case '+':
                                case '-':
                                case '*':
                                case '/':            
                                case '%':            eval(a->r);
                                                    sprintf(VMQ_add_stmt, "p #/-%d", 2*(current_func->var_count + 1));
                                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                                    appendToStrList(&VMQ_list, "c 0 -9", 1);
                                                    appendToStrList(&VMQ_list, "^ 2", 1);

                                                    // Reset any temp variables that were used.
                                                    while(expr_max_temp_vars != 0)
                                                    {                          /* "s tmp tmp tmp" will set tmp to 0. */
                                                        sprintf(VMQ_add_stmt, "s /-%d /-%d /-%d", 2*(expr_max_temp_vars + current_func->var_count), 2*(expr_max_temp_vars + current_func->var_count), 2*(expr_max_temp_vars + current_func->var_count));
                                                        appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                                        expr_max_temp_vars--;
													}
                                                    temp_vars = 0;
                                                    break;
							}
                        break;
        
        // statement
        case ('s'+'t'+'m'+'t'):      eval(a->l); break;
        
        // statements
        case ('s'+'t'+'m'+'t'+'s'):  if(a->l != NULL) eval(a->l); 
                                     if(a->r != NULL) eval(a->r); break;

        // block
        case ('b'+'l'+'k'):         eval(a->l); eval(a->r); break;

        case ('f'+'d'+'s'):            eval(a->l); eval(a->r); break;

        case ('f'+'d'):             if(current_func == NULL) current_func = func_list_head;
                                    else                     current_func = current_func->next;
                                    temp_vars = 0; eval(a->l); eval(a->r); break;

        case ('p'+'r'+'o'+'g'):     eval(a->r); appendToStrList(&VMQ_list, "h", 1); break;
			
			case IF:                    cond = ((struct flow*)a)->cond;
                                    lnode = ((struct flow*)a)->tl;    //Where to go if the cond is true
                                    rnode = ((struct flow*)a)->el;    //Where to go if the cond is flase
                                    //printf("\tCHECKING THE NODETYPE OF COND\n");
                                    switch (cond->nodetype)
                                    {
                                        case OR: /*printf("\tCHECKING THE LEFT SIDE\n");*/ eval(cond->l); /*printf("\tCHECKING THE RIGHT SIDE\n");*/ eval(cond->r);break;
                                        case AND: /*printf("\tCHECKING THE LEFT SIDE\n");*/ eval(cond->l); /*printf("\tCHECKING THE RIGHT SIDE\n");*/ eval(cond->r);break;
									}
                                    break;
                                    
        case NOT:                   /*printf("\tNOT THE RESULT\n");*/ eval(a->l); break;
            
        case '<':                   lnode = a->l; rnode = a->r;
                                    if (lnode && rnode)
                                    {
                                        if (lnode->nodetype == INT_LITERAL && rnode->nodetype == INT_LITERAL)
                                        {
                                            sprintf(VMQ_add_stmt, "L %d %d /-%d", ((struct intval*)lnode)->number->loc, ((struct intval*)rnode)->number->loc, 2*temp_vars); // need to place the temp_vars with a label to go the right statement
                                            appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                        }
                                        else if (lnode->nodetype != INT_LITERAL && rnode->nodetype == INT_LITERAL)
                                        {
                                            // left side is likely a variable or a container of some sort
                                            sprintf(VMQ_add_stmt, "L /@%d %d /-%d", lookup(((struct symref*)lnode)->s->symbol), ((struct intval*)rnode)->number->loc, 2*temp_vars); // need to place the temp_vars with a label to go the right statement
                                            appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                        }
                                        else if (lnode->nodetype == INT_LITERAL && rnode->nodetype != INT_LITERAL)
                                        {
                                            // right side is likely a variable or a container of some sort
                                            sprintf(VMQ_add_stmt, "L %d /@%d /-%d", ((struct intval*)lnode)->number->loc, lookup(((struct symref*)rnode)->s->symbol), 2*temp_vars); // need to place the temp_vars with a label to go the right statement
                                            appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                        }
                                        
                                    }
                                    /*if (((struct intval*)(eval(a->l)))->number->val < ((struct intval*)(eval(a->r)))->number->val)
                                    {
                                        printf("\tLEFT (%d) IS LESS THAN RIGHT (%d)\n", ((struct intval*)(eval(a->l)))->number->val, ((struct intval*)(eval(a->r)))->number->val);
									}*/
                                    break;
            
        case '>':                   lnode = a->l; rnode = a->r;
                                    sprintf(VMQ_add_stmt, "G %d %d /-%d", ((struct intval*)lnode)->number->loc, ((struct intval*)rnode)->number->loc, 2*temp_vars);
                                    appendToStrList(&VMQ_list, VMQ_add_stmt, 1);
                                    /*if (((struct intval*)(eval(a->l)))->number->val > ((struct intval*)(eval(a->r)))->number->val)
                                    {
                                        printf("\tLEFT (%d) IS GREATER THAN RIGHT (%d)\n", ((struct intval*)(eval(a->l)))->number->val, ((struct intval*)(eval(a->r)))->number->val);
									}*/
                                    break;
            
        case ('<'+'='):             /*if (((struct intval*)(eval(a->l)))->number->val <= ((struct intval*)(eval(a->r)))->number->val)
                                    {
                                        printf("\tLEFT (%d) IS LESS THAN OR EQUAL TO RIGHT (%d)\n", ((struct intval*)(eval(a->l)))->number->val, ((struct intval*)(eval(a->r)))->number->val);
									}*/
                                    break;
            
        case ('>'+'='):             /*if (((struct intval*)(eval(a->l)))->number->val >= ((struct intval*)(eval(a->r)))->number->val)
                                    {
                                        printf("\tLEFT (%d) IS GREATER THAN OR EQUAL TO RIGHT (%d)\n", ((struct intval*)(eval(a->l)))->number->val, ((struct intval*)(eval(a->r)))->number->val);
									}*/
                                    break;
            
        case ('='+'='):             /*if (((struct intval*)(eval(a->l)))->number->val == ((struct intval*)(eval(a->r)))->number->val)
                                    {
                                        printf("\tLEFT (%d) IS EQUAL TO RIGHT (%d)\n", ((struct intval*)(eval(a->l)))->number->val, ((struct intval*)(eval(a->r)))->number->val);
									}*/
                                    break;
            
        case ('!'+'='):             /*if (((struct intval*)(eval(a->l)))->number->val != ((struct intval*)(eval(a->r)))->number->val)
                                    {
                                        printf("\tLEFT (%d) IS NOT EQUAL TO RIGHT (%d)\n", ((struct intval*)(eval(a->l)))->number->val, ((struct intval*)(eval(a->r)))->number->val);
									}*/
                                    break;

        default:                    free(VMQ_add_stmt); if(a->l != NULL) eval(a->l); if(a->r != NULL) eval(a->r);
	}

    
        
    return a;

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
    fflush(stdout);

    if (a == NULL) return;
    struct ast *ptr = a;

    if(ptr->nodetype == 0) { printf("\t!! NULLTYPE NODE !!\n"); return; }

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
        else if (ptr->nodetype == '+')            printf("\tADDOP(+) NODE\n");
        else if (ptr->nodetype == '-')            printf("\tADDOP(-) NODE\n");
        else if (ptr->nodetype == '*')            printf("\tMULOP(*) NODE\n");
        else if (ptr->nodetype == '/')            printf("\tMULOP(/) NODE\n");
        else if (ptr->nodetype == '%')            printf("\tMULOP(%%) NODE\n");
        else if (ptr->nodetype == INT_LITERAL)    printf("\tINT_LIT NODE (val == %d)\n", ((struct intval*)a)->number->val);
        else if (ptr->nodetype == STREAMOUT)    printf("\tSTREAMOUT NODE\n");
        else                                    printf("\tOTHER NODE (%d)\n", ptr->nodetype);
        printAST(ptr->r);

        fflush(stdout);
	}
}


void printFuncLists()
{
    FUNC_LIST func_list = func_list_head;
    
    while(func_list != NULL)
    {
        printf("Printing params and vars for function %s...\n", func_list->func->symbol);

        struct var_node* param_list = func_list->params;
        
        if(param_list)    printf("\tParams:\n");
        else            printf("\tNO PARAMS\n");
        
        while(param_list != NULL)
        {
            printf("\t\tParam Name == %s | Loc == @/%d\n", param_list->symbol, param_list->loc);
            param_list = param_list->next;
		}

        struct var_node* var_list = func_list->vars;
        
        if(var_list)    printf("\tVars:\n");
        else            printf("\tNO VARS\n");

        while(var_list != NULL)
        {
            printf("\t\tVar Name == %s | Loc == /-%d\n", var_list->symbol, var_list->loc);
            var_list = var_list->next;
		}

        func_list = func_list->next;
	}

    printf("\nFunc List Param/Var printing complete!\n");

}


