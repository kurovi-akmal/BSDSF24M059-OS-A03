
#include "shell.h"

typedef struct ShellVariable {
    char *name;
    char *value;
    struct ShellVariable *next;
} ShellVariable;

static ShellVariable *variables = NULL;

// Validate variable name
int valid_variable_name(const char *name) {

    if (name == NULL || name[0] == '\0') {
        return 0;
    }

    if (!((name[0] >= 'A' && name[0] <= 'Z') ||
          (name[0] >= 'a' && name[0] <= 'z') ||
          name[0] == '_')) {
        return 0;
    }

    for (int i = 1; name[i] != '\0'; i++) {

        if (!((name[i] >= 'A' && name[i] <= 'Z') ||
              (name[i] >= 'a' && name[i] <= 'z') ||
              (name[i] >= '0' && name[i] <= '9') ||
              name[i] == '_')) {
            return 0;
        }
    }

    return 1;
}

// Assign or update variable
void assign_variable(const char *name, const char *value) {

    ShellVariable *current = variables;

    while (current != NULL) {

        if (strcmp(current->name, name) == 0) {

            char *new_value = strdup(value);

            if (new_value == NULL) {
                perror("strdup");
                return;
            }

            free(current->value);
            current->value = new_value;

            return;
        }

        current = current->next;
    }

    ShellVariable *new_var = malloc(sizeof(ShellVariable));

    if (new_var == NULL) {
        perror("malloc");
        return;
    }

    new_var->name = strdup(name);
    new_var->value = strdup(value);

    if (new_var->name == NULL || new_var->value == NULL) {

        perror("strdup");

        free(new_var->name);
        free(new_var->value);
        free(new_var);

        return;
    }

    new_var->next = variables;
    variables = new_var;
}

// Handle NAME=value assignment
int handle_variable_assignment(const char *command) {

    const char *equal = strchr(command, '=');

    if (equal == NULL) {
        return 0;
    }

    for (const char *p = command; p < equal; p++) {

        if (*p == ' ' || *p == '\t') {
            return 0;
        }
    }

    size_t name_length = equal - command;

    char *name = malloc(name_length + 1);

    if (name == NULL) {
        perror("malloc");
        return 1;
    }

    memcpy(name, command, name_length);
    name[name_length] = '\0';

    if (!valid_variable_name(name)) {

        fprintf(stderr, "Invalid variable name\n");

        free(name);
        return 1;
    }

    const char *value = equal + 1;
    size_t value_length = strlen(value);

    if (value_length >= 2 &&
        ((value[0] == '"' && value[value_length - 1] == '"') ||
         (value[0] == '\'' && value[value_length - 1] == '\''))) {

        char *clean_value = malloc(value_length - 1);

        if (clean_value == NULL) {
            perror("malloc");
            free(name);
            return 1;
        }

        memcpy(clean_value, value + 1, value_length - 2);
        clean_value[value_length - 2] = '\0';

        assign_variable(name, clean_value);

        free(clean_value);

    } else {

        assign_variable(name, value);
    }

    free(name);

    return 1;
}

// Expand $VARIABLE
void expand_variables(char **args) {

    for (int i = 0; args[i] != NULL; i++) {

        if (args[i][0] != '$') {
            continue;
        }

        const char *name = args[i] + 1;

        ShellVariable *current = variables;

        while (current != NULL) {

            if (strcmp(current->name, name) == 0) {

                char *new_value = strdup(current->value);

                if (new_value == NULL) {
                    perror("strdup");
                    break;
                }

                free(args[i]);
                args[i] = new_value;

                break;
            }

            current = current->next;
        }
    }
}

// Display all variables
void print_variables(void) {

    ShellVariable *current = variables;

    while (current != NULL) {

        printf("%s=%s\n",
               current->name,
               current->value);

        current = current->next;
    }
}
