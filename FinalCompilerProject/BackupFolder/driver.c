#include <stdio.h>
#include <string.h>
 
int main(int argc, char **argv)
{
	if(argc > 1)
	{
		char* filename = argv[1];
		FILE *f = fopen(filename, "r"); //"r" to read
		char *dot;
		int  dotIndex;
		dot = strrchr(filename, '.');
		char *slash;
		int slashIndex;
		slash = strrchr(filename, '/');
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
			//puts("remove extension");
			strncpy(outputFilename, filename, dotIndex);
			//puts(outoutFilename);
		}
		else
		{
			strcpy(outputFilename, filename);
		}
//	puts(outputFilename);
		strcat(outputFilename, ".q");
		puts(outputFilename);
		FILE *fp = fopen(outputFilename, "w+");
	}	
	else puts("Error: no source file given");
	return 0;
}
