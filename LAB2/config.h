/*  _       _       */
/* | |_ ___| |__    riddle00 */
/* | __/ __| '_ \   https://www.github.com/00riddle00/ */
/* | |_\__ \ | | |  */
/*  \__|___/_| |_|  */
               
/* See LICENSE file for copyright and license details. */


/*
%u : the username of the current user
%h : hostname
%w : current working directory, with $HOME abbreviated with a tilde
%d : current date in YYYY-MM-DD format
%t : current time in 24-hour HH:MM format
%j : number of jobs currently managed by the shell

Prompt examples:

"┌─{~tsh~}•[%h %w]\n└─╼ "
"┌─{~tsh~}•[%d]•[%u@%h]•[%w]> \n└─╼ "
"┌─{~tsh~}雷[%d]雷[%u@%h]雷[%w]>\n└─╼ "
"┌─{ tsh}[ %d][ %u@%h][ %w]>\n└─╼ "
"┌─{ tsh}[ %d][ %t][ %u@%h][ %w][華 %j]>\n└─╼ "
*/

static char prompt[] = "┌─{ tsh}[ %d][ %t][ %u@%h][ %w][華 %j]>\n└─╼ ";

/*==================================
  Style
==================================*/

#define YES    1
#define NO    -1
#define NONE  -1

/* ANSI color codes for defining 256 
 * colors are used. First 8 colors
 * correspond to the standard 8-bit
 * colors (listed below). For the 
 * complete color table as well as 
 * for further information on ANSI
 * escape codes, see COLORS.md */

static const int col_black    = 0;
static const int col_red      = 1;
static const int col_green    = 2;
static const int col_yellow   = 3;
static const int col_blue     = 4;
static const int col_magenta  = 5;
static const int col_cyan     = 6;
static const int col_white    = 7;

/* make sure to use only the allowed values in parentheses */
/* if bg color is set, the blink effect will not work      */
static const int style[][5]  = {
    /*                        FG              BG           BOLD    UNDERLINE   BLINK    */
    /*                    (color/NONE)    (color/NONE)   (YES/NO)   (YES/NO)  (YES/NO)  */
    /*                                                                                  */
    [StylePrompt]      = { col_blue,         NONE,         YES,       NO,       NO      },
    [StyleErrPrefix]   = { col_cyan,         NONE,         YES,       NO,       NO      },
    [StyleErrMsg]      = { col_red,          NONE,         YES,       NO,       NO      }, 
    [StyleErrInput]    = { col_white,        col_black,    NO,        NO,       NO      }, 
};

static const char alias_cls[]  = "cls";
static const char alias_exit[] = "q";
// TODO make this alias work
static const char alias_ls[]   = "ls --color=auto";

/*==================================
  Startup
==================================*/

/* startup array's size The number of commands 
 * in the array must be equal to it. */
static const int startup_cmd_count = 3;

/* startup array (startup commands can be added here) */
static const char startup[][512] = {

    //"neofetch", 
    
    "echo -e \x1b[0;36m\x1b[1m===========================================",

    "figlet |==tsh==|",
    
    /*
    "echo  ._. . . . . . ._. . . ._. . . . . . . . ._\n"
          "| |_____ _____| |_ ___| |__ ._____ _____| |\n"
          "| |_____|_____| __/ __| '_ \\|_____|_____| |\n"
          "| |_____|_____| |_\\__ \\ | | |_____|_____| |\n"
          "| | . . . . . .\\__|___/_| |_|...........| |\n"
          "|_| . . . . . . . . . . . . . . . . . . |_|",
    */

    "echo ===========================================\n"
          "---Welcome to tsh - a minimal UNIX shell!--\n"
          "===========================================",
};

