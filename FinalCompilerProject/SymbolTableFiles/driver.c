#include <stdio.h>
#include <stdlib.h>
#include "symbol_table.h"

const int TABLE_SIZE = 9997;

int main(int argc, char** argv)
{
        struct symbol_node* symbol_table = generateSymbolTable(TABLE_SIZE);
        char* str = (char*) malloc(256);

        while (1)
        {
                scanf("%s", str);
                int key = hash(str);
                long long mem_adr = (long long) lookup(symbol_table, str);

                printf("Addresss of %s: %llX\n", str, (long long)mem_adr);
        }

        return 0;
}
