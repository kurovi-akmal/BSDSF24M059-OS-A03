
#include "shell.h"

int handle_builtin(char** arglist) {

    // EXIT command
    if (strcmp(arglist[0], "exit") == 0) {
        printf("Exiting shell...\n");
        exit(0);
    }

    // CD command
    else if (strcmp(arglist[0], "cd") == 0) {

        if (arglist[1] == NULL) {
            fprintf(stderr, "Usage: cd <directory>\n");
        }
        else if (chdir(arglist[1]) != 0) {
            perror("cd");
        }

        return 1;
    }

    // HELP command
    else if (strcmp(arglist[0], "help") == 0) {
        printf("Built-in commands:\n");
        printf("  cd <directory> - Change directory\n");
        printf("  exit           - Exit the shell\n");
        printf("  help           - Display help\n");
        printf("  jobs           - Display jobs status\n");

        return 1;
    }

    // JOBS command
    else if (strcmp(arglist[0], "jobs") == 0) {
        printf("Job control not yet implemented.\n");
        return 1;
    }

    // Not a built-in command
    return 0;
}
