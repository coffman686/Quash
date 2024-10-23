#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int quash_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "quash: expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("quash");
        }
    }
    return 1;
}

int quash_pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);
    } else {
        perror("getcwd");
    }
    return 1;
}

int quash_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        printf("%s ", args[i]);
    }
    printf("\n");
    return 1;
}

int quash_export(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "quash: expected argument to \"export\"\n");
    } else {
        char *var = strtok(args[1], "=");
        char *value = strtok(NULL, "=");
        if (setenv(var, value, 1) != 0) {
            perror("export");
        }
    }
    return 1;
}
