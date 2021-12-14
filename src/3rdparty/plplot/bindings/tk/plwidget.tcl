#----------------------------------------------------------------------------
# PLPLOT TK/TCL graphics renderer
# plplot window initialization procs
#
# Maurice LeBrun
# IFS, University of Texas at Austin
# 29-May-1993
#
# Note: to keep namespace problems to a minimum, all procs defined here begin
# with "pl".  These are further subdivided into "plw::" for button- or
# menu-accessible commands, or "pl_" for utility commands.
#
# anything in 'plw::' should really be considered private.  If they are
# needed externally (i.e. in user code) then we should rename them or expose
# them.
#----------------------------------------------------------------------------

# For backwards compatibility
proc plw_create {args} { uplevel 1 plw::create $args }

#----------------------------------------------------------------------------
# plw::create
#
# Front-end routine to create plplot megawidget for use from PLplot tk
# driver.  Right now does nothing special.
#----------------------------------------------------------------------------

namespace eval plw {}

proc plw::create {w {client_id {}}} {
    plxframe $w $client_id
    wm title . [string trim $w .]
}

#----------------------------------------------------------------------------
# plr_create
#
# A front-end to plw::create, used by plrender.
#----------------------------------------------------------------------------

proc plr_create {w {client_id {}}} {
    global is_plrender; set is_plrender 1
    plw::create $w $client_id
}

#----------------------------------------------------------------------------
# plxframe
#
# Creates the "extended" plframe widget.  Eventually may be replaced with
# a real megawidget capability, using itcl.  The actual plframe widget
# is named $w.plwin.  Example usage:
#
# plxframe .plw
# pack append . .plw {bottom fill expand}
#
# The PLplot/TK (or DP) driver works by fork/exec of a plserver (an
# extended wish), and subsequent communication of graphics instructions
# data from the driver via a FIFO or socket.  In this case the client_id
# variable must be specified in the call.  In direct widget instantiation
# the client_id variable should not be used.
#----------------------------------------------------------------------------

proc plxframe {w {client_id {}}} {

# Note the window name w must never be a global.
    global client plot_menu_on

# Save client name

    if {$client_id != ""} then {
	set client $client_id
    }

# Make container frame.  It is mapped later.

# If the container already exists, don't exit

    catch "frame $w"

# Create child plplot widget (plframe), and pack into parent.

    plframe $w.plwin -relief sunken
    pack append $w \
	$w.plwin {bottom expand fill}

# Set up defaults

    plw::setup_defaults $w

# Make frame for top row widgets.
# plframe widget must already have been created (the plframe is queried
# for a list of the valid output devices for page dumps).

    if $plot_menu_on then {
	plw::create_TopRow $w
	pack append $w \
	    $w.ftop {top fill}
	update
    }

# Enable keyboard traversal when widget has the input focus.
# Also grab the initial input focus.

    tk_bindForTraversal $w.plwin
    focus $w.plwin

# Set up bop/eop signal and inform client of plplot widget name for widget
# commands.

    if { [info exists client] } then {
	if $plot_menu_on then {
	    set bop_col [option get $w.ftop.leop off Label]
	    set eop_col [option get $w.ftop.leop on Label]

	    $w.plwin configure -bopcmd "plw::flash $w $bop_col"
	    $w.plwin configure -eopcmd "plw::flash $w $eop_col"

	} else {
	    $w.plwin configure -bopcmd {update}
	    $w.plwin configure -eopcmd {update}
	}

    # Resize binding -- just experimental for now.
    #	bind $w.plwin <Configure> "client_cmd \"plfinfo %w %h\""

	client_cmd "set plwidget $w.plwin"

    } else {

# Set up bop/eop handling when running tcl scripts from a plserver

	global plstate_bopseen; set plstate_bopseen($w) 0
	global plstate_pause; set plstate_pause($w) 1
	$w.plwin configure -bopcmd "plw::bop $w"
	$w.plwin configure -eopcmd "plw::eop $w"
    }

    return $w
}

#----------------------------------------------------------------------------
# plw::setup_defaults
# plw::set_zoom_handler
# plw::set_pause
#
# Set up default settings.
#----------------------------------------------------------------------------

proc plw::setup_defaults {w} {

# In the two cases below, the options can be overridden through the global
# variables zoomopt_0, etc, and saveopt_0, etc.  Only for backward
# compatibility, since options are a much better way of handling this.

# zoom options:
#  0:	0=don't preserve aspect ratio, 1=do
#  1:	0=stretch from corner, 1=stretch from center

    global zoomopts zoomopt_0 zoomopt_1

# Get zoom options from options db
    set zoom_fixaspect [option get $w zoom_fixaspect {}]
    set zoom_startfrom [option get $w zoom_startfrom {}]

# Note: legacy default behavior is both "1"
# Use a negative condition so that if you bogotify your option value, the
# default behavior is retained.
    set zoomopts($w,0) [expr { "$zoom_fixaspect" != "false" }]
    set zoomopts($w,1) [expr { "$zoom_startfrom" != "corner" }]
    if { [info exists zoomopt_0] } {set zoomopts($w,0) $zoomopt_0}
    if { [info exists zoomopt_1] } {set zoomopts($w,1) $zoomopt_1}

# save options:
#  0:   name of default save device
#  1:   0=save 1 plot/file, 1=save multi plots/file (must close!)

    global saveopts saveopt_0 saveopt_1

# Get save options from options db
    set save_dev   [option get $w save_dev {}]
    set save_multi [option get $w save_multi {}]

    set saveopts($w,0) $save_dev
    set saveopts($w,1) [expr { "$save_multi" != "false" }]
    if { [info exists saveopt_0] } {set saveopts($w,0) $saveopt_0}
    if { [info exists saveopt_1] } {set saveopts($w,1) $saveopt_1}

# Set up zoom windows list

    global zidx zidx_max zxl zyl zxr zyr

    set zidx($w) 0
    set zidx_max($w) 0
    set zxl($w,0) 0.0
    set zyl($w,0) 0.0
    set zxr($w,0) 1.0
    set zyr($w,0) 1.0

# Bindings

    bind $w.plwin <Any-KeyPress> \
	"plw::key_filter $w %N %s %x %y %K %A"

    bind $w.plwin <Any-ButtonPress> \
	"plw::user_mouse $w %b %s %x %y"

    bind $w.plwin <B1-Motion> \
	"plw::user_mouse $w %b %s %x %y"

    bind $w.plwin <B2-Motion> \
	"plw::user_mouse $w %b %s %x %y"

    bind $w.plwin <B3-Motion> \
	"plw::user_mouse $w %b %s %x %y"

    bind $w.plwin <Any-Enter> \
	"focus $w.plwin"
}

proc plw::set_zoom_handler {w zoom_handler} {
    global user_zoom_handler
    set user_zoom_handler($w) $zoom_handler
}

proc plw::set_pause {w pause} {
    global plstate_pause

    set plstate_pause($w) $pause
}

#----------------------------------------------------------------------------
# plw::create_TopRow
#
# Create top row widgets.  Page-oriented widgets only have a meaning in
# the context of the PLplot driver, so don't create them if there is no
# client (as occurs for direct widget instantiation).
#----------------------------------------------------------------------------

proc plw::create_TopRow {w} {
    global is_plrender client

    frame $w.ftop

# End of page indicator

    if { [info exists client] } then {
	pack append $w.ftop \
	    [label $w.ftop.leop -relief raised] \
	    {left fill padx 12}

	$w.ftop.leop config -bg [option get $w.ftop.leop on Label]
    }

# Plot menu

    pack append $w.ftop \
	[plw::create_pmenu $w $w.ftop.pmenu] \
	{left fill padx 12}

# Forward and backward (plrender only) page buttons.
# Just a hack until I get around to doing it right.

    if { [info exists client] } then {
	if { [info exists is_plrender] } {
	    pack append $w.ftop \
		[button $w.ftop.bp -text "<<" -relief raised] \
		{left fill padx 10}

	    $w.ftop.bp configure -command \
		"client_cmd {keypress 65288 0 0 0 0. 0. BackSpace}"
	}

	pack append $w.ftop \
	    [button $w.ftop.fp -text ">>" -relief raised] \
	    {left fill padx 10}

	$w.ftop.fp configure -command \
	    "client_cmd {keypress 65293 0 0 0 0. 0. Return}"
    }

# Label widget for status messages.

    label $w.ftop.lstat -anchor w -relief raised
    plw::label_push $w "[string range $w 1 end]"
    pack append $w.ftop $w.ftop.lstat \
	{right expand fill}
}

#----------------------------------------------------------------------------
# plw::create_pmenu
#
# Create plot menu.
#
# It is tempting to create buttons for some of these options, but buttons
# are difficult to effectively place and extend.  Menus have a clear
# placement mechanism and are easy to add to.  Further, TK menus can be
# torn off (select menu with middle mouse button and move to where you
# want it) which makes selecting top-level menu buttons easy.  Finally,
# certain menu options have keyboard equivalents: zoom-select (z),
# zoom-reset (r), print (P), and save-again (s).
#----------------------------------------------------------------------------

proc plw::create_pmenu {w pmbut} {
    global pmenu; set pmenu($w) $pmbut.m

    menubutton $pmbut -text "Plot" -menu $pmenu($w) -relief raised
    menu $pmenu($w)

    plw::create_pmenu_print   $w
    plw::create_pmenu_save    $w
    plw::create_pmenu_orient  $w
    plw::create_pmenu_zoom    $w
    plw::create_pmenu_page    $w
    plw::create_pmenu_palettes $w
    plw::create_pmenu_help    $w
    plw::create_pmenu_exit    $w

    return $pmbut
}

#----------------------------------------------------------------------------
# plw::create_pmenu_exit
#----------------------------------------------------------------------------

proc plw::create_pmenu_exit {w} {
    global pmenu

    $pmenu($w) add command -label "Exit" \
	-command exit
}

#----------------------------------------------------------------------------
# plw::create_pmenu_help
#----------------------------------------------------------------------------

proc plw::create_pmenu_help {w} {
    global pmenu

    $pmenu($w) add command -label "Help" \
	-command "help_keys"
}

#----------------------------------------------------------------------------
# plw::create_pmenu_print
#
# Create plot-print menu
#----------------------------------------------------------------------------

proc plw::create_pmenu_print {w} {
    global pmenu

    $pmenu($w) add command -label "Print" \
	-command "plw::print $w"
}

#----------------------------------------------------------------------------
# plw::create_pmenu_save
#
# Create plot-save menu (cascade)
#----------------------------------------------------------------------------

proc plw::create_pmenu_save {w} {
    global pmenu; set m $pmenu($w).save

    $pmenu($w) add cascade -label "Save" -menu $m
    menu $m

# Save - As

    $m add command -label "As" \
	-command "plw::save_as $w"

# Save - Again

    $m add command -label "Again" \
	-command "plw::save_again $w" \
	-state disabled

# Save - Close

    $m add command -label "Close" \
	-command "plw::save_close $w" \
	-state disabled

    $m add separator

# Save - Set device.. (another cascade)

    $m add cascade -label "Set device" -menu $m.sdev
    menu $m.sdev

# Generate the device list in the "Save/Set device" widget menu, by querying
# the plframe widget for the available output devices (which are listed).

    global saveopts

    set devnames [$w.plwin info devnames]
    set devkeys [$w.plwin info devkeys]
    set ndevs [llength $devnames]
    for {set i 0} {$i < $ndevs} {incr i} {
	set devnam [lindex $devnames $i]
	set devkey [lindex $devkeys $i]

	$m.sdev add radio -label $devnam \
	    -variable saveopts($w,0) -value $devkey
    }

# Save - Set file type.. (another cascade)

    $m add cascade -label "Set file type" -menu $m.sfile
    menu $m.sfile

# Single file (one plot/file)

    $m.sfile add radio -label "Single file (one plot/file)" \
	-variable saveopts($w,1) -value 0

# Archive file (many plots/file)

    $m.sfile add radio -label "Archive file (many plots/file)" \
	-variable saveopts($w,1) -value 1
}

#----------------------------------------------------------------------------
# plw::create_pmenu_orient
#
# Create plot-orient menu (cascade)
#----------------------------------------------------------------------------

proc plw::create_pmenu_orient {w} {
    global pmenu; set m $pmenu($w).orient

    $pmenu($w) add cascade -label "Orient" -menu $m
    menu $m

    $m configure -postcommand "plw::update_orient $w"

# Orient - 0 degrees

    $m add radio -label "0 degrees" \
	-command "plw::orient $w 0"

# Orient - 90 degrees

    $m add radio -label "90 degrees" \
	-command "plw::orient $w 1"

# Orient - 180 degrees

    $m add radio -label "180 degrees" \
	-command "plw::orient $w 2"

# Orient - 270 degrees

    $m add radio -label "270 degrees" \
	-command "plw::orient $w 3"
}

#----------------------------------------------------------------------------
# plw::create_pmenu_zoom
#
# Create plot-zoom menu (cascade)
#----------------------------------------------------------------------------

proc plw::create_pmenu_zoom {w} {
    global pmenu; set m $pmenu($w).zoom

    $pmenu($w) add cascade -label "Zoom" -menu $m
    menu $m

    $m configure -postcommand "plw::update_zoom $w"

# Zoom - select (by mouse)

    $m add command -label "Select" \
	-command "plw::zoom_select $w"

# Zoom - back (go back 1 zoom level)

    $m add command -label "Back" \
	-command "plw::zoom_back $w" \
	-state disabled

# Zoom - forward (go forward 1 zoom level)

    $m add command -label "Forward" \
	-command "plw::zoom_forward $w" \
	-state disabled

# Zoom - enter bounds

    $m add command -label "Enter bounds.." \
	-command "plw::zoom_enter $w"

# Zoom - reset

    $m add command -label "Reset" \
	-command "plw::zoom_reset $w"

# Zoom - options (another cascade)

    $m add cascade -label "Options" -menu $m.options
    menu $m.options

    global zoomopts
    $m.options add check -label "Preserve aspect ratio" \
	-variable zoomopts($w,0)

    $m.options add separator

    $m.options add radio -label "Start from corner" \
	-variable zoomopts($w,1) -value 0

    $m.options add radio -label "Start from center" \
	-variable zoomopts($w,1) -value 1

    $m.options invoke [plw::entry 1]
}

#----------------------------------------------------------------------------
# plw::create_pmenu_page
#
# Create plot-page menu (cascade)
#----------------------------------------------------------------------------

proc plw::create_pmenu_page {w} {
    global pmenu; set m $pmenu($w).page

    $pmenu($w) add cascade -label "Page" -menu $m
    menu $m

# Page - enter bounds

    $m add command -label "Setup.." \
	-command "plw::page_enter $w"

# Page - reset

    $m add command -label "Reset" \
	-command "plw::page_reset $w"
}

#----------------------------------------------------------------------------
# plw::create_pmenu_redraw
#
# Create plot-redraw menu
# I only use this for debugging in cases where the normal redraw capability
# isn't working right.
#----------------------------------------------------------------------------

proc plw::create_pmenu_redraw {w} {
    global pmenu

    $pmenu($w) add command -label "Redraw" \
	-command "$w.plwin redraw"
}

#----------------------------------------------------------------------------
# plw::create_pmenu_palettes
#
# Create plot-palettes menu (cascade)
#----------------------------------------------------------------------------

proc plw::create_pmenu_palettes {w} {
    global pmenu; set m $pmenu($w).palettes
    global plopt_static_redraw plopt_dynamic_redraw pl_itcl_package_name

# The palette tools require Itcl 3.0 or later.

    if [catch {eval package require $pl_itcl_package_name}] { return }

    $pmenu($w) add cascade -label "Palettes" -menu $m
    menu $m

# Set up redraw variables.  Basically if you have r/w colorcells (e.g.
# PseudoColor visual, not sure if any others), you don't need either of
# these -- they are updated automatically.  Otherwise (e.g. TrueColor), you
# definitely want static_redraw set and probably dynamic_redraw.  The latter is
# very cpu intensive as it redraws the plot every time you move one of the
# sliders, similar to a zoom/pan.

# Note: it would be better to reach down to the X driver to get the info on
# whether we have r/w colorcells to set the default, but this procedure is a
# lot easier and almost as good.

# See if we have a visual capable of r/w colorcells.

    set rwcolorcells 0
    set visual [winfo visual $w]
    if { $visual == "pseudocolor" } {
	set rwcolorcells 1
    }

    if $rwcolorcells {
	set plopt_static_redraw($w) 0
	set plopt_dynamic_redraw($w) 0
    } else {
	set plopt_static_redraw($w) 1
	set plopt_dynamic_redraw($w) 1
    }

# Set up palette tools

    $m add command -label "Palette 0" \
	-command "plcmap0_edit $w.plwin $w"

    $m add command -label "Palette 1" \
	-command "plcmap1_edit $w.plwin $w"

# Palettes - options (another cascade)

    $m add cascade -label "Options" -menu $m.options
    menu $m.options

# Setup checkboxes for dynamic/static redraws.  Eventually a resource setting
# will be used to allow overrides that way too, but for now this will do.

    $m.options add check -label "Enable static plot redraws" \
	-variable plopt_static_redraw($w)

    $m.options add check -label "Enable dynamic plot redraws" \
	-variable plopt_dynamic_redraw($w)

# Set up traces to force the following logical relationship:
#
#  dynamic_redraw ==> static_redraw
#
# and its contrapositive.

    trace variable plopt_static_redraw($w) w plw::pmenu_palettes_checkvars
    trace variable plopt_dynamic_redraw($w) w plw::pmenu_palettes_checkvars
}

proc plw::pmenu_palettes_checkvars {var w op} {
    global plopt_static_redraw plopt_dynamic_redraw
    if { $var == "plopt_dynamic_redraw" } {
	if $plopt_dynamic_redraw($w) { set plopt_static_redraw($w) 1 }
    }
    if { $var == "plopt_static_redraw" } {
	if !$plopt_static_redraw($w) { set plopt_dynamic_redraw($w) 0 }
    }
}

#----------------------------------------------------------------------------
# plw::entry
#
# Compensates for tk3.6 -> tk4.0 lunacy, where the menu entries are all
# incremented by 1!  This happens because of the tear-off entry.  You can
# create the menu without a tear-off, but that's even more intrusive.
#----------------------------------------------------------------------------

proc plw::entry {entry} {
    global tk_version
    if { $tk_version >= 4.0 } then {
	incr entry
    }
    return $entry
}

#----------------------------------------------------------------------------
# plw::start
#
# Responsible for plplot graphics package initialization on the widget.
# People driving the widget directly should just use pack themselves.
#
# Put here to reduce the possibility of a time out over a slow network --
# the client program waits until the variable widget_is_ready is set.
#----------------------------------------------------------------------------

proc plw::start {w} {
    global client

# Manage widget hierarchy

    pack append [winfo parent $w] $w \
	{bottom expand fill}

    update

# Inform client that we're done.

    if { [info exists client] } then {
	client_cmd "set widget_is_ready 1"
    }

# Call a user supplied routine to do any necessary post initialization
   catch after_plw::start
}

#----------------------------------------------------------------------------
# plw::key_filter
#
# Front-end to key handler.
# For supported operations it's best to modify the global key variables
# to get the desired action.  More advanced stuff can be done with the
# $user_key_filter proc.  Find anything particularly useful?  Let me know,
# so it can be added to the default behavior.
#----------------------------------------------------------------------------

proc plw::key_filter {w keycode state x y keyname ascii} {
    global user_key_filter

    global key_resume
    global key_zoom_select
    global key_zoom_back
    global key_zoom_forward
    global key_zoom_reset
    global key_print
    global key_save_again
    global key_scroll_right
    global key_scroll_left
    global key_scroll_up
    global key_scroll_down

    global plstate_resume

    #puts "keypress: $keyname $keycode $ascii $state"

# Backward compatibility

    if ![info exists key_resume] {
	set key_resume		"Return"
    }

# Call user-defined key filter, if one exists

    if { [info exists user_key_filter] } then {
	$user_key_filter $w $keyname $keycode $ascii
    }

# Interpret keystroke

    switch $keyname \
	$key_resume		"set plstate_resume($w) 1" \
	$key_zoom_select	"plw::zoom_select $w" \
	$key_zoom_back		"plw::zoom_back $w" \
	$key_zoom_forward	"plw::zoom_forward $w" \
	$key_zoom_reset		"plw::zoom_reset $w" \
	$key_print		"plw::print $w" \
	$key_save_again		"plw::save_again $w" \
	$key_scroll_right	"plw::view_scroll $w  1  0 $state" \
	$key_scroll_left	"plw::view_scroll $w -1  0 $state" \
	$key_scroll_up		"plw::view_scroll $w  0 -1 $state" \
	$key_scroll_down	"plw::view_scroll $w  0  1 $state"

# Pass keypress event info back to client.

    plw::user_key $w $keycode $state $x $y $keyname $ascii
}

#----------------------------------------------------------------------------
# plw::user_key
#
# Passes keypress event information back to client.
# Based on plw::user_mouse.
#----------------------------------------------------------------------------

proc plw::user_key {w keycode state x y keyname ascii} {
    global client

    if { [info exists client] } then {

    # calculate relative window coordinates.

	set xw [expr "$x / [winfo width $w.plwin]."]
	set yw [expr "1.0 - $y / [winfo height $w.plwin]."]

    # calculate normalized device coordinates into original window.

	set view [$w.plwin view]
	set xrange [expr "[lindex $view 2] - [lindex $view 0]"]
	set xnd [expr "($xw * $xrange) + [lindex $view 0]"]
	set yrange [expr "[lindex $view 3] - [lindex $view 1]"]
	set ynd [expr "($yw * $yrange ) + [lindex $view 1]"]

    # send them back to the client.

#	puts "keypress $keycode $state $x $y $xnd $ynd $keyname $ascii"
	client_cmd \
	    [list keypress $keycode $state $x $y $xnd $ynd $keyname $ascii]
    }
}

#----------------------------------------------------------------------------
# plw::user_mouse
#
# Passes buttonpress event information back to client.
# Written by Radey Shouman
#----------------------------------------------------------------------------

proc plw::user_mouse {w button state x y} {
    global client

    if { [info exists client] } then {

    # calculate relative window coordinates.

	set xw [expr "$x / [winfo width $w.plwin]."]
	set yw [expr "1.0 - $y / [winfo height $w.plwin]."]

    # calculate normalized device coordinates into original window.

	set view [$w.plwin view]
	set xrange [expr "[lindex $view 2] - [lindex $view 0]"]
	set xnd [expr "($xw * $xrange) + [lindex $view 0]"]
	set yrange [expr "[lindex $view 3] - [lindex $view 1]"]
	set ynd [expr "($yw * $yrange ) + [lindex $view 1]"]

    # send them back to the client.

	client_cmd \
	    [list buttonpress $button $state $x $y $xnd $ynd]
    }
}

#----------------------------------------------------------------------------
# plw::bop
#
# Beginning of page handling when running tcl scripts from plserver.
# To handle multipage plots correctly, we need to pause between plots.
# But "canonical" gui plots that are single page only should have no
# pause.  Therefore we pause here only if the bop follows a previous bop
# without returning to the interpreter's event loop (i.e. the interpreter is
# kept busy processing a tcl script).
#
# Without a visual cue, this feature is probably too confusing, so it's not
# activated if !plot_menu_on.
#----------------------------------------------------------------------------

proc plw::bop {w} {
    global plot_menu_on; if !$plot_menu_on return

    global plstate_resume plstate_bopseen plstate_pause

# There was a previous bop, deal with it.

    if {$plstate_bopseen($w)} {
	update idletasks
	plw::label_push $w "press <Enter> to continue."
	tkwait variable plstate_resume($w)
	plw::label_set $w "working..."
	update idletasks
	set id1 [after idle "plw::label_pop $w; plw::label_refresh $w"]
	set plstate_resume($w) 0

    } else {
	plw::label_set $w "working..."
	update idletasks
	set id1 [after idle "plw::label_refresh $w"]
    }

# Set up for next possible bop, by setting bopseen to 1 but also setting up an
# idle handler to set it back to 0.  If the idle handler is reached first,
# i.e. control is returned to the interpreter before the next time this proc
# is called, there will be no pause.

    set plstate_bopseen($w) $plstate_pause($w)
    set id2 [after idle "global plstate_bopseen; set plstate_bopseen($w) 0"]
}

#----------------------------------------------------------------------------
# plw::eop
#
# End of page handling.
# Was handy during development but right now does nothing.
#----------------------------------------------------------------------------

proc plw::eop {w} {
}

#----------------------------------------------------------------------------
# plw::flash
#
# Set eop button color to indicate page status.
#----------------------------------------------------------------------------

proc plw::flash {w col} {
    $w.ftop.leop config -bg $col
    update idletasks
}

#----------------------------------------------------------------------------
# plw::end
#
# Executed as part of orderly shutdown procedure.  Eventually will just
# destroy the plframe and surrounding widgets, and server will exit only
# if all plotting widgets have been destroyed and it is a child of the
# plplot/TK driver.  Maybe.
#
# The closelink command was added in the hopes of making the dp driver
# cleanup a bit more robust, but doesn't seem to have any effect except
# to slow things down quite a bit.
#----------------------------------------------------------------------------

proc plw::end {w} {
    global dp
#    $w.plwin closelink
    if { $dp } then {
	global list_sock
	close $list_sock
    }
    #destroy $w
    exit
}

#----------------------------------------------------------------------------
# plw::print
#
# Prints plot.  Uses the "plpr" script, which must be set up for your site
# as appropriate.  There are better ways to do it but this way is safest
# for now.
#----------------------------------------------------------------------------

proc plw::print {w} {
    plw::label_set $w "Printing plot..."
    update
    if { [catch "$w.plwin print" foo] } {
	bogue_out "$foo"
	pl_status_reset $w
    } else {
	pl_status_msg $w "Plot printed."
    }
}

#----------------------------------------------------------------------------
# plw::save_as
#
# Saves plot to default device, prompting for file name.
#----------------------------------------------------------------------------

proc plw::save_as {w} {
    global pmenu saveopts
    set file [getSaveFile $saveopts($w,0)]
    if { [string length $file] > 0 } {
	if { [file exists $file] } {
	    if { ! [confirm "File $file already exists.  Are you sure?"] } {
		return
	    }
	}

	plw::label_set $w "Saving plot..."
	update
	if { [catch "$w.plwin save as $saveopts($w,0) $file" foo] } {
	    bogue_out "$foo"
	    plw::label_refresh $w
	} else {
	    pl_status_msg $w "Plot saved."
	}

	if { $saveopts($w,1) == 0 } then {
	    $w.plwin save close
	} else {
	    $pmenu($w).save entryconfigure Again -state normal
	    $pmenu($w).save entryconfigure Close -state normal
	    bogue_out "Warning: archive files must be closed before using"
	}
    } else {
	bogue_out "No file specified"
    }
}

#----------------------------------------------------------------------------
# plw::save_again
#
# Saves plot to an already open file.
#----------------------------------------------------------------------------

proc plw::save_again {w} {
    if { [catch "$w.plwin save" foo] } {
	bogue_out "$foo"
    } else {
	pl_status_msg $w "Plot saved."
    }
}

#----------------------------------------------------------------------------
# plw::save_close
#
# Close archive save file.
#----------------------------------------------------------------------------

proc plw::save_close {w} {
    global pmenu
    if { [catch "$w.plwin save close" foo] } {
	bogue_out "$foo"
    } else {
	pl_status_msg $w "Archive file closed."
	$pmenu($w).save entryconfigure Again -state disabled
	$pmenu($w).save entryconfigure Close -state disabled
    }
}

#----------------------------------------------------------------------------
# plw::update_zoom
#
# Responsible for making sure zoom menu entries are normal or disabled as
# appropriate.  In particular, that "Back" or "Forward" are only displayed
# if it is possible to traverse the zoom windows list in that direction.
#
# Note you should use the entry name as argument to entryconfigure to guard
# against problems in using the numerical index.  This caused problems going
# from Tcl 7.x -> 8.x (I think in 8.x the tear off entry became numbered where
# previously it wasn't).
#----------------------------------------------------------------------------

proc plw::update_zoom {w} {
    global zidx zidx_max zxl zyl zxr zyr
    global pmenu

# Back

    if { $zidx($w) == 0 } then {
	$pmenu($w).zoom entryconfigure Back -state disabled
    } else {
	$pmenu($w).zoom entryconfigure Back -state normal
    }

# Forward

    if { $zidx_max($w) == 0 || $zidx($w) == $zidx_max($w) } then {
	$pmenu($w).zoom entryconfigure Forward -state disabled
    } else {
	$pmenu($w).zoom entryconfigure Forward -state normal
    }
}

#----------------------------------------------------------------------------
# plw::zoom_select
#
# Zooms plot in response to mouse selection.
#----------------------------------------------------------------------------

proc plw::zoom_select {w} {
    global def_button_cmd zoomopts

    set def_button_cmd [bind $w.plwin <ButtonPress>]

    if { $zoomopts($w,1) == 0 } then {
	plw::label_set $w "Click on one corner of zoom region."
    } else {
	plw::label_set $w "Click on center of zoom region."
    }

    bind $w.plwin <ButtonPress> "plw::zoom_start $w %x %y"
}

#----------------------------------------------------------------------------
# plw::zoom_enter
#
# Zooms plot in response to text entry.
#----------------------------------------------------------------------------

proc plw::zoom_enter {w} {
    global fv00 fv01 fv10 fv11
    global fn00 fn01 fn10 fn11

    set coords [$w.plwin view]
    unlist $coords fv00 fv01 fv10 fv11

    set fn00 xmin
    set fn01 ymin
    set fn10 xmax
    set fn11 ymax

    Form2d .e "Enter window coordinates for zoom.  Each coordinate should range from 0 to 1, with (0,0) corresponding to the lower left hand corner."
    tkwait window .e

    plw::view_select $w $fv00 $fv01 $fv10 $fv11
}

#----------------------------------------------------------------------------
# plw::zoom_reset
#
# Resets after zoom.
# Note that an explicit redraw is not necessary since the packer issues a
# resize after the scrollbars are unmapped.
#----------------------------------------------------------------------------

proc plw::zoom_reset {w} {
    global def_button_cmd

    plw::label_refresh $w
    if [info exists def_button_cmd] {
        bind $w.plwin <ButtonPress> $def_button_cmd
    }
    $w.plwin view reset
    if { [winfo exists $w.hbar] && [winfo ismapped $w.hbar] } then {
	pack unpack $w.hbar
    }
    if { [winfo exists $w.vscroll] && [winfo exists $w.vscroll] } then {
	pack unpack $w.vscroll
    }

# Reset zoom windows list

    global zidx zidx_max zxl zyl zxr zyr

    set zidx($w) 0
    set zidx_max($w) 0
    set zxl($w,0) 0.0
    set zyl($w,0) 0.0
    set zxr($w,0) 1.0
    set zyr($w,0) 1.0
}

#----------------------------------------------------------------------------
# plw::update_orient
#
# Responsible for making sure orientation radio buttons are up to date.
#----------------------------------------------------------------------------

proc plw::update_orient {w} {
    global pmenu
    set rot [$w.plwin orient]
    set entry [expr [format "%.0f" $rot] % 4]
    $pmenu($w).orient invoke [plw::entry $entry]
}

#----------------------------------------------------------------------------
# plw::orient
#
# Changes plot orientation.
#----------------------------------------------------------------------------

proc plw::orient {w rot} {
    if { [$w.plwin orient] != $rot} then {
	$w.plwin orient $rot
    }
}

#----------------------------------------------------------------------------
# plw::page_enter
#
# Changes output page parameters (margins, aspect ratio, justification).
#----------------------------------------------------------------------------

proc plw::page_enter {w} {
    global fv00 fv01 fv10 fv11
    global fn00 fn01 fn10 fn11

    set coords [$w.plwin page]
    unlist $coords fv00 fv01 fv10 fv11

    set fn00 mar
    set fn01 aspect
    set fn10 jx
    set fn11 jy

    Form2d .e "Enter page setup parameters.  mar denotes the fractional page area on each side to use as a margin (0 to 0.5).  jx and jy are the fractional justification relative to the center (-0.5 to 0.5).  aspect is the page aspect ratio (0 preserves original aspect ratio)."
    tkwait window .e

    $w.plwin page $fv00 $fv01 $fv10 $fv11
}

#----------------------------------------------------------------------------
# plw::page_reset
#
# Resets page parameters.
#----------------------------------------------------------------------------

proc plw::page_reset {w} {
    $w.plwin page 0. 0. 0. 0.
}

#----------------------------------------------------------------------------
# plw::zoom_start
#
# Starts plot zoom.
#----------------------------------------------------------------------------

proc plw::zoom_start {w wx wy} {
    global def_button_cmd

# Reset ButtonPress binding to the default
    if [info exists def_button_cmd] {
        bind $w.plwin <ButtonPress> $def_button_cmd
    }

# Use a set rather than a push to replace previous zoom message.
    plw::label_set $w "Select zoom region by dragging mouse, then release."

# Initialize plot, set up zoom bindings, and we're done.
    $w.plwin draw init
    bind $w.plwin <B1-Motion>        "plw::zoom_mouse_draw $w $wx $wy %x %y"
    bind $w.plwin <B1-ButtonRelease> "plw::zoom_mouse_end $w $wx $wy %x %y"
}

#----------------------------------------------------------------------------
# plw::zoom_coords
#
# Transforms the initial and final mouse coordinates to either:
#
# opt = 0	device coordinates
# opt = 1	normalized device coordinates
#
# The global variable "zoomopts" is used to determine zoom behavior:
#
# zoomopts($w,0):
#   0	box follows mouse movements exactly
#   1	box follows mouse movements so that aspect ratio is preserved (default)
#
# zoomopts($w,1):
#   0	first and last points specified determine opposite corners
#	of zoom box.
#   1	box is centered about the first point clicked on,
#	perimeter follows mouse	(default)
#
#----------------------------------------------------------------------------

proc plw::zoom_coords {w x0 y0 x1 y1 opt} {
    global zoomopts

    set Lx [winfo width  $w.plwin]
    set Ly [winfo height $w.plwin]

# Enforce boundaries in device coordinate space

    set bounds [$w.plwin view bounds]
    set xmin [expr [lindex "$bounds" 0] * $Lx]
    set ymin [expr [lindex "$bounds" 1] * $Ly]
    set xmax [expr [lindex "$bounds" 2] * $Lx]
    set ymax [expr [lindex "$bounds" 3] * $Ly]

    set x1 [max $xmin [min $xmax $x1]]
    set y1 [max $ymin [min $ymax $y1]]

# Two-corners zoom.

    if { $zoomopts($w,1) == 0 } then {

    # Get box lengths

	set dx [expr $x1 - $x0]
	set dy [expr $y1 - $y0]

	set sign_dx [expr ($dx > 0) ? 1 : -1]
	set sign_dy [expr ($dy > 0) ? 1 : -1]

	set xl $x0
	set yl $y0

    # Constant aspect ratio

	if { $zoomopts($w,0) == 1 } then {

	# Scale factors used to maintain plot aspect ratio

	    set xscale [expr $xmax - $xmin]
	    set yscale [expr $ymax - $ymin]

	# Adjust box size for proper aspect ratio

	    set rx [expr double(abs($dx)) / $xscale]
	    set ry [expr double(abs($dy)) / $yscale]

	    if { $rx > $ry } then {
		set dy [expr $yscale * $rx * $sign_dy]
	    } else {
		set dx [expr $xscale * $ry * $sign_dx]
	    }

	    set xr [expr $xl + $dx]
	    set yr [expr $yl + $dy]

	# Now check again to see if in bounds, and adjust if not

	    if { $xr < $xmin || $xr > $xmax } then {
		if { $xr < $xmin } then {
		    set dx [expr $xmin - $x0]
		} else {
		    set dx [expr $xmax - $x0]
		}
		set rx [expr double(abs($dx)) / $xscale]
		set dy [expr $yscale * $rx * $sign_dy]
	    }

	    if { $yr < $ymin || $yr > $ymax } then {
		if { $yr < $ymin } then {
		    set dy [expr $ymin - $y0]
		} else {
		    set dy [expr $ymax - $y0]
		}
		set ry [expr double(abs($dy)) / $yscale]
		set dx [expr $xscale * $ry * $sign_dx]
	    }
	}

    # Final box coordinates

	set xr [expr $xl + $dx]
	set yr [expr $yl + $dy]

# zoom from center out, preserving aspect ratio

    } else {

    # Get box lengths, adjusting downward if necessary to keep in bounds

	set dx [expr abs($x1 - $x0)]
	set dy [expr abs($y1 - $y0)]

	set xr [expr $x0 + $dx]
	set xl [expr $x0 - $dx]
	set yr [expr $y0 + $dy]
	set yl [expr $y0 - $dy]

	if { $xl < $xmin } then {
	    set dx [expr $x0 - $xmin]
	}
	if { $xr > $xmax } then {
	    set dx [expr $xmax - $x0]
	}
	if { $yl < $ymin } then {
	    set dy [expr $y0 - $ymin]
	}
	if { $yr > $ymax } then {
	    set dy [expr $ymax - $y0]
	}

    # Constant aspect ratio

	if { $zoomopts($w,0) == 1 } then {

	# Scale factors used to maintain plot aspect ratio

	    set xscale [expr $xmax - $xmin]
	    set yscale [expr $ymax - $ymin]

	# Adjust box size for proper aspect ratio

	    set rx [expr double($dx) / $xscale]
	    set ry [expr double($dy) / $yscale]

	    if { $rx > $ry } then {
		set dy [expr $yscale * $rx]
	    } else {
		set dx [expr $xscale * $ry]
	    }

	    set xr [expr $x0 + $dx]
	    set xl [expr $x0 - $dx]
	    set yr [expr $y0 + $dy]
	    set yl [expr $y0 - $dy]

	# Now check again to see if in bounds, and adjust downward if not

	    if { $xl < $xmin } then {
		set dx [expr $x0 - $xmin]
		set rx [expr double($dx) / $xscale]
		set dy [expr $yscale * $rx]
	    }
	    if { $xr > $xmax } then {
		set dx [expr $xmax - $x0]
		set rx [expr double($dx) / $xscale]
		set dy [expr $yscale * $rx]
	    }
	    if { $yl < $ymin } then {
		set dy [expr $y0 - $ymin]
		set ry [expr double($dy) / $yscale]
		set dx [expr $xscale * $ry]
	    }
	    if { $yr > $ymax } then {
		set dy [expr $ymax - $y0]
		set ry [expr double($dy) / $yscale]
		set dx [expr $xscale * $ry]
	    }
	}

    # Final box coordinates

	set xr [expr $x0 + $dx]
	set xl [expr $x0 - $dx]
	set yr [expr $y0 + $dy]
	set yl [expr $y0 - $dy]
    }

# Optional translation to relative device coordinates.

    if { $opt == 1 } then {
	set wxl [expr "$xl / double($Lx)" ]
	set wxr [expr "$xr / double($Lx)" ]
	set wyl [expr "1.0 - $yr / double($Ly)" ]
	set wyr [expr "1.0 - $yl / double($Ly)" ]

    } else {
	set wxr $xl
	set wxl $xr
	set wyr $yl
	set wyl $yr
    }

    return "$wxl $wyl $wxr $wyr"
}

#----------------------------------------------------------------------------
# plw::zoom_mouse_draw
#
# Draws zoom box in response to mouse motion (with button held down).
#----------------------------------------------------------------------------

proc plw::zoom_mouse_draw {w wx0 wy0 wx1 wy1} {

    set coords [plw::zoom_coords $w $wx0 $wy0 $wx1 $wy1 0]

    unlist $coords xmin ymin xmax ymax
    $w.plwin draw rect $xmin $ymin $xmax $ymax
}

#----------------------------------------------------------------------------
# plw::zoom_mouse_end
#
# Performs actual zoom, invoked when user releases mouse button.
#----------------------------------------------------------------------------

proc plw::zoom_mouse_end {w wx0 wy0 wx1 wy1} {
    global user_zoom_handler

# Remove the zoom box bindings
    bind $w.plwin <B1-ButtonRelease> {}
    bind $w.plwin <B1-Motion> {}

# Reset window label to the default
    plw::label_refresh $w

# Turn off the zoom box
    $w.plwin draw end

# Select new plot region
    if [info exists user_zoom_handler($w)] {
        eval $user_zoom_handler($w) $w $wx0 $wy0 $wx1 $wy1

    } else {
        set coords [plw::zoom_coords $w $wx0 $wy0 $wx1 $wy1 1]
        unlist $coords xmin ymin xmax ymax

        plw::view_zoom $w $xmin $ymin $xmax $ymax
    }
}

#----------------------------------------------------------------------------
# plw::view_select
#
# Handles change of view into plot.
# Given in relative plot window coordinates.
#----------------------------------------------------------------------------

proc plw::view_select {w x0 y0 x1 y1} {

# Adjust arguments to be in bounds and properly ordered (xl < xr, etc)

    set xl [min $x0 $x1]
    set yl [min $y0 $y1]
    set xr [max $x0 $x1]
    set yr [max $y0 $y1]

    set xmin 0.
    set ymin 0.
    set xmax 1.
    set ymax 1.

    set xl [max $xmin [min $xmax $xl]]
    set yl [max $ymin [min $ymax $yl]]
    set xr [max $xmin [min $xmax $xr]]
    set yr [max $ymin [min $ymax $yr]]

# Only create scrollbars if really needed.

    if {($xl == $xmin) && ($xr == $xmax)} \
    then {set hscroll 0} else {set hscroll 1}

    if {($yl == $xmin) && ($yr == $xmax)} \
    then {set vscroll 0} else {set vscroll 1}

    if { ! ($hscroll || $vscroll)} {return}

# Select plot region

    $w.plwin view select $xl $yl $xr $yr

# Fix up view

    plw::fixview $w $hscroll $vscroll
}

#----------------------------------------------------------------------------
# plw::view_zoom
#
# Handles zoom.
# Given in relative device coordinates.
#----------------------------------------------------------------------------

proc plw::view_zoom {w x0 y0 x1 y1} {
    global xl xr yl yr

# Adjust arguments to be properly ordered (xl < xr, etc)

    set xl [min $x0 $x1]
    set yl [min $y0 $y1]
    set xr [max $x0 $x1]
    set yr [max $y0 $y1]

# Check for double-click (specified zoom region less than a few pixels
# wide).  In this case, magnification is 2X in each direction, centered at
# the mouse location.  At the boundary, the magnification is determined
# by the distance to the boundary.

    set stdzoom 0.5
    if { ($xr - $xl < 0.02) && ($yr - $yl < 0.02) } then {
	set nxl [expr $xl - 0.5 * $stdzoom]
	set nxr [expr $xl + 0.5 * $stdzoom]
	if { $nxl < 0.0 } then {
	    set nxl 0.0
	    set nxr [expr 2.0 * $xl]
	}
	if { $nxr > 1.0 } then {
	    set nxr 1.0
	    set nxl [expr 2.0 * $xl - 1.0]
	}
	set xl $nxl
	set xr $nxr

	set nyl [expr $yl - 0.5 * $stdzoom]
	set nyr [expr $yl + 0.5 * $stdzoom]
	if { $nyl < 0.0 } then {
	    set nyl 0.0
	    set nyr [expr 2.0 * $yl]
	}
	if { $nyr > 1.0 } then {
	    set nyr 1.0
	    set nyl [expr 2.0 * $yl - 1.0]
	}
	set yl $nyl
	set yr $nyr
    }

# Adjust arguments to be in bounds (in case margins are in effect).

    set bounds [$w.plwin view bounds]
    unlist $bounds xmin ymin xmax ymax

    set xl [max $xmin [min $xmax $xl]]
    set yl [max $ymin [min $ymax $yl]]
    set xr [max $xmin [min $xmax $xr]]
    set yr [max $ymin [min $ymax $yr]]

# Only create scrollbars if really needed.

    set hscroll [expr ($xl != $xmin) || ($xr != $xmax)]
    set vscroll [expr ($yl != $ymin) || ($yr != $ymax)]

    if { ! ($hscroll || $vscroll)} then {
	$w.plwin redraw
	return
    }

# Select plot region

    $w.plwin view zoom $xl $yl $xr $yr

# Fix up view

    plw::fixview $w $hscroll $vscroll

# Add window to zoom windows list

    global zidx zidx_max zxl zyl zxr zyr

    incr zidx($w)
    set zidx_max($w) $zidx($w)

    set coords [$w.plwin view]
    set zxl($w,$zidx($w)) [lindex "$coords" 0]
    set zyl($w,$zidx($w)) [lindex "$coords" 1]
    set zxr($w,$zidx($w)) [lindex "$coords" 2]
    set zyr($w,$zidx($w)) [lindex "$coords" 3]
}

#----------------------------------------------------------------------------
# plw::zoom_back
#
# Traverses the zoom windows list backward.
#----------------------------------------------------------------------------

proc plw::zoom_back {w} {
    global zidx zxl zyl zxr zyr

    if { $zidx($w) == 0 } then return

    incr zidx($w) -1

    set xl $zxl($w,$zidx($w))
    set yl $zyl($w,$zidx($w))
    set xr $zxr($w,$zidx($w))
    set yr $zyr($w,$zidx($w))

# Select plot region

    $w.plwin view select $xl $yl $xr $yr
}

#----------------------------------------------------------------------------
# plw::zoom_forward
#
# Traverses the zoom windows list forward.
#----------------------------------------------------------------------------

proc plw::zoom_forward {w} {
    global zidx zidx_max zxl zyl zxr zyr

    if { $zidx_max($w) == 0 || $zidx($w) == $zidx_max($w) } then return

    incr zidx($w)

    set xl $zxl($w,$zidx($w))
    set yl $zyl($w,$zidx($w))
    set xr $zxr($w,$zidx($w))
    set yr $zyr($w,$zidx($w))

# Select plot region

    $w.plwin view select $xl $yl $xr $yr
}

#----------------------------------------------------------------------------
# plw::get_scroll_multiplier
#
# Returns scroll multiplier given currently pressed keys.  If base_speed not
# specified, the global key_scroll_speed is used.
#----------------------------------------------------------------------------

proc plw::get_scroll_multiplier {state {base_speed 0}} {
    global key_scroll_mag
    global key_scroll_speed

# Set up base multiplication factor

    if { $base_speed < 0 } {
        error "illegal base_speed $base_speed (must be nonnegative)"
    }

    if { $base_speed == 0 } {
        set mult $key_scroll_speed
    } else {
        set mult $base_speed
    }

# Now handle modifier keys

# shift
    if { $state & 0x01 } then {
	set mult [expr $mult * $key_scroll_mag]
    }

# caps-lock
    if { $state & 0x02 } then {
	set mult [expr $mult * $key_scroll_mag]
    }

# control
    if { $state & 0x04 } then {
	set mult [expr $mult * $key_scroll_mag]
    }

# alt
    if { $state & 0x08 } then {
	set mult [expr $mult * $key_scroll_mag]
    }

    return $mult
}

#----------------------------------------------------------------------------
# plw::view_scroll
#
# Scrolls view incrementally.
# Similar to clicking on arrow at end of scrollbar (but speed is user
# controllable).
#----------------------------------------------------------------------------

proc plw::view_scroll {w dx dy s} {

# Set up multiplication factor
    set mult [get_scroll_multiplier $s]

# Now scroll

    if {($dx != 0) && \
	    [winfo exists $w.hbar] && [winfo ismapped $w.hbar] } then {

	set dx [expr $dx * $mult]
	set first  [lindex [$w.hbar.hscroll get] 2]
	$w.plwin xscroll [expr $first+$dx]
    }
    if {($dy != 0) && \
	    [winfo exists $w.vscroll] && [winfo ismapped $w.vscroll] } then {

	set dy [expr $dy * $mult]
	set first  [lindex [$w.vscroll get] 2]
	$w.plwin yscroll [expr $first+$dy]
    }
}

#----------------------------------------------------------------------------
# plw::fixview
#
# Handles updates of scrollbars & plot after view change.
#----------------------------------------------------------------------------

proc plw::fixview {w hscroll vscroll} {

# Create scrollbars if they don't already exist.

    set created_sb 0
    if { $hscroll && ! [winfo exists $w.hbar] } then {
	set created_sb 1
	frame $w.hbar
	scrollbar $w.hbar.hscroll -relief sunken -orient horiz \
	    -command "$w.plwin xscroll"
	$w.plwin config -xscroll "$w.hbar.hscroll set"
	frame $w.hbar.pad
	pack $w.hbar.hscroll -side left -fill x -expand 1
	pack $w.hbar.pad -side right
    }
    if { $vscroll && ! [winfo exists $w.vscroll] } then {
	set created_sb 1
	scrollbar $w.vscroll -relief sunken \
	    -command "$w.plwin yscroll"
	$w.plwin config -yscroll "$w.vscroll set"
    }

# When scrollbars are first created, it may be necessary to unmap then map
# the plframe widget so that it has a chance to initialize the scrollbars
# before they are mapped.

    if { $created_sb } then {
	pack unpack $w.plwin
	pack append $w $w.plwin {left expand fill}
    }

# Map scrollbars if not already mapped.
# To get packing right, need to unmap then remap plot widget.
# Otherwise need to do explicit redraw.

    if { ($hscroll && ! [winfo ismapped $w.hbar]) || \
         ($vscroll && ! [winfo ismapped $w.vscroll]) } then {

	update
	pack unpack $w.plwin
	if { $hscroll } then {
	    pack append $w $w.hbar {bottom fillx}
	}
	if { $vscroll } then {
	    pack append $w $w.vscroll {right filly}
	}
	pack append $w $w.plwin {expand fill}

	if {$hscroll && $vscroll} {
	    $w.hbar.pad configure -width [winfo reqwidth $w.vscroll]
	}

    } else {
	$w.plwin redraw
    }
}

#----------------------------------------------------------------------------
# plw::update_view
#
# Updates view.  Results in scrollbars being added if they are appropriate.
# Does nothing if the plot window is unchanged from the default.
#----------------------------------------------------------------------------

proc plw::update_view {w} {
    set coords [$w.plwin view]
    unlist $coords xl yl xr yr

    plw::view_select $w $xl $yl $xr $yr
}

#----------------------------------------------------------------------------
# pl_status_msg
#
# Used for temporarily flashing a status message in the status bar.  Better
# than a dialog because it can be ignored and will go away on its own.
#----------------------------------------------------------------------------

proc pl_status_msg {w msg} {
    plw::label_set $w $msg
    after 2500 plw::label_refresh $w
}

#----------------------------------------------------------------------------
# plw::label_set
#
# Sets message in status bar.  Does not put on stack, so it is transient.
#----------------------------------------------------------------------------

proc plw::label_set {w msg} {
    global plot_menu_on; if !$plot_menu_on return

    $w.ftop.lstat configure -text "$msg"
}

#----------------------------------------------------------------------------
# plw::label_refresh
#
# Resets message in status bar to the current default (top one on stack).
#----------------------------------------------------------------------------

proc plw::label_refresh {w} {
    global plot_menu_on; if !$plot_menu_on return

    global plmenu_lstat plmenu_lstat_depth

    set depth $plmenu_lstat_depth($w)
    if { $depth >= 0 } {
        set msg $plmenu_lstat($w,$depth)
    } else {
        set msg ""
    }
    $w.ftop.lstat configure -text "$msg"
}

#----------------------------------------------------------------------------
# plw::label_push
#
# Sets message in status bar, pushing it onto the stack.  So it shouldn't
# be a transient message.  Limit 10 deep to quickly catch recursion errors.
#----------------------------------------------------------------------------

proc plw::label_push {w msg} {
    global plot_menu_on; if !$plot_menu_on return

    global plmenu_lstat plmenu_lstat_depth
    if { ![info exists plmenu_lstat_depth] || ![info exists plmenu_lstat_depth($w)] } {
	set plmenu_lstat_depth($w) -1
    }
    if {$plmenu_lstat_depth($w) < 9} {
	incr plmenu_lstat_depth($w)
    } {
	puts stderr "plw::label_push: you just hit the max stack depth"
    }
    set plmenu_lstat($w,$plmenu_lstat_depth($w)) $msg
    $w.ftop.lstat configure -text "$msg"
}

#----------------------------------------------------------------------------
# plw::label_pop
#
# Pops the top label off the stack, making the new one on top the default.
# It doesn't become active until plw::label_refresh is called.
#----------------------------------------------------------------------------

proc plw::label_pop {w} {
    global plot_menu_on; if !$plot_menu_on return

    global plmenu_lstat plmenu_lstat_depth
    if ![info exists plmenu_lstat_depth] {
	puts stderr "plw::label_pop: no stack defined yet, strange.."
	return
    }
    if {$plmenu_lstat_depth($w) >= 0} {
	incr plmenu_lstat_depth($w) -1
    } {
	puts stderr "plw::label_pop: end of stack!"
    }
}

#----------------------------------------------------------------------------
# plw::unlist
#
# Dumps a list into parts, as specified by variable list.  Should be standard.
# Written from scratch by mjl on 7/12/04.  Example syntax:
#
#   unlist $coords xmin ymin xmax ymax
#----------------------------------------------------------------------------

proc plw::unlist {mylist args} {

    foreach i $args j $mylist {
        upvar $i x
        set x $j
    }
}

#----------------------------------------------------------------------------
# plw::dplink
#
# NOTE: **OBSOLETE** kept only for reference
# Initializes socket data link between widget and client code.
# In addition, as this is the last client/server connection needed, I
# disable further connections.
#----------------------------------------------------------------------------

proc plw::dplink {w client} {
    global list_sock data_sock

    dp_Host +
    set rv [dp_connect -server 0]
    set list_sock [lindex $rv 0]
    set data_port [lindex $rv 1]

    dp_RDO $client set data_port $data_port
    set data_sock [lindex [dp_accept $list_sock] 0]
    $w.plwin openlink socket $data_sock
    dp_Host -
}
