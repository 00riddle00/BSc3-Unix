#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#include "jobs.h"

/* variables */
int mode;
pid_t group_id;

int active_jobs;
JobsList *jobs_list;

/* function implementations */
JobsList *
add_job(pid_t pgid, char *name, int status) 
{
    JobsList *new_job = malloc(sizeof(JobsList));
    new_job->name = (char *) malloc(sizeof(name));
    new_job->name = strcpy(new_job->name, name);
    new_job->pgid = pgid;
    new_job->status = status;
    new_job->next = NULL;

    if(jobs_list == NULL) {
        active_jobs++;
        new_job->id = active_jobs;
        return new_job;
    } else {
        JobsList *tmp_list = jobs_list;
        while (tmp_list->next != NULL) {
            tmp_list = tmp_list->next;
        }
        new_job->id = tmp_list->id + 1;
        tmp_list->next = new_job;
        active_jobs++;
        return jobs_list;
    }
}

JobsList *
del_job(JobsList *job) 
{
    if(jobs_list == NULL) {
        return NULL;
    }
    JobsList *current_job;
    JobsList *prev_job;

    current_job = jobs_list->next;
    prev_job = jobs_list;

    if(prev_job->pgid == job->pgid) {
        prev_job = prev_job->next;
        active_jobs--;
        return current_job;
    }

    while (current_job != NULL) {
         if(current_job->pgid == job->pgid) {
              active_jobs--;
              prev_job->next = current_job->next;
         }
         prev_job = current_job;
         current_job = current_job->next;
    }
    return jobs_list;
}

JobsList *
get_job(int key, int search_parameter) 
{
    JobsList *job = jobs_list;
    if(search_parameter == 1) {
       while (job != NULL) {
            if(job->pgid == key) {
                return job;
            } else {
                job = job->next;
            }
        }
    }
    else if(search_parameter == 0) {
        while (job != NULL) {
            if(job->id == key) {
                return job;
            } else {
                job = job->next;
            }
        }
    }
    
    return NULL;
}

int 
change_job_status(int pid, int status) 
{
    if(jobs_list == NULL) {
        return 0;
    } else {
        JobsList *job = jobs_list;
        while (job != NULL) {
            if(job->pgid == pid) {
                job->status = status;
                return 1;
            }
            job = job->next;
        }
        return 0;
    }
}

void 
wait_job(JobsList *job) 
{
    int termination_status;
    while (waitpid(job->pgid, &termination_status, WNOHANG) == 0) {
        if(job->status == SUSPENDED) {
            return;
        }
    }
    jobs_list = del_job(job);
}


void 
kill_job(int job_id) 
{
    if(jobs_list != NULL) {
        JobsList *job = get_job(job_id, 0);
        kill(job->pgid, SIGKILL);
    }   
}

void 
put_job_foreground(JobsList *job, int continue_job) 
{
    if(job == NULL) {
        return;
    }
    job->status = FOREGROUND;
    tcsetpgrp(STDIN_FILENO, job->pgid);
    if(continue_job) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }

    wait_job(job);
    tcsetpgrp(STDIN_FILENO, group_id);
}

void 
put_job_background(JobsList *job, int continue_job) 
{
    if(job == NULL) {
        return;
    }
    job->status = BACKGROUND;
    if(continue_job) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }
    tcsetpgrp(STDIN_FILENO, group_id);
}

void 
signal_handler_child() 
{
    pid_t pid;
    int termination_status;
    pid = waitpid(WAIT_ANY, &termination_status, WUNTRACED | WNOHANG);
    if (pid > 0) {
        JobsList *job = get_job(pid, 1);
        if (job == NULL) {
            return;
        }

        if (WIFEXITED(termination_status)) {
            if (job->status == BACKGROUND) {
                printf("\n[%d]+  Done\t   %s\n", job->id, job->name);
                jobs_list = del_job(job);
            }
        }
        else if (WIFSIGNALED(termination_status)) {
            printf("\n[%d]+  KILLED\t   %s\n", job->id, job->name);
            jobs_list = del_job(job);
        }
        else if (WIFSTOPPED(termination_status)) {
            tcsetpgrp(STDIN_FILENO, job->pgid);
            change_job_status(pid, SUSPENDED);
            printf("\n[%d]+   stopped\t   %s\n", active_jobs, job->name);
            return;
        } else {
            if (job->status == BACKGROUND) {
                jobs_list = del_job(job);
            }
        }
    }
}

void 
start_job(char *command[], int execution_mode) 
{
    pid_t pid;
    if((pid = fork()) == -1) {
        perror("fork error");
        exit(EXIT_FAILURE);
    }
    else if(pid == 0) {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);
        signal(SIGCHLD, &signal_handler_child);
        signal(SIGTTIN, SIG_DFL);
        setpgrp();
        if(execution_mode == FOREGROUND) {
            tcsetpgrp(STDIN_FILENO, getpid());
        }

        if(execution_mode == BACKGROUND) {
            printf("[%d] %d\n", ++active_jobs, (int) getpid());
        }

        if(execvp(*command, command) == -1) {
            printf("Error. Command not found: %s\n", command[0]);
        }
            
        exit(EXIT_SUCCESS);
    } else {
        setpgid(pid, pid);
        jobs_list = add_job(pid, *(command), (int) execution_mode);
        JobsList *job = get_job(pid, 1);
        if(execution_mode == FOREGROUND) {
             put_job_foreground(job, 0);
        }

        if(execution_mode == BACKGROUND) {
            put_job_background(job, 0);
        }
    }

}

void 
print_jobs() 
{
    JobsList *job = jobs_list;
    printf("________________________________________________________________\n");
    printf("| ID   | PID    | STATUS     | STATE | NAME OF EXEC            |\n");
    printf("----------------------------------------------------------------\n");
    while (job != NULL) {
        printf("| [%d]    %-7d  %-11s  %-6c  %-24s|\n", job->id, job->pgid, "running", job->status, job->name);
        job = job->next;
     }
    printf("----------------------------------------------------------------\n");
}
