#ifndef QUASH_H
#define QUASH_H

#include <stdbool.h>

//ffunction declarations
void execute_command(char *command);
void execute_single_command(char *command);
void execute_piped_commands(char **pipe_commands, int pipe_count);
void redirect_io(char *input_file, char *output_file, bool append);
void restore_io();
void handle_background_jobs();

//built-in commands
int quash_echo(char **args);
int quash_export(char **args);
int quash_cd(char **args);
int quash_pwd();

#endif
