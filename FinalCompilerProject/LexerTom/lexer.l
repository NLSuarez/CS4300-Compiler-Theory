/* Basic lexer created by Thomas Ferguson */

%option noyywrap

UINT (0|([1-9][0-9]*))
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
INCLUDE ("#"[^\n]+)
END_STMT (";")
%%

{ASSIGNOP} { printf("ASSIGNOP detected! STRING = \"%s\"\n", yytext); }

{MULOP} { printf("MULOP detected! STRING = \"%s\"\n", yytext); }

{ADDOP} { printf("ADDOP detected! STRING = \"%s\"\n", yytext); }

{INCOP} { printf("INCOP detected! STRING = \"%s\"\n", yytext); }

{RELOP} { printf("RELOP detected! STRING = \"%s\"\n", yytext); }

{NOT} { printf("NOT detected! STRING = \"%s\"\n", yytext); }

{OR} { printf("OR detected! STRING = \"%s\"\n", yytext); }

{AND} { printf("AND detected! STRING = \"%s\"\n", yytext); }

{FLT_LITERAL} { printf("FLT_LITERAL detected! STRING = \"%s\"\n", yytext); }

{INT_LITERAL} { printf("INT_LITERAL detected! STRING = \"%s\"\n", yytext); }

{STR_LITERAL} { printf("STR_LITERAL detected! STRING = \"%s\"\n", yytext); }

{STREAMIN} { printf("STREAMIN detected! STRING = \"%s\"\n", yytext); }

{STREAMOUT} { printf("STREAMOUT detected! STRING = \"%s\"\n", yytext); }

{KEYWORD} { printf("KEYWORD detected! STRING = \"%s\"\n", yytext); }

{END_STMT} { printf("END_STMT detected! STRING = \"%s\"\n", yytext); }

{ID} { printf("ID detected! STRING = \"%s\"\n", yytext); }

{INCLUDE}  { /* DO NOTHING */ }

%%

int main(int argc, char **argv)
{
    yylex();

    return 0;
}
