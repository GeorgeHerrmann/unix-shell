compile: shell.c
		gcc -Wall -pedantic-errors -o shell shell.c

clean:
		rm -f shell
