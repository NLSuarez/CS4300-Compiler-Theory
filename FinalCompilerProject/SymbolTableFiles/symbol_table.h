#pragma once

<<<<<<< HEAD
enum symbol_type { EMPTY = 0, CIN, COUT, ELSE, ENDL, FLOAT, IF, INT, RETURN, WHILE, VAR, CONSTANT };
extern const unsigned int TABLE_SIZE;

/* Building block of the linked-list-based symbol table */
/* The symbol table will be an array of pointers to struct symbol_record  */

typedef struct symbol_record* SYMBOL_TABLE;

extern const char* C_KEYWORD_ARRAY[];

struct symbol_record
{
        char* symbol;
        enum symbol_type kind;
        struct symbol_record* next;
=======
/* Building block of the linked-list-based symbol table */
/* The symbol table will be an array of struct symbol_node pointers */
struct symbol_node
{
	char* symbol;
	struct symbol_node* next;
>>>>>>> b5030e61e0b658a5df382ab1b1d5bdf1a6a8cc85
};

/* Generates hash key for given symbol */
unsigned int hash(const char* symbol);

<<<<<<< HEAD
/* Looks for symbol in table, adds if not there, returns address of record that contains symbol */
struct symbol_record* lookup(SYMBOL_TABLE symTab, const char* symbol);

/* Generates a new, empty symbol table */
SYMBOL_TABLE generateSymbolTable(unsigned int table_size);

/* Populates passed symbol table with C keywords */
void populateSymbolTable(SYMBOL_TABLE symTab);

=======
/* Looks for symbol in table, adds if not there, returns address of symbol in table */
char* lookup(struct symbol_node* symbol_table, const char* symbol);

/* Generates a new, empty symbol table */
struct symbol_node* generateSymbolTable(unsigned int table_size = 9997);
>>>>>>> b5030e61e0b658a5df382ab1b1d5bdf1a6a8cc85
