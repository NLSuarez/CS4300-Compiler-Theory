#include <stdio.h>
#include <stdlib.h>
#include "symbol_table.h"

const unsigned int TABLE_SIZE = 9997;

int main(int argc, char** argv)
{
        struct symbol_record* symbol_table = generateSymbolTable(TABLE_SIZE);
        struct symbol_record* recordPtr = NULL;
        char* str = (char*) malloc(256);

        while (1)
        {
                scanf("%s", str);
                int key = hash(str);
                recordPtr = lookup(symbol_table, str);

                printf("Addresss of %s:\t\t0x%llX\n", str, (long long) recordPtr);

                if (getc(stdin) == '\n') printf("\n");
        }

        return 0;
}

