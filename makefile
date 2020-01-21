C=gcc -g
L=gcc -g -c

Shell : parser.c
	$C parser.c -o Shell
	chmod 700 *

clean:
	rm -f Shell
