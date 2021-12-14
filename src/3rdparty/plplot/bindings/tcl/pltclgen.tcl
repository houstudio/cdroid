#!/usr/local/bin/tclsh

#  Copyright (C) 2006-2016 Arjen Markus
#  Copyright (C) 2016 Alan W. Irwin
#
#  This file is part of PLplot.
#
#  PLplot is free software; you can redistribute it and/or modify
#  it under the terms of the GNU Library General Public License as published
#  by the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  PLplot is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Library General Public License for more details.
#
#  You should have received a copy of the GNU Library General Public License
#  along with PLplot; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# This script is used to automatically generate most of the functions needed
# to implement the PLplot Tcl API.  It reads a file which specifies the
# PLplot API command arguments and return value (basically like a C
# prototype, but easier to parse) and generates a Tcl command procedure to
# call that function.
#
# Currently it can support arguments of type PLINT, PLFLT, char *, PLINT&
# and PLFLT& where the last two are used for the 'g' series functions which
# provide data to the caller.
#
# What is not supported at this time, but needs to be, is support for (those
# few) PLplot API functions with non-void return types, and (the many)
# PLplot API functions which accept arrays (PLFLT *, etc).  The latter can
# in many cases be correctly treated as 1-d Tcl Matricies.  The code for
# using these should be sufficiently perfunctory to be amenable to an
# approach like that used here.  Automatic support for the 2-d API is
# probably unrealistic.
###############################################################################

# expandmacro:
# Expand the simple macros sz(..), sz1(..) and sz2(..)
#
# Uses raw calculations instead of regular expressions. The regexps
# would probably be rather complicated.
#
proc expandmacro {string} {
    set found 1
    while { $found } {
        set found 0

        set posb [string first "sz" $string]
        if { $posb >= 0 } {
            set found 1

            set dim [string range $string [expr {$posb+2}] [expr {$posb+2}]]
            if { $dim != 1 && $dim !=2 && $dim != "(" } {
                return -code error "error in macro: $string"
            }
            if { $dim == "(" } {
                set dim 0
            } else {
                incr dim -1
            }

            set pose [string first ")" $string $posb]
            set name [string range $string [expr {$posb+3}] [expr {$pose-1}]]
            set replace "mat$name->n\[$dim]"
            set string [string replace $string $posb $pose $replace]
        }

        set posb [string first "type(" $string]
        if { $posb >= 0 } {
            set found 1

            set pose [string first ) $string $posb]
            if { $pose < 0 } {
                return -code error "error in macro: $string"
            }
            set name [string range $string [expr {$posb+5}] [expr {$pose-1}]]
            set replace "mat$name->type"
            set string [string replace $string $posb $pose $replace]
        }
    }
    return $string
}

# Process arguments.
proc process_args { GENFILE round ndefs indent nredacted nargs name_argred name_argref name_argtype name_argname } {

    # All array references in arguments must be passed by name
    # which is documented by the "name_" prefix for all such array names.
    # Obtain access to those arrays (defined in the calling programme)
    # from those names using upvar.
    upvar 1 $name_argred argred
    upvar 1 $name_argref argref
    upvar 1 $name_argtype argtype
    upvar 1 $name_argname argname
    set i -1
    for { set k 0 } { $k < $nargs } { incr k } {
	if { $round == 0 } {
	    incr i
	}
	if { $round == 1 } {
	    if { $argred($k) != "" } {
		continue
	    }
	    incr i
	}
	if { $round == 2 && $argred($k) == "" } {
	    continue
	}
	if { $ndefs > 0 } {
	    puts $GENFILE "    if (argc > $i+1) \{    "
	}
	if { $argref($k) } {
	    puts $GENFILE "/* $argname($i) is for output. */"
	    continue
	}
	switch -- $argtype($k) {
	    "PLINT *" {
		puts $GENFILE "    ${indent}mat$argname($k) = Tcl_GetMatrixPtr( interp, argv\[1+$i\] );"
		puts $GENFILE "    ${indent}if (mat$argname($k) == NULL) return TCL_ERROR;"
		puts $GENFILE "    ${indent}$argname($k) = mat$argname($k)-\>idata;"
	    }
	    "PLUNICODE *" {
		puts $GENFILE "    ${indent}mat$argname($k) = Tcl_GetMatrixPtr( interp, argv\[1+$i\] );"
		puts $GENFILE "    ${indent}if (mat$argname($k) == NULL) return TCL_ERROR;"
		puts $GENFILE "    ${indent}$argname($k) = mat$argname($k)-\>idata;"
	    }
	    "PLFLT \*" {
		puts $GENFILE "    ${indent}mat$argname($k) = Tcl_GetMatrixPtr( interp, argv\[1+$i\] );"
		puts $GENFILE "    ${indent}if (mat$argname($k) == NULL) return TCL_ERROR;"
		puts $GENFILE "    ${indent}$argname($k) = mat$argname($k)-\>fdata;"
	    }
	    "PLINT" {
		# Redacted arguments are always PLINTs
		if { $round != 2 } {
		    puts $GENFILE "    ${indent}$argname($k) = atoi(argv\[1+$i\]);"
		} else {
		    puts $GENFILE "    ${indent}$argname($k) = $argred($k);"
		}
	    }
	    "PLUNICODE" {
		puts $GENFILE "    ${indent}$argname($k) = (PLUNICODE) strtoul(argv\[1+$i\],NULL,10);"
	    }
	    "unsigned int" {
		puts $GENFILE "    ${indent}$argname($k) = (unsigned int) strtoul(argv\[1+$i\],NULL,10);"
	    }
	    "PLFLT" {
		puts $GENFILE "    ${indent}$argname($k) = atof(argv\[1+$i\]);"
	    }
	    "const char *" {
		puts $GENFILE "    ${indent}$argname($k) = argv\[1+$i\];"
	    }
	    "char" {
		puts $GENFILE "    ${indent}$argname($k) = argv\[1+$i\]\[0\];"
	    }
	    default {
		puts "Unrecognized argtype : $argtype($k)"
	    }
	}
	if { $ndefs > 0 } {
	    puts $GENFILE "    \}"
	}
    }
}

# Process a function "prototype".  Suck up the args, then perform the
# needed substitutions to the Tcl command procedure template.
# Generate the three outputs needed for use in tclAPI.c:  the C
# function prototype, the CmdInfo structure initializer, and the
# actual function definition.

proc process_pltclcmd {cmd rtype} {
    global verbose
    global if_non_redacted
    global cmdfile
    global GENHEAD
    global GENSTRUCT
    global GENFILE
    global SPECFILE

    if { $::verbose } {
       puts "\nautogenerating Tcl command proc for $rtype $cmd ()"
    }

    puts $GENHEAD "static int ${cmd}Cmd( ClientData, Tcl_Interp *, int, const char **);"
    puts $GENSTRUCT "    {\"$cmd\",          ${cmd}Cmd},"

    # The following while loop parses the arguments and consistency
    # checks from all parts of a spec other than the first line (which
    # is parsed by the main routine before calling process_pltclcmd).
    # Note an empty line separates one spec from the next).
    set args              ""  ;# Space delimited list of argument names.
    set non_redacted_args ""  ;# Space delimited list of non-redacted argument names.
    set argchk            ""  ;# C code to check arguments.
    set nargs             0   ;# Number of arguments
    set ndefs             0   ;# Number of default values of arguments
    set nredacted         0   ;# Number of redacted arguments
    set refargs           0   ;# Whether argument list includes a returned argument?
    # argname($nargs) stores $vname = argument name
    # argtype($nargs) stores $vtype = argument type
    # argred($nargs)  stores [expandmacro $redactedval]
    # argdef($nargs)  stores $defval = argument default value.
    # argref($nargs)  stores 0 or 1 depending on whether argument is a returned argument.

    while { [gets $SPECFILE line] >= 0 } {

        if { $line == "" } {
            break
        }

        if { [regexp {^(\w+)\s+(.*)$} $line ==> vname vtype] > 0 } {
            set defval ""
            if { $verbose } {
               puts "vname=$vname vtype=$vtype"
            }
	    # The current $SPECFILE does not have lines containing
	    # both "Def:" and "=" so defacto defval and redactedval
	    # are mutually exclusive and therefore the logic below to
	    # determine those two quantities works.
            if { [regexp {(.*)\s+Def:\s+(.*)} $vtype ==> vtype defval] } {
                if { $verbose } {
                    puts "default arg: vtype=$vtype defval=$defval"
                }
            }
            set redactedval  ""
            if { [regexp {(.*)\s+=\s+(.*)} $vtype ==> vtype redactedval] } {
                if { $verbose } {
                    puts "redacted arg: vtype=$vtype redacted=$redactedval"
                }
            }
            set argname($nargs) $vname
            set argtype($nargs) $vtype
            set argred($nargs)  [expandmacro $redactedval]
            set argdef($nargs)  $defval
            set argref($nargs)  0 ;# default.

            # Check to see if this arg is for fetching something from PLplot.

            if { [string first & $vtype] >= 0 || $vtype == "char *" } {
                set refargs 1
                set argref($nargs) 1
            }

            if { $args == "" } {
                set args "$vname"
            } else {
                set args "$args $vname"
            }

	    if { $argred($nargs) == "" } {
		if { $non_redacted_args == "" } {
		    set non_redacted_args "$vname"
		} else {
		    set non_redacted_args "$non_redacted_args $vname"
		}
	    }

            if { $defval != "" } {
                incr ndefs
            }
            if { $redactedval != "" } {
                incr nredacted
            }
            incr nargs
            continue
        }
        # Consistency check
        if { [regexp {^!consistency} $line] } {
            set check  [expandmacro [lindex $line 1]]
            set msg    [lindex $line 2]
            append argchk \
"    if ( ! ($check) ) {
        Tcl_AppendResult( interp, \"$msg\", (char *) NULL );
        return TCL_ERROR;
    }\n"
            continue
        }

        # Unrecognized output.

        puts "bogus: $line"
    }

    if { $verbose } {
        puts "There are $nargs arguments, $ndefs are defaultable.";
        for { set i 0 } { $i < $nargs } { incr i } {
            puts "$argtype($i) $argname($i)";
        }
        if { $refargs > 0 } {
           puts "return string required."
        }
    }

    set TEMPLATE [open "$cmdfile"]

    if { $refargs } {
	if { $if_non_redacted && $nredacted > 0 } {
	    set full_command "\\\"$cmd \?$non_redacted_args\?\\\" or \\\"$cmd \?$args\?\\\""
	} else {
	    set full_command "\\\"$cmd \?$non_redacted_args\?\\\""
	}
    } else {
	if { $if_non_redacted && $nredacted > 0 } {
	    set full_command "\\\"$cmd $non_redacted_args\\\" or \\\"$cmd $args\\\""
	} else {
	    set full_command "\\\"$cmd $non_redacted_args\\\""
	}
    }

    while { [gets $TEMPLATE tmpline] >= 0 } {

        # Emit the argument declarations.  Reference args require special
        # handling, the others should be perfunctory.

        switch -- $tmpline {
            "<argdecls>" {
                for { set i 0 } { $i < $nargs } { incr i } {
                    switch -- $argtype($i) {
                        "PLINT&" {
                            puts $GENFILE "    PLINT $argname($i);"
                        }
                        "PLUNICODE&" {
                            puts $GENFILE "    PLUNICODE $argname($i);"
                        }
                        "PLFLT&" {
                            puts $GENFILE "    PLFLT $argname($i);"
                        }
                        "char&" {
                            puts $GENFILE "    char $argname($i);"
                        }
                        "PLINT *" {
                            puts $GENFILE "    PLINT *$argname($i);"
                            puts $GENFILE "    tclMatrix *mat$argname($i);"
                        }
                        "PLUNICODE *" {
                            puts $GENFILE "    PLUNICODE *$argname($i);"
                            puts $GENFILE "    tclMatrix *mat$argname($i);"
                        }
                        "PLFLT *" {
                            puts $GENFILE "    PLFLT *$argname($i);"
                            puts $GENFILE "    tclMatrix *mat$argname($i);"
                        }
                        "const char *" {
                            puts $GENFILE "    const char *$argname($i);"
                        }
                        "char *" {
                            puts $GENFILE "    char $argname($i)\[200\];"
                        }
                        default {
                            if { $argdef($i) != "" } {
                                puts $GENFILE "    $argtype($i) $argname($i) = $argdef($i);"
                            } else {
                                puts $GENFILE "    $argtype($i) $argname($i);"
                            }
                        }
                    }
                }
            }
            "<consistency>" {
                puts $GENFILE $argchk
            }
            "<getargs>" {
                # Obtain the arguments which we need to pass to the PLplot API call,
                # from the argc/argv list passed to the Tcl command proc.  Each
                # supported argument type will need special handling here.

		set round 0
		if { $nredacted == 0 } {
		    # For nredacted = 0, all arguments are ordinary
		    # and a call to process_args with round == 0 processes all of them.

                    set indent ""
		    # Note arrays must be passed by name.
		    process_args $GENFILE $round $ndefs $indent $nredacted $nargs argred argref argtype argname
		} else {
		    # For nredacted > 0, there are three (if_non_redacted !=0) or
		    # two (if_non_redacted == 0) rounds of calls to process_args
		    if { $if_non_redacted } {
			set indent "    "
			puts $GENFILE "    if (argc == 1+$nargs) \{"
			# round == 0 processes both ordinary and redacted
			# arguments to generate non-redacted API.
			process_args $GENFILE $round $ndefs $indent $nredacted $nargs argred argref argtype argname
			puts $GENFILE "    \} else \{"
		    } else {
			set indent ""
		    }
		    incr round
		    # round == 1 skips processing all redacted
		    # arguments so just handles generating the
		    # redacted API for ordinary arguments.
		    process_args $GENFILE $round $ndefs $indent $nredacted $nargs argred argref argtype argname
		    incr round
		    # round == 2 skips processing all ordinary
		    # arguments so just handles generating the
		    # redacted API for redacted arguments.
		    process_args $GENFILE $round $ndefs $indent $nredacted $nargs argred argref argtype argname
		    if { $if_non_redacted } {
			puts $GENFILE "    \}"
		    }
                }
            }

            # Call the PLplot API function.

            "<plcmd>" {
                puts -nonewline $GENFILE "    $cmd ( "
                for { set i 0 } { $i < $nargs } { incr i } {
                    if { [string match "*&" $argtype($i)] } {
                           puts -nonewline $GENFILE "&$argname($i)"
                    } else {
                        puts -nonewline $GENFILE "$argname($i)"
                    }

                    if { $i < $nargs-1 } {
                       puts -nonewline $GENFILE ", "
                    }
                 }
                 puts $GENFILE " );"
             }

             # If there were reference arguments, fetch their contents and stuff them
             # into their corresponding Tcl variables.

             # NOTE: This should be improved so take into account the current value
             # of tcl_precision.

             "<fetch_result>" {
                 if { $refargs > 0 } {
                     for { set i 0 } { $i < $nargs } { incr i } {
                         if { ! $argref($i) } {
                             continue
                         }
                         if { $i > 0 } {
                             puts $GENFILE "    if (argc == 1)"
                             puts $GENFILE "        Tcl_AppendResult( interp, \" \", (char *) NULL );"
                         }
                         switch -- $argtype($i) {
                             "PLINT&" {
                                 puts $GENFILE "    sprintf( buf, \"%d\", $argname($i) );"
                                 puts $GENFILE "    if (argc > 1)"
                                 puts $GENFILE "        Tcl_SetVar( interp, argv\[1+$i\], buf, 0 );"
                                 puts $GENFILE "    else"
                                 puts $GENFILE "        Tcl_AppendResult( interp, buf, (char *) NULL );"
                             }

                             "PLUNICODE&" {
                                 puts $GENFILE "    sprintf( buf, \"%u\", $argname($i) );"
                                 puts $GENFILE "    if (argc > 1)"
                                 puts $GENFILE "        Tcl_SetVar( interp, argv\[1+$i\], buf, 0 );"
                                 puts $GENFILE "    else"
                                 puts $GENFILE "        Tcl_AppendResult( interp, buf, (char *) NULL );"
                             }

                             "char *" {
                                 puts $GENFILE "    if (argc > 1)"
                                 puts $GENFILE "       Tcl_SetVar( interp, argv\[1+$i\], $argname($i), 0 );"
                                 puts $GENFILE "    else"
                                 puts $GENFILE "        Tcl_AppendResult( interp, $argname($i), (char *) NULL );"
                             }

                             "char&" {
                                 puts $GENFILE "    sprintf( buf, \"%c\", $argname($i) );"
                                 puts $GENFILE "    if (argc > 1)"
                                 puts $GENFILE "        Tcl_SetVar( interp, argv\[1+$i\], buf, 0 );"
                                 puts $GENFILE "    else"
                                 puts $GENFILE "        Tcl_AppendResult( interp, buf, (char *) NULL );"
                             }

                             # The following needs to be corrected to work with the Tcl
                             # precision standard (global var tcl_precision).

                             "PLFLT&" {
                                 puts $GENFILE "    Tcl_PrintDouble( interp, $argname($i), buf );"
                                 puts $GENFILE "    if (argc > 1)"
                                 puts $GENFILE "        Tcl_SetVar( interp, argv\[1+$i\], buf, 0 );"
                                 puts $GENFILE "    else";
                                 puts $GENFILE "        Tcl_AppendResult( interp, buf, (char *) NULL );"
                             }

                             default {
                                 puts $GENFILE "Unsupported arg type."
                             }
                         }
                     }
                 }
             }
             default {

                 # substitutions here...
                 set line [string map [list %full_command% $full_command %cmd% $cmd %nargs% $nargs %nredacted% $nredacted %ndefs% $ndefs %isref% $refargs %if_non_redacted $if_non_redacted] $tmpline]

                 puts $GENFILE $line
             }
        }
    }

    close $TEMPLATE
}

# main code --
#
set verbose [expr {[lsearch $argv "-v"] >= 0}]

set if_non_redacted 0 ;# Whether to generate non-redacted API when nredacted > 0?

# Find the source tree directory that must be specified on the command line.
set sourcedir [lindex $argv 0]                    ;# Get the current source directory - for "out of source tree builds"
set specfile  [file join $sourcedir "plapi.tpl"]  ;# PLplot API template specification file.
set genfile   "tclgen.c"                          ;# Generated functions go here.
set genhead   "tclgen.h"                          ;# Prototypes for generated functions.
set genstruct "tclgen_s.h"                        ;# Initializers for CmdInfo struct.
set cmdfile   [file join $sourcedir "tclcmd.tpl"] ;# Template file for generated functions.

set SPECFILE  [open $specfile]
set GENFILE   [open $genfile "w"]
set GENHEAD   [open $genhead "w"]
set GENSTRUCT [open $genstruct "w"]

# Scan the PLplot API template specification file looking for function
# "prototypes".  These are introduced with the token "pltclcmd".  When
# we find one, go process it.  Anything other than a comment or a
# valid function "prototype" is considered an error, and is printed to
# stdout.

while { [gets $SPECFILE line] >= 0 } {
    regsub {#.*$} $line {} line
    if { $line == "" } continue

    if { [regexp {^pltclcmd (\w+) (.*)} $line ==> cmd rtype] } {
        process_pltclcmd $cmd $rtype
        continue
    }

# Just print the unrecognized output to stdout.

    puts "? $line"
}
