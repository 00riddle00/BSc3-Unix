#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "utils.h"

void 
clear_screen(int force) 
{
    if (force < 0 || force > 1) {
        printf("invalid argument to clear_screen()");
        exit(1);
    }

    const char *CLEAR_SCREEN_ANSI = " \x1b[1;1H\x1b[2J";
    static int first_time = 1; // clear screen for the first time

    if (first_time) {
        first_time = 0;
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    } else if (force) {
        write(STDOUT_FILENO, CLEAR_SCREEN_ANSI, 12);
    }
}

char *
create_style(const int *style)
{
    int _fg    = (style[FG] >= 0 && style[FG] <= 255) ? style[FG]   : -1;  /* ANSI: ESC[38;5;{0-255}m -> foreground */
    int _bg    = (style[BG] >= 0 && style[BG] <= 255) ? style[BG]   : -1;  /* ANSI: ESC[48;5;{0-255}m -> background */
    int _bold  = (style[BOLD]  == 1)                  ? 1           : -1;  /* ANSI: ESC[1m            -> bold       */
    int _uline = (style[UNDERLINE] == 1)              ? 4           : -1;  /* ANSI: ESC[4m            -> underline  */
    int _blink = (style[BLINK] == 1)                  ? 5           : -1;  /* ANSI: ESC[5             -> blink      */

    // TODO avoid magic numbers
    char *style_str = malloc(sizeof(char) * 64);

    sprintf( style_str,
             "\x1b[38;5;%dm"\
             "\x1b[48;5;%dm"\
             "\x1b[%dm"\
             "\x1b[%dm"\
             "\x1b[%dm",
             _fg,
             _bg,
             _bold,
             _uline,
             _blink );

    return style_str;
}

char *set_style(char* style_ansi, char* str) {
    char *styled_str = malloc(sizeof(char) * 256);
    sprintf(styled_str, "%s%s%s", style_ansi, str, RESET_STYLE);
    return styled_str;
}

// You must free the result if result is non-NULL.
char *
str_replace(char *orig, char *rep, char *with) 
{
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
