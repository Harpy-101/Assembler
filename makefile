assembler: main.o lexer.o panic.o ast.o symbol_table.o transletor.o preprocessor.o file_generator.o error_flags.o
	gcc -ansi -pedantic -Wall -g lexer.o panic.o ast.o symbol_table.o main.o transletor.o preprocessor.o file_generator.o error_flags.o -o assembler

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

preprocessor.o: preprocessor.c preprocessor.h
	gcc -ansi -pedantic -Wall -g -c preprocessor.c

file_generator.o: file_generator.c file_generator.h
	gcc -ansi -pedantic -Wall -g -c file_generator.c

error_flags.o: error_flags.c error_flags.h
	gcc -ansi -pedantic -Wall -g -c error_flags.c

clean:
	rm -f *.o assembeler