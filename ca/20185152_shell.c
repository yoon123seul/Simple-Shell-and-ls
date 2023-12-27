/* $begin shellmain */
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>


#define MAXARGS   128
#define	MAXLINE	 8192  /* Max text line length */
#define MAXBUF   8192  /* Max I/O buffer size */

/* Function prototypes */
void eval(char *cmdline); // funtion for parsing IO redirection and parsing 
int parseline(char *buf, char **argv);
int builtin_command(char **argv); 
void sigint_handler (int signo);

extern char **environ; /* Defined by libc */

int main() 
{
	if (signal(SIGINT, sigint_handler) == SIG_ERR) { // install signal handler for SIGINT (ctrl + c)
		fprintf(stderr, "Error setting up SIGINT handler.\n");
		exit(1);
	}

	char cmdline[MAXLINE]; /* Command line */

	while (1) {
		/* Read */
		printf("$ ");                   
		fgets(cmdline, MAXLINE, stdin);  // get cmd from user
		if (feof(stdin))
			exit(0);

		/* Evaluate */
		eval(cmdline); // execute user cmd
		// printf("returned to main\n");
	} 
}
/* $end shellmain */

/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
	char *argv[MAXARGS]; /* Argument list execve() */
	char buf[MAXLINE];   /* Holds modified command line */
	int bg;              /* Should the job run in bg or fg? */
	pid_t pid, pipe_id;           /* Process id */

	strcpy(buf, cmdline);
	bg = parseline(buf, argv); 
	if (argv[0] == NULL)  
		return;   /* Ignore empty lines */

	if (!builtin_command(argv)) {  // if cmd is not builtin then run this block

		


		if ((pid = fork()) == 0) {   // creat new Child process and make it to runs user job
			int pipe_index[MAXARGS]; // first check if cmd has pipe or not. this array will save index of pipe
			int i = 0; 
			int num_pipes = 0; // integer for storing number of pipes
			pipe_index[0] = -1; // initialize pipe[0] with -1 for future uses
			while (argv[i] != NULL) { // find every pipe in cmd and store the index and number
				if (strcmp(argv[i], "|") == 0) {
					num_pipes++;
					pipe_index[num_pipes] = i;
					argv[i] = NULL;
				}
				i++;
			}
			if (num_pipes > 0){ // if we have pipe then run this block!
				int input_fd_pipe = 0; //fd for input 
				int j = 0;
				for (j = 0; j < num_pipes + 1; j++){ //make the pipe chain by iterating pipe index
					// printf("num_pipes : %d\n", num_pipes);
					// printf("argv[pipe_index[%d] + 1] :: %s\n", j, argv[pipe_index[j] + 1]);
					int pipe_fd[2]; // fd for pipe
					pipe(pipe_fd); // make pipe
					if ((pipe_id = fork()) == 0) { // Child process will execute left cmd and send the result to right cmd
						close(pipe_fd[0]); // to send via pipe
						dup2(input_fd_pipe, STDIN_FILENO); //connect input pipe with previous output or initialize value 0
						if (j == 0){ // if we have IO redirection in first part of cmd then run this block
							int jj = 0;
							while (argv[jj] != NULL){
								if (strcmp(argv[jj], "<") == 0) { // check for INPUT redirection
										argv[jj] = NULL;
										int fd;
										fd = open(argv[jj + 1], O_RDONLY); //open input file and send it to stdin of cmd
										dup2(fd, STDIN_FILENO);
										close(fd);
								}
								 jj++;
							}
						}
						if (j < num_pipes) // connect pipe to stdout
							dup2(pipe_fd[1], STDOUT_FILENO);
						if (j >= num_pipes){ // if we have IO redirection in last part of cmd we run this block
							int jj = pipe_index[j] + 1;
							while (argv[jj] != NULL){
								if (strcmp(argv[jj], ">") == 0) { // check for output redirection
										argv[jj] = NULL;
										int fd;
										fd = open(argv[jj + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
										dup2(fd, STDOUT_FILENO);
										close(fd);
								}
								 jj++;
							}
						}
						close(pipe_fd[1]);
						// printf("argv[pipe_index[j] + 1] :: %s\n", argv[pipe_index[j] + 1]);
						// char argv1[100];
						// int k = pipe_index[j] + 1;
						execvp(argv[pipe_index[j] + 1], &argv[pipe_index[j] + 1]); //run current cmd by index slicing
						perror("execvp");
						exit(EXIT_FAILURE);
					}
					else {
						waitpid(pipe_id, NULL, 0); //wait for child to terminate
						// printf("child is done \n");
						close(pipe_fd[1]);
						input_fd_pipe = pipe_fd[0]; //connect input fd to pipe
					}
					// printf("for is over %d\n", j);
				}
				exit(0);
			}
										// below codes are for IO redirection
			int output_index = 0; 
			int pid_pipe;
			int input_index = 0;
			int index = 0;
			int append_index = 0;
			int output_fd;
			int input_fd;
			int append_fd;
			while (argv[index] != NULL) { // checking for > < >>
				// printf ("index is : %d, argv is %s\n", index, argv[index]);
				if (strcmp(argv[index], ">") == 0) {
					argv[index] = NULL;
					output_index = index; 
					output_fd = open(argv[output_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644); //open output file
					dup2(output_fd, STDOUT_FILENO); //connect stdout to output fd
					close(output_fd);
					index ++;
					
					continue;;
				}
				else if (strcmp(argv[index], ">>") == 0) {
					argv[index] = NULL;
					append_index = index;
					append_fd = open(argv[append_index + 1], O_WRONLY | O_CREAT | O_APPEND, 0644); //open output file with append option
					dup2(append_fd, STDOUT_FILENO); // connect stdout with output fd
					close(append_fd);
					index ++;
					continue;
				}
				else if (strcmp(argv[index], "<") == 0) {
					// fprintf(stderr, "%s: Command not found.\n", argv[0]);
					// fprintf(stdout, "get < !!!!!!!!!!!!!!!!!!!!");
					argv[index] = NULL;
					input_index = index;
					input_fd = open(argv[input_index + 1], O_RDONLY, 0644); // open input file with Read permission
					dup2(input_fd, STDIN_FILENO); //connect std in with input file fd
					close(input_fd);
					index ++;
					continue;
				}
			
				index ++;
			}
			execvp(argv[0], argv); // run the code!!!

            // If execlp fails, print an error message
            printf("%s: Command not found.\n", argv[0]);

            // exit(EXIT_FAILURE);
        }

		/* Parent waits for foreground job to terminate */
		if (!bg) {
			int status;
			if (waitpid(pid, &status, 0) < 0)
				fprintf(stderr, "waitfg: waitpid error\n");
		}
		else
			printf("%d %s", pid, cmdline);
	}
	return;
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
	if (!strcmp(argv[0], "quit")) /* quit command */
		exit(0);  
	if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
		return 1;
	return 0;                     /* Not a builtin command */
}
/* $end eval */

/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
	char *delim;         /* Points to first space delimiter */
	int argc;            /* Number of args */
	int bg;              /* Background job? */

	buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
	while (*buf && (*buf == ' ')) /* Ignore leading spaces */
		buf++;

	/* Build the argv list */
	argc = 0;
	while ((delim = strchr(buf, ' '))) {
		argv[argc++] = buf;
		*delim = '\0';
		buf = delim + 1;
		while (*buf && (*buf == ' ')) /* Ignore spaces */
			buf++;
	}
	argv[argc] = NULL;

	if (argc == 0)  /* Ignore blank line */
		return 1;

	/* Should the job run in the background? */
	if ((bg = (*argv[argc-1] == '&')) != 0)
		argv[--argc] = NULL;

	return bg;
}
/* $end parseline */



void sigint_handler(int signo) {  //signal handler for sigint
	printf("\nYou are in my custom shell. See you again. Bye!\n");
	exit(0);
}