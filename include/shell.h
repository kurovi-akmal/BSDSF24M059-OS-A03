#ifndef SHELL_H
#define SHELL_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_LEN 512
#define MAXARGS 10
#define ARGLEN 30
#define PROMPT "FCIT> "
#define HISTORY_SIZE 20
#define MAX_JOBS 100

typedef struct {
    pid_t pid;
    char command[MAX_LEN];
} Job;

extern Job jobs[MAX_JOBS];
extern int job_count;
extern char* history[HISTORY_SIZE];
extern int history_count;

// Function prototypes
char* read_cmd(char* prompt, FILE* fp);
char** tokenize(char* cmdline);
int execute(char** arglist, int background, const char* command);
int handle_builtin(char** arglist);
int handle_builtin(char** arglist);
void add_job(pid_t pid, const char* command);
void check_background_jobs(void);

void assign_variable(const char* name, const char* value);
int handle_variable_assignment(const char* command);
void expand_variables(char** args);
void print_variables(void);


#endif // SHELL_H
