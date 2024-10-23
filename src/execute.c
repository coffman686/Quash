#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_INPUT 1024

int main() {
    char input[MAX_INPUT];
    printf("Welcome to Quash!\n");

    while (true) {
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

void execute_command(char *command) {
    // Example to parse command and execute built-in or external command
    char *args[MAX_INPUT];
    char *token = strtok(command, " ");
    int i = 0;

    while (token != NULL) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    // Check for built-ins (like cd, pwd, etc.)
    if (strcmp(args[0], "cd") == 0) {
        quash_cd(args);
    } else if (strcmp(args[0], "pwd") == 0) {
        quash_pwd();
    } else if (strcmp(args[0], "echo") == 0) {
        quash_echo(args);
    } else if (strcmp(args[0], "export") == 0) {
        quash_export(args);
    } else {
        // External command handling (e.g., ls, grep)
        pid_t pid = fork();
        if (pid == 0) {
            execvp(args[0], args);
            perror("execvp");
            exit(EXIT_FAILURE);
        } else {
            waitpid(pid, NULL, 0);
        }
    }
}
