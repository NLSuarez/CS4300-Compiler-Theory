/*
Spring 2017
CS4300
Final Project: Compiler
Lex Programmer: Stefan Suarez, Thomas Ferguson

Token Setup:
ASSIGNOP stands for the lexeme =
MULOP is one of * / %
ADDOP is one of + -
INCOP is one of += -=
RELOP is one of < > < = > =  ==  !=
NOT stands for the lexeme !
OR stands for the lexeme ||
AND stands for the lexeme &&
FLT_LITERAL is a float constant without a sign
(at least 1 digit before and after decimal pt.; possible exponent)
INT_LITERAL is an integer constant without a sign
STR_LITERAL is a string enclosed in quotes (ÃƒÆ’Ã†â€™Ãƒâ€ Ã¢â‚¬â„¢ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã†â€™Ãƒâ€šÃ‚Â¢ÃƒÆ’Ã‚Â¢ÃƒÂ¢Ã¢â‚¬Å¡Ã‚Â¬Ãƒâ€¦Ã‚Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â¬ÃƒÆ’Ã†â€™ÃƒÂ¢Ã¢â€šÂ¬Ã…Â¡ÃƒÆ’Ã¢â‚¬Å¡Ãƒâ€šÃ‚Â), not longer than 1 line
STREAMIN is >>
STREAMOUT is <<
ID follows the usual rules for C++ identifiers, and may be any length
CIN, COUT, ELSE, ENDL, FLOAT, IF, INT, RETURN, and WHILE are the keywords with those spellings
( ) [ ] {} ; and , are single-character tokens representing themselves
Additional lexical conventions:
Comments may be entered using either blocks or single lines using c syntax. Includes are also
comments to simplify the program.
*/

%option noyywrap nodefault yylineno

%{
#include "helper_functions.h"
#include "parser.tab.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned int yycolumn = 1;

#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;    yylloc.first_column = yycolumn; yylloc.last_column = yycolumn+yyleng-1; yycolumn += yyleng;

extern unsigned int DEBUG;
extern unsigned int LEX_DEBUG;
extern struct scope_node* global_scope;
extern struct scope_node* current_scope;
extern STRLIT_LIST str_list_head;
int addNextIDToken = 0;
%}

UINT (0|([1-9][0-9]*))
NEWLINE (\n)
ASSIGNOP ("=")
MULOP("*"|"/"|"%")
ADDOP("+"|"-")
INCOP("+="|"-=")
RELOP("<"|">"|"<="|">="|"=="|"!=")
NOT ("!")
OR ("||")
AND ("&&")
FLT_LITERAL ({UINT}"."[0-9]+)
INT_LITERAL ({UINT})
STR_LITERAL (\"[^\"\n]*\")
UNTERMINATED_STR_LITERAL (\"[^\"\n]*$)
STREAMIN (">>")
STREAMOUT ("<<")
ID ([A-Za-z_][A-Za-z_0-9]*)
CIN ("cin")
COUT ("cout")
ELSE ("else")
ENDL ("endl")
FLOAT ("float")
IF ("if")
INT ("int")
RETURN ("return")
WHILE ("while")
KEYWORD ({CIN}|{COUT}|{ELSE}|{ENDL}|{FLOAT}|{IF}|{INT}|{RETURN}|{WHILE})
COMMENT_LINE ("//"[^\n]+)
COMMENT_BLOCK ("/*"[^"*/"]+"*/")
PRE_PROC ("#"[^\n]+)
END_STMT (";")
O_PAREN ("(")
C_PAREN (")")
O_SUBSCRIPT ("[")
C_SUBSCRIPT ("]")
O_BRACE ("{")
C_BRACE ("}")
COMMA (",")
%%

{ASSIGNOP} { yycolumn++; return ASSIGNOP; }

{MULOP} { yycolumn += strlen(yytext); switch(yytext[0]){ case '*': yylval.fn = 1; break; case '/': yylval.fn = 2; break; case '%': yylval.fn = 3; } return MULOP; }

{ADDOP} { yycolumn += strlen(yytext); if(strcmp(yytext, "+") == 0) yylval.fn = 1; else yylval.fn = 2; return ADDOP; }

{INCOP} { yycolumn += strlen(yytext); if(strcmp(yytext, "+=") == 0) yylval.fn = 1; else yylval.fn = 2; return INCOP; }

{RELOP} { yycolumn += strlen(yytext); char* arr[] = {">","<","!=","==",">=","<="}; char** relops = arr; for(int i=0;i<6;++i){if(strcmp(yytext,relops[i])==0){yylval.fn=(i+1); break;}} return RELOP; }

{NOT} { yycolumn += strlen(yytext); return NOT; }

{OR} { yycolumn += strlen(yytext); return OR; }

{AND} { yycolumn += strlen(yytext); return AND; }

{FLT_LITERAL}/{ID} { pError(error, "INVALID ID ENTERED!"); }

{FLT_LITERAL} { yycolumn += strlen(yytext); return FLT_LITERAL; }

{INT_LITERAL}/{ID} { pError(error, "INVALID ID ENTERED!"); }

{INT_LITERAL} { yycolumn += strlen(yytext); return INT_LITERAL; }

{STR_LITERAL} { yycolumn += strlen(yytext); if(lookup(yytext) == NULL) { addSymbol(global_scope->symTab, yytext, STR_LITERAL); } yylval.sln = appendToStrList(&str_list_head, yytext, 0); return STR_LITERAL; }

{UNTERMINATED_STR_LITERAL} {pError(error, "Unterminated string.");}

{STREAMIN} { yycolumn += strlen(yytext); return STREAMIN; }

{STREAMOUT} { yycolumn += strlen(yytext); return STREAMOUT; }

{KEYWORD} { if(DEBUG || LEX_DEBUG) printf("Matched keyword: %s\n", yytext); fflush(stdout); if(strcmp(yytext, "int") == 0 || strcmp(yytext, "float") == 0) addNextIDToken = 1;  yycolumn += strlen(yytext); if(getKind(yytext) == ENDL && lookup("\\n") == NULL) { addSymbol(global_scope->symTab, "\\n", STR_LITERAL); }  yylval.sln = appendToStrList(&str_list_head, "\\n", 0); if(strcmp(yytext, "endl") != 0) yylval.s = lookup(yytext); return getKind(yytext); }

{END_STMT} { yycolumn += strlen(yytext); return yytext[0]; }

{ID} { if(DEBUG || LEX_DEBUG) printf("Matched ID: %s\n", yytext); fflush(stdout); if(addNextIDToken == 1) addSymbol(current_scope->symTab, yytext, ID); yycolumn += strlen(yytext); yylval.s = lookup(yytext); if(lookup(yytext) == NULL && (DEBUG || LEX_DEBUG)) printf("!!!%s NOT FOUND IN SYMBOL TABLE(S)!!!\n", yytext); return ID; }

{PRE_PROC}  { if(DEBUG || LEX_DEBUG) printf("Matched preprocessor statement\n"); fflush(stdout); yycolumn += strlen(yytext); /* **IGNORE PREPROCESSOR DIRECTIVES** */ }

{COMMENT_LINE} { yycolumn += strlen(yytext); /* **IGNORE COMMENTS** */ }

{COMMENT_BLOCK} { yycolumn += strlen(yytext); /* **IGNORE COMMENTS** */ }

{O_PAREN} { if(global_scope != NULL && global_scope == current_scope) pushScope(); yycolumn += strlen(yytext); return yytext[0]; }

{C_PAREN} { yycolumn += strlen(yytext); return yytext[0]; }

{O_SUBSCRIPT} { yycolumn += strlen(yytext); return yytext[0]; }

{C_SUBSCRIPT} { yycolumn += strlen(yytext); return yytext[0]; }

{O_BRACE} { yycolumn += strlen(yytext); if(current_scope != NULL) { if(current_scope->is_new_scope == 0) pushScope(); else current_scope->is_new_scope = 0; } return yytext[0]; }

{C_BRACE} { yycolumn += strlen(yytext); if(current_scope != NULL) popScope(); return yytext[0]; }

{COMMA} { yycolumn += strlen(yytext); return yytext[0]; }

{NEWLINE} { yylineno++; yycolumn = 1; /*return EOL*/ addNextIDToken = 0; }

[ \t\v]+ { yycolumn += strlen(yytext); /* Ignore non-newline white-space */ }

. { /* Call Scooby-Doo and the Gang, because we have a mystery on our hands... */ yycolumn += strlen(yytext); pError(warning, "Mystery char %c\n", *yytext); }

%%



