/*
 *	Our parser.
*/

%{
/*
 *	Including some standard libaries that we're likely to use.
*/
	#include "helperFunctions.h"
	#include "Parser.tab.h"
}%

/*	Code that we need to make use of the custom yyerror function	
 *	%code requires makes sure that our code is placed before the default versions
 *	otherwise bison will place the following code below the default generated code
*/
%code requires
{	
	/*	redefining the YYLTYPE in order to include the filename and highestErrorLevel	*/
	typedef struct YYLTYPE
	{
  		int first_line;
  		int first_column;
  		int last_line;
  		int last_column;
  		int hel;	/* no errors = 0, warning = 1, error = 2, fatal = 3*/
	} YYLTYPE;

	# define YYLTYPE_IS_DECLARED 1	/*	Lets the parser know that it should use our YYLTYPE instead	*/
	# define YYLLOC_DEFAULT(Current, Rhs, N)	\
    	do	\
      		if (N) \
			{\
	  			(Current).first_line = YYRHSLOC (Rhs, 1).first_line;	\
	  			(Current).first_column = YYRHSLOC (Rhs, 1).first_column;\
	  			(Current).last_line = YYRHSLOC (Rhs, N).last_line;		\
	  			(Current).last_column = YYRHSLOC (Rhs, N).last_column;	\
	  			(Current).hel	= YYRHSLOC (Rhs, 1).hel;	\
			}\
      		else	\
			{ /* empty RHS */	\
	  			(Current).first_line = (Current).last_line = YYRHSLOC (Rhs, 0).last_line;		\
	  			(Current).first_column = (Current).last_column = YYRHSLOC (Rhs, 0).last_column;	\
	  			(Current).hel = 0;	\
			}\
    	while (0)
}

/*	union variables go here	*/
%union{
	struct ast *a;
 	float <f>;
 	int <d>;
 	//Figure out how to include strings later
 	struct symbol *s;
 	int fn;
}

/*	Tokens go here; Names and Literal values	*/
	%token <d> INT_LITERAL
 	%token <f> FLT_LITERAL
 	//Include when strings are in %token <str> STR_LITERAL
 	%token <s> ID
 	%token EOL
 
 	%token IF CIN COUT ELSE ENDL WHILE FLOAT INT RETURN
 
 	/* Precedence rules here. Could use some help. */
 	%nonassoc <fn> RELOP
 	%right ASSIGNOP
 	%left <fn> ADDOP
 	%left <fn> MULOP
 	%nonassoc NOT UNARY
	/* UNARY is just a token that we'll use to give things the highest precedence, 
	 * in order to use it we put %prec UNARY after the grammar but before the c code for it.
	 * In the book, they used it for unary minus to recongize negative numbers. They didn't
	 * put the negative sign in the regular expression for INT or FLOAT, they let the grammar
	 * handle the negative numbers. For more details check out the fb3-2.l and fb3-2.y book examples.
	 */
 
 	%start program

/*	Grammer rules go here?	*/
%%

	program:	/* Nothing */
		| variable_definitions function_definitions
		;
		
	function_definitions:	function_head block
		| function_definitions function_head block
		;
		
	identifier_list:	ID
		| ID '[' INT_LITERAL ']'
		| identifier_list ',' ID
		| identifier_list ',' ID '[' INT_LITERAL ']'
		;
		
	variable_definitions:	/* I think that's the set symbol */
		| variable_definitions type identifier_list
		;
		
	type:	INT
		| FLOAT
		;
		
	function_head:	type ID arguments
		;
		
	arguments:	'(' parameter_list ')'
		;
		
	parameter_list:	/* I think that's the set symbol */
		| parameters
		;
		
	parameters:	type ID
		| type ID '[' ']'
		| parameters ',' type ID
		| parameters ',' type ID '[' ']'
		;
		
	block:	'{' variable_definitions statements '}'
		;
		
	statements:	/* I think that's the set symbol */
		| statements statement
		;
		
	statement:	expression ';'
		| compound_statement
		| RETURN expression ';'
		| IF '(' bool_expression ')' statement ELSE statement	{ $$= newflow('I', $3, $5, $7);	}
		| WHILE '(' bool_expression ')' statement				{ $$ = newflow('W', $3, $5, NULL); }
		| input_statement ';'
		| output_statement ';'
		;
		
	input_statement:	CIN
		| input_statement STREAMIN variable
		;
		
	out_statement:	COUT
		| out_statement STREAMOUT expression
		| out_statement STREAMOUT STR_LITERAL
		| out_statement STREAMOUT ENDL
		;
		
	compound_statement:	'{' statements '}'
		;
		
	variable:	ID
		| ID '[' expression ']'
		;
		
	expression_list:	/* I think that's the set symbol */
		| expressions
		;
		
	expressions:	expression
		| expressions ',' expression
		;
		
	expression:	variable ASSIGNOP expression	{ $$ = newasgn($1, $3); }
		| variable INCOP expression				{ $$ = newast($2, $1, $3); }
		| simple_expression
		;
		
	simple_expression:	term
		| ADDOP term							{ $$ = newast($1, $2); } //possible error
		| simple_expression ADDOP term			{ $$ = newast($2, $1, $3); }
		;
		
	term:	factor
		| term MULOP factor						{ $$ = newast($2, $1, $3); }
		;
		
	factor:	ID
		| ID '(' expression_list ')'
		| literal
		| '(' expression ')'
		| ID '[' expression ']'
		;
		
	literal:	INT_LITERAL
		| FLOAT_LITERAL
		;
		
	bool_expression:	bool_term
		| bool_expression OR bool_term
		;
		
	bool_term:	bool_factor
		| bool_term AND bool_factor
		;
		
	bool_factor:	NOT bool_factor
		| '(' bool_expression ')'
		| simple_expression RELOP simple_expression	{ $$ = newrel($2, $1, $3); }
		;
	
%%