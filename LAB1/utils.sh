# ================================== #
#          _   _ _                   #
#    _   _| |_(_) |___               #   
#   | | | | __| | / __| shell        #   
#   | |_| | |_| | \__ \  utility     #   
#    \__,_|\__|_|_|___/   functions  #
#                                    #   
# ================================== #
 
## make cmdline aliases for zsh
ma() { echo alias "$1='$2'" >> "$HOME/.config/zsh/aliases.zsh"; zsh; }

# add numbers to non empty lines in a file
# usage: num <file>
num() { nl -s ' ' "$1" > tmp && mv tmp "$1" && sed "s/^[ \t]*//" -i "$1" && cat "$1" | xclip }

# make symbolic links
# if 1st arg is a dir, no "/" should be appended
# ln -s {FILE_PATH} {SYMLINK_PATH}
sym() { ln -s "$(pwd)/$1" "$2"; }
sym.dir() { ln -s "$(pwd)/$1" "$2/$1"; }

# go to command's flag description in  manpage, ex. `manf grep -r`
manf () { man "$1" | less -p "^ +$2"; }

# find all files from current folder | prints extension of files if any | make a unique sorted list
ext() { find . -type f | perl -ne 'print $1 if m/\.([^.\/]+)$/' | sort -u; }

# opens a temporary file with a given extension
# (default extension: .md)
# file is opened in /tmp dir, it's name contains a timestamp
temp() {
    ext="$1";
    [[ -z "$1" ]] && ext="md"
    vim "/tmp/temp_$(date +%F_%H_%M_%S).$ext";
}

# returns "A" \ "B" (subtracting identical lines)
a_minus_b() { grep -Fvx -f "$2" "$1" | sort; }

# returns "A" & "B" (identical lines)
a_and_b() { comm -12 <( sort "$1" ) <( sort "$2" ) }

# returns "A" U "B" (union of lines, removes duplicate lines)
a_union_b() { cat "$1" "$2" | sort -u }


