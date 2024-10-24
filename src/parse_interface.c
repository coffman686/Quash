#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

int saved_stdout = -1;
int saved_stdin = -1;

void redirect_io(char *input_file, char *output_file, bool append) {
    // Handle input redirection
    if (input_file != NULL) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
            perror("open input");
            exit(EXIT_FAILURE);
        }
        saved_stdin = dup(STDIN_FILENO); // Save current stdin
        dup2(fd, STDIN_FILENO); // Redirect stdin to file
        close(fd);
    }

    // Handle output redirection
    if (output_file != NULL) {
        saved_stdout = dup(STDOUT_FILENO); // Save current stdout

        // Open the file for writing (truncate or append mode)
        int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
        int fd = open(output_file, flags, 0644);
        if (fd < 0) {
            perror("open output");
            exit(EXIT_FAILURE);
        }

        dup2(fd, STDOUT_FILENO); // Redirect stdout to file
        close(fd);
    }
}

void restore_io() {
    // Restore stdin if it was redirected
    if (saved_stdin != -1) {
        dup2(saved_stdin, STDIN_FILENO);
        close(saved_stdin);
        saved_stdin = -1;
    }

    // Restore stdout if it was redirected
    if (saved_stdout != -1) {
        dup2(saved_stdout, STDOUT_FILENO);
        close(saved_stdout);
        saved_stdout = -1;
    }
}
