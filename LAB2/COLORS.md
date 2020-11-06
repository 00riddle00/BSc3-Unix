
# ANSI Escape Sequences for colors

## General

Standard escape codes are prefixed with `Escape`:

- Ctrl-Key: `^[`
- Octal: `\033`
- Unicode: `\u001b`
- Hexadecimal: `\x1b`
- Decimal: `27`

Followed by the command, usually delimited by opening square bracket (`[`) and optionally followed by arguments and the command itself.

Arguments are delimeted by semi colon (`;`).

For example:

```sh
\x1b[1;31m  # Set style to bold, red foreground.
```

## Colors / Graphics Mode

| ESC Code Sequence | Description |
|:------------------|:------------|
| `ESC[{...}m` | Set styles and colors for cell and onward. |
| `ESC[0m` | reset all styles and colors |
| `ESC[1m` | set style to bold |
| `ESC[2m` | set style to dim |
| `ESC[2m` | set style to dim |

### Color codes

Most terminals support 8 and 16 colors, as well as 256 (8-bit) colors. These colors are set by the user, but have commonly defined meanings.

#### 8-16 Colors

| Color Name | Foreground Color Code | Background Color Code |
|:------------------|:------------|:------------|
| Black | `30` | `40` |
| Red | `31` | `41` |
| Green | `32` | `42` |
| Yellow | `33` | `43` |
| Blue | `34` | `44` |
| Magenta | `35` | `45` |
| Cyan | `36` | `46` |
| White | `37` | `47` |
| Reset | `0` | `0` |

> Note: The **Reset** is the reset code that resets _all_ colors and text effects.

Most terminals, apart from the basic set of 8 colors, also support the "bright" or "bold" colors. These have their own set of codes, mirroring the normal colors, but with an additional `;1` in their codes:

```sh
# Set style to bold, red foreground.
\x1b[1;31mHello
# Set style to dimmed white foreground with red background.
\x1b[2;37;41mWorld
```

#### 256 Colors

The following escape code tells the terminal to use the given color ID:

```
ESC[38;5;${ID}m
```

| ESC Code Sequence | Description |
|:------------------|:------------|
| `ESC[38;5;${ID}m` | Set foreground color. |
| `ESC[48;5;${ID}m` | Set background color. |

Where **ID** is the color index from 0 to 255 of the color table:

![256 Color table](https://user-images.githubusercontent.com/995050/47952855-ecb12480-df75-11e8-89d4-ac26c50e80b9.png)

The table starts with the original 16 colors (0-15).

The proceeding 216 colors (16-231) or formed by a 3bpc RGB value offset by 16, packed into a single value.

The final 24 colors (232-256) are grayscale starting from a shade slighly lighter than black, ranging up to shade slightly darker than white.

Some emulators interpret these steps as linear increments from (`256 / 24`) on all three channels, although some emulators may explicitly define these values.

## Resources

- [Gist this info was taken from](https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797)
- [Wikipedia: ANSI escape code](https://en.wikipedia.org/wiki/ANSI_escape_code)
- [Build your own Command Line with ANSI escape codes](http://www.lihaoyi.com/post/BuildyourownCommandLinewithANSIescapecodes.html)
- [ascii-table: ANSI Escape sequences](http://ascii-table.com/ansi-escape-sequences.php)
- [bluesock: ansi codes](https://bluesock.org/~willkg/dev/ansi.html)
- [bash-hackers: Terminal Codes (ANSI/VT100) introduction](http://wiki.bash-hackers.org/scripting/terminalcodes)
