C=gcc -g
L=gcc -g -c

shell : parser.c
	$C parser.c -o shell
	chmod 700 *

clean:
	rm -f shell
	rm -f *.tar

tar:
	tar -cf project1_luzader_matulonis.tar README.md makefile parser.c
