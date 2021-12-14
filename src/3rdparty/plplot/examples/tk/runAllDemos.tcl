#----------------------------------------------------------------------------
# Source this file into a working Tk interpreter to run all the Tcl demos
# in a nice window with buttons for each demo you'd like to run.
#
# Vince Darley
# vince@santafe.edu
#
#----------------------------------------------------------------------------

#
# Set of disabled examples:
# examples 14 and 31 do not work properly
# example 32 has deliberately (not yet) been propagated to all languages
#
set greyedOut {14 31 32}

if {[catch {file readlink [info script]} path]} {
    set path [info script]
}
lappend auto_path "[file join [file dirname $path] .. tcl]"

# In order to distinguish whether this is a plserver or wish
# environment we assume that [info nameofexecutable] has the string
# "plserver", "wish" or "tclsh" in it.  Some contrived examples can be
# figured out where this assumption is not correct, and for those
# cases we simply emit an error message and return.  But normally this
# assumption is correct, and it is certainly correct for our tests.
switch -glob -- [info nameofexecutable] {
    "*plserver*" {
        # use 'plserver' method
        plstdwin .
        plxframe .p
        set plwin .p.plwin
        button .bnextpage -text "Page" -command [list event generate $plwin <Enter>]
    }
    "*wish*" -
    "*tclsh*" {
        # use 'wish" method
        plstdwin .
        plframe .p -eopcmd waitForReturn
        set plwin .p
        button .bnextpage -text "Page" -command [list $plwin nextpage]
    }
    default {
        puts stderr "Error: argv0 = \"$argv0\"\ndoes not contain either the substrings \"plserver\", \"tclsh\", or \"wish\""
        puts stderr "Therefore cannot decide how to proceed with runAllDemos.tcl so giving up"
        return
    }
}

bind $plwin <Key-Return> stopWaiting

proc stopWaiting {} {
    set ::waitForReturn 1
}
proc waitForReturn {} {
    .l configure -text "Press <enter> for the next page or complete the program"
    vwait ::waitForReturn
}

grid .p -columnspan 5 -sticky news
grid rowconfigure . 0 -weight 1
for {set i 0} {$i < 5} {incr i} {
    grid columnconfigure . $i -weight 1
}

# turn on pauses
$plwin cmd plspause 1

button .cexit -text "Quit" -command exit
if {$tcl_platform(platform) != "unix"} {
    button .cshell -text "Shell" -command "console show"
}

#
# The reload button is probably not very useful to most users
# as it reloads the set of demos and commands. So disable it
# at least the time being
#
button .creload -text "Reload" -command reload -state disabled

set buttons [concat [info commands .c*] .bnextpage]

proc reload {} {
    global demos
    foreach demo $demos {
	catch {rename $demo {}}
    }
    auto_reset
}

proc run {demo} {
    global plwin
    $plwin configure -eopcmd waitForReturn
    .l configure -text "Starting $demo"
    setButtonState disabled
    update idletasks
    $plwin cmd plbop
    if {[catch {$demo $plwin} err]} {
	puts stderr $err
    }
    $plwin configure -eopcmd [list .bnextpage configure -state normal]
    $plwin cmd pleop
    .l configure -text "$demo complete"
    setButtonState normal
    .bnextpage configure -state disabled
}

proc setButtonState {state} {
    global greyedOut
    foreach b [info commands .b*] {
	if { [lsearch $greyedOut [string range $b 2 end]] < 0 } {
	    $b configure -state $state
	} else {
	    $b configure -state disabled
	}
    }
}

for {set i 0} {$i <= 33} {incr i} {
    set demo x[format "%02d" $i]
    button .b$i -text "Demo $i" -command [list run $demo]
    lappend demos $demo
    lappend buttons .b$i
    if {[llength $buttons] == 5} {
	eval grid $buttons -sticky ew
	set buttons {}
    }
}

foreach i $greyedOut {
    .b$i configure -state disabled
}

if {[llength $buttons]} {
    eval grid $buttons -sticky ew
}

label .l
grid .l -sticky ew -columnspan 5
