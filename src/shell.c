
#include "shell.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Read command from input
char* read_cmd(char* prompt, FILE* fp) {

    printf("%s", prompt);
    fflush(stdout);

    char* cmdline = malloc(sizeof(char) * MAX_LEN);

    if (cmdline == NULL) {
        perror("malloc");
        return NULL;
    }

    int c;
    int pos = 0;

    while ((c = getc(fp)) != EOF) {

        if (c == '\n') {
            break;
        }

        if (pos >= MAX_LEN - 1) {
            fprintf(stderr, "Command too long!\n");
            free(cmdline);
            return NULL;
        }

        cmdline[pos++] = c;
    }

    if (c == EOF && pos == 0) {
        free(cmdline);
        return NULL;
    }

    cmdline[pos] = '\0';

    return cmdline;
}


// Tokenize command into arguments
char** tokenize(char* cmdline) {

    if (cmdline == NULL || cmdline[0] == '\0') {
        return NULL;
    }

    char** arglist = malloc(sizeof(char*) * (MAXARGS + 1));

    if (arglist == NULL) {
        perror("malloc");
        return NULL;
    }

    int argnum = 0;
    char* cp = cmdline;

    while (*cp != '\0') {

        // Skip spaces and tabs
        while (*cp == ' ' || *cp == '\t') {
            cp++;
        }

        if (*cp == '\0') {
            break;
        }

        // Recognize special operators
        if (*cp == '<' || *cp == '>' || *cp == '|') {

            if (argnum >= MAXARGS) {
                fprintf(stderr, "Too many arguments!\n");
                goto error;
            }

            arglist[argnum] = malloc(2);

            if (arglist[argnum] == NULL) {
                perror("malloc");
                goto error;
            }

            arglist[argnum][0] = *cp;
            arglist[argnum][1] = '\0';

            argnum++;
            cp++;
        }

        else {

            char* start = cp;

            while (*cp != '\0' &&
                   *cp != ' ' &&
                   *cp != '\t' &&
                   *cp != '<' &&
                   *cp != '>' &&
                   *cp != '|') {

                cp++;
            }

            int len = cp - start;

            if (argnum >= MAXARGS) {
                fprintf(stderr, "Too many arguments!\n");
                goto error;
            }

            arglist[argnum] = malloc(len + 1);

            if (arglist[argnum] == NULL) {
                perror("malloc");
                goto error;
            }

            memcpy(arglist[argnum], start, len);
            arglist[argnum][len] = '\0';

            argnum++;
        }
    }

    if (argnum == 0) {
        free(arglist);
        return NULL;
    }

    arglist[argnum] = NULL;

    return arglist;

error:

    for (int i = 0; i < argnum; i++) {
        free(arglist[i]);
    }

    free(arglist);

    return NULL;
}