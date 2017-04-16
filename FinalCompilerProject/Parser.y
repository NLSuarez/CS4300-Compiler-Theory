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
%%