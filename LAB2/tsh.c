#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#ifdef READLINE
#include <readline/readline.h>
#endif /* READLINE */
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>

#include "util.h"

/* macros */
#include "dbg.h" /* useful debugging macros */

// ============================ jobs macros ==================================
#define MAX_LENGTH 100
#define MAX_PIPES 10
#define MAX_ARGUMENTS 20
#define FOREGROUND 'F'
#define BACKGROUND 'B'
#define SUSPENDED 'S'
// ============================ jobs macros (END) ==================================

/* enums */
enum { StylePrompt, StyleErrPrefix, StyleErrMsg, StyleErrInput }; /* style */
enum { FG, BG, BOLD, UNDERLINE, BLINK };                              /* style */

/* variables */
static const int cmd_buff_size = 512;
static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;
char **command;
int command_index;

// ============================ jobs variables ==================================
int activeJobs;
int mode;

pid_t groupID;

typedef struct job {
    int id;
    char *name;
    pid_t pid;
    pid_t pgid;
    int status;
    struct job *next;
} JobsList;

JobsList *jobsList;
// ============================ jobs variables (END) ==================================

/* function declarations */
void sigint_handler();
char *read_line(char *, int);
char **get_input(char *, int);

// ============================ jobs function declarations ==================================
JobsList *add_Job(pid_t pgid, char *name, int status);
int change_job_status(int pid, int status);
JobsList *del_job(JobsList *job);
JobsList *get_job(int key, int searchParameter);
void wait_job(JobsList *job);
void kill_job(int jobID);
void put_job_foreground(JobsList *job, int continueJob);
void put_job_background(JobsList *job, int continueJob);
void signal_handler_child();
void start_job(char *command[], int executionMode);
void print_jobs();
// ============================ jobs function declarations (END) ==================================

/* configuration, allows nested code to access above variables */
#include "config.h"

// ============================ jobs function implementations ======================
JobsList *
add_job(pid_t pgid, char *name, int status) 
{
    JobsList *newJob = malloc(sizeof(JobsList));
    newJob->name = (char *) malloc(sizeof(name));
    newJob->name = strcpy(newJob->name, name);
    newJob->pgid = pgid;
    newJob->status = status;
    newJob->next = NULL;

    if(jobsList == NULL) {
        activeJobs++;
        newJob->id = activeJobs;
        return newJob;
    } else {
        JobsList *tmpList = jobsList;
        while (tmpList->next != NULL) {
            tmpList = tmpList->next;
        }
        newJob->id = tmpList->id + 1;
        tmpList->next = newJob;
        activeJobs++;
        return jobsList;
    }
}
//-----------------------------------------------------------------------------
int 
change_job_status(int pid, int status) 
{
    if(jobsList == NULL) {
        return 0;
    } else {
        JobsList *job = jobsList;
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
//-----------------------------------------------------------------------------
JobsList *
del_job(JobsList *job) 
{
    if(jobsList == NULL) {
        return NULL;
    }
    JobsList *currentJob;
    JobsList *prevJob;

    currentJob = jobsList->next;
    prevJob = jobsList;

    if(prevJob->pgid == job->pgid) {
        prevJob = prevJob->next;
        activeJobs--;
        return currentJob;
    }

    while (currentJob != NULL) {
         if(currentJob->pgid == job->pgid) {
              activeJobs--;
              prevJob->next = currentJob->next;
         }
         prevJob = currentJob;
         currentJob = currentJob->next;
    }
    return jobsList;
}
//-----------------------------------------------------------------------------
JobsList *
get_job(int key, int searchParameter) 
{
    JobsList *job = jobsList;
    if(searchParameter == 1) {
       while (job != NULL) {
            if(job->pgid == key) {
                return job;
            } else {
                job = job->next;
            }
        }
    }
    else if(searchParameter == 0) {
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
//-----------------------------------------------------------------------------
void 
wait_job(JobsList *job) 
{
    int terminationStatus;
    while (waitpid(job->pgid, &terminationStatus, WNOHANG) == 0) {
        if(job->status == SUSPENDED) {
            return;
        }
    }
    jobsList = del_job(job);
}

//-----------------------------------------------------------------------------
void 
kill_job(int jobID) 
{
    if(jobsList != NULL) {
        JobsList *job = get_job(jobID, 0);
        kill(job->pgid, SIGKILL);
    }   
}
//-----------------------------------------------------------------------------
void 
put_job_foreground(JobsList *job, int continueJob) 
{
    if(job == NULL) {
        return;
    }
    job->status = FOREGROUND;
    tcsetpgrp(STDIN_FILENO, job->pgid);
    if(continueJob) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }

    wait_job(job);
    tcsetpgrp(STDIN_FILENO, groupID);
}
//-----------------------------------------------------------------------------
void 
put_job_background(JobsList *job, int continueJob) 
{
    if(job == NULL) {
        return;
    }
    job->status = BACKGROUND;
    if(continueJob) {
        if(kill(job->pgid, SIGCONT) < 0) {
            perror("kill (SIGCONT)");
        }
    }
    tcsetpgrp(STDIN_FILENO, groupID);
}
//-----------------------------------------------------------------------------
void 
signal_handler_child() 
{
    pid_t pid;
    int terminationStatus;
    pid = waitpid(WAIT_ANY, &terminationStatus, WUNTRACED | WNOHANG);
    if (pid > 0) {
        JobsList *job = get_job(pid, 1);
        if (job == NULL) {
            return;
        }

        if (WIFEXITED(terminationStatus)) {
            if (job->status == BACKGROUND) {
                printf("\n[%d]+  Done\t   %s\n", job->id, job->name);
                jobsList = del_job(job);
            }
        }
        else if (WIFSIGNALED(terminationStatus)) {
            printf("\n[%d]+  KILLED\t   %s\n", job->id, job->name);
            jobsList = del_job(job);
        }
        else if (WIFSTOPPED(terminationStatus)) {
            tcsetpgrp(STDIN_FILENO, job->pgid);
            change_job_status(pid, SUSPENDED);
            printf("\n[%d]+   stopped\t   %s\n", activeJobs, job->name);
            return;
        } else {
            if (job->status == BACKGROUND) {
                jobsList = del_job(job);
            }
        }
    }
}
//-----------------------------------------------------------------------------
void 
start_job(char *command[], int executionMode) 
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
        if(executionMode == FOREGROUND) {
            tcsetpgrp(STDIN_FILENO, getpid());
        }

        if(executionMode == BACKGROUND) {
            printf("[%d] %d\n", ++activeJobs, (int) getpid());
        }

        if(execvp(*command, command) == -1) {
            printf("Error. Command not found: %s\n", command[0]);
        }
            
        exit(EXIT_SUCCESS);
    } else {
        setpgid(pid, pid);
        jobsList = add_job(pid, *(command), (int) executionMode);
        JobsList *job = get_job(pid, 1);
        if(executionMode == FOREGROUND) {
             put_job_foreground(job, 0);
        }

        if(executionMode == BACKGROUND) {
            put_job_background(job, 0);
        }
    }

}
//-----------------------------------------------------------------------------
void 
print_jobs() 
{
    JobsList *job = jobsList;
    while (job != NULL) {
        printf("%d\t%c\t%s\t%d\n", job->id, job->status, job->name, job->pgid);
        job = job->next;
     }
}
// ========================= jobs function implementations (END) ==================

/* function implementations */
void 
sigint_handler() 
{
    if (!jump_active) {
        return;
    }
    // TODO avoid magic numbers
    siglongjmp(env, 42);
}

#ifndef READLINE
char *
read_line(char *prompt, int buffsize) 
{

    printf("%s%s%s", 
            set_style( style[StylePrompt][FG],
                       style[StylePrompt][BG],
                       style[StylePrompt][BOLD],
                       style[StylePrompt][UNDERLINE],
                       style[StylePrompt][BLINK] ),
            prompt,
            reset_style() );

    char *line = malloc(buffsize * sizeof(char));
    if (line == NULL) {
        perror("malloc failed");
        exit(1);
    }

    int count = 0;
    int blank_line = 1;

    // Read one line
    for (;;) {
        int c = fgetc(stdin);

        if (c == EOF) {
            if (!count) {
                return (char *) NULL;
            } else {
                continue;
            }
        }

        if (c == '\n') {
            if (blank_line) {
                line[0] = '\0';
                break;
            } else {
                line[count] = '\0';
                break;
            }
        }

        if (c != ' ') {
            blank_line = 0;
        }

        line[count++] = (char) c;
    }

    return line;
}
#else
char *
read_line(char *prompt, int buffsize) 
{
    return readline(prompt);
}
#endif /* READLINE */

char **
get_input(char *input, int buffsize) 
{
    char **command = malloc(buffsize * sizeof(char *));
    if (command == NULL) {
        perror("malloc failed");
        exit(1);
    }

    char *separator = " ";
    char *parsed;
    // FIXME avoid using this global var
    command_index = 0;

    parsed = strtok(input, separator);
    while (parsed != NULL) {
        command[command_index++] = parsed;
        parsed = strtok(NULL, separator);
    }

    command[command_index] = NULL;
    return command;
}

int 
main() 
{
    errno = 0;

    command_index = 0;

    char *input;
    pid_t child_pid;
    int stat_loc;
    int startup_curr_cmd = 0;
// ============================ jobs variable definitions in main() ==================================
    activeJobs = 0;
    jobsList = NULL;
    groupID = getpgrp();
// ============================ jobs variable definitions in main() (END) ==================================

    /* Setup info to be displayed at the prompt */
    char *user_name = getenv("USER");

    // TODO get rid of magic numbers
    char home_dir[20];
    sprintf(home_dir, "/home/%s", user_name);

    char host_name[20];
    gethostname(host_name, 20);

    char current_dir[512];
    getcwd(current_dir, 512);

    time_t curr_time;
    char *time_str;
    struct tm *time_info;

    char date_buffer[10];
    char time_buffer[5];

    /* Setup SIGINT */
    struct sigaction s;
    s.sa_handler = sigint_handler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = SA_RESTART;
    sigaction(SIGINT, &s, NULL);

// ============================ Job control signals in parent ==============================
    signal(SIGTTOU, SIG_IGN);  //ttyout
    signal(SIGTTIN, SIG_IGN);  //ttyin
    signal(SIGCHLD, &signal_handler_child);
// ============================ Job control signals in parent (END) ==============================

    while (1) {   
        if (sigsetjmp(env, 1) == 42) {
            printf("\n");
            continue;
        }

        jump_active = 1;

        clear_screen(0);

        if (startup_curr_cmd != startup_cmd_count) {
            input = strdup(startup[startup_curr_cmd]);
            startup_curr_cmd++;
        } else {
            curr_time = time(NULL);
            time_str = ctime(&curr_time);
            time_str[strlen(time_str)-1] = '\0';
            time_info = localtime(&curr_time);

            strftime(date_buffer, 10, "%Y-%m-%d", time_info);
            strftime(time_buffer, 5, "%H:%M", time_info);

            // TODO optimize: check all the flags beforehand
            input = read_line(
                str_replace(
                    str_replace(
                        str_replace(
                            str_replace(
                                str_replace(
                                    str_replace(
                                        prompt,
                                        "%u",
                                        user_name),
                                    home_dir,
                                    "~"),
                                "%h",
                                host_name),
                            "%w", 
                            getcwd(current_dir, 512) ),
                        "%d", date_buffer),
                    "%t", time_buffer),

                    cmd_buff_size
                );
        }

        if (input == NULL) { /* Exit on Ctrl-D */
            printf("exit\n");
            exit(0);
        }

        /* Handle empty input */
        if (input[0] == '\0') {
            continue;
        }

        command = get_input(input, cmd_buff_size);

        if (strcmp(command[0], "cd") == 0) {
            if (chdir(command[1]) < 0) {
                perror(command[1]);
            }

            // Skip the fork
            continue;
        }

        if (strcmp(command[0], "exit") == 0 ||
            strcmp(command[0], alias_exit) == 0) {

            exit(0);
        }

        if (strcmp(command[0], alias_cls) == 0) {
            clear_screen(1);

            // Skip the fork
            continue;
        }

// ======================= processing jobs commands==========================
        if(strcmp("bg", command[0]) == 0)
        {
            if(command[1] == NULL) {
                continue;
            } else {
                int jobID = atoi(command[1]);
                JobsList *job = get_job(jobID, 0);
                put_job_background(job, 1);
                continue;
            }
        }

        if(strcmp("fg", command[0]) == 0) {
            if(command[1] == NULL) {
                continue;
            }

            int jobID = atoi(command[1]);
            JobsList *job = get_job(jobID, 0);
            if(job == NULL) {
                continue;
            }

            if(job->status == SUSPENDED) {
                put_job_foreground(job, 1);
            } else {
                put_job_foreground(job, 0);
            }

            continue;
        }

        if(strcmp("jobs", command[0]) == 0) {
            print_jobs();
            continue;
        }

        if(strcmp("kill", command[0]) == 0) {
            if(command[1] == NULL) {
                continue;
            }
            kill_job(atoi(command[1]));
            continue;
        }

        if((strcmp(command[command_index - 1], "&") == 0)) {
            command[--command_index] = NULL;
            start_job(command, BACKGROUND);
            continue;
       } else {
            start_job(command, FOREGROUND);
            continue;
       }

// ====================== processing jobs commands (END) ========================

        child_pid = fork();
        if (child_pid <0) {
            perror("Fork failed");
            exit(1);
        }

        if (child_pid == 0) {
            struct sigaction s;
            s.sa_handler = sigint_handler;
            sigemptyset(&s.sa_mask);
            s.sa_flags = SA_RESTART;
            sigaction(SIGINT, &s, NULL);

            /* Never returns if the call is successful */
            if (execvp(command[0], command) < 0) {

                // TODO print "command not found" instead
                // of "no such file or directory"
                fprintf(stderr,
                        "%stsh: %s%s%s: %s%s%s%s\n", 
                        set_style( style[StyleErrPrefix][FG],
                                   style[StyleErrPrefix][BG],
                                   style[StyleErrPrefix][BOLD],
                                   style[StyleErrPrefix][UNDERLINE],
                                   style[StyleErrPrefix][BLINK] ),
                        reset_style(),
                        set_style( style[StyleErrMsg][FG],
                                   style[StyleErrMsg][BG],
                                   style[StyleErrMsg][BOLD],
                                   style[StyleErrMsg][UNDERLINE],
                                   style[StyleErrMsg][BLINK] ),
                        strerror(errno),
                        reset_style(),
                        set_style( style[StyleErrInput][FG],
                                   style[StyleErrInput][BG],
                                   style[StyleErrInput][BOLD],
                                   style[StyleErrInput][UNDERLINE],
                                   style[StyleErrInput][BLINK] ),
                        command[0],
                        reset_style() );

                exit(1);
            }
        } else {
            waitpid(child_pid, &stat_loc, WUNTRACED);
        }

        if (!input)
            free(input);
        if (!command)
            free(command);
    }

    return 0;
}

