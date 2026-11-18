
#include "shell.h"

#define MAX_BLOCK_SIZE 4096

/* Function declarations */
void execute_line(char* line);
char* trim_spaces(char* str);
void read_if_block_with_condition(const char* condition);

/* Command history */
char* history[HISTORY_SIZE];
int history_count = 0;


/* Add command to history */
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


/* Expand !n history command */
int expand_history(char** cmdline) {

    if ((*cmdline)[0] != '!') {
        return 1;
    }

    char* endptr;
    long n = strtol(*cmdline + 1, &endptr, 10);

    if ((*cmdline)[1] == '\0' ||
        *endptr != '\0' ||
        n < 1 ||
        n > history_count) {

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


/* Execute the condition of an if statement */
int execute_condition(char* command) {

    char* p = command;

    while (*p == ' ' || *p == '\t') {
        p++;
    }

    char** args = tokenize(p);

    if (args == NULL) {
        return 1;
    }

    /* Expand variables in condition */
    expand_variables(args);

    int status = execute(args, 0, command);

    for (int i = 0; args[i] != NULL; i++) {
        free(args[i]);
    }

    free(args);

    return status;
}


/* Execute then or else block */
void execute_if_block(char* condition,
                      char* then_block,
                      char* else_block,
                      int has_else) {

    int status = execute_condition(condition);

    if (status == 0) {
        execute_line(then_block);
    }
    else if (has_else) {
        execute_line(else_block);
    }
}


/* Remove leading and trailing spaces */
char* trim_spaces(char* str) {

    while (*str == ' ' || *str == '\t') {
        str++;
    }

    int len = strlen(str);

    while (len > 0 &&
           (str[len - 1] == ' ' ||
            str[len - 1] == '\t' ||
            str[len - 1] == '\n')) {

        str[--len] = '\0';
    }

    return str;
}


/* Read multiline if-then-else-fi block */
void read_if_block_with_condition(const char* condition) {

    char* then_block = calloc(MAX_BLOCK_SIZE, 1);
    char* else_block = calloc(MAX_BLOCK_SIZE, 1);

    if (then_block == NULL || else_block == NULL) {

        perror("Memory allocation");

        free(then_block);
        free(else_block);

        return;
    }

    int found_then = 0;
    int found_else = 0;
    int found_fi = 0;
    int error = 0;

    while (1) {

        char* line = readline("...> ");

        if (line == NULL) {
            error = 1;
            break;
        }

        char* command = trim_spaces(line);

        if (strcmp(command, "then") == 0) {

            if (found_then) {
                fprintf(stderr, "Unexpected then\n");
                error = 1;
                free(line);
                break;
            }

            found_then = 1;

            free(line);
            continue;
        }

        if (strcmp(command, "else") == 0) {

            if (!found_then || found_else) {
                fprintf(stderr, "Unexpected else\n");
                error = 1;
                free(line);
                break;
            }

            found_else = 1;

            free(line);
            continue;
        }

        if (strcmp(command, "fi") == 0) {

            found_fi = 1;

            free(line);
            break;
        }

        if (!found_then) {

            fprintf(stderr, "Expected then\n");

            error = 1;
            free(line);
            break;
        }

        char* target = found_else ? else_block : then_block;

        size_t used = strlen(target);
        size_t length = strlen(command);

        if (used + length + 2 > MAX_BLOCK_SIZE) {

            fprintf(stderr, "If block is too large\n");

            error = 1;
            free(line);
            break;
        }

        strcat(target, command);
        strcat(target, ";");

        free(line);
    }

    if (!found_then || !found_fi) {

        fprintf(stderr, "Incomplete if block\n");

        error = 1;
    }

    if (!error) {

        execute_if_block(
            (char*)condition,
            then_block,
            else_block,
            found_else
        );
    }

    free(then_block);
    free(else_block);
}


/* Execute commands and handle Feature 8 variables */
void execute_line(char* line) {

    char* current = line;

    while (current != NULL) {

        char* separator = strchr(current, ';');

        if (separator != NULL) {
            *separator = '\0';
        }

        current = trim_spaces(current);

        if (*current != '\0') {

            char* command_copy = strdup(current);

            if (command_copy == NULL) {
                perror("strdup");
                return;
            }

            /*
             * Feature 8:
             * Check variable assignment before tokenization.
             */

            if (handle_variable_assignment(current)) {

                free(command_copy);
            }

            else {

                /* Tokenize command */
                char** arglist = tokenize(current);

                if (arglist != NULL) {

                    /* Expand variables */
                    expand_variables(arglist);

                    int count = 0;

                    while (arglist[count] != NULL) {
                        count++;
                    }

                    /* Detect background execution */
                    int background = 0;

                    if (count > 0 &&
                        strcmp(arglist[count - 1], "&") == 0) {

                        background = 1;

                        free(arglist[count - 1]);

                        arglist[count - 1] = NULL;
                    }

                    /* Execute built-in or external command */
                    if (arglist[0] != NULL) {

                        if (!handle_builtin(arglist)) {

                            execute(
                                arglist,
                                background,
                                command_copy
                            );
                        }
                    }

                    /* Free arguments */
                    for (int i = 0; arglist[i] != NULL; i++) {
                        free(arglist[i]);
                    }

                    free(arglist);
                }

                free(command_copy);
            }
        }

        if (separator == NULL) {
            break;
        }

        current = separator + 1;
    }
}


/* Main shell loop */
int main() {

    char* cmdline;

    while (1) {

        /* Check completed background jobs */
        check_background_jobs();

        /* Read command */
        cmdline = readline(PROMPT);

        if (cmdline == NULL) {
            break;
        }

        /* Expand history */
        if (!expand_history(&cmdline)) {

            free(cmdline);
            continue;
        }

        /* Store non-empty command */
        char* p = cmdline;

        while (*p == ' ' || *p == '\t') {
            p++;
        }

        if (*p != '\0') {

            add_to_history(cmdline);

            add_history(cmdline);
        }

        /* Handle if-then-else-fi */
        char* command = trim_spaces(cmdline);

        if (strcmp(command, "if") == 0 ||
            strncmp(command, "if ", 3) == 0 ||
            strncmp(command, "if\t", 3) == 0) {

            char* condition = trim_spaces(command + 2);

            if (*condition != '\0') {

                read_if_block_with_condition(condition);
            }
            else {

                fprintf(stderr, "Missing condition\n");
            }
        }

        else {

            execute_line(cmdline);
        }

        free(cmdline);
    }

    /* Free history memory */
    for (int i = 0; i < history_count; i++) {
        free(history[i]);
    }

    printf("\nShell exited.\n");

    return 0;
}
