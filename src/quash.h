#ifndef QUASH_H
#define QUASH_H

#include <stdbool.h>

// Function declarations
void execute_command(char *command);
void execute_single_command(char *command);
void execute_piped_commands(char **pipe_commands, int pipe_count);
void run_builtin(char **args);
void handle_jobs();
void redirect_io(char *input_file, char *output_file, bool append);
void handle_pipes(char **commands);
void export_variable(char *env_var);
void change_directory(char *path);
void print_working_directory();
void handle_background_jobs();

// Built-in commands
int quash_echo(char **args);
int quash_export(char **args);
int quash_cd(char **args);
int quash_pwd();
int quash_jobs();

#endif
