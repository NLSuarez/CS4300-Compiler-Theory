/* include the following at the beginning of the parser*/
%code requires {
char *filename;

typedef struct YYLTYPE {
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char *filename;
  int highestErrorLevel;	/* no errors = 0, warning = 1, error = 2, fatal = 3*/
} YYLTYPE;
# define YYLTYPE_IS_DECLARED 1

# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (N)                                                            \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	  (Current).filename     = YYRHSLOC (Rhs, 1).filename;	        \
	  (Current).highestErrorLevel	 = YYRHSLOC (Rhs, 1).highestErrorLevel;	\
	}								\
      else								\
	{ /* empty RHS */						\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	  (Current).filename  = NULL;					\
	  (Current).highestErrorLevel = 0;					\
	}								\
    while (0)
}

/*
	Can place this towards the end of the Parser
	The error function takes in an int (1-3) as the severity level of the error
	and the error string that will be ouputted for the programmer to debug.
*/
void yyerror(int errLevel, char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  char errorlvl[8];

  if (!yylloc.highestErrorLevel)
  {
  	if (yylloc.highestErrorLevel < errLevel)
  	{
  		yylloc.highestErrorLevel = errLevel;
  	}
  }
  else
  {
  	yylloc.highestErrorLevel = errLevel;
  }
  
  switch (errLevel)
  {
  	case 1:
  		errorlvl = "warning";
  		break;
  	case 2:
  		errorlvl = "error";
  		break;
  	case 3:
  		errorlvl = "fatal";
  		break;
  	default:
  		break;
  }
  	
  if(yylloc.first_line)
    fprintf(stderr, "%s:%d.%d-%d.%d: error: %s ", yylloc.filename, yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column, errLevel);
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");

}