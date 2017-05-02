#pragma once

extern char* C_KEYWORD_ARRAY[];	//	size of 9 at the moment

unsigned int DEBUG;
int hel;	// To keep track of the highest severity of error, 0 = no errors, 1 = warning, 2 = error, 3 = fatal

/* Used in symbol_record, will hold the value of the given symbol if the symbol's kind is ID (a variable).*/
union data
{
	int d;
	double f;
};

struct symbol_node
{
	char* symbol;
	union data val;
	int kind;
	struct symbol_node* next;
};

/* symbol tables are a dynamically allocated array of symbol_node structs */
typedef struct symbol_node* SYMBOL_TABLE;

struct scope_node
{
	SYMBOL_TABLE symTab;
	struct scope_node* next;
	int is_new_scope;
};

/*
	Scope node will be used to keep track of the existing variables within the "live" scopes.
	Global scope will always exist, and as the parser enters new functions and blocks "deeper"
	scopes will come into existance.  A pointer will be kept that points to the global scope_node.
	Variables will be checked to see if they exist in global scope first and if not found then subsequent
	scopes will be checked.  If the variable is not found, then we have two basic options:

	1)	Declare it an error, in violation of C++ rules.
	
							OR

	2)	Add it to the symbol table and work under the assumption that it is an integer intialized to 0.
*/

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
	int nodetype;
	char* strval;
}

//int literal
struct intval {
 int nodetype;
 int number;
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
 int nodetype;	/* Potentially type N like the calculator */
 struct symbol_node *s;
}

//assignment node
struct symasgn {
 int nodetype;	/* type = */
 struct symbol_node *s;
 struct ast *v; /* value */
};

 /*
  * functions to build the AST
  */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newstr(char* strliteral);
struct ast *newint(int num);
struct ast *newfloat(float num);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newref(struct symbol_node *s);
struct ast *newasgn(struct symbol_node *s, struct ast *v);
//struct ast *newrel(int reltype, struct ast *l, struct ast *r);
//Add later.
  /*
   * Function to delete and free an AST
   */
void treefree(struct ast *);

