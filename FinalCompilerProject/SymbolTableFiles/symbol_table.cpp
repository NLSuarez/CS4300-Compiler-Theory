#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>

static unsigned int hash(char* symbol)
{
	/* Credit goes to Flex & Bison author John Levine for this function code */
	unsigned int hashKey = 0;
	unsigned c;
	while (c = *symbol++) hashKey = hashKey * 9 ^ c;

	return hashKey;
}

static char* lookup(struct symbol_node* symbol_table, char* symbol)
{
	unsigned int hashKey = hash(symbol);
	symbol_node* listPtr = &symbol_table[hashKey];

/* Look for symbol in table */
	
	/* Check initial node */
	if (strcmp(symbol, listPtr->symbol) == 0)
		return listPtr->symbol;
	
	/* Check additional nodes in list, if any */
	while (listPtr->next != NULL)
	{
		listPtr = listPtr->next;
		if (strcmp(symbol, listPtr->symbol) == 0)
			return listPtr->symbol;
	}

/* Symbol not found in table - add new symbol */
	if (listPtr->symbol != NULL)
	{
		/* Node is occupied - create next one in list */
		listPtr->next = (struct symbol_node*) malloc(sizeof(symbol_node));
		
		if (listPtr->next == NULL)
			return NULL;	/* Couldn't allocate memory */
			
		listPtr = listPtr->next;
	}

	/* Allocate space for new string */
	listPtr->symbol = (char*) malloc(strlen(symbol) + 1);
	if (listPtr->symbol == NULL) 
		return NULL;	/* Couldn't allocate memory */
	
	/* Copy string and set pointer to NULL */
	strcpy_s(listPtr->symbol, strlen(symbol) + 1, symbol);
	listPtr->next = NULL;

	return listPtr->symbol;
}

static struct symbol_node* generateSymbolTable(unsigned int tableSize)
{
	/* Allocate memory for symbol table */
	struct symbol_node* symbol_table = 
		(struct symbol_node*) malloc(tableSize * sizeof(struct symbol_node));
	
	if (symbol_table == NULL)
		return NULL;	/* Couldn't allocate memory */

	/* Initialize all elements of new table to NULL */
	for (unsigned int i = 0; i < tableSize; ++i)
	{
		symbol_table[i].symbol = NULL;
		symbol_table[i].next = NULL;
	}

	return symbol_table;
}
