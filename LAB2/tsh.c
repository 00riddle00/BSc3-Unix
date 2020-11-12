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

#include "utils.h"
#include "jobs.h"

/* macros */
#include "dbg.h" /* useful debugging macros */

/* enums */
enum { StylePrompt, StyleErrPrefix, StyleErrMsg, StyleErrInput }; /* style */
enum { FG, BG, BOLD, UNDERLINE, BLINK };                          /* style */

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
    group_id = getpgrp();

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
                    "%j", active_jobs_str),

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

        /* ----------- processing job commands ---------- */

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

        if (strcmp(command[0], "jobs") == 0 ||
            strcmp(command[0], alias_jobs) == 0) {
            print_jobs();
            continue;
        }

        if(strcmp(command[0], "kill") == 0) {
            if(command[1] == NULL) {
                continue;
            }
            kill_job(atoi(command[1]));
            continue;
        }

        /* ---------------------------------------------- */

        if((strcmp(command[command_index - 1], "&") == 0)) {
            command[--command_index] = NULL;
			exec_mode = BACKGROUND;
        } else {
			exec_mode = FOREGROUND;
        }

        child_pid = fork();
        if (child_pid < 0) {
            perror("Fork failed");
            exit(1);
        }

        if (child_pid == 0) {
			signal(SIGINT, SIG_DFL);
			signal(SIGQUIT, SIG_DFL);
			signal(SIGTTOU, SIG_DFL);
			signal(SIGTTIN, SIG_DFL);
			signal(SIGTSTP, SIG_DFL);
			signal(SIGCHLD, &signal_handler_child);
			setpgrp();

			if(exec_mode == FOREGROUND) {
				tcsetpgrp(STDIN_FILENO, getpid());
			}

			if(exec_mode == BACKGROUND) {
				printf("[%d] %d\n", ++active_jobs, (int) getpid());
			}

            if (execvp(command[0], command) < 0) {

                // TODO print "command not found" instead
                // of "no such file or directory"
                fprintf(stderr,
                        "%stsh: %s%scommand not found: %s%s%s%s\n", 
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
                        reset_style(),
                        set_style( style[StyleErrInput][FG],
                                   style[StyleErrInput][BG],
                                   style[StyleErrInput][BOLD],
                                   style[StyleErrInput][UNDERLINE],
                                   style[StyleErrInput][BLINK] ),
                        command[0],
                        reset_style() );

                exit(0);
            }
        } else {
			setpgid(child_pid, child_pid);
			jobs_list = add_job(child_pid, *(command), (int) exec_mode);
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

