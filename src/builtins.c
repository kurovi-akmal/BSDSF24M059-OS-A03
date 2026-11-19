
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

    // HISTORY command
    else if (strcmp(arglist[0], "history") == 0) {

        for (int i = 0; i < history_count; i++) {
            printf("%d  %s\n", i + 1, history[i]);
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
        printf("  history        - Display command history\n");
        printf("  set            - Display shell variables\n");

        return 1;
    }

    // SET command - Feature 8
    else if (strcmp(arglist[0], "set") == 0) {

        print_variables();

        return 1;
    }

    // JOBS command
    else if (strcmp(arglist[0], "jobs") == 0) {

        if (job_count == 0) {

            printf("No background jobs running.\n");
        }
        else {

            printf("Active Background Jobs:\n");

            for (int i = 0; i < job_count; i++) {

                printf("[%d] PID: %d  %s\n",
                       i + 1,
                       jobs[i].pid,
                       jobs[i].command);
            }
        }

        return 1;
    }

    // Not a built-in command
    return 0;
}
