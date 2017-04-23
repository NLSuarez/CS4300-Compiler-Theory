

/*
 *	Our parser.
*/

%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include "helper_functions.h"
	#include "parser.tab.h"

	int yylex();
/*	Code that we need to make use of the custom yyerror function	
 *	%code requires makes sure that our code is placed before the default versions
 *	otherwise bison will place the following code below the default generated code
*/
%}

/*	union variables go here	*/
%union {
//	struct ast *a;
	int d;
	double f;
	char* str;
	struct symbol_record *s;
	int fn;
}

/*	Tokens go here; Names and Literal values	*/
	%token <d> INT_LITERAL
	%token <f> FLT_LITERAL
	%token <s> ID STR_LITERAL CIN COUT ELSE ENDL FLOAT IF INT RETURN WHILE 
	%token <fn> FUNC
	%token EOL
 
	%token ASSIGNOP MULOP ADDOP INCOP RELOP NOT OR AND STREAMIN STREAMOUT
 
	/* Precedence rules here. Could use some help. */
	%nonassoc RELOP
	%right ASSIGNOP
	%left ADDOP
	%left MULOP
	%nonassoc NOT UNARY
	/* UNARY is just a token that we'll use to give things the highest precedence, 
	 * in order to use it we put %prec UNARY after the grammar but before the c code for it.
	 * In the book, they used it for unary minus to recongize negative numbers. They didn't
	 * put the negative sign in the regular expression for INT or FLOAT, they let the grammar
	 * handle the negative numbers. For more details check out the fb3-2.l and fb3-2.y book examples.
	 */

	%type <str> out_statement expression term factor simple_expression variable
	%type <f> literal

	%start out_statement
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
		| WHILE '(' bool_expression ')' statement
		| input_statement ';'
		| out_statement ';'
		;
		
	input_statement:	CIN
		| input_statement STREAMIN variable
		;
		
	out_statement:	COUT						{ printf("%s ", $1->symbol); }
		| out_statement STREAMOUT expression	{ printf("<< %s ", $3); }
		| out_statement STREAMOUT STR_LITERAL	{ printf("<< %s ", $3->symbol); }
		| out_statement STREAMOUT ENDL			{ printf("<< %s;\n", $3->symbol); }
		;
		
	compound_statement:	'{' statements '}'
		;
		
	variable:	ID								{ $$ = $1->symbol; }
		| ID '[' expression ']'					{ $$ = $1->symbol; }
		;
		
	expression_list:	/* I think that's the set symbol */
		| expressions
		;
		
	expressions:	expression
		| expressions ',' expression
		;
		
	expression:	variable ASSIGNOP expression	{ strcpy($$, $1); strcat($$, $3);  }
		| variable INCOP expression
		| simple_expression				{ $$ = $1; }
		;
		
	simple_expression:	term			{ $$ = $1; }
		| ADDOP term		
		| simple_expression ADDOP term
		;
		
	term:	factor						{ $$ = $1; }
		| term MULOP factor
		;
		
	factor:	ID							{ $$ = $1->symbol; }
		| ID '(' expression_list ')'
		| literal						
		| '(' expression ')'
		| ID '[' expression ']'
		;
		
	literal:	INT_LITERAL				{ $$ = $1; }
		| FLT_LITERAL					{ $$ = $1; }
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

