#ifndef COMMAND_H
#define COMMAND_H

#define MAX_ENV_VARS 100
#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 64

typedef struct {
    char **env_vars;
    int env_count;
} ShellState;

void change_directory(char *path);
void set_var(ShellState *shell, char *var, char *value);
void unset_var(ShellState *shell, char *var);
void expand_variables(ShellState *shell, char *command_line, char *result);
void handle_external_command(ShellState *shell, char **args, char *redirect_in, char *redirect_out, int run_in_background);
void parse_command(char *input, char **args, char **redirect_in, char **redirect_out, int *run_in_background);

#endif