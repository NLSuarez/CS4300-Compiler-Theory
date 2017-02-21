/* Authors: Manjinder Singh, */

/* Just creating this space for when we need it -M */
%{
	// C code goes into here.-M
	
	/*
		possibleNumbers will keep track of how many inputs were given
		actualNumbers will keep track of how many inputs actually matched our number patterns
		failed numbers will keep track of how many inputs failed to match our number patterns
		-M
	*/
	int possibleNumbers = 0;
	int actualNumbers = 0;
	int failedNumbers = 0;
%}

/*
	Creating some name definitions to make the patterns to be easier to read -M
*/
/*	We can use these to make the first pattern easier to read -M
	These can be used like so: ^{Integers}{Float}?{Exp}?$ I'll you guys decide which way is better	-M
Integers	[-]?(0|[1-9][0-9]*)
Float		[.][0-9]+
Exp			[eE][-]?(0|[1-9][0-9]*)
*/

/*	This is section where our patterns to recognize numbers goes
	here along with their reaction C code. -M
*/
%%

^([-]?(0|[1-9][0-9]*)([.][0-9]+)?)(([eE]([-]?(0|[1-9][0-9]*)))?)$	{
							/* This pattern will recognize a finite series of decimal digits,
							   without leading 0's, and an optional minus sign. It will also
							   be able to recognize decimal numbers that contain a "." followed
							   by additional finite series of decimal digits. -M
							*/ 
							/*
								I'm just going to break the pattern down so I can understand it
								easily when I come back to it at a later time. -M
								
								[-]?			-> number may start with an optional "-"
								(0|[1-9][0-9]*)	-> number may be 0 or may start with a digit [1-9] followed by 0 or more digits [0-9]
								([.][0-9]+)?	-> number may have an optional "." in it, followed by 1 or more digits [0-9]
								[eE]			-> number may be in the format of being followed by e or E
								^				-> starts to read the expression from the beginning of the line
								$				-> reads the expression until the end of line
							*/
							printf("%s is a number.", yytext);
							possibleNumbers++;	// increment by 1 as an input was given -M
							actualNumbers++;	// increment by 1 as an input did match our pattern -M
							
							/*	This pattern breaks if you add a space between numbers. -M
								What still needs to be done is to find out how to break a line up at spaces. -M
							*/
						}
					
^([-]?(00|0[1-7][0-7]*))$	{
								/*
									This pattern should recognize a finite digits containing at least 2 digits
									with an optional "-", and exactly 1 leading 0.
								*/
								printf("%s is a number.", yytext);
								possibleNumbers++;	// increment by 1 as an input was given -M
								actualNumbers++;	// increment by 1 as an input did match our pattern -M
							}
							
^([-]?(0[xX][0-9A-F]+))$	{
								/*
									This pattern should recognize a finite string of hexadeciaml containing at 
									least 1 hexadecimal digit with an optional "-", and no leading 0's.
								*/
								printf("%s is a number.", yytext);
								possibleNumbers++;	// increment by 1 as an input was given -M
								actualNumbers++;	// increment by 1 as an input did match our pattern -M
							}

.$	{	printf("%s is not a number.", yytext);
			possibleNumbers++;	// increment by 1 as an input was given -M
			failedNumbers++;	// increment by 1 as the input has failed to match our patterns -M
		}

%%

main(int argc, char **argv)
{
	yylex();
	// print out the report. -M
	printf("Out of %d possible numbers, there were %d numbers, and %d not numbers.\n", possibleNumbers, actualNumbers, failedNumbers);
}
