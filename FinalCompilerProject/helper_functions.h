#pragma once

extern const unsigned int TABLE_SIZE;

/* Building block of the linked-list-based symbol table */
/* The symbol table will be an array of pointers to struct symbol_record  */

typedef struct symbol_record* SYMBOL_TABLE;

extern char* C_KEYWORD_ARRAY[];

unsigned int DEBUG;
int hel;	// To keep track of the highest severity of error, 0 = no errors, 1 = warning, 2 = error, 3 = fatal

/* Used in symbol_record, will hold the value of the given symbol if the symbol's kind is ID (a variable).*/
union data
{
	int d;
	double f;
};	

struct symbol_record
{
    char* symbol;
	union data val;
    int kind;
    struct symbol_record* next;
};

SYMBOL_TABLE symTab;

/* Generates hash key for given symbol */
unsigned int hash(const char* symbol);

/* Looks for symbol in table, adds if not there, returns address of record that contains symbol */
struct symbol_record* lookup(SYMBOL_TABLE symTab, const char* symbol, int kind);

/* Generates a new, empty symbol table */
SYMBOL_TABLE generateSymbolTable(unsigned int table_size);

/* Populates passed symbol table with C keywords */
void populateSymbolTable(SYMBOL_TABLE symTab);

/* Prints value of passed symbol_record ptr, as well as info about the record pointed to. */
void printRecordData(struct symbol_record* record);

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
 struct symbol_record *s;
}
//assignment node
struct symasgn {
 int nodetype;	/* type = */
 struct symbol_record *s;
 struct ast *v; /* value */
};
 /*
  * functions to build the AST
  */
struct ast *newast(int nodetype, struct ast *l, struct ast *r);
struct ast *newint(int num);
struct ast *newfloat(float num);
struct ast *newflow(int nodetype, struct ast *cond, struct ast *tl, struct ast *tr);
struct ast *newref(struct symbol_record *s);
struct ast *newasgn(struct symbol_record *s, struct ast *v);

  /*
   * FUnction to delete and free an AST
   */
 void treefree(struct ast *);

