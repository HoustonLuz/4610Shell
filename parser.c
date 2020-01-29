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
#include <fcntl.h>

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
void executep(char **cmd);
void execute(char **cmd, int bg);

void Redirect(instruction* instr, int bg);
void Pipe(instruction* instr, int numPipes);
void Background(instruction* instr);

int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

	int exitFlag = 0;
	int pipeFlag, redirectFlag, groundFlag;
	int numCommands = 0;
	int numPipes;
	int i, stack;

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

		if(strcmp(instr.tokens[0],"exit") == 0){

			exitFlag = 1;

		//echo
		} else if(strcmp(instr.tokens[0],"echo") == 0){
			str = (char*) calloc(instr.numTokens * (strlen(instr.tokens[0]) + 1), sizeof(char));
			int t = 1, q = 0;    // token and quotation counter
			if(instr.numTokens == 2)
				putchar('\n');
			else{
				do {     // loop through each remaining token
					if(instr.tokens[t][0] == '"'){
						memmove(instr.tokens[t], instr.tokens[t] + 1,
							strlen(instr.tokens[t]));
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
						memmove(instr.tokens[t], instr.tokens[t] + 1,
							strlen(instr.tokens[t]));
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
				} while (instr.tokens[t] != '\0');   // until last if(q % 2)
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
		}// end cd

		else{
			pipeFlag = 0;
			redirectFlag = 0;
			groundFlag = 0;

			for(i = 0;i < instr.numTokens - 1;i++) {
				if((strcmp(instr.tokens[i], "&")) == 0){
					groundFlag = 1;
				}
				if((strcmp(instr.tokens[i], ">") == 0)
				|| (strcmp(instr.tokens[i], "<") == 0)){
					redirectFlag = 1;
				}
				if((strcmp(instr.tokens[i], "|")) == 0){
					pipeFlag = 1;
				}
			}

			//Simplistic if statements becuase assumption 5 states
			//"Pipes and I/O redirection will not occur together"

			if(groundFlag == 1) {
				Background(&instr);
			} else if(pipeFlag == 1) {
				numPipes = 0;
				stack = 0;
				for(i = 0;i < instr.numTokens - 1;i++){
					if(strcmp(instr.tokens[i], "|") == 0) {
						stack--;
						numPipes++;
					} else {
						if(stack == 0)
							stack++;
					}
					if(stack < 0){
						printf("Invalid null command.\n");
					}
				}
				if(stack != 1){
					printf("Invalid null command.\n");
				} else {
					Pipe(&instr, numPipes);
				}
			} else if (redirectFlag == 1) {
				Redirect(&instr, 0);
			} else{
				execute(instr.tokens, 0);
			}
		}

		//printTokens(&instr);
		clearInstruction(&instr);

		numCommands++;
	}

	printf("Exiting now!\n");
	printf("   Commands executed: %d\n", numCommands);

	return 0;
}

void Redirect(instruction* instr, int bg)
{
   int numIn = 0, numOut = 0;
   int t = 0;
   int numRedir;

   do{
      if(strcmp(instr->tokens[t],">") == 0)
         numOut++;
      if(strcmp(instr->tokens[t],"<") == 0)
         numIn++;
      t++;
   } while(instr->tokens[t] != '\0');

   numRedir = numIn + numOut;

   if((numIn > 1) || (numOut > 1)){
      printf("Error: Too many redirections\n");
      return;
   }                             

   t = 0;

   // error check first
   do{
      if((strcmp(instr->tokens[t],">") == 0) ||
         (strcmp(instr->tokens[t],"<") == 0)){
         if((instr->tokens[t+1] == '\0') || t == 0){
            printf("Error: Invalid syntax\n");
            break;
         }                             
      }       
      t++;                                   
   } while(instr->tokens[t] != '\0');

   // fork process and loop until end of redirections
   int fd0, fd1;        
   int pid = fork();
   if(pid < 0)
      perror("fork");
   else if (pid == 0){
      t = 1; 
      do{
         if(strcmp(instr->tokens[t],"<") == 0){
            fd0 = open(resolvePath(instr->tokens[t+1]), O_RDONLY);
            close(0);
            dup(3);
            close(3);
            instr->tokens[t] = '\0';
            numRedir--;
         }
         else if(strcmp(instr->tokens[t],">") == 0){
            fd1 = open(resolvePath(instr->tokens[t+1]), O_RDWR|O_CREAT|O_TRUNC);
            close(1);
            dup(3);
            close(3);
            instr->tokens[t] = '\0';
            numRedir--;
         }
         t++;
      } while(numRedir > 0);
      execute(instr->tokens, bg);
      exit(1);
   }
   else
      wait();
}


void Pipe(instruction* instr, int numPipes)
{
	//Only dealing with cases of two pipes or less because of assumption 2 which states
	//"No more than two pipes will appear in a single line"

	int fd1[2];
	int fd2[2];
	instruction instrs[3];
	int i, j;
	pid_t pids[3];
	int status;

	if(numPipes > 2){
		printf("More than two pipes not implemented.\n");
		return;
	}

	for(i = 0;i < 3;i++){
		instrs[i].tokens = NULL;
		instrs[i].numTokens = 0;
	}

	//Breaking instr into multiple instructions
	i = 0;
	for(j = 0;j < numPipes;j++){
		do{
			addToken(&instrs[j], instr->tokens[i]);
			i++;
		} while (strcmp(instr->tokens[i], "|") != 0);
		addNull(&instrs[j]);

		i++;
	}

	do{
		addToken(&instrs[j], instr->tokens[i]);
		i++;
	} while (i < instr->numTokens - 1);
	addNull(&instrs[j]);

	
	//Code from slides for actual execution.
	if(numPipes == 1){
		pipe(fd1);

		pids[0] = fork();
		if(pids[0] == 0){
			//Child (cmd1 | cmd2)
			pids[1] = fork();
			if(pids[1] == 0){
				//cmd1 (Writer)
				close(1);
				dup(4);
				close(3);
				close(4);
				execvp(resolveExec(instrs[0].tokens[0]), instrs[0].tokens);
			} else {
				//cmd2 (Reader)
				close(0);
				dup(3);
				close(3);
				close(4);
				execvp(resolveExec(instrs[1].tokens[0]), instrs[1].tokens);
			}
		} else {
			//Parent(shell)
			close(3);
			close(4);
		}

		waitpid(pids[0], &status, 0);
		waitpid(pids[1], &status, 0);

	} else if(numPipes == 2){
		pipe(fd1);
		pipe(fd2);

		pids[0] = fork();
		if(pids[0] = 0){

			pids[1] = fork();
			if(pids[1] == 0){

				pids[2] = fork();
				if(pids[2] == 0){
					//cmd 1 (Writer)
					close(1);
					dup(fd1[1]);
					close(fd1[0]);
					close(fd1[1]);
					execvp(resolveExec(instrs[0].tokens[0]), instrs[0].tokens);
				}
				else {
					//cmd 2 (Writer and Reader)
					close(1);
					dup(fd2[1]);
					close(0);
					dup(fd1[0]);
					close(fd1[1]);
					close(fd2[0]);
					execvp(resolveExec(instrs[1].tokens[0]), instrs[1].tokens);
				}
			} else {
				//cmd 3 (Reader)
				close(0);
				dup(fd2[0]);
				close(fd2[0]);
				close(fd2[1]);
				execvp(resolveExec(instrs[2].tokens[0]), instrs[2].tokens);
			}
		} else {
			//parent

			close(fd2[0]);
			close(fd2[1]);
			close(fd1[0]);
			close(fd1[1]);
		}
		waitpid(pids[0], &status, 0);
		waitpid(pids[1], &status, 0);
		waitpid(pids[2], &status, 0);
	}


	for(i = 0;i < 3;i++)
		clearInstruction(&instrs[i]);

}

void Background(instruction* instr){
   int t = 0;        
   int q = 0;

   if(strcmp(instr->tokens[0],"&") == 0){
      // execute cmd in foreground, ignore &
      do{   // shift instr left one token
         instr->tokens[t] = instr->tokens[t+1];
         t++;
      } while(instr->tokens[t] != '\0');
      execute(instr->tokens, 0);  
      return;
   }

   t = 0;                 

   // error checking 
   do{             
      if(strcmp(instr->tokens[t],"&") == 0){
         printf("Error: Invalid syntax\n");
         return;
      }
      t++;
   } while(instr->tokens[t+1] != '\0');

   if(strcmp(instr->tokens[instr->numTokens - 2],"&") == 0){
      // execute cmd in background
      instr->tokens[instr->numTokens - 2] = '\0';
      execute(instr->tokens, 1);
   }

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
void execute(char **cmd, int bg){
   int status, q = 0;
   pid_t pid = fork();
   pid_t wp;    

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
      if(bg == 0) // foreground
         waitpid(pid, &status, 0);
      if(bg == 1){
         q++;
         wp = waitpid(pid, &status, WNOHANG);
         printf("[%d]   [%d]\n", q, pid);
         waitpid(pid, &status, 0);
//       if(wp > 0)
            printf("[%d]+  [%d]\n", q, cmd[0]);
      }
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
