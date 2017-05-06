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
    int fn;
}

/*    Tokens go here; Names and Literal values    */
    %token <d> INT_LITERAL
    %token <f> FLT_LITERAL
    %token <sln> STR_LITERAL
    %token <s> CIN COUT ELSE
    %token <sln> ENDL
    %token <s> FLOAT IF INT RETURN WHILE ID 
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

    %type <a> variable program function_definitions function_head block identifier_list variable_definitions arguments parameter_list parameters statements statement compound_statement bool_expression expressions input_statement expression_list bool_term bool_factor output_statement expression simple_expression factor literal
    %type <d> type
    %start program
%%

    program: variable_definitions function_definitions { if(DEBUG || PAR_DEBUG) printf("!!PROGRAM PARSED!!\n"); $$ = ast_root = newast(('h'), $1, $2); if(DEBUG || PAR_DEBUG) printAST($$); }
        ;
        
    function_definitions:    function_head block     { $$ = newast('f'+'d', $1, $2); }
        | function_definitions function_head block  { if(DEBUG || PAR_DEBUG) printf("function_definitions PARSED\n"); }
        ;
        
    identifier_list:    ID                                { $$ = newref($1); }
        | ID '[' INT_LITERAL ']'                        { $$ = newref($1); }
        | identifier_list ',' ID                        { $$ = newast('I'+'D'+'L', $1, newref($3)); }
        | identifier_list ',' ID '[' INT_LITERAL ']'    { $$ = newast('I'+'D'+'L', $1, newref($3)); } 
        ;
        
    variable_definitions:                                              { /*$$ = newast('v'+'a'+'r', NULL, NULL);*/ $$ = NULL;}
        | variable_definitions type identifier_list ';'               { $$ = newast('v'+'a'+'r', $1, $3); }
        ;
        
    type:    INT { $$ = 'f'; }
        | FLOAT { $$ = 'F'; }
        ;
        
    function_head:    type ID arguments { $$ = newast('f'+'h', newref($2), $3); }
        ;
        
    arguments:    '(' parameter_list ')' { $$ = newast('a'+'r'+'g', $2, NULL); }
        ;
        
    parameter_list:                                        { /*$$ = newast('p'+'l', NULL, NULL);*/ $$ = NULL; }
        | parameters                                    { $$ = newast('p'+'l', $1, NULL); }
        ;
        
    parameters:    type ID                                        { $$ = newref($2); }
        | type ID '[' ']'                                    { $$ = newref($2); }
        | parameters ',' type ID         %prec ASSIGNOP        { $$ = newast('p'+'a'+'r', $1, newref($4)); }
        | parameters ',' type ID '[' ']' %prec ASSIGNOP        { $$ = newast('p'+'a'+'r', $1, newref($4)); }
        ;
        
    block:    '{' variable_definitions statements '}' { fflush(stdout); $$ = newast(('b'+'l'+'k'), $2, $3); }
        ;
        
    statements:                                        { /*$$ = newast('s'+'t'+'m'+'t'+'s', NULL, NULL);*/ $$ = NULL;}
        | statements statement                        { $$ = newast('s'+'t'+'m'+'t'+'s', $1, $2); }
        ;
        
    statement:    expression ';'                                    { if(DEBUG || PAR_DEBUG) printf("statement PARSED!\n"); }
        | compound_statement                                    { if(DEBUG || PAR_DEBUG) printf("statement PARSED!\n"); }
        | RETURN expression ';'                                    { $$ = NULL; }
        | IF '(' bool_expression ')' statement ELSE statement    { if(DEBUG || PAR_DEBUG) printf("statement PARSED!\n"); }
        | WHILE '(' bool_expression ')' statement                { if(DEBUG || PAR_DEBUG) printf("statement PARSED!\n"); }
        | input_statement ';'                                    { if(DEBUG || PAR_DEBUG) printf("statement PARSED!\n"); }
        | output_statement ';'                                    { $$ = newast(('s'+'t'+'m'+'t'), $1, NULL); }
        ;
        
    input_statement:    CIN                                        { if(DEBUG || PAR_DEBUG) printf("input_statement PARSED!\n"); }
        | input_statement STREAMIN variable         %prec ASSIGNOP    { if(DEBUG || PAR_DEBUG) printf("input_statement PARSED!\n"); }
        ;
        
    output_statement:    COUT                                        { $$ = newast(COUT, NULL, NULL); }
        | output_statement STREAMOUT expression     %prec ASSIGNOP        { $$ = newast('o', $1, $3); }
        | output_statement STREAMOUT STR_LITERAL                    { $$ = newast('o', $1, newstr($3)); }
        | output_statement STREAMOUT ENDL                            { $$ = newast('o', $1, newstr($3)); }
        ;
        
    compound_statement:    '{' statements '}'        { if(DEBUG || PAR_DEBUG) printf("compound_statement PARSED!\n"); }
        ;
        
    variable:    ID                                { if(DEBUG || PAR_DEBUG) printf("variable PARSED!\n"); }
        | ID '[' expression ']'                    { if(DEBUG || PAR_DEBUG) printf("variable PARSED!\n"); }
        ;
        
    expression_list:                            { /* nothing */ }
        | expressions                            { if(DEBUG || PAR_DEBUG) printf("expression_list PARSED!\n"); }
        ;
        
    expressions:    expression                                { if(DEBUG || PAR_DEBUG) printf("expressions PARSED!\n"); }
        | expressions ',' expression    %prec ASSIGNOP        { if(DEBUG || PAR_DEBUG) printf("expressions PARSED!\n"); }
        ;
        
    expression:    variable ASSIGNOP expression    { if(DEBUG || PAR_DEBUG) printf("expression PARSED!\n"); }
        | variable INCOP expression                { if(DEBUG || PAR_DEBUG) printf("expression PARSED!\n"); }
        | simple_expression                        { if(DEBUG || PAR_DEBUG) printf("expression PARSED!\n"); }
        ;
        
    simple_expression:    term            { if(DEBUG || PAR_DEBUG) printf("simple_expression PARSED!\n"); }
        | ADDOP term                    { if(DEBUG || PAR_DEBUG) printf("simple_expression PARSED!\n"); }
        | simple_expression ADDOP term    { if(DEBUG || PAR_DEBUG) printf("simple_expression PARSED!\n"); } 
        ;
        
    term:    factor                        { if(DEBUG || PAR_DEBUG) printf("term PARSED!\n"); }
        | term MULOP factor                { if(DEBUG || PAR_DEBUG) printf("term PARSED!\n"); }
        ;
        
    factor:    ID                            { if(DEBUG || PAR_DEBUG) printf("factor PARSED!\n"); }
        | ID '(' expression_list ')'    { if(DEBUG || PAR_DEBUG) printf("factor PARSED!\n"); }
        | literal                        { if(DEBUG || PAR_DEBUG) printf("factor PARSED!\n"); }
        | '(' expression ')'            { if(DEBUG || PAR_DEBUG) printf("factor PARSED!\n"); }
        | ID '[' expression ']'            { if(DEBUG || PAR_DEBUG) printf("factor PARSED!\n"); }
        ;
        
    literal:    INT_LITERAL                { $$ = newint($1); if(DEBUG || PAR_DEBUG) printf("literal PARSED!\n"); }
        | FLT_LITERAL                    { $$ = newfloat($1); if(DEBUG || PAR_DEBUG) printf("literal PARSED!\n"); }
        ;
        
    bool_expression:    bool_term            { if(DEBUG || PAR_DEBUG) printf("bool_expression PARSED!\n"); }
        | bool_expression OR bool_term        { if(DEBUG || PAR_DEBUG) printf("bool_expression PARSED!\n"); }
        ;
        
    bool_term:    bool_factor                    { if(DEBUG || PAR_DEBUG) printf("bool_term PARSED!\n"); }
        | bool_term AND bool_factor            { if(DEBUG || PAR_DEBUG) printf("bool_term PARSED!\n"); }
        ;
        
    bool_factor:    NOT bool_factor                        { if(DEBUG || PAR_DEBUG) printf("bool_factor PARSED!\n"); }
        | '(' bool_expression ')'                        { if(DEBUG || PAR_DEBUG) printf("bool_factor PARSED!\n"); }
        | simple_expression RELOP simple_expression        { if(DEBUG || PAR_DEBUG) printf("bool_factor PARSED!\n"); }
        ;
    
%%




