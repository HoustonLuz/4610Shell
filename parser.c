//COP4610
//Project 1 Starter Code
//example code for initial parsing

//*** if any problems are found with this code,
//*** please report them to the TA

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);

//void cd(instruction* instr);
char* resolvePath(char* path);

int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

	int exitFlag = 0;
	int numOfCommands = 0;

	while (exitFlag == 0) {
		printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getenv("PWD"));

		// loop reads character sequences separated by whitespace
		do {
			//scans for next token and allocates token var to size of scanned token
			scanf("%ms", &token);
			temp = (char*)malloc((strlen(token) + 1) * sizeof(char));

			int i;
			int start = 0;
			for (i = 0; i < strlen(token); i++) {
				//pull out special characters and make them into a separate token in the instruction
				if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&') {
					if (i-start > 0) {
						memcpy(temp, token + start, i - start);
						temp[i-start] = '\0';
						addToken(&instr, temp);
					}

					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					addToken(&instr,specialChar);

					start = i + 1;
				}
			}

			if (start < strlen(token)) {
				memcpy(temp, token + start, strlen(token) - start);
				temp[i-start] = '\0';
				addToken(&instr, temp);
			}

			//free and reset variables
			free(token);
			free(temp);

			token = NULL;
			temp = NULL;
		} while ('\n' != getchar());    //until end of line is reached

		addNull(&instr);

//		printTokens(&instr);

		if(strcmp(instr.tokens[0],"exit") == 0
		   || strcmp(instr.tokens[0],"x") == 0){
			//My own preference is that x is aliased to exit.
			//x leading to exit should not be in final turn in.

			exitFlag = 1;
		} else {
			//In this iteration I focused on path res so all
			// non-exits will be resolved.
			printf("%s\n", resolvePath(instr.tokens[0]));
		}

      // echo
      if(strcmp(instr.tokens[0],"echo") == 0){
         if(instr.tokens[1][0] != '$'){
            // print all remaining tokens after echo until end of string
            int t = 1;     // token counter
            do {
               printf("%s ", instr.tokens[t]);
               t++;
            } while(instr.tokens[t] != '\0');    // until last token
            putchar('\n');
         }
         else{
            // discard $ in arguement string and print if EV exists
            memmove(instr.tokens[1], instr.tokens[1] + 1, strlen(instr.tokens[1]));
            if(getenv(instr.tokens[1]))
               printf("%s\n", getenv(instr.tokens[1]));
            else
               printf("%s: Undefined variable.\n", instr.tokens[1]);
         }
      } // end echo

		clearInstruction(&instr);

		numOfCommands++;
	}

	printf("Exiting now!\n");
	printf("   Commands executed: %d\n", numOfCommands);

	return 0;
}

char* resolvePath(char* path)
{
	//Since I chose to just return the resolved path, I needed to make
	// the string static and deallocate it it isn't null.
	//If I didn't do this, I think it would create a memory leak if
	// this function was called enough times.

	static char* resPath;
	int i;

	if(resPath != NULL){
		free(resPath);
		resPath = 0;
	}

	if (path[0] == '~') {
		resPath = (char*) malloc
		((strlen(path) + strlen(getenv("HOME")) + 1) * sizeof(char));

		strcpy(resPath, getenv("HOME"));
		strcat(resPath, path);

		for(i = strlen(getenv("HOME"));i < strlen(resPath);i++){

			resPath[i] = resPath[i+1];

		}

		return resPath;
	} else if (path[0] == '/') {

		return path;

	} else if (strstr(path, "/../") != NULL
		|| strstr(path,  "../") != NULL
		|| strstr(path,  "/..") != NULL){

		return ".. path";		

	} else if (strstr(path, "/./") != NULL
		|| strstr(path,  "./") != NULL
		|| strstr(path,  "/.") != NULL){

		return ". path";		

	} else {
		resPath = (char*) malloc
		((strlen(path) + strlen(getenv("PWD")) + 2) * sizeof(char));

		strcpy(resPath, getenv("PWD"));

		strcat(resPath, "/");
		strcat(resPath, path);

		return resPath;

	}
}


//reallocates instruction array to hold another token
//allocates for new token within instruction array
void addToken(instruction* instr_ptr, char* tok)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

void addNull(instruction* instr_ptr)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;
	instr_ptr->numTokens++;
}

void printTokens(instruction* instr_ptr)
{
	int i;
	printf("Tokens:\n");
	for (i = 0; i < instr_ptr->numTokens; i++) {
		if ((instr_ptr->tokens)[i] != NULL)
			printf("%s\n", (instr_ptr->tokens)[i]);
	}
}

void clearInstruction(instruction* instr_ptr)
{
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
}
