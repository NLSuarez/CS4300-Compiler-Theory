

/*
 *    Our parser.
*/

/*    Directive to allow us to use the built in location tracking */
%locations

%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "helper_functions.h"
    #include "parser.tab.h"

    extern unsigned int DEBUG;
    extern unsigned int PAR_DEBUG;
    int yylex();
/*    Code that we need to make use of the custom yyerror function    
 *    %code requires makes sure that our code is placed before the default versions
 *    otherwise bison will place the following code below the default generated code
*/

    extern struct scope_node* global_scope;
    extern struct scope_node* current_scope;
    extern struct ast* ast_root;
%}

/*    union variables go here    */
%union {
    struct ast *a;
    int d;
    double f;
    char* str;
    struct symbol_node *s;
    struct strlit_node *sln;
	struct intlit_node *iln;
	struct var_node *vn;
    int fn;
}

/*    Tokens go here; Names and Literal values    */
    %token <iln> INT_LITERAL
    %token <f> FLT_LITERAL
    %token <sln> STR_LITERAL
    %token <s> CIN COUT ELSE
    %token <sln> ENDL
    %token <s> FLOAT IF INT RETURN WHILE
	%token <vn> ID
    %token <fn> FUNC
    %token EOL
 
    %token ASSIGNOP 
    %token <fn> MULOP  
	%token <fn> INCOP RELOP 
	%token NOT OR AND STREAMIN STREAMOUT
 
    /* Precedence rules here. Could use some help. */
    %nonassoc RELOP
    %right ASSIGNOP
    %left <fn> ADDOP
    %left MULOP
    %nonassoc NOT UNARY
    /* UNARY is just a token that we'll use to give things the highest precedence, 
     * in order to use it we put %prec UNARY after the grammar but before the c code for it.
     * In the book, they used it for unary minus to recongize negative numbers. They didn't
     * put the negative sign in the regular expression for INT or FLOAT, they let the grammar
     * handle the negative numbers. For more details check out the fb3-2.l and fb3-2.y book examples.
     */

    %type <a> variable program function_definitions function_head block identifier_list variable_definitions arguments parameter_list parameters statements statement compound_statement bool_expression expressions input_statement expression_list bool_term bool_factor output_statement expression simple_expression factor literal term
    %type <d> type
    %start program
%%

    program: variable_definitions function_definitions { $$ = ast_root = newast(('p'+'r'+'o'+'g'), $1, $2); }
        ;
        
    function_definitions:    function_head block     { $$ = newast('f'+'d', $1, $2); }
        | function_definitions function_head block   { $$ = newast('f'+'d'+'s', $1, newast('f'+'d', $2, $3)); }
        ;
        
    identifier_list:    ID                              { $$ = newref($1); }
        | ID '[' INT_LITERAL ']'                        { $$ = newref($1); }
        | identifier_list ',' ID                        { $$ = newast('I'+'D'+'L', $1, newref($3)); }
        | identifier_list ',' ID '[' INT_LITERAL ']'    { $$ = newast('I'+'D'+'L', $1, newref($3)); } 
        ;
        
    variable_definitions:                                   { $$ = NULL;}
        | variable_definitions type identifier_list ';'     { $$ = newast('v'+'a'+'r', $1, $3); }
        ;
        
    type:    INT { $$ = INT; }
        | FLOAT { $$ = FLOAT; }
        ;
        
    function_head:    type ID arguments                 { $$ = newast('f'+'h', newref($2), $3); }
        ;
        
    arguments:    '(' parameter_list ')'                { $$ = $2; }
        ;
        
    parameter_list:                                     { $$ = NULL; }
        | parameters                                    { $$ = newast('p'+'l', $1, NULL); }
        ;
        
    parameters:    type ID                                     { $$ = newref($2); }
        | type ID '[' ']'                                      { $$ = newref($2); }
        | parameters ',' type ID         %prec ASSIGNOP        { $$ = newast('p'+'a'+'r', $1, newref($4)); }
        | parameters ',' type ID '[' ']' %prec ASSIGNOP        { $$ = newast('p'+'a'+'r', $1, newref($4)); }
        ;
        
    block:    '{' variable_definitions statements '}'          { $$ = newast(('b'+'l'+'k'), $2, $3); }
        ;
        
    statements:                             { $$ = NULL; }
        | statements statement				{ $$ = newast('s'+'t'+'m'+'t'+'s', $1, newast('s'+'t'+'m'+'t', $2, NULL)); }
        ;
        
    statement:    expression ';'                                  { $$ = $1; }
        | compound_statement                                      { $$ = $1; }
        | RETURN expression ';'                                   { $$ = newast(RETURN, $2, NULL); }
        | IF '(' bool_expression ')' statement ELSE statement     { $$ = newflow(IF, $3, $5, $7); }
        | WHILE '(' bool_expression ')' statement                 { $$ = newflow(WHILE, $3, $5, NULL); }
        | input_statement ';'                                     { $$ = newast(CIN, $1, NULL); }
        | output_statement ';'                                    { $$ = newast(COUT, $1, NULL); }
        ;
        
    input_statement:    CIN                                        { $$ = newast(0, NULL, NULL); }
        | input_statement STREAMIN variable                        { $$ = newast(STREAMIN, $1, $3); }
        ;
        
    output_statement:    COUT                                       { $$ = newast(0, NULL, NULL); }
        | output_statement STREAMOUT expression                     { $$ = newast(STREAMOUT, $1, $3); }
        | output_statement STREAMOUT STR_LITERAL                    { $$ = newast(STREAMOUT, $1, newstr($3)); }
        | output_statement STREAMOUT ENDL                           { $$ = newast(STREAMOUT, $1, newstr($3)); }
        ;
        
    compound_statement:    '{' statements '}'        { $$ = $2; }
        ;
        
    variable:    ID                                  { $$ = newref($1); }
        | ID '[' expression ']'                      { $$ = newref($1); }
        ;
        
    expression_list:                             { $$ = NULL; }
        | expressions                            { $$ = $1; }
        ;
        
    expressions:    expression                     { $$ = $1; }
        | expressions ',' expression               { $$ = newast('e'+'x'+'p'+'s', $1, $3); }
        ;
        
    expression:    variable ASSIGNOP expression    { $$ = newast(ASSIGNOP, $1, $3); }
        | variable INCOP expression                { if($2 == 1) $$ = newast('+'+'=', $1, $3); else $$ = newast('-'+'=', $1, $3); }
        | simple_expression                        { $$ = $1; }
        ;
        
    simple_expression:    term				            { $$ = $1; }
        | ADDOP term				   %prec UNARY		{ if($1 == 1) $$ = newast('+', NULL, $2); else $$ = newast('-', NULL, $2); }
        | simple_expression ADDOP term		            { if($2 == 1) $$ = newast('+', $1, $3); else $$ = newast('-', $1, $3); } 
        ;
        
    term:    factor                        { $$ = $1; }
        | term MULOP factor                { if($2 == 1) $$ = newast('*', $1, $3); else if($2 == 2) $$ = newast('/', $1, $3); else $$ = newast('%', $1, $3); }
        ;
        
    factor:    ID                                   { $$ = newref($1); }
        | ID '(' expression_list ')'		        { /* This is a function call */ $$ = newast(FUNC, newref($1), $3); }
        | literal							        { /* This is an INT or FLT literal */ $$ = $1; }
        | '(' expression ')'		%prec UNARY		{ /* This is any expr */ $$ = $2; }
        | ID '[' expression ']'                     { /* This is an array access */ $$ = newast('a'+'r'+'r', newref($1), $3); }
        ;
        
    literal:    INT_LITERAL					{ $$ = newint($1); }
        | FLT_LITERAL						{ $$ = newfloat($1); }
        ;
        
    bool_expression:    bool_term            { $$ = $1; if(DEBUG || PAR_DEBUG) printf("bool_expression PARSED!\n"); }
        | bool_expression OR bool_term        { $$ = newrel(OR, $1, $3); if(DEBUG || PAR_DEBUG) printf("bool_expression PARSED!\n"); }
        ;
        
    bool_term:    bool_factor                    { $$ = $1; if(DEBUG || PAR_DEBUG) printf("bool_term PARSED!\n"); }
        | bool_term AND bool_factor            { $$ = newrel(AND, $1, $3); if(DEBUG || PAR_DEBUG) printf("bool_term PARSED!\n"); }
        ;
        
    bool_factor:    NOT bool_factor                        { $$ = newrel(NOT, $2, NULL); if(DEBUG || PAR_DEBUG) printf("bool_factor PARSED!\n"); }
        | '(' bool_expression ')'                        { $$ = $2; if(DEBUG || PAR_DEBUG) printf("bool_factor PARSED!\n"); }
        | simple_expression RELOP simple_expression        { if ($2 == 1) $$ = newrel('>', $1, $3);
                                                             else if ($2 == 2) $$ = newrel('<', $1, $3);
                                                             else if ($2 == 3) $$ = newrel('!'+'=', $1, $3);
                                                             else if ($2 == 4) $$ = newrel('='+'=', $1, $3);
                                                             else if ($2 == 5) $$ = newrel('>'+'=', $1, $3);
                                                             else $$ = newrel('<'+'=', $1, $3); if(DEBUG || PAR_DEBUG) printf("bool_factor PARSED!\n"); }
        ;
    
%%








