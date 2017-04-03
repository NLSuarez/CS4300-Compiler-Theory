#pragma once

/* Building block of the linked-list-based symbol table */
/* The symbol table will be an array of struct symbol_node pointers */
struct symbol_node
{
	char* symbol;
	struct symbol_node* next;
};

/* Generates hash key for given symbol */
static unsigned int hash(char* symbol);

/* Looks for symbol in table, adds if not there, returns address of symbol in table */
static char* lookup(struct symbol_node* symbol_table, char* symbol);

/* Generates a new, empty symbol table */
static struct symbol_node* generateSymbolTable(unsigned int table_size = 9997);