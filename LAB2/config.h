/*  _       _       */
/* | |_ ___| |__    riddle00 */
/* | __/ __| '_ \   https://www.github.com/00riddle00/ */
/* | |_\__ \ | | |  */
/*  \__|___/_| |_|  */
               
/* See LICENSE file for copyright and license details. */

/*=============================
 Appearance
=============================*/

/* ANSI color codes for defining 256 
 * colors are used. First 8 colors
 * correspond to the standard 8-bit
 * colors (listed below). For the 
 * complete color table as well as 
 * for further information on ANSI
 * escape codes, see COLORS.md */

/*
------------------
 8-bit colors
------------------
* BLACK     "0"
* RED       "1"
* GREEN     "2"
* YELLOW    "3"
* BLUE      "4"
* MAGENTA   "5"
* CYAN      "6"
* WHITE     "7"
*/

static const char prompt[]          = "{~tsh~}> ";

static const char col_black[]       = "0";
static const char col_red[]         = "1";
static const char col_blue[]        = "4";
static const char col_cyan[]        = "6";
static const char col_white[]       = "7";

static const char *colors[][5]         = {
    /*                    fg           bg          bold  underline  blink   */
    [ColPrompt]       = { col_blue,    NULL,       1,    0,         0       },
    [ColErrPrefix]    = { col_cyan,    NULL,       1,    0,         0       },
    [ColErrMsg]       = { col_red,     NULL,       1,    0,         0       }, 
    [ColErrInput]     = { col_white,   col_black,  0,    0,         0       }, 
};

