#include "quash.h"
#include "jobs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

Job jobs[MAX_JOBS]; // create array for jobs to be stored in
int job_count = 0;  // create count for jobs

int main() { // main loop to start quash
    char input[MAX_INPUT]; // init array for user input
    printf("Welcome to Quash!\n"); 

    while (1) { // this loop keeps quash running after commands are executed
        handle_background_jobs(); // checks if jobs are done
        printf("[QUASH]$ "); 
        if (fgets(input, MAX_INPUT, stdin) == NULL) { // get input from user
            perror("fgets");
            continue;
        }
        input[strcspn(input, "\n")] = '\0'; // format user input

        if (strlen(input) == 0) { // if no command is entered, just re-run quash loop
            continue;
        }

        if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) { // kill loop on exit or quit
            break;
        }

        execute_command(input); // execute given user command
    }

    return 0;
}

void execute_command(char *command) { // func to decide if command is singluar or has pipes
    char *pipe_commands[MAX_INPUT];
    int pipe_count = 0;

    char *token = strtok(command, "|"); // if command has a pipe seperate those commands
    while (token != NULL) {
        pipe_commands[pipe_count++] = token;
        token = strtok(NULL, "|");
    }
    pipe_commands[pipe_count] = NULL; // null terminate the command list

    if (pipe_count > 1) {
        execute_piped_commands(pipe_commands, pipe_count); // if pipes are present, go to pipe execute
    } else {
        execute_single_command(command); // if pipes are not present, just execute the single command
    }
}

void execute_piped_commands(char **pipe_commands, int pipe_count) { // function to execute command if pipes are present
    int pipefd[2]; // create pipe fd
    pid_t pid; // init process id
    int fd_in = 0;  

    for (int i = 0; i < pipe_count; i++) { // iterate for every pipe
        pipe(pipefd); // create a pipe so commands can communicate

        if ((pid = fork()) == -1) { // create child processess to run all commands between pipes
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) { // if child process

            if (fd_in != 0) { // if this command is not the first, read from previous pipe
                dup2(fd_in, STDIN_FILENO);
                close(fd_in);
            }

            if (i < pipe_count - 1) { // if this is not the last command, pipe to next command
                dup2(pipefd[1], STDOUT_FILENO);
            }


            close(pipefd[0]); //close pipe ends
            close(pipefd[1]);

            execute_single_command(pipe_commands[i]); // execute each command
            exit(EXIT_FAILURE); // end child process
        } else { // for parent process
            close(pipefd[1]); // close write end of pipe
            fd_in = pipefd[0]; // save read for next command
        }
    }

    for (int i = 0; i < pipe_count; i++) { // wait for all child process to finish
        wait(NULL);
    }
}

void execute_single_command(char *command) { // function to execute all singular commands
    char *args[MAX_INPUT]; // array to hold arguments
    char *input_file = NULL; // for redirection
    char *output_file = NULL; // redirection
    int append = 0; // for redirection if we need to redirect output
    int background = 0; // background process
    
    char command_copy[MAX_INPUT]; // create copy of command for job recognition
    strncpy(command_copy, command, MAX_INPUT); 
    
    // this block of code parses the command into each command and argument and 
    // determines if any redirection or background job command are involved
    char *token = strtok(command, " ");
    int i = 0;
    while (token != NULL) {
        if (strcmp(token, ">") == 0) {
            output_file = strtok(NULL, " ");
        } else if (strcmp(token, ">>") == 0) {
            output_file = strtok(NULL, " ");
            append = 1; // flag that output should be appended
        } else if (strcmp(token, "<") == 0) {
            input_file = strtok(NULL, " ");
        } else if (strcmp(token, "&") == 0) {
            background = 1; // flag that job should run in background
        } else {
            args[i++] = token; 
        }
        token = strtok(NULL, " ");
    }
    args[i] = NULL; // null terminate array

    
    if (input_file != NULL || output_file != NULL) { // if redirection is present, handle it in redirect.io function
        redirect_io(input_file, output_file, append);
    }

    // this block checks to see if commands are build in, if so, the function for that specific
    // build in should be called and handled there
    if (strcmp(args[0], "cd") == 0) {
        quash_cd(args);
    } else if (strcmp(args[0], "pwd") == 0) {
        quash_pwd();
    } else if (strcmp(args[0], "echo") == 0) {
        quash_echo(args);
    } else if (strcmp(args[0], "export") == 0) {
        quash_export(args);
    } else if (strcmp(args[0], "jobs") == 0) {
        list_jobs(); // if jobs is ran, list all current running jobs
    } else { // this else is for all commands that are not built in
        pid_t pid = fork(); // create child process to handle command
        if (pid == 0) { // if child process
            if (execvp(args[0], args) == -1) { // call execvp to handle command
                fprintf(stderr, "command not found: %s\n", args[0]);
                exit(EXIT_FAILURE); 
            }
        } else if (pid > 0) { // parent process
            if (background) { // if background job
                // stores jobs running and prints background job
                jobs[job_count].pid = pid;
                jobs[job_count].job_id = job_count + 1; 
                strncpy(jobs[job_count].command, command_copy, MAX_INPUT);
                printf("Background job started: [%d] %d %s\n", jobs[job_count].job_id, pid, command_copy);
                job_count++;
            } else { // if not background, wait for all process to finish
                waitpid(pid, NULL, 0);
            }
        } else {
            perror("fork");
        }
    }

    restore_io(); // call restore_io to get the originial IO after command is done
}