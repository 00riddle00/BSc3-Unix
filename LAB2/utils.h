/* macros */

/* resets the color and style of text */
#define RESET_STYLE "\x1b[0m"

/* enums */
enum { FG, BG, BOLD, UNDERLINE, BLINK }; /* style */

/* function declarations */

/** function to clear the screen on terminal emulator
 *
 * @param: "force"=1 forces to clear the screen
 *          regardless of whether this is the first loop
 *          of the shell (startup) or not.
 */
void clear_screen(int force);

/** function to create ANSI escape sequence for 
 *   colorizing and styling text in the terminal
 *
 * @param: "style" array containing numeric values for color
 *          and style options, ie. one of the elements from
 *          styles multi-array defined in config.h (customization file)
 *
 * @return: ANSI escape sequence for styling text
 */
char *create_style(const int *style);

/** function to apply style to string
 *
 * @param: "style_ansi" - string containing style escape sequences
 * @param: "str" - string to be styled
 *
 * @return: string concatenated with styling escape sequences
 *           and with reset style escape sequence at the end
 */
char *set_style(char* style_ansi, char* str);

/** function to get the string representing foreground color
 * 
 * @param: "col256_no" - color's number (out of 256, see COLORS.md)
 * 
 * @return: ANSI escape sequence for fg color
 */
char *get_fg_color(int col256_no);

/** function to replace one pattern with another in a string
 *
 * @param: "orig" - original string
 * @param: "rep"  - pattern to be replaced
 * @param: "with" - pattern to be replaced with
 *
 * @return: a string with replacements done. 
 *           The result must be freed if it is non-NULL.
 */
char *str_replace(char *orig, char *rep, char *with);
