#----------------------------------------------------------------------------
# Sets configuration options for plplot/TK driver.
# This file contains a dummy proc.  You can customize plplot/TK by copying
# settings from pldefaults.tcl into this file and modifying them to taste.
# See the notes in pldefaults.tcl for more info.
#----------------------------------------------------------------------------

proc plconfig {} {

# Uncomment the next line to enable my_key_filter to receive key info.

#    global user_key_filter;	set user_key_filter "my_key_filter"
}

# Demo user key filter.

proc my_key_filter {w client k n a} {

    puts "keypress: $k $n $a"
}
