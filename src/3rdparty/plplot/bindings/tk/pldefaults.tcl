# Copyright (C) 2004 Maurice LeBrun
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
#
#----------------------------------------------------------------------------
# Sets default configuration options for plplot/TK driver.
# Maurice LeBrun
# IFS, University of Texas
#
# It is very easy to customize plplot/TK settings for a particular site
# or user.  The steps are:
#
# 1. Create a directory for holding app-defaults files.  $HOME/app-defaults/
#    is a good choice.  Then set the XAPPLRESDIR environmental to this
#    pathname (keep trailing slash).
#
# 2. Create a plplot app-defaults file by the name of PLplot or plplot (PLplot
#    is loaded last and thus has preference, if both exist).  It should use
#    standard X11 resource syntax.
#
# 3. If you wish to load resources based on a different class name, you should
#    call loadAppDefaults with suitable arguments after the pldefaults call.
#
# The older syntax using the plconfig.tcl file may still work but is
# deprecated.
#
# BUGS:
#  - should probably replace some of the global variables with resources.
#
# Use something like this for widget font settings:
# option add Tk.BoldFont "*-lucida sans-Bold-R-Normal-*-100-*" widgetDefault
#
#----------------------------------------------------------------------------

# From the TK FAQ, and modified to taste.

# pl_getenv varName
#   Looks up the environment variable named $varName and returns its value
#   OR {} if it does not exist

proc pl_getenv varName {
    global env
    if {[info exists env($varName)]} { return $env($varName) }
}

# loadAppDefaults classNameList ?priority?
#   Searches for the app-default files corresponding to classNames in
#   the order specified by X Toolkit Intrinsics, and loads them with
#   the priority specified (default: startupFile).  Stops after first
#   file found for each class.

proc pl_loadAppDefaults {classNameList {priority startupFile}} {
    set filepath "[split [pl_getenv XUSERFILESEARCHPATH] :] \
                  [pl_getenv XAPPLRESDIR] \
                  [split [pl_getenv XFILESEARCHPATH] :] \
                  /usr/lib/X11"
    foreach i $classNameList {
	foreach j $filepath {
	    if {[file exists $j/$i]} {
		option readfile $j/$i $priority; break
	    }
	}
    }
}

#----------------------------------------------------------------------------
# Called by plplot based Tcl apps at startup to set resources.

proc pldefaults {} {
    #puts "loading library defaults"
    pl_libdefaults
    #puts "loading user defaults"
    pl_loadAppDefaults {plplot PLplot} userDefault
}

#----------------------------------------------------------------------------
# Default resources.
# This is largely based on options.motif.tk from Ioi K Lam's Tix package.
# The TK options are set to be somewhat Motif-like.

proc pl_libdefaults {} {
    global tk_version
    global gen_font
    global gen_bold_font
    global gen_menu_font
    global gen_italic_font
    global gen_font_small
    global gen_bold_font_small
    global gen_fixed_font

    global dialog_font
    global dialog_bold_font

# Font-related resources.

    set gen_font		-*-helvetica-medium-r-normal-*-*-180-*
    set gen_bold_font		-*-helvetica-bold-r-normal-*-*-180-*
    set gen_menu_font		-*-helvetica-medium-o-normal-*-*-180-*
    set gen_italic_font		-*-helvetica-bold-o-normal-*-*-180-*
    set gen_font_small		-*-helvetica-medium-r-normal-*-*-120-*
    set gen_bold_font_small	-*-helvetica-bold-r-normal-*-*-120-*
    set gen_fixed_font		-*-courier-medium-r-normal-*-*-180-*

    set dialog_font		-*-times-medium-r-normal-*-*-180-*
    set dialog_bold_font	-*-times-bold-r-normal-*-*-180-*

#----------------------------------------------------------------------------
# Color-related resources.
# Sort of Motif-y.
# It doesn't actually hurt to do this if not on a color system.

    set gen_bg		lightgray
    set gen_fg		black
    set gen_darker_bg	gray
    set gen_darker_fg	black
    set gen_active_bg	$gen_bg
    set gen_active_fg	$gen_fg

    global tcl_platform
    if {$tcl_platform(platform) == "unix"} {
    # geometry of main window if plstdwin is used
    # "auto" means derive size from root window dimensions
	option add *geometry		auto			startupFile

    # fonts
	option add *font		$gen_font		startupFile
	option add *Entry.font		$gen_font		startupFile
	option add *Menu*font		$gen_menu_font		startupFile
	option add *Menubutton*font	$gen_menu_font		startupFile
	option add *Scale.font		$gen_bold_font_small	startupFile
	option add *color.font		$gen_fixed_font		startupFile

    # colors
	option add *background			$gen_bg		startupFile
	option add *foreground			$gen_fg		startupFile
	option add *activeBackground		$gen_active_bg	startupFile
	option add *activeForeground		$gen_active_fg	startupFile
	option add *disabledForeground		gray45		startupFile
	option add *Checkbutton.selector	yellow		startupFile
	option add *Radiobutton.selector	yellow		startupFile
	#option add *Entry.background		#c07070		startupFile
	option add *Entry.foreground		black		startupFile
	option add *Entry.insertBackground	black		startupFile
	option add *Listbox.background		$gen_darker_bg	startupFile
	option add *Scale.foreground		$gen_fg		startupFile
	option add *Scale.activeForeground	$gen_bg		startupFile
	option add *Scale.background		$gen_bg		startupFile
	option add *Scale.sliderForeground	$gen_bg		startupFile
	option add *Scale.sliderBackground	$gen_darker_bg	startupFile
	option add *Scrollbar.background	$gen_bg		startupFile
	option add *Scrollbar.troughColor	$gen_darker_bg	startupFile

    # End of page indicator

	option add *leop.off			$gen_bg		startupFile
	option add *leop.on			gray45		startupFile

    # This specifies the default plplot widget background color.
    # A white background looks better on grayscale or mono.

	if {[winfo depth .] == 1} {
	    option add *Plframe.background	white		startupFile
	} else {
	    option add *Plframe.background	black		startupFile
	}

    #----------------------------------------------------------------------------
    # Miscellaneous

	option add *anchor			w		startupFile
	option add *Button.borderWidth		2		startupFile
	option add *Button.anchor		c		startupFile
	option add *Checkbutton.borderWidth	2		startupFile
	option add *Radiobutton.borderWidth	2		startupFile
	option add *Label.anchor		w		startupFile
	option add *Labelframe.borderWidth	2		startupFile
	option add *Entry.relief		sunken		startupFile
	option add *Scrollbar.relief		sunken		startupFile

    # Preserve aspect ratio on zooming.  Valid values: true|false.
	option add *zoom_fixaspect		true		startupFile

    # Location of initial point when zooming.  Valid values: corner|center.
	option add *zoom_startfrom		center		startupFile

    # Name of default save device.  Valid values are installation-dependent,
    # but you might try one of: ps, psc, plm, png, jpeg.
	option add *save_dev			psc		startupFile

    # Multi plots per file.  If true, must explicitly close it before exiting!
	option add *save_multi			false		startupFile

    # I have this in here so that applications written before Tk 4.0 still
    # look the same.  More selectivity might be better.

	option add *highlightThickness		0		startupFile

    # Have focus follow mouse, only available in Tk 4.0+
    # This is needed if you want to control scales using keystrokes.

	tk_focusFollowsMouse
    }

# Various options -- use global variables for simplicity.

# Scale widget bindings

    bind Scale <Button-1>  {%W set [%W get %x %y] }
    bind Scale <B1-Motion> {%W set [%W get %x %y] }

# Key shortcut definitions -- change them if you want!
# Turn them into an empty string to disable.

    global key_resume;		set key_resume		"Return"
    global key_zoom_select;	set key_zoom_select	"z"
    global key_zoom_back;	set key_zoom_back	"b"
    global key_zoom_forward;	set key_zoom_forward	"f"
    global key_zoom_reset;	set key_zoom_reset	"r"
    global key_print;		set key_print		"P"
    global key_save_again;	set key_save_again	"s"
    global key_scroll_right;	set key_scroll_right	"Right"
    global key_scroll_left;	set key_scroll_left	"Left"
    global key_scroll_up;	set key_scroll_up	"Up"
    global key_scroll_down;	set key_scroll_down	"Down"
    global key_scroll_mag;	set key_scroll_mag	"5"
    global key_scroll_speed;	set key_scroll_speed	"1"

# enable/disable top plot and file menu

    global file_menu_on;	set file_menu_on "1"
    global plot_menu_on;	set plot_menu_on "1"

    bind . <m> {toggle_menus %W}

    setup_cursorkeybindings
}

proc toggle_menus {w} {
    global file_menu_on plot_menu_on

    if {$file_menu_on} {
	pack forget .menu
	pack forget [winfo parent $w].ftop
	set file_menu_on 0
    } else {
	pack .menu -fill x
	pack [winfo parent $w].ftop -fill x -before [winfo parent $w].plwin
	set file_menu_on 1
    }
}

proc plw_moveCursor {w x y xd yd} {
    # If not an activeplot, then return.
    if {[catch {set activeplot [$w cget -activeplot]}]} {return}
    if {!$activeplot} {return}
    incr x $xd
    incr y $yd
    # Move the cursor as directed.
    event generate $w <Motion> -warp 1 -x $x -y $y
}

# Set up cursor-keybindings so that they can be used to
# manipulate the pointer on top of active Plframes.
proc setup_cursorkeybindings {} {
    foreach combo {
	{}
	Shift Option Control
	{Shift Option} {Shift Control} {Option Control}
	{Shift Option Control}
    } {
	set multiply 1
	for {set i 0} {$i < [llength $combo]} {incr i} {
	    set multiply [expr {$multiply * 5}]
	}
	if {[llength $combo]} {
	    set prefix "[join $combo -]-"
	} else {
	    set prefix ""
	}
	foreach dir {Left Right Up Down} x {-1 1 0 0} y {0 0 -1 1} {
	    bind Plframe <${prefix}$dir> "plw_moveCursor %W %x %y\
		  [expr {$x * $multiply}] [expr {$y * $multiply}]"
	}
    }
}
