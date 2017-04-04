#include "symbol_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

unsigned int hash(const char* symbol)
{
	unsigned int hashKey = 0;
	int i;
	for (i = 0; symbol[i] != 0; ++i)
		hashKey += symbol[i];

	// THIS VERSION IS INHERENTLY DEPENDENT ON TABLE SIZE BEING SET TO 9997.
	// This will be adjusted in later version.
	hashKey = hashKey % 9997;

	return hashKey;
}

char* lookup(struct symbol_node* symbol_table, const char* symbol)
{
	// Hash the key.
	unsigned int hashKey = hash(symbol);

	// Pointer to entry-point in symbol table.
	symbol_node* listPtr = &symbol_table[hashKey];

	if (listPtr->symbol == NULL)
	{
		// New symbol AND current list is empty.
		listPtr->symbol = (char*) malloc(strlen(symbol) + 1);
		strcpy_s(listPtr->symbol, strlen(symbol) + 1, symbol);
	}
	else
	{
		// Current list is not empty; new symbol status unknown.
		while (listPtr->next != NULL)
		{
			if (strcmp(listPtr->symbol, symbol) == 0)
				return listPtr->symbol;	// Symbol found, return address.

			listPtr = listPtr->next;
		}

		// Check final node
		if (strcmp(listPtr->symbol, symbol) == 0)
			return listPtr->symbol;		// Symbol found, return address.
		else
		{
			// New symbol, append to end of current list
			listPtr->next = (struct symbol_node*) malloc(sizeof(symbol_node));
			listPtr = listPtr->next;

			listPtr->next = NULL;
			listPtr->symbol = (char*) malloc(strlen(symbol) + 1);
			strcpy_s(listPtr->symbol, strlen(symbol) + 1, symbol);
		}
	}

	return listPtr->symbol;
}

struct symbol_node* generateSymbolTable(unsigned int tableSize)
{
	// Allocate memory for symbol table
	struct symbol_node* symbol_table =
		(struct symbol_node*) malloc(tableSize * sizeof(struct symbol_node));

	if (symbol_table == NULL)
		return NULL;	// Couldn't allocate memory

	// Initialize all elements of new table to NULL
	for (unsigned int i = 0; i < tableSize; ++i)
	{
		symbol_table[i].symbol = NULL;
		symbol_table[i].next = NULL;
	}

	return symbol_table;
}
