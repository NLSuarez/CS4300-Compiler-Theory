#include <stdio.h>
#include <string.h>
 
int main(int argc, char **argv)
{
	if(argc > 1)    //ensures function was passed source file name
	{
		char* filename = argv[1];
		FILE *f = fopen(filename, "r");    //"r" to read
		char *dot;
		int  dotIndex;
		dot = strrchr(filename, '.');    //sets pointer to last dot in source name
		char *slash;
		int slashIndex;
		slash = strrchr(filename, '/');    //sets pointer to last slash in source name
		
		int hasDot = dot != NULL;
		if (hasDot) dotIndex = (int) (dot - filename);
		else dotIndex = -1;
//	printf("dot found at %d\n", dotIndex);
		
		int hasSlash = slash != NULL;
		if (hasSlash) slashIndex = (int) (slash - filename);
		else slashIndex = -1;
//	printf("slash found at %d\n", slashIndex);
		
		char outputFilename[256];
		memset(outputFilename, '\0', sizeof(outputFilename));	
		/* if ((hasDot) && (dotIndex > slashIndex))
			I think this functions the same because 
			slashIndex is -1 when !hasSlash */
		if(((hasDot && hasSlash) && (dotIndex > slashIndex)) || (hasDot && !hasSlash)) 
		{
	//	puts("remove extension");
			strncpy(outputFilename, filename, dotIndex);    //copy filename up to dot into outputFilename
	//	puts(outoutFilename);
		}
		else
		{
			strcpy(outputFilename, filename);    //copy filename into outputFilename
		}
//	puts(outputFilename);
		strcat(outputFilename, ".q");    //appends .q to outputFilename
		puts(outputFilename);
		FILE *fp = fopen(outputFilename, "w+");    //opens new writeable file with outputFilename
	}	
	else puts("Error: no source file given");    //print error if argc <= 1
	return 0;
}
