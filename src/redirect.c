// header files
#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Global variables to save the original file descriptors of stdout and stdin
int saved_stdout = -1;
int saved_stdin = -1;

//Function to handle input and output redirection
void redirect_io(char *input_file, char *output_file, bool append) {

    if (input_file != NULL) { //if file is not null
        int fd = open(input_file, O_RDONLY); //open in read only
        if (fd < 0) { //check for errors
            perror("open input");
            exit(EXIT_FAILURE);
        }
        saved_stdin = dup(STDIN_FILENO); // Save current stdin
        dup2(fd, STDIN_FILENO); // Redirect stdin to opened file
        close(fd); //close file descriptor
    }

    // Handle output redirection
    if (output_file != NULL) {
        saved_stdout = dup(STDOUT_FILENO); // Save current stdout

        // Open the file for writing with appropriate flags
        int flags = O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC);
        int fd = open(output_file, flags, 0644);
        if (fd < 0) { //check if file has no errors
            perror("open output");
            exit(EXIT_FAILURE);
        }

        dup2(fd, STDOUT_FILENO); // Redirect stdout to file
        close(fd); //close file descriptor
    }
}

void restore_io() {
    // Restore stdin if it was redirected
    if (saved_stdin != -1) { // Check if stdin was redirected
        dup2(saved_stdin, STDIN_FILENO); //restore original stdin from saved descriptor
        close(saved_stdin); //close saved descriptor to free resources
        saved_stdin = -1; // restore stdin
    }

    if (saved_stdout != -1) { //check if stdout was redirected
        dup2(saved_stdout, STDOUT_FILENO); //restore original stdout from saved descriptor
        close(saved_stdout); //close the saved descriptor to free resources 
        saved_stdout = -1; // reset saved_stdout to indicate restoration
    }
}
