#include "helperFunctions.h"
#include "Parser.tab.h"

/*
 * The error function takes in the error string that will be ouputted for the programmer to debug.
 * The output will consist of the filename, linenumber, column, and followed by the error's description.
*/
void yyerror(char *s, ...)
{
  va_list ap;
  va_start(ap, s);

  if(yylloc.first_line)
  {
    fprintf(stderr, "%s:%d.%d-%d.%d: ", yylloc.filename, yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column);
  }
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");
}

/*
 * The error function takes in an enum errorLevel as the severity level of the error
 * and the error string that will be ouputted for the programmer to debug.
 * The function will update the highestErrorLevel if the error is of a higher severity
 * than previously recorded. The output will consist of the linenumber, column,
 * the severity level of the error, and followed by the error's description.
*/
void myyyerror(errorLevel el, char *s, ...)
{
	va_list ap;
	va_start(ap, s);
	char *els[3] = {"Warning", "Error", "Fatal"};

	fprintf(stderr, "%s: %d.%d-%d.%d: ", els[el - 1, yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column];
	vfprintf(stderr, s, ap);
	fprintf(stderr, "\n");
}

/*
 * functions to build the AST
 */
 struct ast *
 newast(int nodetype, struct ast *l, struct ast *r)
 {
   struct ast *a = malloc(sizeof(struct ast));

   if(!a) {
     yyerror("out of space");
     exit(0);
   }
   a->nodetype = nodetype;
   a->l = l;
   a->r = r;
   return a;
 }
 struct ast *newint(int num);
 struct ast *newfloat(float num);

 /*
  * FUnction to delete and free an AST
  */
  void treefree(struct ast *);
