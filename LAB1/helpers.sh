# returns "A" \ "B" (subtracting identical lines)
a_minus_b() { grep -Fvx -f "$2" "$1" | sort; }

# returns "A" & "B" (identical lines)
a_and_b() { comm -12 <( sort "$1" ) <( sort "$2" ) }

# returns "A" U "B" (union of lines, removes duplicate lines)
a_union_b() { cat "$1" "$2" | sort -u }
