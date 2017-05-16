compiler:	lexer.l parser.y helper_functions.h helper_functions.c
	bison -d parser.y && \
	flex -olexer.c lexer.l && \
	cc -g -o $@ parser.tab.c lexer.c helper_functions.c -lm

clean:
	rm -f compiler output *.q \
	lexer.c parser.tab.c parser.tab.h
	rm -f -r *.dSYM

clean_q:
	rm -f *.q

