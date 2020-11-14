/*  _       _       */
/* | |_ ___| |__    riddle00 */
/* | __/ __| '_ \   https://www.github.com/00riddle00/ */
/* | |_\__ \ | | |  */
/*  \__|___/_| |_|  */
               
/* See LICENSE file for copyright and license details. */


/*==================================
  Prompt
==================================*/

/*
---------------------
  info placeholders
---------------------
%u : the username of the current user
%h : hostname
%w : current working directory, with $HOME abbreviated with a tilde
%d : current date in YYYY-MM-DD format
%t : current time in 24-hour HH:MM format
%j : number of jobs currently managed by the shell

-----------------------
  color placeholders
-----------------------
Only these eight colors are available for placeholders:

${col_black}
${col_white}
${col_red}
${col_green}
${col_blue}
${col_yellow}
${col_purple}
${col_aqua}

However, you can also color the whole prompt with 
any one of 256 colors (see Text Style section).
-----------------------

Prompt examples:

"┌─{~tsh~}•[%h %w]\n└─╼ "
"┌─{~tsh~}•[%d]•[%u@%h]•[%w]> \n└─╼ "
"┌─{~tsh~}雷[%d]雷[%u@%h]雷[%w]>\n└─╼ "
"┌─{ tsh}[ %d][ %u@%h][ %w]>\n└─╼ "
"┌─{ tsh}[ %d][ %t][ %u@%h][ %w][華 %j]>\n└─╼ "
"┌─{ tsh}[ %d][ %t]%{col_magenta}[ %u@%h][ %w]%{col_green}[華 %j]>\n%{col_blue}└─╼ ";
*/
static char *prompt = "┌─{ tsh}[ %d][ %t]${col_purple}[ %u@%h][ %w]${col_green}[華 %j]>\n${col_blue}└─╼ ";

/*==================================
  Aliases
==================================*/

static const char alias_cls[]  = "c";
static const char alias_exit[] = "q";
static const char alias_jobs[] = "j";

/*==================================
  Startup
==================================*/

/* startup array's size. The number of commands 
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

/*==================================
  Text style
==================================*/

/* ANSI color codes for defining 256 
 * colors are used. For the complete 
 * color table as well as for further 
 * information on ANSI escape codes, 
 * see COLORS.md */
static const int col_black    = 0;
static const int col_maroon   = 1;
static const int col_green    = 2;
static const int col_olive    = 3;
static const int col_navy     = 4;
static const int col_purple   = 5;
static const int col_teal     = 6;
static const int col_silver   = 7;

static const int col_grey     = 8;
static const int col_red      = 9;
static const int col_lime     = 10;
static const int col_yellow   = 11;
static const int col_blue     = 12;
static const int col_fuchsia  = 13;
static const int col_aqua     = 14;
static const int col_white    = 15;
/* ... */
/* ... */
static const int col_orchid   = 170;
/* ... */
/* ... */
static const int col_grey93   = 255;

#define YES    1
#define NO    -1
#define NONE  -1

/* make sure to use only the allowed values in parentheses */
/* if bg color is set, the blink effect will not work      */
static const int styles[][5]  = {
    /*                        FG              BG           BOLD    UNDERLINE   BLINK    */
    /*                    (color/NONE)    (color/NONE)   (YES/NO)   (YES/NO)  (YES/NO)  */
    /*                                                                                  */
    [StylePrompt]      = { col_blue,         NONE,         YES,       NO,       NO      },
    [StyleErrPrefix]   = { col_aqua,         NONE,         YES,       NO,       NO      },
    [StyleErrMsg]      = { col_red,          NONE,         YES,       NO,       NO      }, 
    [StyleErrInput]    = { col_white,        col_black,    NO,        NO,       NO      }, 
};

