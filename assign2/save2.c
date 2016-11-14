#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
char cmdargs[13][20];
char name[100];
int pids[100];
char pnames[100][16];
char * token, *ampcheck;
int q = 0;
int pi = 0;
int bgflag = 0, another = 0;
int pid = 0;
int status;
int inFlag = 0, outFlag = 0;
char inFile[20];

void Init(){
	if(pi > 100)
		pi = 0;
	if(another == 0){
		printf("cs350sh>");
		scanf ("%[^\n]%*c", name);
		token = strtok(name, " ");
	}
	if(another == 1)
		another = 0;
}

void listjobs(){
	for(int i = 0; i < 100; i++){
		if(strcmp(pnames[i], "thereare15chars") != 0){
			if(waitpid(pids[i], &status, WNOHANG) == 0){
				printf("%s with PID %d Status:RUNNING\n", pnames[i], pids[i]);
			}
			else{
				printf("%s with PID %d Status:FINISHED\n", pnames[i], pids[i]);
				strcpy(pnames[i], "thereare15chars");
			}
		}
	}
}

int checkFirst(){
	if(strcmp(token, "exit") == 0)//checks to see if shell should exit
		exit(0);
	else if(strcmp(token, "listjobs") == 0){
		listjobs();
		return 1;
	}
	else if(strcmp(token, "fg") == 0){
		token = strtok(NULL, " ");
		waitpid(atoi(token), &status, 0);
		return 1;
	}
	else
		return -1;
}

int hasInput(){
	for(int i = 0; i < q; i++){
		if(strcmp(cmdargs[i] , "<") == 0)
			return i;
	}
	return -1;
}

int hasOutput(){
	for(int i = 0; i < q; i++){
		if(strcmp(cmdargs[i] , ">") == 0)
			return i;
	}
	return -1;
}

void tokenize(){
	inFlag = 0;
	for(q = 0; token != NULL; q++){
		if(strcmp(token, "&") == 0){//checks for "&" to see if parent should wait
			bgflag = 1;
			token = strtok(NULL, " ");
			if(token != NULL)
				another = 1;
			break;
		}
		else if(inFlag == 0){
			strcpy(cmdargs[q], token);
			token = strtok(NULL, " ");
		}
	}
}

int pidStore(){
	while(strcmp(pnames[pi], "thereare15chars") != 0){
		pi++;
	}
	strcpy(pnames[pi], cmdargs[0]);
	pids[pi] = fork();
	pid = pids[pi];
	pi++;
}

void redirectIn(int i){
	int in = open(cmdargs[i + 1], O_RDONLY);
	dup2(in, 0);
	close(in);
}

void redirectOut(int i){
	int out = open(cmdargs[i + 1], O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR);
	dup2(out, 1);
	close(out);
}

void runCommand(){
	inFlag = hasInput();
	outFlag = hasOutput();
	if(inFlag != -1)
		q -= 2;
	if(outFlag != -1)
		q -=2;
	char *cmdargs1[q+1];
	for(int i = 0; i <= q; i++){
		cmdargs1[i] = cmdargs[i];
	}
	cmdargs1[q] = NULL;
	//handling background processes and storing pids and process names. Fork occurs here
	if(bgflag ==1)
		pidStore();
	else{
		pid = fork();
	}
	if(pid < 0)
		perror("fork");
	if(pid == 0){//executing the command with arguments in child
		if(inFlag != -1)
			redirectIn(inFlag);
		if(outFlag != -1)
			redirectOut(outFlag);
		execvp(cmdargs[0], cmdargs1);
	}
	else{
		//printf("bgflag = %d", bgflag);
		//if "&" set bgflag to 1 skips waiting for child and reset bgflag, otherwise wait for process
		if(bgflag == 0){
			waitpid(pid, &status, 0);
		}
		else
			bgflag = 0;
		//wait(0);
		//printf("pnames[0] = %s, pnames[1] = %s", pnames[0], pnames[1]);
	}
}

int main(char argc, char ** argv){
	
	for(int i = 0; i < 100; i++){
		strcpy(pnames[i], "thereare15chars");
	}
	while(1){
		//prompts user for input and stores it in name to be tokenized
		Init();
		//printf("%s", pnames[0]);
		//tokenize string and store it in array to be passed to execvp
		if(checkFirst() == -1){
			tokenize();
			runCommand();	
		}
	}
	return 0;
}
