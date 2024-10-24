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
    char *input_file = NULL;
    char *output_file = NULL;
    int append = 0; // Flag to indicate if we should append to the output file
    int background = 0;
    
    // Create a copy of the original command for background job tracking
    char command_copy[MAX_INPUT];
    strncpy(command_copy, command, MAX_INPUT);

    // Tokenize the command string, checking for redirection operators
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            output_file = strtok(NULL, " ");
        } else if (strcmp(token, ">>") == 0) {
            output_file = strtok(NULL, " ");
            append = 1;
        } else if (strcmp(token, "<") == 0) {
            input_file = strtok(NULL, " ");
        } else if (strcmp(token, "&") == 0) {
            background = 1; // Run in background
        } else {
            args[i++] = token; // Normal command/argument
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Handle redirection if needed
    if (input_file != NULL || output_file != NULL) {
        redirect_io(input_file, output_file, append);
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
            if (execvp(args[0], args) == -1) {
                // If execvp fails, print custom error message
                fprintf(stderr, "command not found: %s\n", args[0]);
                exit(EXIT_FAILURE); // Ensure the child process terminates
            }
        } else if (pid > 0) {
            if (background) {
                // Store job details for background jobs
                jobs[job_count].pid = pid;
                jobs[job_count].job_id = job_count + 1; 
                strncpy(jobs[job_count].command, command_copy, MAX_INPUT);
                printf("Background job started: [%d] %d %s\n", jobs[job_count].job_id, pid, command_copy);
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

    // Restore original I/O after the command finishes
    restore_io();
}



