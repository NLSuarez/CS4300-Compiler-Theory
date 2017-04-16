#include "symbol_table.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

extern unsigned int DEBUG;

// This should be kept up-to-date if more keywords are added to the set.
const unsigned int KEYWORD_COUNT = 11;

const char* C_KEYWORD_ARRAY[KEYWORD_COUNT] = { "cin", "cout", "else", "endl", "float", "if",
                                               "int", "return", "while", "var", "const" };


unsigned int hash(const char* symbol)
{
	unsigned int hashKey = 0;
	int i;
	for (i = 0; symbol[i] != 0; ++i)
		hashKey += symbol[i];

	hashKey = hashKey % 9997;

	return hashKey;
}

struct symbol_record* lookup(SYMBOL_TABLE symTab, const char* symbol)
{
	// Hash the key.
	unsigned int hashKey = hash(symbol);

	// Pointer to entry-point in symbol table.
	struct symbol_record* listPtr = &symTab[hashKey];

	if (listPtr->symbol == NULL)
	{
		// New symbol AND current list is empty.
		listPtr->symbol = (char*) malloc(strlen(symbol) + 1);
		strcpy(listPtr->symbol, symbol);
	}
	else
	{
		// Current list is not empty; new symbol status unknown.
		while (listPtr->next != NULL)
		{
			if (strcmp(listPtr->symbol, symbol) == 0)
				return listPtr;		// Symbol found, return pointer to record.

			listPtr = listPtr->next;
		}

		// Check final node
		if (strcmp(listPtr->symbol, symbol) == 0)
			return listPtr;			// Symbol found, return pointer to record.
		else
		{
			// New symbol, append to end of current list
			listPtr->next = (struct symbol_record*) malloc(sizeof(struct symbol_record));
			listPtr = listPtr->next;

			listPtr->next = NULL;
			listPtr->symbol = (char*) malloc(strlen(symbol) + 1);
			strcpy(listPtr->symbol, symbol);
		}
	}

	return listPtr;
}

SYMBOL_TABLE generateSymbolTable(unsigned int tableSize)
{
	// Allocate memory for symbol table
	SYMBOL_TABLE symTab = (SYMBOL_TABLE) malloc(tableSize * sizeof(struct symbol_record));

	if (symTab == NULL)
		return NULL;	// Couldn't allocate memory

	// Initialize all elements of new table to NULL
	for (unsigned int i = 0; i < tableSize; ++i)
	{
		symTab[i].symbol = NULL;
		symTab[i].next = NULL;
		symTab[i].kind = EMPTY;
	}

	populateSymbolTable(symTab);

	return symTab;
}

void populateSymbolTable(SYMBOL_TABLE symTab)
{
	if (symTab == NULL)
		return;

	struct symbol_record* recordPtr = NULL;
	for (int i = 0; i < KEYWORD_COUNT; ++i)
	{
		recordPtr = lookup(symTab, C_KEYWORD_ARRAY[i]);
		recordPtr->kind = (enum symbol_type)(i + 1);
		if (DEBUG)
		printf("\nKEYWORD %s added:\t\tkind = (%d)\t\taddress = (0x%llX)\n",
		       recordPtr->symbol, recordPtr->kind, (long long) recordPtr);;
	}

	if (DEBUG) printf("\n");

	return;
}
