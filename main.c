#include "command.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main() {
    ShellState shell;
    shell.env_vars = malloc(MAX_ENV_VARS * sizeof(char *));
    shell.env_count = 0;

    char input[MAX_INPUT_SIZE];
    char expanded_input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];
    char *redirect_in, *redirect_out;
    int run_in_background;

    while (1) {
        printf("xsh# ");
        if (!fgets(input, sizeof(input), stdin)) {
            break;
        }

        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            break;
        }

        expanded_input[0] = '\0';
        expand_variables(&shell, input, expanded_input);


        parse_command(expanded_input, args, &redirect_in, &redirect_out, &run_in_background);

 
        if (args[0] == NULL) {
            continue;
        } else if (strcmp(args[0], "cd") == 0) {
            if (args[1] != NULL) {
                change_directory(args[1]);
            } else {
                fprintf(stderr, "cd: missing argument\n");
            }
        } else if (strcmp(args[0], "pwd") == 0) {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s\n", cwd);
            } else {
                perror("pwd");
            }
        } else if (strcmp(args[0], "set") == 0) {
            if (args[1] != NULL && args[2] != NULL) {
                set_var(&shell, args[1], args[2]);
            } else {
                fprintf(stderr, "set: missing arguments\n");
            }
        } else if (strcmp(args[0], "unset") == 0) {
            if (args[1] != NULL) {
                unset_var(&shell, args[1]);
            } else {
                fprintf(stderr, "unset: missing argument\n");
            }
        } else {
            handle_external_command(&shell, args, redirect_in, redirect_out, run_in_background);
        }
    }

    for (int ix = 0; ix < shell.env_count; ix++) {
        free(shell.env_vars[ix]);
    }
    free(shell.env_vars);

    return 0;
}