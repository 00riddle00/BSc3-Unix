#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <setjmp.h>
#include <time.h>
#ifdef READLINE
#include <readline/readline.h>
#endif /* READLINE */

#include "utils.h"
#include "jobs.h"

/* macros */
#include "dbg.h" /* useful debugging macros */

/* enums */
enum { StylePrompt, StyleErrPrefix, StyleErrMsg, StyleErrInput }; /* style */

/* variables */
static const int cmd_buff_size = 512;
static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;
char **command;
int command_index;
pid_t group_id;
int exec_mode;

/* function declarations */
void sigint_handler();
char *read_line(char *prompt, int buffsize);
char **get_input(char *input, int buffsize);

/* configuration, allows nested code to access above variables */
#include "config.h"

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

// TODO move to utils file
#ifdef READLINE
char *
read_line(char *prompt, int buffsize) 
{
    return readline(prompt);
}

#else
char *
read_line(char *prompt, int buffsize) 
{
    printf(prompt);

    char *line = malloc(buffsize * sizeof(char));
    if (line == NULL) {
        perror("malloc failed");
        exit(1);
    }

    int count = 0;
    int blank_line = 1;

    // Read one line
    for (;;) {
        /* "stdin" is a default FILE pointer 
         * (FILE * as defined by the standard c library)
         *  used to get input from standard in. */
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
#endif /* READLINE */

// TODO move to utils file
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

    /* NULL terminated array, so that exec function
     * (evecvp, execve, etc.) knows where the list
     * of arguments ends */
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
    int startup_curr_cmd = 0;

    /* variables for jobs */
    active_jobs = 0;
    char active_jobs_str[4]; /* for printing job count */
    jobs_list = NULL;
    char job_name[cmd_buff_size];

    /* retrieving the calling process's PGID */
    group_id = getpgrp();

    /* Set up info to be displayed at the prompt */
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

    char date_buffer[16];
    char time_buffer[16];

    /* Set up prompt style */
    char *prompt_style = create_style(styles[StylePrompt]);

    /* Set up error styles */
    char *err_prefix_style = create_style(styles[StyleErrPrefix]);
    char *err_msg_style    = create_style(styles[StyleErrMsg]);
    char *err_input_style  = create_style(styles[StyleErrInput]);

    /* ------- signal handling --------- */

    /* SIGINT */
    struct sigaction s;
    s.sa_handler = sigint_handler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = SA_RESTART;
    sigaction(SIGINT, &s, NULL);

    // TODO change signal fn to sigaction
    signal(SIGTTOU, SIG_IGN);  // ttyout
    signal(SIGTTIN, SIG_IGN);  // ttyin
    signal(SIGTSTP, SIG_IGN);  // ^Z
    signal(SIGCHLD, &signal_handler_child);
    /* --------------------------------- */

    while (1) {   
		exec_mode = 0;
        job_name[0] = '\0';
        sprintf(active_jobs_str, "%d", active_jobs);

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

            strftime(date_buffer, 16, "%Y-%m-%d", time_info);
            strftime(time_buffer, 16, "%H:%M", time_info);

            /* TODO optimize: check all the flags beforehand
                plus some flags are not changed during shell runtime */

        char* curr_prompt;

        /* TODO move those str_replace() calls 
         * which do not change over shell's 
         * runtime outside of while loop.
         */
        curr_prompt = 
            str_replace(
            str_replace(
            str_replace(
            str_replace(
            str_replace(
            str_replace(
            str_replace(
            str_replace(

            str_replace(
            str_replace(
            str_replace(
            str_replace(
            str_replace(
            str_replace(
            str_replace(

                prompt, 

            "%u",     user_name),
            "%h",     host_name),
            "%w",     getcwd(current_dir, 512)),
            home_dir, "~"),
            "%d",     date_buffer),
            "%t",     time_buffer),
            "%j",     active_jobs_str),

            "${col_black}",  get_fg_color(col_black)),
            "${col_white}",  get_fg_color(col_white)),
            "${col_red}",    get_fg_color(col_red)),
            "${col_green}",  get_fg_color(col_green)),
            "${col_blue}",   get_fg_color(col_blue)),
            "${col_yellow}", get_fg_color(col_yellow)),
            "${col_purple}", get_fg_color(col_purple)),
            "${col_aqua}",   get_fg_color(col_aqua));

            input = read_line(set_style(prompt_style, curr_prompt), cmd_buff_size);
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

            continue;
        }

        /* ----------- processing job commands ---------- */

        /* send a job to foreground */
        if(strcmp(command[0], "fg") == 0) {
            if(command[1] == NULL) {
                continue;
            }

            int job_id = atoi(command[1]);
            JobsList *job = get_job(job_id, 0);
            if(job == NULL) {
                continue;
            }

            if(job->status == SUSPENDED) {
                put_job_foreground(job, 1, group_id);
            } else {
                put_job_foreground(job, 0, group_id);
            }

            continue;
        }

        /* send a job to background */
        if(strcmp(command[0], "bg") == 0) {
            if(command[1] == NULL) {
                continue;
            } else {
                int job_id = atoi(command[1]);
                JobsList *job = get_job(job_id, 0);
                put_job_background(job, 1, group_id);
                continue;
            }
        }

        /* list currently active jobs */
        if (strcmp(command[0], "jobs") == 0 ||
            strcmp(command[0], alias_jobs) == 0) {
            print_jobs();
            continue;
        }

        /* kill a job (with SIGTERM) */
        if(strcmp(command[0], "kill") == 0) {

            /* make "kill" accept only one argument */
            if(command[2] != NULL) {
                printf("kill: usage: 'kill %%<job_id>'\n");
                continue;
            }

            if(command[1] != NULL) {
                if (strchr(command[1]++, '%')) {
                    if (*command[1] == '\0') {
                        // TODO: kill latest process
                        printf("kill: usage: 'kill %%<job_id>'\n");
                        continue;
                    } else {
                        int cmd = atoi(command[1]);
                        if (cmd != 0) {
                            kill_job(cmd);
                        } else {
                            printf("kill: usage: 'kill %%<job_id>'\n");
                        }
                        continue;
                    }
                } 
            }
        }

        /* ---------------------------------------------- */

        /* run command either in 
         * foreground or in background */
        if((strcmp(command[command_index - 1], "&") == 0)) {
            command[--command_index] = NULL;
			exec_mode = BACKGROUND;
        } else {
			exec_mode = FOREGROUND;
        }

        /* make a fork (ie. create a child process) */
        child_pid = fork();
        if (child_pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        if (child_pid == 0) { /* child process */
            signal(SIGINT,  SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			signal(SIGTTOU, SIG_DFL);
			signal(SIGTTIN, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);
			signal(SIGCHLD, &signal_handler_child);

            /* "int setpgid(pid_t pid, pid_t pgid)"
             *
             * this function sets the PGID of the process specified by pid to pgid.
             * If "pid" = 0, then the process ID of the calling process is used.
             * If "pgid" = 0, then the PGID of the process specified by pid is made
             * the same as its process ID. 
             *
             * this command makes the current child's
             * process group be the same as child's pid.
             * This lets the child be the leader of the 
             * group (ie. the current job)
             */
            setpgid(0,0);

			if(exec_mode == FOREGROUND) {

                /* "int tcsetpgrp (int fd, pid_t pgid)"
                 *  function is used to set a terminal’s 
                 *  foreground process group ID. 
                 *
                 *  "fd" is a descriptor which specifies the terminal.
                 *  (STDIN_FILENO is the default standard input 
                 *  file descriptor number which is 0).
                 *
                 *  "pgid" specifies the process group. 
                 *
                 *  The calling process must be a member 
                 *  of the same session as pgid and must 
                 *  have the same controlling terminal.
                 *
                 *  This command is needed to attach 
                 *  terminal to the current child process,
                 *  making it a foreground process */
				tcsetpgrp(STDIN_FILENO, getpid());
			}

            /* the child notifies about it being put in the 
             * background, by printing its id and pid, for example: 
             * [1] 248601 */
			if(exec_mode == BACKGROUND) {
				printf("[%d] %d\n", ++active_jobs, (int) getpid());
			}

            /* the child process is replaced with a new process,
             * by using any of the function from "exec" family.
             * The pid of the process stays the same though.
             *
             * exec never returns if the call is successful
             * 'v' in execvp means passing an array of arguments
             * 'p' means do path searching for commands (ex. /usr/bin/ls)
             * no 'e' means inherit the environment from the parent
             */
            if (execvp(command[0], command) < 0) {
                fprintf( stderr,
                         "%s%s%s\n", 
                         set_style(err_prefix_style, "tsh: "),
                         set_style(err_msg_style, "command not found: "),
                         set_style(err_input_style, command[0]) );
                exit(0);
            }
        } else { /* parent process */

            /* parent process should also 
             * set the child as the leader
             * of child's group */
			setpgid(child_pid, child_pid);

            int i = 0;
            while (i != command_index) {
                strcat(job_name, command[i]);
                strcat(job_name, " ");
                i++;
            }

            /* parent process is managing job control.
             * It adds a new job, identified by the group id, 
             * which is equal to child_pid (the group leader) */
			jobs_list = add_job(child_pid, job_name, (int) exec_mode);

            /* parent process tracks the statuses of its
             * child processes (foreground/background/suspended).
             *
             * depending on this, parent process controls 
             * access to the terminal, can change processes'
             * execution mode (status), and is also prepared 
             * to perform kill, suspend or continue operations
             * on its managed processes.
             */
			JobsList *job = get_job(child_pid, 1);

			if(exec_mode == FOREGROUND) {
				 put_job_foreground(job, 0, group_id);
			}

			if(exec_mode == BACKGROUND) {
				put_job_background(job, 0, group_id);
			}
		}

		free(input);
		free(command);
    }

    return 0;
}

