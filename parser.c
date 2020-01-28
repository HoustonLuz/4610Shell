//COP4610
//Project 1 Starter Code
//example code for initial parsing

//*** if any problems are found with this code,
//*** please report them to the TA

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);

char* resolvePath(char* path);
char* resolveExec(char* path);
void execute(char **cmd);

int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

	int exitFlag = 0;
	int numOfCommands = 0;

	char cwd[256];
	static char* str = NULL;

	while (exitFlag == 0) {
		printf("%s@%s:%s> ", getenv("USER"), getenv("MACHINE"), getcwd(cwd, 255));

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

      } else if(strcmp(instr.tokens[0],"echo") == 0){
         str = (char*) calloc(instr.numTokens * (strlen(instr.tokens[0]) + 1), sizeof(char));
         int t = 1, q = 0;    // token and quotation counter
         if(instr.numTokens == 2)
            putchar('\n');
         else{
            do {     // loop through each remaining token
               if(instr.tokens[t][0] == '"'){
                  memmove(instr.tokens[t], instr.tokens[t] + 1, strlen(instr.tokens[t]));
                  q++;
               }
               if(instr.tokens[t][strlen(instr.tokens[t]) - 1] == '"'){
                  instr.tokens[t][strlen(instr.tokens[t]) - 1] = '\0';
                  q++;
               }
               if(instr.tokens[t][0] != '$'){
                  strcat(str, " ");
                  strcat(str, instr.tokens[t]);
               }
               else{
                  // discard $ in arguement string and print if EV exists
                  memmove(instr.tokens[t], instr.tokens[t] + 1, strlen(instr.tokens[t]));
                  if(getenv(instr.tokens[t])){
                     strcat(str, " ");
                     strcat(str, getenv(instr.tokens[t]));
                  }
                  else{
                     printf("%s: Undefined variable.\n", instr.tokens[t]);
                     break;
                  }
               }
               t++;
            } while(instr.tokens[t] != '\0');   // until last token
            if(q % 2)
               printf("Unmatched \".\n");
            else{
               memmove(str, str + 1, strlen(str));
               printf("%s\n", str);
            }
         }
			free(str);
      } // end echo

		// cd
		else if(strcmp(instr.tokens[0],"cd") == 0){
			if(chdir(resolvePath(instr.tokens[1])) != 0)
			perror(resolvePath(instr.tokens[1]));
		}		// end cd

		//execution
		else{
			execute(instr.tokens);
		}


		clearInstruction(&instr);

		numOfCommands++;
	}

	printf("Exiting now!\n");
	printf("   Commands executed: %d\n", numOfCommands);

	return 0;
}

char* resolveExec(char* path)
{
	static char* resExec;
	char* paths = getenv("PATH");
	int i, j;

	if(resExec != 0){
		free(resExec);
		resExec = 0;
	}

	if(strstr(path, "/") != 0){
		resExec = (char*) malloc
		((strlen(resolvePath(path)) + 1) * sizeof(char));
		strcpy(resExec, resolvePath(path));
		return resExec;
	} else {
		resExec = (char*) malloc
		((strlen(getenv("PATH")) + 1) * sizeof(char));

		i = 0;
		j = 0;
		do {
			resExec[j] = paths[i];
			i++;
			j++;
			if(paths[i - 1] == ':'){
				resExec[j - 1] = '\0';
				strcat(resExec, "/");
				strcat(resExec, path);

				if(checkFileExist(resExec) == 1){
					return resExec;
				} else {
					j = 0;
				}
			}
		} while (i < strlen(getenv("PATH")));

		return resolvePath(path);
	}
}

int checkFileExist(char* path)
{
	FILE *check;

	if(check = fopen(path, "r")){
		fclose(check);
		return 1;
	} else {
		return 0;
	}
}

char* resolvePath(char* path)
{
	//Since I chose to just return the resolved path, I needed to make
	// the string static and deallocate if it isn't null.
	//If I didn't do this, I think it would create a memory leak if
	// this function was called enough times.

	static char* resPath;
	int i, count;

	static char cwd[256];

	getcwd(cwd, 255);

	if(resPath != 0){
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

	} else if (strcmp(path,  "..") == 0) {
		resPath = (char *) malloc((strlen(cwd) + 1) * sizeof(char));

		strcpy(resPath, cwd);

		while (resPath[strlen(resPath) -1] != '/'){
			resPath[strlen(resPath) - 1] = resPath[strlen(resPath)];
		}

		return resPath;

	} else if (strstr(path,  "../") != 0) {
		int removedStrLen, j;

		resPath = (char*) malloc((strlen(path) + 1) * sizeof(char));
		strcpy(resPath, path);

		count = 0;
		do {
			for(i = 0;i < strlen(resPath) - 2;i++){
				resPath[i] = resPath[i + 3];
			}
			count++;
		} while (strstr(resPath, "../") != 0);

		if(strcmp(resPath, "..") == 0){
			count++;
		}

		free(resPath);
		resPath = (char*) malloc
		((strlen(cwd) + 1) * sizeof(char));

		strcpy(resPath, cwd);

		i = 0;
		do {
			if(resPath[strlen(resPath) - 1] == '/')
				i++;

			resPath[strlen(resPath) - 1] = resPath[strlen(resPath)];
		} while (i < count);
		removedStrLen = strlen(resPath);

		strcat(resPath, "/");
		strcat(resPath, path);

		for(i = 0;i < count;i++){
			for(j = removedStrLen;j < strlen(resPath) - 2;j++){
				resPath[j] = resPath[j + 3];
			}
		}

		return resPath;

	} else if (strstr(path,  "./") != 0
		|| strcmp(path, ".")   == 0   ) {

		if(strcmp(path, ".") == 0) {
			return cwd;
		} else {
			resPath = (char*) malloc
			((strlen(path) + strlen(cwd) + 1) * sizeof(char));

			strcpy(resPath, cwd);
			strcat(resPath, path);

			for(i = strlen(cwd);i < strlen(resPath);i++){
				resPath[i] = resPath[i+1];
			}
			return resPath;
		}
	} else {
		resPath = (char*) malloc
		((strlen(path) + strlen(cwd) + 2) * sizeof(char));

		strcpy(resPath, cwd);

		strcat(resPath, "/");
		strcat(resPath, path);

		return resPath;

	}
}

// execution
void execute(char **cmd){
	int status;
	pid_t pid = fork();
	if(pid == -1){
		// Error
		printf("ERROR\n");
		exit(1);
	}
	else if (pid == 0){
		// Child
		execv(resolveExec(cmd[0]), cmd);
		printf("Problem executing %s\n", cmd[0]);
		exit(1);
	}
	else{
		// Parent
		waitpid(pid, &status, 0);
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
