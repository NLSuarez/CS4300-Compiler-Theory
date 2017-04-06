/*
 * The error function takes in an enum errorLevel as the severity level of the error
 * and the error string that will be ouputted for the programmer to debug.
 * The function will update the highestErrorLevel if the error is of a higher severity
 * than previously recorded. The output will consist of the filename, linenumber, column,
 * the severity level of the error, and followed by the error's description.
*/
void yyerror(errorLevel errlvl, char *s, ...)
{
  va_list ap;
  va_start(ap, s);
  char *errLvls[3] = {"Warning", "Error", "Fatal"};

  if (!yylloc.highestErrorLevel)
  {
  	if (yylloc.highestErrorLevel < errlvl)
  	{
  		yylloc.highestErrorLevel = errlvl;
  	}
  }
  else
  {
  	yylloc.highestErrorLevel = errlvl;
  }
  
  	
  if(yylloc.first_line)
  {
    fprintf(stderr, "%s:%d.%d-%d.%d: %s: ", yylloc.filename, yylloc.first_line, yylloc.first_column,
	    yylloc.last_line, yylloc.last_column, errLvls[errlvl]);
  }
  vfprintf(stderr, s, ap);
  fprintf(stderr, "\n");

}
