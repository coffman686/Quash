#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

void redirect_io(char *input_file, char *output_file, bool append) {
    if (input_file != NULL) {
        int fd = open(input_file, O_RDONLY);
        if (fd < 0) {
            perror("open input");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDIN_FILENO);
        close(fd);
    }

    if (output_file != NULL) {
        int fd = open(output_file, append ? O_APPEND | O_WRONLY : O_WRONLY | O_TRUNC, 0644);
        if (fd < 0) {
            perror("open output");
            exit(EXIT_FAILURE);
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }
}
