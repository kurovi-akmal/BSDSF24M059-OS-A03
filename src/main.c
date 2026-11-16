#include "shell.h"

char* history[HISTORY_SIZE];
int history_count = 0;


void add_to_history(const char* command) {

    // If history is full, remove the oldest command
    if (history_count == HISTORY_SIZE) {

        free(history[0]);

        for (int i = 1; i < HISTORY_SIZE; i++) {
            history[i - 1] = history[i];
        }

        history_count--;
    }

    // Store a copy of the new command
    history[history_count] = strdup(command);

    if (history[history_count] != NULL) {
        history_count++;
    }
}


int expand_history(char** cmdline) {

    // Normal command: nothing to expand
    if ((*cmdline)[0] != '!') {
        return 1;
    }

    // Extract the history number
    char* endptr;
    long n = strtol(*cmdline + 1, &endptr, 10);

    // Validate the number
    if ((*cmdline)[1] == '\0' || *endptr != '\0' ||
        n < 1 || n > history_count) {

        fprintf(stderr, "Invalid history number!\n");
        return 0;
    }

    // Copy the selected command
    char* expanded = strdup(history[n - 1]);

    if (expanded == NULL) {
        perror("strdup");
        return 0;
    }

    // Replace !n with the actual command
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

        // Trim leading spaces
        while (*current == ' ' || *current == '\t') {
            current++;
        }

        // Trim trailing spaces
        int len = strlen(current);

        while (len > 0 &&
               (current[len - 1] == ' ' ||
                current[len - 1] == '\t')) {
            current[--len] = '\0';
        }

        if (*current != '\0') {

            // Preserve original command for job list
            char* command_copy = strdup(current);

            if (command_copy == NULL) {
                perror("strdup");
                return;
            }

            char** arglist = tokenize(current);

            if (arglist != NULL) {

                int count = 0;

                while (arglist[count] != NULL) {
                    count++;
                }

                // Detect & at the end
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

                // Free arguments
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
    char** arglist;

    while (1) {
        check_background_jobs();
        cmdline = readline(PROMPT);

        if (cmdline == NULL) {
            break;
        }

        // Step 1: Expand !n before storing
        if (!expand_history(&cmdline)) {
            free(cmdline);
            continue;
        }

        // Step 2: Store non-empty commands
        char* p = cmdline;

        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p != '\0') {
            add_to_history(cmdline);
            add_history(cmdline);
        }

        // Step 3: Tokenize
        execute_line(cmdline);

        free(cmdline);
    }

    // Free history memory
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    printf("\nShell exited.\n");
    return 0;
}
