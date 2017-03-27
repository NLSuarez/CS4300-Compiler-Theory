/*
Spring 2017
CS4300
Final Project: Compiler
Lex Programmer: Stefan Suarez(with assistance from Thomas on the numbers)
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
OCT -?0(0|([1-7][0-7]*))
HEX -?0[xX](0|([1-9A-F][0-9A-F]*))
WHITE_SPACE [ \t\n\v\f\r]+
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

 /* assignop */

 /* comments */
"//".*
"/*".*"*/"
 /* ignore white space */


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

/* PROGRAM OUTPUT GOES HERE - USE VARS FROM ABOVE */

//printf("Out of %d possible numbers, there were %d numbers, and %d not numbers\n", input_size, number_count, not_number_count);

return 0;
}
