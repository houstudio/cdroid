#----------------------------------------------------------------------------
# PLplot TK demos
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 26-Jan-1995
#
# To plot these, start up plserver.  Type "source tkdemos.tcl", at which
# point a plframe extended widget is created.  Then type "1" for the first
# demo, "2" for the second, and so on.
#
# Note: each demo proc is supplied a widget argument if run from a widget,
# or "loopback" if being run from pltcl.  In the latter case, the
# "loopback cmd" does nothing, but is required two make the two styles of
# Tcl scripts compatible.
#----------------------------------------------------------------------------

proc sourceUtf8 {sourceFile} {

    if { [encoding system] != "utf-8" } {
        if { [package vsatisfies [info patch] 8.5] } {

            source -encoding utf-8 $sourceFile

        } else {

            #
            # Pre-Tcl 8.5
            #
            set infile [open $sourceFile]
            set contents [read $infile]
            close $infile
            eval [encoding convertfrom utf-8 \
                [encoding convertto [encoding system] $contents]]
        }
    } else {
        source $sourceFile
    }
}

set  utf8_examples {4 18 24 26 33}

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
        plxframe .plw
        set plwin .plw.plwin
    }
    "*wish*" -
    "*tclsh*" {
        # use 'wish" method (size of the window is determined by plstdwin)
        plstdwin .
        plframe .plw
        set plwin .plw
    }
    default {
        puts stderr "Error: argv0 = \"$argv0\"\ndoes not contain either the substrings \"plserver\", \"tclsh\", or \"wish\""
        puts stderr "Therefore cannot decide how to proceed with tkdemos.tcl so giving up"
        return
    }
}
puts "append ..."
pack append . .plw {left expand fill}

for {set i 0} {$i <= 33} {incr i} {

    puts "$i ..."
    if {$i != 32} {
	set demo x[format "%02d" $i]

	#
	# If the source code contains UTF-8 characters (beyond the
	# ASCII-7 encoding), take special measures
	#

	if { [lsearch $utf8_examples $i] < 0 } {
	    source $demo.tcl
	} else {
	    sourceUtf8 $demo.tcl
	}

	proc $i {} "
            global plwin
            $demo $plwin
            $plwin cmd plcol0 1
            $plwin cmd plsori 0
            $plwin cmd plspal0 cmap0_default.pal
            $plwin cmd plspal1 cmap1_default.pal 1
            $plwin cmd plstransform NULL
        "
    }
}
