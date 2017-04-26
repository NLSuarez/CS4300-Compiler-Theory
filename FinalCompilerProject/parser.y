/*
 *	Our parser.
*/

/*	Directive to allow us to use the built in location tracking */
%locations

%{
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
	#include "helper_functions.h"
	#include "parser.tab.h"

	extern unsigned int DEBUG;
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
	%right ASSIGNOP VAR_DEF
	%left ADDOP
	%left MULOP
	%nonassoc NOT UNARY
	/* UNARY is just a token that we'll use to give things the highest precedence, 
	 * in order to use it we put %prec UNARY after the grammar but before the c code for it.
	 * In the book, they used it for unary minus to recongize negative numbers. They didn't
	 * put the negative sign in the regular expression for INT or FLOAT, they let the grammar
	 * handle the negative numbers. For more details check out the fb3-2.l and fb3-2.y book examples.
	 */

	%type <str> output_statement expression term factor simple_expression variable program function_definitions	function_head block identifier_list variable_definitions type arguments parameter_list parameters statements statement compound_statement bool_expression expressions input_statement expression_list literal bool_term bool_factor
	//%type <f> literal

	%start program
%%

	program: variable_definitions function_definitions { if(DEBUG) printf("program PARSED\n"); }
		;
		
	function_definitions:	function_head block     { if(DEBUG) printf("function_definitions PARSED\n"); }
		| function_definitions function_head block  { if(DEBUG) printf("function_definitions PARSED\n"); }
		;
		
	identifier_list:	ID			{ if(DEBUG) printf("identifier_list PARSED\n"); }
		| ID '[' INT_LITERAL ']'	{ if(DEBUG) printf("identifier_list PARSED\n"); }
		| identifier_list ',' ID	{ if(DEBUG) printf("identifier_list PARSED\n"); }
		| identifier_list ',' ID '[' INT_LITERAL ']' { if(DEBUG) printf("identifier_list PARSED\n"); } 
		;
		
	variable_definitions:											  { /* nothing */ }
		| variable_definitions type identifier_list ';' %prec VAR_DEF { if(DEBUG) printf("variable_definitions PARSED\n"); }
		;
		
	type:	INT { if(DEBUG) printf("type(INT) PARSED\n"); }
		| FLOAT { if(DEBUG) printf("type(FLOAT) PARSED\n"); }
		;
		
	function_head:	type ID arguments { if(DEBUG) printf("function_head PARSED\n"); }
		;
		
	arguments:	'(' parameter_list ')' { if(DEBUG) printf("arguments PARSED\n"); }
		;
		
	parameter_list:										{ /* nothing */ }
		| parameters									{ if(DEBUG) printf("parameter_list PARSED\n"); }
		;
		
	parameters:	type ID										{ if(DEBUG) printf("parameters PARSED\n"); }
		| type ID '[' ']'									{ if(DEBUG) printf("parameters PARSED\n"); }
		| parameters ',' type ID		 %prec ASSIGNOP		{ if(DEBUG) printf("parameters PARSED\n"); }
		| parameters ',' type ID '[' ']' %prec ASSIGNOP		{ if(DEBUG) printf("parameters PARSED\n"); }
		;
		
	block:	'{' variable_definitions statements '}' { if(DEBUG) printf("block PARSED\n"); }
		;
		
	statements:										{ /* nothing */ }
		| statements statement						{ if(DEBUG) printf("statements PARSED!\n"); }
		;
		
	statement:	expression ';'									{ if(DEBUG) printf("statement PARSED!\n"); }
		| compound_statement									{ if(DEBUG) printf("statement PARSED!\n"); }
		| RETURN expression ';'									{ if(DEBUG) printf("statement PARSED!\n"); }
		| IF '(' bool_expression ')' statement ELSE statement	{ if(DEBUG) printf("statement PARSED!\n"); }
		| WHILE '(' bool_expression ')' statement				{ if(DEBUG) printf("statement PARSED!\n"); }
		| input_statement ';'									{ if(DEBUG) printf("statement PARSED!\n"); }
		| output_statement ';'									{ if(DEBUG) printf("statement PARSED!\n"); }
		;
		
	input_statement:	CIN										{ if(DEBUG) printf("input_statement PARSED!\n"); }
		| input_statement STREAMIN variable		 %prec ASSIGNOP	{ if(DEBUG) printf("input_statement PARSED!\n"); }
		;
		
	output_statement:	COUT										{ if(DEBUG) printf("output_statement PARSED!\n"); }
		| output_statement STREAMOUT expression	 %prec ASSIGNOP		{ if(DEBUG) printf("output_statement PARSED!\n"); }
		| output_statement STREAMOUT STR_LITERAL %prec ASSIGNOP		{ if(DEBUG) printf("output_statement PARSED!\n"); }
		| output_statement STREAMOUT ENDL		 %prec ASSIGNOP		{ if(DEBUG) printf("output_statement PARSED!\n"); }
		;
		
	compound_statement:	'{' statements '}'		{ if(DEBUG) printf("compound_statement PARSED!\n"); }
		;
		
	variable:	ID								{ if(DEBUG) printf("variable PARSED!\n"); }
		| ID '[' expression ']'					{ if(DEBUG) printf("variable PARSED!\n"); }
		;
		
	expression_list:							{ /* nothing */ }
		| expressions							{ if(DEBUG) printf("expression_list PARSED!\n"); }
		;
		
	expressions:	expression								{ if(DEBUG) printf("expressions PARSED!\n"); }
		| expressions ',' expression	%prec ASSIGNOP		{ if(DEBUG) printf("expressions PARSED!\n"); }
		;
		
	expression:	variable ASSIGNOP expression	{ if(DEBUG) printf("expression PARSED!\n"); }
		| variable INCOP expression				{ if(DEBUG) printf("expression PARSED!\n"); }
		| simple_expression						{ if(DEBUG) printf("expression PARSED!\n"); }
		;
		
	simple_expression:	term			{ if(DEBUG) printf("simple_expression PARSED!\n"); }
		| ADDOP term					{ if(DEBUG) printf("simple_expression PARSED!\n"); }
		| simple_expression ADDOP term	{ if(DEBUG) printf("simple_expression PARSED!\n"); } 
		;
		
	term:	factor						{ if(DEBUG) printf("term PARSED!\n"); }
		| term MULOP factor				{ if(DEBUG) printf("term PARSED!\n"); }
		;
		
	factor:	ID							{ if(DEBUG) printf("factor PARSED!\n"); }
		| ID '(' expression_list ')'	{ if(DEBUG) printf("factor PARSED!\n"); }
		| literal						{ if(DEBUG) printf("factor PARSED!\n"); }
		| '(' expression ')'			{ if(DEBUG) printf("factor PARSED!\n"); }
		| ID '[' expression ']'			{ if(DEBUG) printf("factor PARSED!\n"); }
		;
		
	literal:	INT_LITERAL				{ $$ = newint($1); if(DEBUG) printf("literal PARSED!\n"); }
		| FLT_LITERAL					{ $$ = newfloat($1); if(DEBUG) printf("literal PARSED!\n"); }
		;
		
	bool_expression:	bool_term			{ if(DEBUG) printf("bool_expression PARSED!\n"); }
		| bool_expression OR bool_term		{ if(DEBUG) printf("bool_expression PARSED!\n"); }
		;
		
	bool_term:	bool_factor					{ if(DEBUG) printf("bool_term PARSED!\n"); }
		| bool_term AND bool_factor			{ if(DEBUG) printf("bool_term PARSED!\n"); }
		;
		
	bool_factor:	NOT bool_factor						{ if(DEBUG) printf("bool_factor PARSED!\n"); }
		| '(' bool_expression ')'						{ if(DEBUG) printf("bool_factor PARSED!\n"); }
		| simple_expression RELOP simple_expression		{ if(DEBUG) printf("bool_factor PARSED!\n"); }
		;
	
%%



