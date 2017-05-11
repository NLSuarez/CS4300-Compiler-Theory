#pragma once

extern char* C_KEYWORD_ARRAY[];    //    size of 9 at the moment

unsigned int DEBUG;
unsigned int PAR_DEBUG;
unsigned int LEX_DEBUG;
int hel;    // To keep track of the highest severity of error, 0 = no errors, 1 = warning, 2 = error, 3 = fatal


/* Used in symbol_node struct, will hold the value of the given symbol if the symbol's kind is ID (a variable).*/
union data
{
    int d;
    double f;
};

/* Basic element of the symbol table, which is a hash table with linked-list collision resolution */
struct symbol_node
{
    char* symbol;
    union data val;
    int kind;	// INT, FLOAT, or FUNC
    struct symbol_node* next;
};

/* Element of funcvar_list, used to track function's non-argument local variables */
struct funcvar_node
{
	struct symbol_node* var;
	struct symbol_node* func;
};

/* List that is used to track a function's non-arg local variables */
struct funcvar_list
{
	struct funcvar_node* head;
	struct funcvar_list* next;
};

/* symbol tables are a dynamically allocated array of symbol_node structs */
typedef struct symbol_node* SYMBOL_TABLE;

/* Used to keep track of the local variables within each function */
typedef struct funcvar_list** CURRENT_FUNCVAR_LIST;
typedef struct funcvar_list** HEAD_FUNCVAR_LIST;

/* Used to track existing scopes - effectively implemented as a stack of scope_node structs */
struct scope_node
{
    SYMBOL_TABLE symTab;
    struct scope_node* next;
    int is_new_scope;
};

/* 
 * Used to track string literals found by the lexer, length will effectively be the starting memory address of
 * the string in VMQ's global memory space.  NOTE: THIS ASSUMES NO GLOBAL VARIABLES IN THE C++ FILE BEING COMPILED.
*/
struct strlit_node
{
    char* str;
    unsigned int loc;
    struct strlit_node* next;
};

struct intlit_node
{
	int val;
	unsigned int loc;
	struct intlit_node* next;
};

typedef struct strlit_node* STRLIT_LIST;
typedef struct intlit_node* INTLIT_LIST;

struct strlit_node* appendToStrList(STRLIT_LIST* head, char* str, int eval_state);
struct intlit_node* appendToIntList(INTLIT_LIST* head, int val);


/* Deletes current_scope struct, sets current_scope struct to correct scope post-pop */
void popScope();

/* Generates and initializes new scope struct, returns scope struct pointer of new scope */
struct scope_node* pushScope();

/* Generates hash key for given symbol */
unsigned int hash(const char* symbol);

/* Looks for symbol in tables, returns address of record that contains symbol (NULL, if not found) */
struct symbol_node* lookup(const char* symbol);

/* Adds symbol to current table, in the current scope.  Returns the address of the symbol_node. */
struct symbol_node* addSymbol(SYMBOL_TABLE symTab, char* symbol, int kind);

/* Generates a new scope_node struct and initilizes its symbol table. */
struct scope_node* initializeScope();

/* Generates a new, empty symbol table */
SYMBOL_TABLE generateSymbolTable(unsigned int table_size);

/* Populates passed symbol table with C keywords */
void populateSymbolTable(SYMBOL_TABLE symTab);

/* Prints value of passed symbol_record ptr, as well as info about the record pointed to. */
void printSymbolData(struct symbol_node* sym);

/* Returns the correct symbol_type of a string that is a C keyword for the project */
int getKind(char *str);

/* Prints the string equivalent of any valid symbol_type value passed to the function */
char* kindToString(int kind);

/*
 * Enum to pass to the yyerror function, making it easier to see 
 * the level of severity of the error.
 */
typedef enum errorSeverity { warning = 1, error, fatal } errorLevel;

/*    
 * Error function that can takes in a string description of the error that will be outputted to User.
 */
void yyerror(char *s, ...);

/*
 * Error function that takes in the enum indicating severity of the error and also
 * a string description of the error.
 */
void pError(errorLevel el, char *s, ...);

/*
 * AST node types.
 * There will be multiple types of ast nodes according to the specifications
 * in the book. These are rough drafts.
 */

//Base tree node
struct ast {
 int nodetype;
 struct ast *l;
 struct ast *r;
};

//string literal
struct stringval {
    int nodetype;    // = STR_LITERAL
    struct strlit_node* str; // points to struct that contains string and VMQ global memory location of string.
};

//int literal
struct intval {
 int nodetype;		// = INT_LITERAL
 struct intlit_node* number;
};

//float literal
struct floatval {
 int nodetype;
 float number;
};

//flow node
struct flow {
 int nodetype; /* Should accept type I(f), W(while) and F(or) */
 struct ast *cond; /* pointer to bool condition */
 struct ast *tl; /* if true, parse this tree */
 struct ast *el; /* else parse this tree */
};

//reference node(i.e. if you need to call a variable)
struct symref {
 int nodetype;    /* Potentially type N like the calculator */
 struct symbol_node *s;
};

//assignment node
struct symasgn {
 int nodetype;    /* type = */
 struct symbol_node *s;
 struct ast *v; /* value */
};

 /*
  * functions to build the AST
  */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newstr(struct strlit_node* strliteral);
struct ast *newint(struct intlit_node* intliteral);
struct ast *newfloat(float num);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newref(struct symbol_node *s);
struct ast *newasgn(struct symbol_node *s, struct ast *v);
struct ast *newrel(int reltype, struct ast *l, struct ast *r);
//Add later.
  /*
   * Function to delete and free an AST
   */
void treefree(struct ast *);

// Pointer to head of stack containing VMQ statements (strings).
typedef struct strlit_node* VMQ_STACK;

// Function that will evaluate the AST, recursively.
struct ast* eval(struct ast *a);

void transferStack(VMQ_STACK dest, VMQ_STACK src);

void pushToStrStack(VMQ_STACK stk, char* str);

void popStrStack(VMQ_STACK stk);

// DEBUG print statement for AST's
void printAST(struct ast *a);



