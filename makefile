assembler: main.o lexer.o panic.o ast.o symbol_table.o transletor.o
	gcc -ansi -pedantic -Wall -g lexer.o panic.o ast.o symbol_table.o main.o transletor.o -o assembler

lexer.o: lexer.c lexer.h
	gcc -ansi -pedantic -Wall -g -c lexer.c

panic.o: panic.c panic.h
	gcc -ansi -pedantic -Wall -g -c panic.c

ast.o: ast.c ast.h
	gcc -ansi -pedantic -Wall -g -c ast.c

symbol_table.o: symbol_table.c symbol_table.h
	gcc -ansi -pedantic -Wall -g -c symbol_table.c

main.o: main.c
	gcc -ansi -pedantic -Wall -g -c main.c

transletor.o: transletor.c transletor.h
	gcc -ansi -pedantic -Wall -g -c transletor.c
	
clean:
	rm -f *.o assembeler