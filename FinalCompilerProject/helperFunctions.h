/*
 * Place the prototypes of any helpers functions, structs, enum that the parser
 * will need here.
*/

/*
 * Enum to pass to the yyerror function, making it easier to see 
 * the level of severity of the error.
 */
typedef enum { warning = 1, error, fatal } errorLevel;

/*	
 * Error function that can takes in a enum indictating the severity of the error
 * and also a string description of the error that will be outputted to User.
 */
void yyerror(errorLevel errlvl, char *s, ...);