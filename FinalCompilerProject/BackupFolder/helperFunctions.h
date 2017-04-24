/*
 * Place the prototypes of any helpers functions, structs, enum that the parser
 * will need here.
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

/*
 * Enum to pass to the yyerror function, making it easier to see
 * the level of severity of the error.
 */
typedef enum errorSeverity { warning = 1, error, fatal } errorLevel;

/*
 * Error function that can takes in a string description of the error that will be outputted to User.
 */
void yyerror(char *s, ...);

/*
 * Error function that takes in the enum indicating severity of the error and also
 * a string description of the error.
 */
void myyyerror(errorLevel el, char *s, ...);

/*
 * AST node types.
 * There will be multiple types of ast nodes according to the specifications
 * in the book. These are rough drafts.
 */

 //Normal tree
 struct ast {
   int nodetype;
   struct ast *l;
   struct ast *r;
 };

//int literal
 struct intval {
   int nodetype;
   int number;
 };

//float literal
 struct floatval {
   int nodetype;
   float number;
 };

 /*
  * functions to build the AST
  */
  struct ast *newast(int nodetype, struct ast *l, struct ast *r);
  struct ast *newint(int num);
  struct ast *newfloat(float num);

  /*
   * FUnction to delete and free an AST
   */
   void treefree(struct ast *);
