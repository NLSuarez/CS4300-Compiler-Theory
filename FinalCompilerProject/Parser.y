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
}

/*	Tokens go here; Names and Literal values	*/

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
		| IF '(' bool_expression ')' statement ELSE statement
		| WHILE '(' bool_expression ') statement
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
		
	expression:	variable ASSIGNOP expression
		| variable INCOP expression
		| simple_expression
		;
		
	simple_expression:	term
		| ADDOP term
		| simple_expression ADDOP term
		;
		
	term:	factor
		| term MULOP factor
		;
		
	factor:	ID
		| ID '(' expression_list ')
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
		| simple_expression RELOP simple_expression
		;
	
%%