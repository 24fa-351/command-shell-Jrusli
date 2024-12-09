#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

void change_directory(char *path) {
    if (chdir(path) != 0) {
        perror("cd");
    }
}

void set_var(ShellState *shell, char *var, char *value) {
    char *entry = malloc(strlen(var) + strlen(value) + 2);
    sprintf(entry, "%s=%s", var, value);


    for (int ix = 0; ix < shell->env_count; ix++) {
        if (strncmp(shell->env_vars[ix], var, strlen(var)) == 0 && shell->env_vars[ix][strlen(var)] == '=') {
            free(shell->env_vars[ix]);
            shell->env_vars[ix] = entry;
            return;
        }
    }

    shell->env_vars = realloc(shell->env_vars, (shell->env_count + 1) * sizeof(char *));
    shell->env_vars[shell->env_count++] = entry;
}

void unset_var(ShellState *shell, char *var) {
    for (int ia = 0; ia < shell->env_count; ia++) {
        if (strncmp(shell->env_vars[ia], var, strlen(var)) == 0 && shell->env_vars[ia][strlen(var)] == '=') {
            free(shell->env_vars[ia]);
            for (int jx = ia; jx < shell->env_count - 1; jx++) {
                shell->env_vars[jx] = shell->env_vars[jx + 1];
            }
            shell->env_count--;
            shell->env_vars = realloc(shell->env_vars, shell->env_count * sizeof(char *));
            return;
        }
    }
}

void expand_variables(ShellState *shell, char *command_line, char *result) {
    char *start = command_line;
    char *dollar;

    while ((dollar = strchr(start, '$')) != NULL) {
        strncat(result, start, dollar - start);

        char *end = dollar + 1;
        while (isalnum(*end) || *end == '_') {
            end++;
        }

        size_t var_len = end - (dollar + 1);

        char var_name[var_len + 1];
        strncpy(var_name, dollar + 1, var_len);
        var_name[var_len] = '\0';

        char *value = NULL;
        for (int ik = 0; ik < shell->env_count; ik++) {
            if (strncmp(shell->env_vars[ik], var_name, var_len) == 0 && shell->env_vars[ik][var_len] == '=') {
                value = shell->env_vars[ik] + var_len + 1;
                break;
            }
        }

        if (value) {
            strcat(result, value);
        } 
        start = end;
    }
    strcat(result, start);
}

void handle_external_command(ShellState *shell, char **args, char *redirect_in, char *redirect_out, int run_in_background) {
    (void)shell; // Suppress unused parameter warning

    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        if (redirect_in) {
            int fd_in = open(redirect_in, O_RDONLY);
            if (fd_in < 0) {
                perror("input redirection");
                exit(1);
            }
            dup2(fd_in, STDIN_FILENO);
            close(fd_in);
        }

        if (redirect_out) {
            int fd_out = open(redirect_out, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("output redirection");
                exit(1);
            }
            dup2(fd_out, STDOUT_FILENO);
            close(fd_out);
        }

        execvp(args[0], args);
        perror("execvp");
        exit(1);
    } else if (pid < 0) {
        // Fork failed
        perror("fork");
    } else {
        // Parent process
        if (!run_in_background) {
            waitpid(pid, NULL, 0);
        }
    }
}

void parse_command(char *input, char **args, char **redirect_in, char **redirect_out, int *run_in_background) {
    *redirect_in = NULL;
    *redirect_out = NULL;
    *run_in_background = 0;

    char *token = strtok(input, " \t\n");
    int i = 0;

    while (token) {
        if (strcmp(token, "<") == 0) {
            token = strtok(NULL, " \t\n");
            *redirect_in = token;
        } else if (strcmp(token, ">") == 0) {
            token = strtok(NULL, " \t\n");
            *redirect_out = token;
        } else if (strcmp(token, "&") == 0) {
            *run_in_background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
}