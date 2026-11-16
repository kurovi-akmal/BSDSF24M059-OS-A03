
#include "shell.h"

Job jobs[MAX_JOBS];
int job_count = 0;

void add_job(pid_t pid, const char* command) {

    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "Maximum background jobs reached!\n");
        return;
    }

    jobs[job_count].pid = pid;

    snprintf(jobs[job_count].command,
             MAX_LEN, "%s", command);

    job_count++;

    printf("[%d] %d\n", job_count, pid);
}

void check_background_jobs(void) {

    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

        for (int i = 0; i < job_count; i++) {

            if (jobs[i].pid == pid) {

                printf("\n[%d] Done  %s\n",
                       i + 1, jobs[i].command);

                for (int j = i; j < job_count - 1; j++) {
                    jobs[j] = jobs[j + 1];
                }

                job_count--;
                break;
            }
        }
    }
}
