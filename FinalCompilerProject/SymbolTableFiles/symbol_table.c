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

        hashKey = hashKey % 9997;

        return hashKey;
}

struct symbol_record* lookup(struct symbol_record* symbol_table, const char* symbol)
{
        // Hash the key.
        unsigned int hashKey = hash(symbol);

        // Pointer to entry-point in symbol table.
        struct symbol_record* listPtr = &symbol_table[hashKey];

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
                                return listPtr;         // Symbol found, return pointer to record.

                        listPtr = listPtr->next;
                }

                // Check final node
                if (strcmp(listPtr->symbol, symbol) == 0)
                        return listPtr;                 // Symbol found, return pointer to record.
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

struct symbol_record* generateSymbolTable(unsigned int tableSize)
{
        // Allocate memory for symbol table
        struct symbol_record* symbol_table =
                (struct symbol_record*) malloc(tableSize * sizeof(struct symbol_record));

        if (symbol_table == NULL)
                return NULL;    // Couldn't allocate memory

        // Initialize all elements of new table to NULL
        for (unsigned int i = 0; i < tableSize; ++i)
        {
                symbol_table[i].symbol = NULL;
                symbol_table[i].next = NULL;
        }

        return symbol_table;
}
