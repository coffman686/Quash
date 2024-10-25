//header files
#include "quash.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

///function to change directory based on path or env variables
int quash_cd(char **args) {
    char *path = args[1]; //get path from arguments

    if (path == NULL) { 
        // if no path provided default to the home directory
        path = getenv("HOME");
    } else if (path[0] == '$') {
        // if the path starts with $ treat it as an env variable
        char *env_var = path + 1;  // skip $ to get actual env variable name
        char *env_value = getenv(env_var);  // get the value of the environment variable

        if (env_value != NULL) { //if env_value is not NULL
            path = env_value;  // use the value of the env variable as the path
        } else {
            fprintf(stderr, "quash: %s: No such environment variable\n", path); //print error message for null env variable
            return 1;
        }
    }

    // attempt to change directory to resolved path
    if (chdir(path) != 0) {
        perror("quash"); //print error if chdir fails
    } else {
        // Update the pwd env variable with the current working directory (cwd)
        char cwd[1024]; //buffer holds the cwd
        if (getcwd(cwd, sizeof(cwd)) != NULL) {  //if actual directory exists
            setenv("PWD", cwd, 1);  // Set PWD to the actual directory
        }
    }

    return 1;
}


//Function to print current working directory
int quash_pwd() {
    char cwd[1024]; //buffer to hold cwd
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s\n", cwd);  // print actual current working directory
    } else {
        perror("getcwd"); //else print error if getcwd fails
    }
    return 1;
}

// Function to print text or the value of environment variables
int quash_echo(char **args) {
    for (int i = 1; args[i] != NULL; i++) { //Loop through all arguments
        if (args[i][0] == '$') {
            // if the argument starts with $ treat it as the env variable
            char *env_var = args[i] + 1;  // get the env variable name
            char *env_value = getenv(env_var); //fetch the value of the env variable

            if (env_value != NULL) {
                printf("%s ", env_value);  // print the value of the environment variable if not null
            } else {
                printf("%s ", args[i]);  // if the variable doesn't exist print the original argument
            }
        } else {
            printf("%s ", args[i]);  // if it's not an env variable print argument
        }
    }
    printf("\n"); //for spacing
    return 1;
}

//Function for setting env variables 
int quash_export(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "quash: expected argument to \"export\"\n"); //print err if no argument is provided
    } else {
        char *var = strtok(args[1], "=");  // get variable name
        char *value = strtok(NULL, "=");   // get value 

        //if the value starts with $ treat it as an env variable
        if (value != NULL && value[0] == '$') {
            char *env_var = value + 1;  // skip $ to get the actual env variable name
            char *env_value = getenv(env_var);  // fetch the value of the env variable

            if (env_value != NULL) {
                value = env_value;  // replace the value with the expanded env variable's value
            }
        }

        // set the env variable using setenv()
        if (setenv(var, value, 1) != 0) {  // set the environment variable replacing if it already exists
            perror("export"); //print error if it fails
        }
    }
    return 1;
}
