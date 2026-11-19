
#include "shell.h"

char* history[HISTORY_SIZE];
int history_count = 0;

void add_to_history(const char* command) {

    if (history_count == HISTORY_SIZE) {

        free(history[0]);

        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }

        history_count--;
    }

    history[history_count] = strdup(command);

    if (history[history_count] != NULL) {
        history_count++;
    }
}

int expand_history(char** cmdline) {

    if ((*cmdline)[0] != '!') {
        return 1;
    }

    char* endptr;
    long n = strtol(*cmdline + 1, &endptr, 10);

    if ((*cmdline)[1] == '\0' || *endptr != '\0' ||
        n < 1 || n > history_count) {

        fprintf(stderr, "Invalid history number!\n");
        return 0;
    }

    char* expanded = strdup(history[n - 1]);

    if (expanded == NULL) {
        perror("strdup");
        return 0;
    }

    free(*cmdline);
    *cmdline = expanded;

    return 1;
}

void execute_line(char* line) {

    char* current = line;

    while (current != NULL) {

        char* separator = strchr(current, ';');

        if (separator != NULL) {
            *separator = '\0';
        }

        while (*current == ' ' || *current == '\t') {
            current++;
        }

        int len = strlen(current);

        while (len > 0 &&
               (current[len - 1] == ' ' ||
                current[len - 1] == '\t')) {
            current[--len] = '\0';
        }

        if (*current != '\0') {

            // Feature 8: Handle variable assignment
            if (handle_variable_assignment(current)) {

                if (separator == NULL) {
                    break;
                }

                current = separator + 1;
                continue;
            }

            char* command_copy = strdup(current);

            if (command_copy == NULL) {
                perror("strdup");
                return;
            }

            char** arglist = tokenize(current);

            if (arglist != NULL) {

                // Feature 8: Expand shell variables
                expand_variables(arglist);

                int count = 0;

                while (arglist[count] != NULL) {
                    count++;
                }

                int background = 0;

                if (count > 0 &&
                    strcmp(arglist[count - 1], "&") == 0) {

                    background = 1;
                    free(arglist[count - 1]);
                    arglist[count - 1] = NULL;
                }

                if (arglist[0] != NULL) {

                    if (!handle_builtin(arglist)) {
                        execute(arglist, background, command_copy);
                    }
                }

                for (int i = 0; arglist[i] != NULL; i++) {
                    free(arglist[i]);
                }

                free(arglist);
            }

            free(command_copy);
        }

        if (separator == NULL) {
            break;
        }

        current = separator + 1;
    }
}

int main() {

    char* cmdline;

    while (1) {

        check_background_jobs();

        cmdline = readline(PROMPT);

        if (cmdline == NULL) {
            break;
        }

        if (!expand_history(&cmdline)) {
            free(cmdline);
            continue;
        }

        char* p = cmdline;

        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p != '\0') {
            add_to_history(cmdline);
            add_history(cmdline);
        }

        execute_line(cmdline);

        free(cmdline);
    }

    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    printf("\nShell exited.\n");

    return 0;
}
