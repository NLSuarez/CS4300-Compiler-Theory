#pragma once

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
};

/* Generates hash key for given symbol */
unsigned int hash(const char* symbol);

/* Looks for symbol in table, adds if not there, returns address of record that contains symbol */
struct symbol_record* lookup(SYMBOL_TABLE symTab, const char* symbol);

/* Generates a new, empty symbol table */
SYMBOL_TABLE generateSymbolTable(unsigned int table_size);

/* Populates passed symbol table with C keywords */
void populateSymbolTable(SYMBOL_TABLE symTab);

