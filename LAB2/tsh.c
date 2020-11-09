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

/* enums */
enum { StylePrompt, StyleErrPrefix, StyleErrMsg, StyleErrInput }; /* style */
enum { fg, bg, bold, uline, blink };                              /* style */

/* function declarations */
void sigint_handler();
char *read_line(char *, int);

/* variables */
static const int cmd_buff_size = 512;
static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;

/* configuration, allows nested code to access above variables */
#include "config.h"

/* function implementations */
void sigint_handler() {
    if (!jump_active) {
        return;
    }
    // TODO avoid magic numbers
    siglongjmp(env, 42);
}

#ifndef READLINE
char *read_line(char *prompt, int buffsize) {

    printf("%s%s%s", 
            set_style( style[StylePrompt][fg],
                       style[StylePrompt][bg],
                       style[StylePrompt][bold],
                       style[StylePrompt][uline],
                       style[StylePrompt][blink] ),
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
char *read_line(char *prompt, int buffsize) {
    return readline(prompt);
}
#endif /* READLINE */

int main() {
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;
    int startup_curr_cmd = 0;
    /*int job_count = 0;*/

    errno = 0;

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
    struct tm* time_info;

    char date_buffer[10];
    char time_buffer[5];

    /* Setup SIGINT */
    struct sigaction s;
    s.sa_handler = sigint_handler;
    sigemptyset(&s.sa_mask);
    s.sa_flags = SA_RESTART;
    sigaction(SIGINT, &s, NULL);

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
                        set_style( style[StyleErrPrefix][fg],
                                   style[StyleErrPrefix][bg],
                                   style[StyleErrPrefix][bold],
                                   style[StyleErrPrefix][uline],
                                   style[StyleErrPrefix][blink] ),
                        reset_style(),
                        set_style( style[StyleErrMsg][fg],
                                   style[StyleErrMsg][bg],
                                   style[StyleErrMsg][bold],
                                   style[StyleErrMsg][uline],
                                   style[StyleErrMsg][blink] ),
                        strerror(errno),
                        reset_style(),
                        set_style( style[StyleErrInput][fg],
                                   style[StyleErrInput][bg],
                                   style[StyleErrInput][bold],
                                   style[StyleErrInput][uline],
                                   style[StyleErrInput][blink] ),
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

