/*  _       _       */
/* | |_ ___| |__    riddle00 */
/* | __/ __| '_ \   https://www.github.com/00riddle00/ */
/* | |_\__ \ | | |  */
/*  \__|___/_| |_|  */
               
/* See LICENSE file for copyright and license details. */

/*==================================
  Prompt
==================================*/
static const char prompt[]       = "{~tsh~}> ";

/*==================================
  Colors
==================================*/

/* ANSI color codes for defining 256 
 * colors are used. First 8 colors
 * correspond to the standard 8-bit
 * colors (listed below). For the 
 * complete color table as well as 
 * for further information on ANSI
 * escape codes, see COLORS.md */

/*
-----------------
 8-bit colors
-----------------
* BLACK     0
* RED       1
* GREEN     2
* YELLOW    3
* BLUE      4
* MAGENTA   5
* CYAN      6
* WHITE     7
*/

#define YES     1
#define NO     -1
#define NONE   -1

static const int col_black     = 0;
static const int col_red       = 1;
static const int col_blue      = 4;
static const int col_cyan      = 6;
static const int col_white     = 7;

static const int colors[][5]         = {
    /*                         fg            bg           bold    underline   blink    */
    /*                    (color/NONE)  (color/NONE)    (YES/NO)   (YES/NO)  (YES/NO)  */
    /*                                                                                 */
    [ColPrompt]       = { col_blue,         NONE,         YES,       NO,       NO      },
    [ColErrPrefix]    = { col_cyan,         NONE,         YES,       NO,       NO      },
    [ColErrMsg]       = { col_red,          NONE,         YES,       NO,       NO      }, 
    [ColErrInput]     = { col_white,        col_black,    NO,        NO,       NO      }, 
};
