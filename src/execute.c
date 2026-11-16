
#include "shell.h"
#include <fcntl.h>

// Apply redirection and execute command
void run_command(char** args) {
    char* clean_args[MAXARGS + 1];
    int j = 0;

    for (int i = 0; args[i] != NULL; i++) {

        // Output redirection >
        if (strcmp(args[i], ">") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Missing output filename\n");
                exit(1);
            }

            int fd = open(args[i + 1],
                          O_WRONLY | O_CREAT | O_TRUNC, 0644);

            if (fd == -1 || dup2(fd, STDOUT_FILENO) == -1) {
                perror("Output redirection");
                if (fd != -1) close(fd);
                exit(1);
            }

            close(fd);
            i++;
        }

        // Input redirection <
        else if (strcmp(args[i], "<") == 0) {
            if (args[i + 1] == NULL) {
                fprintf(stderr, "Missing input filename\n");
                exit(1);
            }

            int fd = open(args[i + 1], O_RDONLY);

            if (fd == -1 || dup2(fd, STDIN_FILENO) == -1) {
                perror("Input redirection");
                if (fd != -1) close(fd);
                exit(1);
            }

            close(fd);
            i++;
        }

        else {
            clean_args[j++] = args[i];
        }
    }

    clean_args[j] = NULL;

    if (clean_args[0] == NULL) {
        fprintf(stderr, "Missing command\n");
        exit(1);
    }

    execvp(clean_args[0], clean_args);
    perror("Command execution failed");
    exit(1);
}

int execute(char* arglist[]) {

    int pipe_pos = -1;

    // Find pipe operator
    for (int i = 0; arglist[i] != NULL; i++) {
        if (strcmp(arglist[i], "|") == 0) {
            pipe_pos = i;
            break;
        }
    }

    // Normal command without pipe
    if (pipe_pos == -1) {
        int pid = fork();

        if (pid == -1) {
            perror("fork");
            return 1;
        }

        if (pid == 0) {
            run_command(arglist);
        }

        int status;
        waitpid(pid, &status, 0);

        return WIFEXITED(status) ? WEXITSTATUS(status) : 1;
    }

    // Pipe requires commands on both sides
    if (pipe_pos == 0 || arglist[pipe_pos + 1] == NULL) {
        fprintf(stderr, "Invalid pipe command\n");
        return 1;
    }

    char* left[MAXARGS + 1];
    char* right[MAXARGS + 1];

    int i;

    for (i = 0; i < pipe_pos; i++) {
        left[i] = arglist[i];
    }
    left[i] = NULL;

    int j = 0;
    for (i = pipe_pos + 1; arglist[i] != NULL; i++) {
        right[j++] = arglist[i];
    }
    right[j] = NULL;

    int fd[2];

    if (pipe(fd) == -1) {
        perror("pipe");
        return 1;
    }

    // First child: writes into pipe
    int pid1 = fork();

    if (pid1 == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        return 1;
    }

    if (pid1 == 0) {
        close(fd[0]);

        if (dup2(fd[1], STDOUT_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }

        close(fd[1]);
        run_command(left);
    }

    // Second child: reads from pipe
    int pid2 = fork();

    if (pid2 == -1) {
        perror("fork");
        close(fd[0]);
        close(fd[1]);
        waitpid(pid1, NULL, 0);
        return 1;
    }

    if (pid2 == 0) {
        close(fd[1]);

        if (dup2(fd[0], STDIN_FILENO) == -1) {
            perror("dup2");
            exit(1);
        }

        close(fd[0]);
        run_command(right);
    }

    // Parent closes both pipe ends
    close(fd[0]);
    close(fd[1]);

    int status1, status2;

    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    return WIFEXITED(status2) ? WEXITSTATUS(status2) : 1;
}