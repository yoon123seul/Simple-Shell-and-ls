#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>

void eval(char *cmdline);

int main() {
    char cmdline[1024];

    while (1) {
        printf("$ ");
        fgets(cmdline, sizeof(cmdline), stdin);
        if (feof(stdin))
            exit(0);
        eval(cmdline);
    }

    return 0;
}

void eval(char *cmdline) {
    char *argv[1024];
    char buf[1024];
    int bg = 0;
    int index = 0;

    // Parsing the command line
    strcpy(buf, cmdline);
    buf[strlen(buf)-1] = ' ';
    while (*buf && (*buf == ' '))
        buf++;

    // Build the argv list
    while ((argv[index] = strtok(buf, " ")) != NULL) {
        index++;
        buf = NULL;
    }

    if (index == 0)
        return;

    // Check for background process
    if (strcmp(argv[index-1], "&") == 0) {
        bg = 1;
        argv[index-1] = NULL;
    }

    // Check for redirection and pipe
    int output_index = -1;
    int input_index = -1;
    int append_index = -1;

    for (int i = 0; i < index; i++) {
        if (strcmp(argv[i], ">") == 0) {
            output_index = i;
        } else if (strcmp(argv[i], ">>") == 0) {
            append_index = i;
        } else if (strcmp(argv[i], "<") == 0) {
            input_index = i;
        } else if (strcmp(argv[i], "|") == 0) {
            argv[i] = NULL;

            int pipe_fd[2];
            pid_t pid;

            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                exit(EXIT_FAILURE);
            }

            if ((pid = fork()) == 0) { // Child process
                close(pipe_fd[0]);	
                dup2(pipe_fd[1], STDOUT_FILENO);
                close(pipe_fd[1]);
                execvp(argv[0], argv);
                perror("execvp");
                exit(EXIT_FAILURE);
            } else if (pid > 0) { // Parent process
                close(pipe_fd[1]);
                dup2(pipe_fd[0], STDIN_FILENO);
                close(pipe_fd[0]);
                waitpid(pid, NULL, 0);
            } else {
                perror("fork");
                exit(EXIT_FAILURE);
            }
        }
    }

    // Execute the command
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (output_index != -1) {
            int output_fd = open(argv[output_index + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            dup2(output_fd, STDOUT_FILENO);
            close(output_fd);
        } else if (append_index != -1) {
            int append_fd = open(argv[append_index + 1], O_WRONLY | O_CREAT | O_APPEND, 0644);
            dup2(append_fd, STDOUT_FILENO);
            close(append_fd);
        } else if (input_index != -1) {
            int input_fd = open(argv[input_index + 1], O_RDONLY, 0644);
            dup2(input_fd, STDIN_FILENO);
            close(input_fd);
        }

        execvp(argv[0], argv);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        if (!bg) {
            waitpid(pid, NULL, 0);
        }
    } else {
        // Fork failed
        perror("fork");
        exit(EXIT_FAILURE);
    }
}
