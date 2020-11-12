/* function declarations */

/** function to clear the screen on terminal emulator
 *
 * @param: "force"=1 forces to clear the screen
 *          regardless of whether this is the first loop
 *          of the shell (startup) or not.
 */
void clear_screen(int force);

/** function to colorize and style text in the terminal
 *
 * @params: numeric values for color and style options
 *           taken from config.h (customization file)
 *
 * @return: ANSI escape sequence for styling text
 */
char *set_style(int fg, int bg, int bold, int uline, int blink);

/** function to reset the color and style of text
 *
 * @return: ANSI escape sequence
 */
char *reset_style();

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
