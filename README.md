# 4610Shell

COP4610
Project 1: Shell

Team Members:
	Houston Luzader
	Brandon Matulonis

Division of Labor:
	Houston Luzader
		-Part 3: Prompt
		-Part 4: Shortcut Resolution
		-Part 5: PATH Resolution
		-Part 8: Pipes
		-Part 10: Built-ins (exit)
	Brandon Matulonis
		-Part 6: Execution
		-Part 7: I/O Redirection
		-Part 9: Background Processing
		-Part 10: Built-ins (echo, cd)
		-Documentation/README

Tar contents:
	-makefile: makefile to compile parser.c into shell executable
	-parser.c: Entire shell code based on parser template
	-log.txt : Git commit log

How to compile executables:
	>make
	>shell

Known bugs:
	-echo spaces (runtime): typing echo followed by spaces will parse until the next token is typed, and will not print properly
	-alias is a built in bash, and alias was not included in the built ins we needed to make, so we didn't make it.

Unfinished sections:
	-double pipes coded as I would've thought it would work but it creates a second shell inside instead.
          In my code I split the individual commands into individual instructions and then execute them
          through execvp, I've error checked these individual instructions and they aren't the issue. So it must
          be that I am closing the wrong file descriptors or using the wrong file descriptors as streams for some
	  or all of the processes.
	-jobs was not implemented before deadline

Extra credit:
	-Shell-ception: Our shell can continue to run the shell until exiting each one [1]
