/*
Spring 2017
CS4300
Final Project: Compiler
Lex Programmer: Stefan Suarez(with assistance from Thomas on the numbers)

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
STR_LITERAL is a string enclosed in quotes (”), not longer than 1 line
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
/*
Header includes here
*/
%}

INT -?(0|([1-9][0-9]*))
FLT {INT}[.][0-9]+
POW ({INT}|{FLT})(e|E){INT}
WHITE_SPACE [ \t]+
NON_WHITE_SPACE [^ \t\n\v\f\r]+

%%

 /* single character ops */
")" |
"[" |
"]" |
"{" |
"}" |
";" |
","	{ return yytext[0]; }

 /* ASSIGNOP stands for the lexeme = */

"=" { return ASSIGNOP; }

 /* MULOP is one of * / % */

"*" { yylval.fn = 1; return MULOP; }
"/" { yylval.fn = 2; return MULOP; }
"%" { yylval.fn = 3; return MULOP; }

 /* ADDOP is one of + - */

"+" { yylval.fn = 1; return ADDOP; }
"-" { yylval.fn = 2; return ADDOP; }

 /* INCOP is one of += -= */



 /* RELOP is one of < > < = > =  ==  != */



 /* NOT stands for the lexeme ! */



 /* OR stands for the lexeme || */



 /* AND stands for the lexeme && */



 /* FLT_LITERAL is a float constant without a sign
(at least 1 digit before and after decimal pt.; possible exponent) */



 /* INT_LITERAL is an integer constant without a sign */



 /* STR_LITERAL is a string enclosed in quotes (”), not longer than 1 line */



 /* STREAMIN is >> */



 /* STREAMOUT is << */



 /* ID follows the usual rules for C++ identifiers, and may be any length */



 /* keywords CIN, COUT, ELSE, ENDL, FLOAT, IF, INT, RETURN, and WHILE */

"if"		{ return IF; }
"cin"		{ return CIN; }
"cout"		{ return COUT; }
"else"		{ return ELSE; }
"endl"		{ return ENDL; }
"while" 	{ return WHILE; }
"float"		{ return FLOAT; }
"int"		{ return INT; }
"return"	{ return RETURN;}
 
 /* comments */
"//".*
"/*".*"*/"
"#".*
 /* ignore white space */
{WHITE_SPACE}
 /* EOL catcher */
"\n" { return EOL; }

 /* Example
{INT} { return yytext; }
*/
 //Example
//{FLT} { ++input_size; ++number_count; printf("%s is a number\n", yytext); /*printf("%s\t\t-> pattern FLT\n", yytext);*/ }


%%

/* Eventually, there won't be a main as this will be covered in the other files */
int main(int argc, char **argv)
{
yylex();
return 0;
}
