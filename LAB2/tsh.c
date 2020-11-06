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

#define CMD_BUFF_SIZE    256

/* enums */
enum { ColPrompt, ColErrPrefix, ColErrMsg, ColErrInput }; /* colors */

/* configuration */
#include "config.h"

/*colors[0][0]*/
/*colors[0][1] = -1*/
/*colors[0][2] =  1*/
/*colors[0][3] = -1*/
/*colors[0][4] = -1*/

        /*"\x1b[38;5;7m" "\x1b[48;5;-1m" "\x1b[1m" "\x1b[4m" "\x1b[5m",*/

/*char prompt_color[100];*/
/*sprintf(prompt_color, "\x1b[38;5;%sm \x1b[48;5;%sm \x1b[%sm \x1b[%sm \x1b[%sm", colors[0][0], colors[0][1], colors[0][2], colors[0][3], colors[0][4]);*/
/*sprintf(prompt_color, "\x1b[38;5;1m");*/

char *read_line(const char *);
char **get_input(char *);
void clear_screen();
int  cd(char *);
void sigint_handler(int);

static sigjmp_buf env;
static volatile sig_atomic_t jump_active = 0;

int main() {
    char *test[100];
    /*sprintf(test, "textas");*/
    /*sprintf(test, "col: '%s'", colors[ColPrompt][0]);*/

    /*sprintf(test, "\x1b[38;5;%sm \x1b[48;5;%sm \x1b[%sm \x1b[%sm \x1b[%sm", "7", "-1", "1", "4", "5");*/
    /*sprintf(test, "\x1b[38;5;4m");*/

    /*printf("col: '%s'", colors[ColPrompt][0]);*/
    /*printf(test "%s" ANSI_COLOR_RESET, "tsh>");*/
    /*printf(ANSI_COLOR_BLUE_BOLD "%s" ANSI_COLOR_RESET, prompt); // disp prompt*/

            /*"7", "-1", "1", "4", "5"*/

/*    printf("\x1b[38;5;%sm\x1b[48;5;%dm\x1b[%dm\x1b[%dm\x1b[%dmtest\x1b[0m ", */
            /*colors[ColPrompt][0],*/
            /*colors[ColPrompt][1],*/
            /*colors[ColPrompt][2],*/
            /*colors[ColPrompt][3],*/
            /*colors[ColPrompt][4]);*/



    char **command;
    char *input;
    pid_t child_pid;
    int stat_loc;

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

        clear_screen();
        input = read_line(prompt);

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

        if (strcmp(command[0], "exit") == 0) {
            exit(0);
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
                /*fprintf(stderr, ANSI_COLOR_CYAN_BOLD "tsh: " ANSI_COLOR_RED_BOLD "%s: " ANSI_COLOR_WHITE "%s\n" ANSI_COLOR_RESET, strerror(errno), command[0]);*/
                fprintf(stderr,
                        "\x1b[38;5;%sm"
                        "\x1b[48;5;%dm"
                        "\x1b[%dm"
                        "\x1b[%dm"
                        "\x1b[%dm"
                        "tsh: "
                        "\x1b[38;5;%sm"
                        "\x1b[48;5;%dm"
                        "\x1b[%dm"
                        "\x1b[%dm"
                        "\x1b[%dm"
                        "%s: "
                        "\x1b[38;5;%sm"
                        "\x1b[48;5;%dm"
                        "\x1b[%dm"
                        "\x1b[%dm"
                        "\x1b[%dm"
                        "%s\n"
                        "\x1b[0m", 
                        colors[ColErrPrefix][0],
                        colors[ColErrPrefix][1],
                        colors[ColErrPrefix][2],
                        colors[ColErrPrefix][3],
                        colors[ColErrPrefix][4],
                        colors[ColErrMsg][0],
                        colors[ColErrMsg][1],
                        colors[ColErrMsg][2],
                        colors[ColErrMsg][3],
                        colors[ColErrMsg][4],
                        strerror(errno),
                        colors[ColErrInput][0],
                        colors[ColErrInput][1],
                        colors[ColErrInput][2],
                        colors[ColErrInput][3],
                        colors[ColErrInput][4],
                        command[0]);

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

    printf("\x1b[38;5;%sm"
           "\x1b[48;5;%dm"
           "\x1b[%dm"
           "\x1b[%dm"
           "\x1b[%dm"
           "%s"
           "\x1b[0m", 
           colors[ColPrompt][0],
           colors[ColPrompt][1],
           colors[ColPrompt][2],
           colors[ColPrompt][3],
           colors[ColPrompt][4],
           prompt);

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

void clear_screen() {
    static int first_time = 1; // clear screen for the first time
    if (first_time) {
        const char *CLEAR_SCREEN_ANSI = " \x1b[1;1H\x1b[2J";
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
        first_time = 0;
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
