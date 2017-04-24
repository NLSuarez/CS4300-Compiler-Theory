

#include "helper_functions.h"
#include "parser.tab.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>	// need it for va_list, va_start in the error functions

// Useful for debugging and populating the symbol table - need a better, non-hack solution for this in the future.
#define INT_LITERAL 258
#define FLT_LITERAL 259
#define ID 260
#define STR_LITERAL 261
#define CIN 262
#define COUT 263
#define ELSE 264
#define ENDL 265
#define FLOAT 266
#define IF 267
#define INT 268
#define RETURN 269
#define WHILE 270
#define FUNC 271
#define EOL 272
#define ASSIGNOP 273
#define MULOP 274
#define ADDOP 275
#define INCOP 276
#define RELOP 277
#define NOT 278
#define OR 279
#define AND 280
#define STREAMIN 281
#define STREAMOUT 282
#define UNARY 283

// To keep track of the highest severity of error, 0 = no errors, 1 = warning, 2 = error, 3 = fatal
int hel = 0;

// This should be kept up-to-date if more keywords are added to the set.
enum { KEYWORD_COUNT = 9997 };

char* C_KEYWORD_ARRAY[KEYWORD_COUNT] = { "cin", "cout", "else", "endl", "float",
						"if", "int", "return", "while" };

unsigned int hash(const char* symbol)
{
	unsigned int hashKey = 0;
	int i;
	for (i = 0; symbol[i] != 0; ++i)
		hashKey += symbol[i];

	hashKey = hashKey % 9997;

	return hashKey;
}

struct symbol_record* lookup(SYMBOL_TABLE symTab, const char* symbol, int kind)
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
		listPtr->kind = kind;
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
			listPtr->kind = kind;
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
		symTab[i].kind = 0;
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
		recordPtr = lookup(symTab, C_KEYWORD_ARRAY[i], CIN + i);

	return;
}


void printRecordData(struct symbol_record* record)
{
	printf("Address of record:\t0x%llX\n", (unsigned long long)record);
	printf("Symbol of record:\t%s\n", record->symbol);
	printf("Symbol kind:\t\t%s\n\n", kindToString(record->kind));
}



int getKind(char *str)
{
	for(int i = 0; i < KEYWORD_COUNT; ++i)
	{
		if (strcmp(str, C_KEYWORD_ARRAY[i]) == 0)
			return (CIN + i);
	}

	printf("!!getKind: -1 return on str = %s\n\n", str);
	return -1;
}



char* kindToString(const int kind)
{
	// Stupid solution, but it works.
	if (kind == CIN)			return "CIN";
	if (kind == COUT)			return "COUT";
	if (kind == ELSE)			return "ELSE";
	if (kind == ENDL)			return "ENDL";
	if (kind == FLOAT)			return "FLOAT";
	if (kind == IF)				return "IF";
	if (kind == INT)			return "INT";
	if (kind == RETURN)			return "RETURN";
	if (kind == WHILE)			return "WHILE";
	if (kind == STR_LITERAL)	return "STR_LITERAL";
	if (kind == ID)				return "ID";

	return "UNKNOWN";
}


int main(int argc, char **argv)
{
    extern unsigned int DEBUG;

    if (argc > 1)
    {
        for (int i = 1; i < argc; ++i)
            if (strcmp(argv[i], "-d") == 0) DEBUG = 1;
    }


	if(DEBUG) printf("!!!DEBUG ON!!!\n\n");

    symTab = generateSymbolTable(TABLE_SIZE);
    populateSymbolTable(symTab);

   return yyparse();

}


void yyerror(char* s, ...)
{
	va_list ap;
	va_start(ap, s);

	// print out where the error occurs
	if(yylloc.first_line)
	{
		fprintf(stderr, "%d.%d-%d.%d: ", yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column);
	}
	vfprintf(stderr, s, ap);	// print out the error decription
	fprintf(stderr, "\n");
	//printf("%s\n", s);
}

void pError(errorLevel el, char* s, ...)
{
	va_list ap;
	va_start(ap, s);
	char *els[3] = {"Warning", "Error", "Fatal"};

	// update the severity level countered
	if (el > hel)
	{
		hel = el;
	}

	fprintf(stderr, "%s: %d.%d-%d.%d: ", els[el - 1], yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column);
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
	//printf("%s\n", s);
}

/*
 * functions to build the AST
 */
 struct ast *
 newast(int nodetype, struct ast *l, struct ast *r)
 {
   struct ast *a = malloc(sizeof(struct ast));

   if(!a) {
     pError(fatal, "out of space");
     exit(0);
   }
   a->nodetype = nodetype;
   a->l = l;
   a->r = r;
   return a;
 }

 struct ast *
 newint(int num)
 {
	 struct intval *a = malloc(sizeof(struct intval));
	 if(!a) {
		 pError(fatal, "out of space");
		 exit(0);
	 }
	 a->nodetype = 'f'; //VMQ defines an int using f
	 a->number = num;
	 return (struct ast *)a;
 }

 struct ast *
 newfloat(float num)
 {
	 struct floatval *a = malloc(sizeof(struct floatval));
	 if(!a) {
		 pError(fatal, "out of space");
		 exit(0);
	 }
	 a->nodetype = 'F'; //VMQ defines a float using F
	 a->number = num;
	 return (struct ast *)a;
 }

 /*
  * FUnction to delete and free an AST
  */
  void
	treefree(struct ast *a)
	{
		//switch(a->nodetype) {
			/* cases here will be based on parser */
		//}
	}
