#include <stdio.h>
#include <stdlib.h>
#include "symbol_table.h"


int main(int argc, char** argv)
{
	struct symbol_node* symbol_table = generateSymbolTable();
	char* str = (char*) malloc(256);

	while (true)
	{
		scanf("%s", str);
		int key = hash(str);
		char* mem_adr = lookup(symbol_table, str);
		
		printf("Addresss of %s: %X\n", str, (long long)mem_adr);
	}

	return 0;
}