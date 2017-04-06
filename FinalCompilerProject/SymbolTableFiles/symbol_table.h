#pragma once

enum symbol_type { EMPTY = 0, CIN, COUT, ELSE, ENDL, FLOAT, IF, INT, RETURN, WHILE, VAR, CONSTANT };
extern const unsigned int TABLE_SIZE;

/* Building block of the linked-list-based symbol table */
/* The symbol table will be an array of pointers to struct symbol_record  */
struct symbol_record
{
        char* symbol;
        enum symbol_type kind;
        struct symbol_record* next;
};

/* Generates hash key for given symbol */
unsigned int hash(const char* symbol);

/* Looks for symbol in table, adds if not there, returns address of record that contains symbol */
struct symbol_record* lookup(struct symbol_record* symbol_table, const char* symbol);

/* Generates a new, empty symbol table */
struct symbol_record* generateSymbolTable(unsigned int table_size);
