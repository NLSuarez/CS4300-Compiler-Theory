/*
Spring 2017
CS4300
Final Project: Compiler
Lex Programmer: Stefan Suarez(with assistance from Thomas on the numbers)
*/


%option noyywrap

%{
int input_size = 0;
int number_count = 0;
int not_number_count = 0;
%}

INT -?(0|([1-9][0-9]*))
FLT {INT}[.][0-9]+
POW ({INT}|{FLT})(e|E){INT}
OCT -?0(0|([1-7][0-7]*))
HEX -?0[xX](0|([1-9A-F][0-9A-F]*))
WHITE_SPACE [ \t\n\v\f\r]+
NON_WHITE_SPACE [^ \t\n\v\f\r]+
%%

{INT} { ++input_size; ++number_count; printf("%s is a number\n", yytext); /*printf("%s\t\t-> pattern INT\n", yytext);*/ }
{FLT} { ++input_size; ++number_count; printf("%s is a number\n", yytext); /*printf("%s\t\t-> pattern FLT\n", yytext);*/ }
{POW} { ++input_size; ++number_count; printf("%s is a number\n", yytext); /*printf("%s\t\t-> pattern POW\n", yytext);*/ }
{OCT} { ++input_size; ++number_count; printf("%s is a number\n", yytext); /*printf("%s\t\t-> pattern OCT\n", yytext);*/ }
{HEX} { ++input_size; ++number_count; printf("%s is a number\n", yytext); /*printf("%s\t\t-> pattern HEX\n", yytext);*/ }
{WHITE_SPACE} { /* NO_OP */}
{NON_WHITE_SPACE}    { ++input_size; ++not_number_count; printf("%s is not a number\n", yytext); /*printf("%s\t\t-> !! NO PATTERN !!\n", yytext);*/ }

%%

int main(int argc, char **argv)
{
yylex();

/* PROGRAM OUTPUT GOES HERE - USE VARS FROM ABOVE */

printf("Out of %d possible numbers, there were %d numbers, and %d not numbers\n", input_size, number_count, not_number_count);

return 0;
}
