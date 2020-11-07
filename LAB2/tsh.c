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
/* useful debugging macros */
#include "dbg.h"

#ifndef CMD_BUFF_SIZE
#define CMD_BUFF_SIZE 256
#endif

/* enums */
enum { StylePrompt, StyleErrPrefix, StyleErrMsg, StyleErrInput }; /* style */
enum { fg, bg, bold, uline, blink };                              /* style */

/* configuration, allows nested code to access above variables */
#include "config.h"

char *read_line(const char *);
char **get_input(char *);
void clear_screen(int);
int  cd(char *);
void sigint_handler(int);
char *set_style(int, int, int, int, int);
char *reset_style();

static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;

int main() {
    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;
    int startup_curr_cmd = 0;

    errno = 0;

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
            input = read_line(prompt);
        }

        if (input == NULL) { /* Exit on Ctrl-D */
            printf("exit\n");
            exit(0);
        }

        /* Handle empty input */
        if (input[0] == '\0') {
            continue;
        }

        command = get_input(input);

        if (strcmp(command[0], "cd") == 0) {
            if (cd(command[1]) < 0) {
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

#ifdef READLINE
char *read_line(const char *prompt) {
    return readline(prompt);
}
#else
char *read_line(const char *prompt) {

    printf("%s%s%s", 
            set_style( style[StylePrompt][fg],
                        style[StylePrompt][bg],
                        style[StylePrompt][bold],
                        style[StylePrompt][uline],
                        style[StylePrompt][blink] ),
            prompt,
            reset_style() );

    char *line = malloc(CMD_BUFF_SIZE * sizeof(char));
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

char **get_input(char *input) {
    char **command = malloc(CMD_BUFF_SIZE * sizeof(char *));
    if (command == NULL) {
        perror("malloc failed");
        exit(1);
    }

    char *separator = " ";
    char *parsed;
    int index = 0;

    parsed = strtok(input, separator);
    while (parsed != NULL) {
        command[index++] = parsed;
        parsed = strtok(NULL, separator);
    }

    command[index] = NULL;
    return command;
}

void clear_screen(int do_it) {
    if (do_it < 0 || do_it > 1) {
        printf("invalid argument to clear_screen()");
        exit(1);
    }

    const char *CLEAR_SCREEN_ANSI = " \x1b[1;1H\x1b[2J";
    static int first_time = 1; // clear screen for the first time

    if (first_time) {
        first_time = 0;
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    } else if (do_it) {
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    }
}

int cd(char *path) {
    return chdir(path);
}

void sigint_handler(int signo) {
    if (!jump_active) {
        return;
    }
    siglongjmp(env, 42);
}

char *set_style(int fg, int bg, int bold, int uline, int blink) {

    char *cls = malloc(sizeof(char) * 100);

    // TODO check for out of bounds values
    uline = (uline == -1) ? uline : uline + 3;
    blink = (blink == -1) ? blink : blink + 4;

    sprintf(cls, "\x1b[38;5;%dm\x1b[48;5;%dm\x1b[%dm\x1b[%dm\x1b[%dm", fg, bg, bold, uline, blink);

    return cls;
}

char *reset_style() {
    return "\x1b[0m";
}
