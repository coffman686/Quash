#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024
#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    int job_id;
    char command[MAX_INPUT];
} Job;

static Job jobs[MAX_JOBS]; // Array to keep track of background jobs
static int job_count = 0;   // Counter for background jobs

int main() {
    char input[MAX_INPUT];
    printf("Welcome to Quash!\n");

    while (true) {
        handle_background_jobs(); // Check for completed background jobs
        printf("[QUASH]$ ");
        if (fgets(input, MAX_INPUT, stdin) == NULL) {
            perror("fgets");
            continue;
        }

        // Remove newline character from input
        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;  // Skip processing and prompt again
        }

        // If the command is "exit" or "quit"
        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
            break;
        }

        // Execute the command
        execute_command(input);
    }

    return 0;
}

// Function to execute commands
void execute_command(char *command) {
    char *pipe_commands[MAX_INPUT];
    int pipe_count = 0;

    // Split by pipe
    char *token = strtok(command, "|");
    while (token != NULL) {
        pipe_commands[pipe_count++] = token;
        token = strtok(NULL, "|");
    }
    pipe_commands[pipe_count] = NULL;

    if (pipe_count > 1) {
        execute_piped_commands(pipe_commands, pipe_count);
    } else {
        // Handle non-piped command
        execute_single_command(command);
    }
}

// Function to execute piped commands
void execute_piped_commands(char **pipe_commands, int pipe_count) {
    int pipefd[2];
    pid_t pid;
    int fd_in = 0;  // Initial input is from stdin

    for (int i = 0; i < pipe_count; i++) {
        pipe(pipefd);  // Create a pipe

        if ((pid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Child process

            // If this is not the first command, get input from the previous pipe
            if (fd_in != 0) {
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            // If this is not the last command, pipe output to the next command
            if (i < pipe_count - 1) {
                dup2(pipefd[1], STDOUT_FILENO);
            }

            // Close pipe read end
            close(pipefd[0]);
            close(pipefd[1]);

            // Execute the command
            execute_single_command(pipe_commands[i]);
            exit(EXIT_FAILURE);
        } else {
            // Parent process
            close(pipefd[1]); // Close write end in the parent
            fd_in = pipefd[0]; // Save the read end for the next command
        }
    }

    // Wait for all children to complete
    for (int i = 0; i < pipe_count; i++) {
        wait(NULL);
    }
}

// Function to execute a single command (built-in or external)
void execute_single_command(char *command) {
    char *args[MAX_INPUT];
    char *token = strtok(command, " ");
    int i = 0;
    int background = 0; // Flag for background execution

    // Tokenize input and check for background execution
    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Check for background execution
    if (i > 0 && strcmp(args[i - 1], "&") == 0) {
        background = 1;
        args[i - 1] = NULL; // Remove '&' from args
    }

    // Check if it's a built-in command
    if (strcmp(args[0], "cd") == 0) {
        quash_cd(args);
    } else if (strcmp(args[0], "pwd") == 0) {
        quash_pwd();
    } else if (strcmp(args[0], "echo") == 0) {
        quash_echo(args);
    } else if (strcmp(args[0], "export") == 0) {
        quash_export(args);
    } else {
        // External command handling
        pid_t pid = fork();
        if (pid == 0) {
            // Child process
            execvp(args[0], args);
            perror("execvp"); // Exec only returns on error
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            // Parent process
            if (background) {
                // Store job details
                jobs[job_count].pid = pid;
                jobs[job_count].job_id = job_count + 1; // Job IDs start from 1
                strncpy(jobs[job_count].command, command, MAX_INPUT);
                printf("Background job started: [%d] %d %s\n", jobs[job_count].job_id, pid, command);
                job_count++;
            } else {
                // Wait for the foreground job to complete
                waitpid(pid, NULL, 0);
            }
        } else {
            // Fork error
            perror("fork");
        }
    }
}
