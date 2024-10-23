#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>


int quash_cd(char **args) {
    char *path = args[1];

    if (path == NULL) {
        // If no path is provided, default to the HOME directory
        path = getenv("HOME");
    } else if (path[0] == '$') {
        // If the path starts with '$', treat it as an environment variable
        char *env_var = path + 1;  // Skip the '$' to get the actual environment variable name
        char *env_value = getenv(env_var);  // Fetch the value of the environment variable

        if (env_value != NULL) {
            path = env_value;  // Use the value of the environment variable as the path
        } else {
            fprintf(stderr, "quash: %s: No such environment variable\n", path);
            return 1;
        }
    }

    // Change directory to the resolved path
    if (chdir(path) != 0) {
        perror("quash");
    } else {
        // Update the PWD environment variable with the actual current working directory
        char cwd[1024];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            setenv("PWD", cwd, 1);  // Set PWD to the actual directory
        }
    }

    return 1;
}



int quash_pwd() {
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);  // Print the actual current working directory
    } else {
        perror("getcwd");
    }
    return 1;
}


int quash_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        if (args[i][0] == '$') {
            // If the argument starts with '$', treat it as an environment variable
            char *env_var = args[i] + 1;  // Get the environment variable name (skip the '$')
            char *env_value = getenv(env_var);  // Fetch the value of the environment variable

            if (env_value != NULL) {
                printf("%s ", env_value);  // Print the value of the environment variable
            } else {
                printf("%s ", args[i]);  // If the variable doesn't exist, print the original argument
            }
        } else {
            printf("%s ", args[i]);  // If it's not an environment variable, just print the argument
        }
    }
    printf("\n");
    return 1;
}


int quash_export(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "quash: expected argument to \"export\"\n");
    } else {
        char *var = strtok(args[1], "=");  // Get the variable name (e.g., "PATH")
        char *value = strtok(NULL, "=");   // Get the value (e.g., "$HOME")

        // If the value starts with a '$', treat it as an environment variable
        if (value != NULL && value[0] == '$') {
            char *env_var = value + 1;  // Skip the '$' to get the actual environment variable name
            char *env_value = getenv(env_var);  // Fetch the value of the environment variable

            if (env_value != NULL) {
                value = env_value;  // Replace the value with the expanded environment variable's value
            }
        }

        // Set the environment variable using setenv()
        if (setenv(var, value, 1) != 0) {
            perror("export");
        }
    }
    return 1;
}
