/*  _       _       */
/* | |_ ___| |__    riddle00 */
/* | __/ __| '_ \   https://www.github.com/00riddle00/ */
/* | |_\__ \ | | |  */
/*  \__|___/_| |_|  */
               
/* See LICENSE file for copyright and license details. */

/*==================================
  Prompt
==================================*/

static const char prompt[] = "{~tsh~}> ";

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
    /*                        fg              bg           bold    underline   blink    */
    /*                    (color/NONE)    (color/NONE)   (YES/NO)   (YES/NO)  (YES/NO)  */
    /*                                                                                  */
    [StylePrompt]      = { col_blue,         NONE,         YES,       NO,       NO      },
    [StyleErrPrefix]   = { col_cyan,         NONE,         YES,       NO,       NO      },
    [StyleErrMsg]      = { col_red,          NONE,         YES,       NO,       NO      }, 
    [StyleErrInput]    = { col_white,        col_black,    NO,        NO,       NO      }, 
};

static const char alias_cls[]  = "cls";
static const char alias_exit[] = "quit";
static const char alias_ls[]   = "ls --color=auto";

/*==================================
  Startup
==================================*/

#define CMD_BUFF_SIZE 256

/* used to track startup array's size */
static const int startup_cmd_count = 2;

/* startup commands can be added or removed here. 
 * The number of commands must match "startup_cmd_count" */
static const char startup[][CMD_BUFF_SIZE] = {
    "neofetch", 
    "echo ===============\n" "Welcome to tsh!\n" "===============",
};

