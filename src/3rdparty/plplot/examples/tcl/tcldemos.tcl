#----------------------------------------------------------------------------
# PLplot TCL demos
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 23-Jun-1994
#
# To plot these, start up pltcl with the argument "standard_examples".
# Choose one of the devices to start the demo. Press the space bar
# to move to the next window or to the next example
#
# Note: each demo proc is supplied a widget argument if run from a widget,
# or "loopback" if being run from pltcl.  In the latter case, the
# "loopback cmd" does nothing but is required to make the two styles of
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

for {set i 0} {$i <= 33} {incr i} {
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

	# restore defaults
	proc $i {} "
            $demo
            loopback cmd pleop
            loopback cmd plcol0 1
            loopback cmd plsori 0
            loopback cmd plspal0 cmap0_default.pal
            loopback cmd plspal1 cmap1_default.pal 1
            loopback cmd plstransform NULL
        "
    }
}
