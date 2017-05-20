

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
    char* symbol;						// Name of the symbol (ID)
//	union data val;						// Not really needed, commented out for the moment.
    int kind;	// INT, FLOAT, or FUNC	// What kind of symbol it is.
    struct symbol_node* next;			// Pointer to next symbol.
};

struct var_node
{
	unsigned int isParam;	// Flag that signifies if the variable in question is a parameter.
	unsigned int isGlobal;	// Flag that signifies if the variable in question is a global var.
	char* symbol;			// Name of the variable (probably not needed.)
	int kind;				// Kind of variable (INT or FLOAT).
	int loc;				// VMQ location of variable.
	struct var_node* next;	// Pointer to next variable.
};

/* symbol tables are a dynamically allocated array of symbol_node structs */
typedef struct symbol_node* SYMBOL_TABLE;

/* Used to track existing scopes - effectively implemented as a stack of scope_node structs */
struct scope_node
{
    SYMBOL_TABLE symTab;		// Contains the function scope's symbol table.
    struct scope_node* next;	// Points to next (deeper) scope.
    int is_new_scope;			// Flag that is useful to ensure an extra symbol table isn't generated
								// when getting into a new function.
};

/* 
 * Used to track string literals found by the lexer, length will effectively be the starting memory address of
 * the string in VMQ's global memory space.  NOTE: THIS ASSUMES NO GLOBAL VARIABLES IN THE C++ FILE BEING COMPILED.
*/
struct strlit_node
{
    char* str;					// Value of string literal the node represents.
    unsigned int loc;			// VMQ location of string literal.
    struct strlit_node* next;	// Pointer to next string literal.
};

struct intlit_node
{
	int val;					// Value of integer literal the node represents.
	unsigned int loc;			// VMQ location of integer literal
	struct intlit_node* next;	// Pointer to next integer literal
};

/* Useful typedef's to simplify coding */
typedef struct strlit_node* STRLIT_LIST;
typedef struct intlit_node* INTLIT_LIST;
typedef struct func_node* FUNC_LIST;

/*
	Used to keep track of each function's parameters and variables.
	Also stores the list of VMQ commands that are generated for each
	respective function.
*/
struct func_node
{
    struct symbol_node* func;		// Function name

    struct var_node* params;		// List of function's parameters		(Not sure if needed.)
    struct var_node* vars;			// List of function's local variables.	(Not sure if needed.)

    unsigned int param_count;		// Number of parameters
    unsigned int var_count;			// Number of local variables

	unsigned int end_var_addr;		// VMQ address of last local variable	(Not sure if needed.)
	unsigned int end_param_addr;	// VMQ address of last parameter		(Not sure if needed.)

	STRLIT_LIST VMQ_stack_frame;	// Contains the command to create a stack frame, if the function needs it.
	STRLIT_LIST VMQ_list;			// Head of linked list containing function's equivalent VMQ statements.
	unsigned int VMQ_line_start;	// Stores the line number of the first VMQ statement for this function.
	unsigned int VMQ_line_count;	// Stores the number of VMQ statements generated for this function.
	struct func_node* next;			// Pointer to next func_node (next function in the program).
};

/* Function prototypes useful for keeping track of program state, generating VMQ code */
struct strlit_node* appendToStrList(STRLIT_LIST* head, char* str, int eval_state);
struct intlit_node* appendToIntList(INTLIT_LIST* head, int val);
struct var_node* appendToGlobalVarList(char* var_id, int kind);
struct var_node* appendToFuncVars(struct symbol_node* var);
struct var_node* appendToFuncParams(struct symbol_node* param);
struct func_node* appendNewFunc(struct symbol_node* func);
struct var_node* getGlobalVar(char* str);
struct var_node* getFuncVar(char* str);

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
 int nodetype;
 struct var_node *vn;
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
struct ast *newref(struct var_node* vn);
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

// DEBUG print statement for FUNCVARS_LIST
void printFuncLists();



