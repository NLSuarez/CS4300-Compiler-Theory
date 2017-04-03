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
		if (strcmp(symbol, listPtr->symbol) == 0)
			return listPtr->symbol;
		else
			listPtr = listPtr->next;
	}

	/* Symbol not found in table - add new symbol */
	if (listPtr->symbol != NULL)
	{
		/* Node is occupied - create next one in list */
		listPtr->next = (struct symbol_node*) malloc(sizeof(symbol_node));
		listPtr = listPtr->next;
	}

	/* Copy string and set pointer to NULL */
	listPtr->symbol = (char*) malloc(strlen(symbol) + 1);

	if (listPtr->symbol == NULL) /* Couldn't allocate memory */
		return NULL;

	strcpy_s(listPtr->symbol, strlen(symbol) + 1, symbol);
	listPtr->next = NULL;

	return listPtr->symbol;
}

static struct symbol_node* generateSymbolTable(unsigned int tableSize)
{
	/* Dynamically allocate symbol table */
	struct symbol_node* symbol_table = 
		(struct symbol_node*) malloc(tableSize * sizeof(struct symbol_node));

	/* Initialize all elements of fresh table to NULL */
	for (unsigned int i = 0; i < tableSize; ++i)
	{
		symbol_table[i].symbol = NULL;
		symbol_table[i].next = NULL;
	}

	return symbol_table;
}
