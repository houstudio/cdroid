## -*-Tcl-*-
 # ###################################################################
 #
 #  FILE: "Pltkwin.tcl"
 #				      created: 19/7/96 {1:43:00	pm}
 #				  last update: 4/10/96 {1:41:46 pm} pm}
 #  Author: Vince Darley
 #  E-mail: <vince@das.harvard.edu>
 #    mail: Division of	Applied	Sciences, Harvard University
 #	    Oxford Street, Cambridge MA	02138, USA
 #     www: <http://www.fas.harvard.edu/~darley/>
 #
 #  Description:
 #    A	unified	Itk 'extended' widget for Plplot.
 #
 #    Derives from the plain PLWin widget which	wraps a	plplot driver
 #
 #    Grabbed all the old legacy code and put it together in one
 #    piece.  Should be	much more clear	what's going on	now. It	has
 #    also all been updated for	Itk 2.1, using that package's mega-
 #    widget capability.
 #
 #    Am considering making these Itcl classes and/or the plframe
 #    derive their Tcl-plplot interface	from my	'pltcl_stream' class.
 #
 #    The advantages are: more sophisticated, better error messages, ...
 #    The disadvantage is that you must	use the	'Cpptcl' package and C++.
 #
 #  modified by	 rev reason
 #  -------- --- --- -----------
 #  19/7/96  VMD 1.0 original -	many minor bugs	probably exist.
 # ###################################################################
 ##

# We don't need	version	numbers	because	we use the first versions
# which	actually use the 'package' mechanism.
package	require	Pltk
eval package require $pl_iwidgets_package_name

itcl::class Pltkwin {
    inherit PLWin

    constructor	{args} {}
    protected method plwin {} {	return $itk_interior.plwin }

    public method setup_defaults {} {}
    public method create_pmenu {} {}
    public method create_pmenu_print {}	{}
    public method create_pmenu_save {} {}
    public method create_pmenu_orient {} {}
    public method create_pmenu_zoom {} {}
    public method create_pmenu_page {} {}
    public method create_pmenu_redraw {} {}
    public method create_pmenu_options {} {}
    public method start	{} {}
    public method key_filter {keycode state x y	keyname	ascii} {}
    public method user_key {keycode state x y keyname ascii} {}
    public method user_mouse {button state x y}	{}
    public method flash	{col} {}
    public method end {} {}
    public method print	{} {}
    public method save_as {} {}
    public method save_again {}	{}
    public method save_close {}	{}
    public method update_zoom {} {}
    public method zoom_select {} {}
    public method zoom_enter {}	{}
    public method zoom_reset {}	{}
    public method update_orient	{} {}
    public method orient {rot} {}
    public method page_enter {}	{}
    public method page_reset {}	{}
    public method zoom_start {wx wy} {}
    public method zoom_coords {x0 y0 x1	y1 opt}	{}
    public method zoom_mouse_draw {wx0 wy0 wx1 wy1} {}
    public method zoom_mouse_end {wx0 wy0 wx1 wy1} {}
    public method view_select {x0 y0 x1	y1} {}
    public method view_zoom {x0	y0 x1 y1} {}
    public method zoom_back {} {}
    public method zoom_forward {} {}
    public method view_scroll {dx dy s}	{}
    public method fixview {hscroll vscroll} {}
    public method update_view {} {}
    public method label_reset {} {}
    public method label_set {msg} {}
    public method dplink {client} {}
    public method status_msg {msg}  {}
}

body Pltkwin::constructor {args} {
# Set up defaults

    setup_defaults

# Make frame for top row widgets.  plframe widget must already have been
# created (the plframe is queried for a	list of	the valid output devices
# for page dumps).

    itk_component add ftop {
	frame $itk_interior.ftop
    } {
	keep -background -cursor
    }
    pack $itk_component(ftop) -side top	-fill x -before $hull.plwin
#-fill both
#-expand yes
    itk_component add pmenub {
	menubutton $itk_interior.ftop.pmenu -text "Plot" \
	    -menu $itk_interior.ftop.pmenu.m -relief	raised
    } {
	keep -background -cursor
    }
    itk_component add pmenu {
	menu $itk_interior.ftop.pmenu.m
    } {
	#keep -background -cursor
    }

    pack append	$itk_interior.ftop $itk_interior.ftop.pmenu \
	{left fill padx	12}
    create_pmenu

    itk_component add lstat {
	label $itk_interior.ftop.lstat -anchor w -relief raised
    } {
	#keep -background -cursor
    }
    label_reset
    pack append	$itk_interior.ftop  $itk_interior.ftop.lstat \
	{right expand fill}

# Initialize plplot widget.  Enable keyboard traversal when widget has
# the input focus.  Also grab the initial input	focus.

    tk_bindForTraversal	[plwin]
    focus [plwin]
}

#----------------------------------------------------------------------------
# setup_defaults
#
# Set up default settings.
#----------------------------------------------------------------------------

body Pltkwin::setup_defaults	{}  {

# In the two cases below, the options can be specified in advance through
# the global variables zoomopt_0, etc, and saveopt_0, etc.  Not	a great
# solution but will have to do for now.

# zoom options:
#  0:	0=don't	preserve aspect	ratio, 1=do
#  1:	0=stretch from corner, 1=stretch from center

    global zoomopts zoomopt_0 zoomopt_1

    set	zoomopts($this,0) 1
    set	zoomopts($this,1) 1
    if { [info exists zoomopt_0] } {set	zoomopts($this,0) $zoomopt_0}
    if { [info exists zoomopt_1] } {set	zoomopts($this,1) $zoomopt_1}

# save options:
#  0:	name of	default	save device
#  1:	0=save 1 plot/file, 1=save multi plots/file (must close!)

    global saveopts saveopt_0 saveopt_1

    set	saveopts($this,0) "psc"
    set	saveopts($this,1) 0
    if { [info exists saveopt_0] } {set	saveopts($this,0) $saveopt_0}
    if { [info exists saveopt_1] } {set	saveopts($this,1) $saveopt_1}

# Set up zoom windows list

    global zidx	zidx_max zxl zyl zxr zyr

    set	zidx($this) 0
    set	zidx_max($this)	0
    set	zxl($this,0) 0.0
    set	zyl($this,0) 0.0
    set	zxr($this,0) 1.0
    set	zyr($this,0) 1.0

# Bindings

    bind [plwin] <Any-KeyPress>	\
	[code $this key_filter	%N %s %x    %y %K %A]

    bind [plwin] <Any-ButtonPress>  \
	[code $this user_mouse	%b %s %x    %y]

    bind [plwin] <Any-Enter> \
	"focus [plwin]"
}


#----------------------------------------------------------------------------
# create_pmenu
#
# Create plot menu.
#
# It is	tempting to create buttons for some of these options, but buttons
# are difficult	to effectively place and extend.  Menus	have a clear
# placement mechanism and are easy to add to.  Further,	TK menus can be
# torn off (select menu	with middle mouse button and move to where you
# want it) which makes selecting top-level menu	buttons	easy.  Finally,
# certain menu options have keyboard equivalents: zoom-select (z),
# zoom-reset (r), print	(P), and save-again (s).
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu {}	{
    create_pmenu_print
    create_pmenu_save
    create_pmenu_orient
    create_pmenu_zoom
    create_pmenu_page
    create_pmenu_options
}

#----------------------------------------------------------------------------
# create_pmenu_print
#
# Create plot-print menu
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu_print    {}	{
    $itk_component(pmenu) add command -label "Print" \
	-command [code $this print ]
}

#----------------------------------------------------------------------------
# create_pmenu_save
#
# Create plot-save menu	(cascade)
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu_save	{} {
    set	m $itk_component(pmenu).save

    $itk_component(pmenu) add cascade -label "Save" -menu $m
    menu $m

# Save - As

    $m add command -label "As" \
	-command [code $this save_as ]

# Save - Again

    $m add command -label "Again" \
	-command [code $this save_again	] \
	-state disabled

# Save - Close

    $m add command -label "Close" \
	-command [code $this save_close	] \
	-state disabled

    $m add separator

# Save - Set device.. (another cascade)

    $m add cascade -label "Set device" -menu $m.sdev
    menu $m.sdev

# Generate the device list in the "Save/Set device" widget menu, by querying
# the plframe widget for the available output devices (which are listed).

    global saveopts

    set	devnames [[plwin] info	devnames]
    set	devkeys	[[plwin] info devkeys]
    set	ndevs [llength $devnames]
    for	{set i 0} {$i <	$ndevs}	{incr i} {
	set devnam [lindex $devnames $i]
	set devkey [lindex $devkeys $i]

	$m.sdev	add radio -label $devnam \
	    -variable saveopts($this,0)	-value	$devkey
    }

# Save - Set file type.. (another cascade)

    $m add cascade -label "Set file type" -menu	$m.sfile
    menu $m.sfile

# Single file (one plot/file)

    $m.sfile add radio -label "Single file (one	plot/file)" \
	-variable saveopts($this,1) -value  0

# Archive file (many plots/file)

    $m.sfile add radio -label "Archive file (many plots/file)" \
	-variable saveopts($this,1) -value  1
}

#----------------------------------------------------------------------------
# create_pmenu_orient
#
# Create plot-orient menu (cascade)
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu_orient {} {
    set	m $itk_component(pmenu).orient

    $itk_component(pmenu) add cascade -label "Orient" -menu $m
    menu $m

    $m configure -postcommand [code $this update_orient	]

# Orient - 0 degrees

    $m add radio -label	"0 degrees" \
	-command [code $this orient	0]

# Orient - 90 degrees

    $m add radio -label	"90 degrees" \
	-command [code $this orient	1]

# Orient - 180 degrees

    $m add radio -label	"180 degrees" \
	-command [code $this orient	2]

# Orient - 270 degrees

    $m add radio -label	"270 degrees" \
	-command [code $this orient	3]
}

#----------------------------------------------------------------------------
# create_pmenu_zoom
#
# Create plot-zoom menu	(cascade)
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu_zoom	{} {
    set	m $itk_component(pmenu).zoom

    $itk_component(pmenu) add cascade -label "Zoom" -menu $m
    menu $m

    $m configure -postcommand [code $this update_zoom ]

# Zoom - select	(by mouse)

    $m add command -label "Select" \
	-command [code $this zoom_select ]

# Zoom - back (go back 1 zoom level)

    $m add command -label "Back" \
	-command [code $this zoom_back ]    \
	-state disabled

# Zoom - forward (go forward 1 zoom level)

    $m add command -label "Forward" \
	-command [code $this zoom_forward ] \
	-state disabled

# Zoom - enter bounds

    $m add command -label "Enter bounds.." \
	-command [code $this zoom_enter	]

# Zoom - reset

    $m add command -label "Reset" \
	-command [code $this zoom_reset	]

# Zoom - options (another cascade)

    $m add cascade -label "Options" -menu $m.options
    menu $m.options

    global zoomopts

    $m.options add check -label	"Preserve aspect ratio"	\
	-variable zoomopts($this,0)

    $m.options add separator

    $m.options add radio -label	"Start from corner" \
	-variable zoomopts($this,1) -value  0

    $m.options add radio -label	"Start from center" \
	-variable zoomopts($this,1) -value  1

    $m.options invoke 1
}

#----------------------------------------------------------------------------
# create_pmenu_page
#
# Create plot-page menu	(cascade)
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu_page	{} {
    set	m $itk_component(pmenu).page

    $itk_component(pmenu) add cascade -label "Page" -menu $m
    menu $m

# Page - enter bounds

    $m add command -label "Setup.." \
	-command [code $this page_enter	]

# Page - reset

    $m add command -label "Reset" \
	-command [code $this page_reset	]
}

#----------------------------------------------------------------------------
# create_pmenu_redraw
#
# Create plot-redraw menu
# I only use this for debugging	in cases where the normal redraw capability
# isn't	working	right.
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu_redraw {} {
    $itk_component(pmenu) add command -label "Redraw" \
	-command "[plwin] redraw"
}

#----------------------------------------------------------------------------
# create_pmenu_options
#
# Create plot-options menu (cascade)
#----------------------------------------------------------------------------

body Pltkwin::create_pmenu_options {} {
    set	m $itk_component(pmenu).options

    $itk_component(pmenu) add cascade -label "Options" -menu $m
    menu $m

    $m add command -label "Palette 0" \
	-command "plcmap0_edit [plwin]"

    $m add command -label "Palette 1" \
	-command "plcmap1_edit [plwin]"
}

#----------------------------------------------------------------------------
# start
#
# Responsible for plplot graphics package initialization on the	widget.
# People driving the widget directly should just use pack themselves.
#
# Put here to reduce the possibility of	a time out over	a slow network --
# the client program waits until the variable widget_is_ready is set.
#----------------------------------------------------------------------------

body Pltkwin::start {} {
    global client

# Manage widget	hierarchy

    pack append	[winfo parent $this] $this \
	{bottom	expand fill}

    update

# Inform client	that we're done.

    if { [info exists client] }	then {
	client_cmd "set	widget_is_ready	1"
    }
}

#----------------------------------------------------------------------------
# key_filter
#
# Front-end to key handler.
# For supported	operations it's	best to	modify the global key variables
# to get the desired action.  More advanced stuff can be done with the
# $user_key_filter proc.  Find anything	particularly useful?  Let me know,
# so it	can be added to	the default behavior.
#----------------------------------------------------------------------------

body Pltkwin::key_filter    {keycode state x y keyname ascii}	{
    global user_key_filter

    global key_zoom_select
    global key_zoom_reset
    global key_print
    global key_save_again
    global key_scroll_right
    global key_scroll_left
    global key_scroll_up
    global key_scroll_down

#    puts "keypress: $keyname $keycode $ascii $state"

# Call user-defined key	filter,	if one exists

    if { [info exists user_key_filter] } then {
	$user_key_filter $keyname $keycode $ascii
    }

# Interpret keystroke

    switch $keyname \
	$key_zoom_select	[code $this zoom_select	] \
	"b"			[code $this zoom_back ]	\
	"f"			[code $this zoom_forward ] \
	$key_zoom_reset		[code $this zoom_reset ]    \
	$key_print		[code $this print ] \
	$key_save_again		[code $this save_again ]    \
	$key_scroll_right	[code $this view_scroll	     1	0 $state] \
	$key_scroll_left	[code $this view_scroll	    -1	0 $state] \
	$key_scroll_up		[code $this view_scroll	     0 -1 $state] \
	$key_scroll_down	[code $this view_scroll	     0	1 $state]

# Pass keypress	event info back	to client.

    user_key $keycode $state	$x $y $keyname $ascii
}

#----------------------------------------------------------------------------
# user_key
#
# Passes keypress event	information back to client.
# Based	on user_mouse.
#----------------------------------------------------------------------------

body Pltkwin::user_key {keycode	state x	y keyname ascii} {
    global client

    if { [info exists client] }	then {

    # calculate	relative window	coordinates.

	set xw [expr "$x / [winfo width	[plwin]]."]
	set yw [expr "1.0 - $y / [winfo	height [plwin]]."]

    # calculate	normalized device coordinates into original window.

	set view [[plwin] view]
	set xrange [expr "[lindex $view	2] - [lindex $view 0]"]
	set xnd	[expr "($xw * $xrange) + [lindex $view 0]"]
	set yrange [expr "[lindex $view	3] - [lindex $view 1]"]
	set ynd	[expr "($yw * $yrange )	+ [lindex $view	1]"]

    # send them	back to	the client.

#	puts "keypress $keycode	$state $x $y $xnd $ynd $keyname	$ascii"
	client_cmd \
	    [list keypress $keycode $state $x $y $xnd $ynd $keyname $ascii]
    }
}

#----------------------------------------------------------------------------
# user_mouse
#
# Passes buttonpress event information back to client.
# Written by Radey Shouman
#----------------------------------------------------------------------------

body Pltkwin::user_mouse    {button state   x y} {
    global client

    if { [info exists client] }	then {

    # calculate	relative window	coordinates.

	set xw [expr "$x / [winfo width	[plwin]]."]
	set yw [expr "1.0 - $y / [winfo	height [plwin]]."]

    # calculate	normalized device coordinates into original window.

	set view [[plwin] view]
	set xrange [expr "[lindex $view	2] - [lindex $view 0]"]
	set xnd	[expr "($xw * $xrange) + [lindex $view 0]"]
	set yrange [expr "[lindex $view	3] - [lindex $view 1]"]
	set ynd	[expr "($yw * $yrange )	+ [lindex $view	1]"]

    # send them	back to	the client.

	client_cmd \
	    [list buttonpress $button $state $x	$y $xnd	$ynd]
    }
}

#----------------------------------------------------------------------------
# flash
#
# Set eop button color to indicate page	status.
#----------------------------------------------------------------------------

body Pltkwin::flash {col} {
    $itk_component(ftop).leop config -bg $col
    update idletasks
}

#----------------------------------------------------------------------------
# end
#
# Executed as part of orderly shutdown procedure.  Eventually will just
# destroy the plframe and surrounding widgets, and server will exit only
# if all plotting widgets have been destroyed and it is	a child	of the
# plplot/TK driver.  Maybe.
#
# The closelink	command	was added in the hopes of making the dp	driver
# cleanup a bit	more robust, but doesn't seem to have any effect except
# to slow things down quite a bit.
#----------------------------------------------------------------------------

body Pltkwin::end {} {
    global dp
#    [plwin] closelink
    if { $dp } then {
	global list_sock
	close $list_sock
    }
    exit
}

#----------------------------------------------------------------------------
# print
#
# Prints plot.	Uses the "plpr"	script,	which must be set up for your site
# as appropriate.  There are better ways to do it but this way is safest
# for now.
#----------------------------------------------------------------------------

body Pltkwin::print {} {
    label_set "Printing	plot..."
    update
    if { [catch	"[plwin] print"	foo] }	{
	bogue_out "$foo"
    } else {
	status_msg "Plot printed."
    }
}

#----------------------------------------------------------------------------
# save_as
#
# Saves	plot to	default	device,	prompting for file name.
#----------------------------------------------------------------------------

body Pltkwin::save_as {} {
    global pmenu saveopts
    set	file [getSaveFile $saveopts($this,0)]
    if { [string length	$file] > 0 } {
	if { [file exists $file] } {
	    if { ! [confirm "File $file	already	exists.	 Are you sure?"] } {
		return
	    }
	}

	label_set "Saving plot..."
	update
	if { [catch "[plwin] save as $saveopts($this,0)	$file" foo] }	{
	    label_reset
	    bogue_out "$foo"
	} else {
	    status_msg "Plot saved."
	}

	if { $saveopts($this,1)	==  0 }	then {
	    [plwin] save close
	} else {
	    $pmenu($this).save entryconfigure Again -state normal
	    $pmenu($this).save entryconfigure Close -state normal
	    bogue_out "Warning:	archive	files must be closed before using"
	}
    } else {
	bogue_out "No file specified"
    }
}

#----------------------------------------------------------------------------
# save_again
#
# Saves	plot to	an already open	file.
#----------------------------------------------------------------------------

body Pltkwin::save_again    {}	{
    if { [catch	"[plwin] save"	foo] } {
	bogue_out "$foo"
    } else {
	status_msg "Plot saved."
    }
}

#----------------------------------------------------------------------------
# save_close
#
# Close	archive	save file.
#----------------------------------------------------------------------------

body Pltkwin::save_close    {}	{
    global pmenu
    if { [catch	"[plwin] save close" foo] } {
	bogue_out "$foo"
    } else {
	status_msg "Archive file closed."
	$pmenu($this).save  entryconfigure Again -state disabled
	$pmenu($this).save  entryconfigure Close -state disabled
    }
}

#----------------------------------------------------------------------------
# update_zoom
#
# Responsible for making sure zoom menu	entries	are normal or disabled as
# appropriate.	In particular, that "Back" or "Forward"	are only displayed
# if it	is possible to traverse	the zoom windows list in that direction.
#----------------------------------------------------------------------------

body Pltkwin::update_zoom {} {
    global zidx	zidx_max zxl zyl zxr zyr
    global pmenu

# Back

    if { $zidx($this) == 0  } then {
	$pmenu($this).zoom  entryconfigure Back -state disabled
    } else {
	$pmenu($this).zoom  entryconfigure Back -state normal
    }

# Forward

    if { $zidx_max($this) == 0	|| $zidx($this)	==  $zidx_max($this) }	then {
	$pmenu($this).zoom  entryconfigure Forward -state disabled
    } else {
	$pmenu($this).zoom  entryconfigure Forward -state normal
    }
}

#----------------------------------------------------------------------------
# zoom_select
#
# Zooms	plot in	response to mouse selection.
#----------------------------------------------------------------------------

body Pltkwin::zoom_select {} {
    global def_button_cmd zoomopts

    set	def_button_cmd [bind [plwin] <ButtonPress>]

    if { $zoomopts($this,1) ==	0 } then {
	label_set "Click on one	corner of zoom region."
    } else {
	label_set "Click on center of zoom region."
    }

    bind [plwin] <ButtonPress>	[code $this zoom_start	%x %y]
}

#----------------------------------------------------------------------------
# zoom_enter
#
# Zooms	plot in	response to text entry.
#----------------------------------------------------------------------------

body Pltkwin::zoom_enter    {}	{
    global fv00	fv01 fv10 fv11
    global fn00	fn01 fn10 fn11

    set	coords [[plwin]	view]

    set	fv00 [lindex "$coords" 0]
    set	fv01 [lindex "$coords" 1]
    set	fv10 [lindex "$coords" 2]
    set	fv11 [lindex "$coords" 3]

    set	fn00 xmin
    set	fn01 ymin
    set	fn10 xmax
    set	fn11 ymax

    Form2d .e "Enter window coordinates	for zoom.  Each	coordinate should range	from 0 to 1, with (0,0)	corresponding to the lower left	hand corner."
    tkwait window .e

    view_select	$fv00 $fv01 $fv10 $fv11
}

#----------------------------------------------------------------------------
# zoom_reset
#
# Resets after zoom.
# Note that an explicit	redraw is not necessary	since the packer issues	a
# resize after the scrollbars are unmapped.
#----------------------------------------------------------------------------

body Pltkwin::zoom_reset    {}	{
    global def_button_cmd

    label_reset
    bind [plwin] <ButtonPress>	$def_button_cmd
    [plwin] view reset
    if { [winfo	exists $itk_interior.hscroll] && [winfo	ismapped $itk_interior.hscroll]	}   then {
	pack unpack $itk_interior.hscroll
    }
    if { [winfo	exists $itk_interior.vscroll] && [winfo	exists	$itk_interior.vscroll]	} then {
	pack unpack $itk_interior.vscroll
    }

# Reset	zoom windows list

    global zidx	zidx_max zxl zyl zxr zyr

    set	zidx($this) 0
    set	zidx_max($this)	0
    set	zxl($this,0) 0.0
    set	zyl($this,0) 0.0
    set	zxr($this,0) 1.0
    set	zyr($this,0) 1.0
}

#----------------------------------------------------------------------------
# update_orient
#
# Responsible for making sure orientation radio	buttons	are up to date.
#----------------------------------------------------------------------------

body Pltkwin::update_orient {} {
    set	rot [orient]
    set	entry [expr [format "%.0f" $rot] % 4]
    $itk_component(pmenu).orient invoke	$entry
}

#----------------------------------------------------------------------------
# orient
#
# Changes plot orientation.
#----------------------------------------------------------------------------

body Pltkwin::orient	{rot}	{
    if { [orient] != $rot}  then {
	orient	$rot
    }
}

#----------------------------------------------------------------------------
# page_enter
#
# Changes output page parameters (margins, aspect ratio, justification).
#----------------------------------------------------------------------------

body Pltkwin::page_enter    {}	{
    global fv00	fv01 fv10 fv11
    global fn00	fn01 fn10 fn11

    set	coords [[plwin]	page]

    set	fv00 [lindex "$coords" 0]
    set	fv01 [lindex "$coords" 1]
    set	fv10 [lindex "$coords" 2]
    set	fv11 [lindex "$coords" 3]

    set	fn00 mar
    set	fn01 aspect
    set	fn10 jx
    set	fn11 jy

    Form2d .e "Enter page setup	parameters.  mar denotes the fractional	page area on each side to use as a margin (0 to	0.5).  jx and jy are the fractional justification relative to the center (-0.5 to 0.5).	 aspect	is the page aspect ratio (0 preserves original aspect ratio)."
    tkwait window .e

    [plwin] page $fv00	$fv01 $fv10 $fv11
}

#----------------------------------------------------------------------------
# page_reset
#
# Resets page parameters.
#----------------------------------------------------------------------------

body Pltkwin::page_reset    {}	{
    [plwin] page 0. 0.	0. 0.
}

#----------------------------------------------------------------------------
# zoom_start
#
# Starts plot zoom.
#----------------------------------------------------------------------------

body Pltkwin::zoom_start    {wx	wy} {
    global def_button_cmd

    bind [plwin] <ButtonPress>	$def_button_cmd
    label_set "Select zoom region by dragging mouse, then release."

    [plwin] draw init
    bind [plwin] <B1-Motion>	     [code $this zoom_mouse_draw  $wx $wy %x	%y]
    bind [plwin] <B1-ButtonRelease> [code $this	zoom_mouse_end	$wx $wy	%x %y]
}

#----------------------------------------------------------------------------
# zoom_coords
#
# Transforms the initial and final mouse coordinates to	either:
#
# opt =	0	device coordinates
# opt =	1	normalized device coordinates
#
# The global variable "zoomopts" is used to determine zoom behavior:
#
# zoomopts($this,0):
#   0	box follows mouse movements exactly
#   1	box follows mouse movements so that aspect ratio is preserved (default)
#
# zoomopts($this,1):
#   0	first and last points specified	determine opposite corners
#	of zoom	box.
#   1	box is centered	about the first	point clicked on,
#	perimeter follows mouse	(default)
#
#----------------------------------------------------------------------------

body Pltkwin::zoom_coords {x0 y0 x1 y1	opt} {
    global zoomopts

    set	Lx [winfo width	 [plwin]]
    set	Ly [winfo height [plwin]]

# Enforce boundaries in	device coordinate space

    set	bounds [[plwin]	view bounds]
    set	xmin [expr [lindex "$bounds" 0]	* $Lx]
    set	ymin [expr [lindex "$bounds" 1]	* $Ly]
    set	xmax [expr [lindex "$bounds" 2]	* $Lx]
    set	ymax [expr [lindex "$bounds" 3]	* $Ly]

    set	x1 [max	$xmin [min $xmax $x1]]
    set	y1 [max	$ymin [min $ymax $y1]]

# Two-corners zoom.

    if { $zoomopts($this,1) ==	0 } then {

    # Get box lengths

	set dx [expr $x1 - $x0]
	set dy [expr $y1 - $y0]

	set sign_dx [expr ($dx > 0) ? 1	: -1]
	set sign_dy [expr ($dy > 0) ? 1	: -1]

	set xl $x0
	set yl $y0

    # Constant aspect ratio

	if { $zoomopts($this,0)	==  1 }	then {

	# Scale	factors	used to	maintain plot aspect ratio

	    set	xscale [expr $xmax - $xmin]
	    set	yscale [expr $ymax - $ymin]

	# Adjust box size for proper aspect ratio

	    set	rx [expr double(abs($dx)) / $xscale]
	    set	ry [expr double(abs($dy)) / $yscale]

	    if { $rx > $ry } then {
		set dy [expr $yscale * $rx * $sign_dy]
	    } else {
		set dx [expr $xscale * $ry * $sign_dx]
	    }

	    set	xr [expr $xl + $dx]
	    set	yr [expr $yl + $dy]

	# Now check again to see if in bounds, and adjust if not

	    if { $xr < $xmin ||	$xr > $xmax } then {
		if { $xr < $xmin } then	{
		    set	dx [expr $xmin - $x0]
		} else {
		    set	dx [expr $xmax - $x0]
		}
		set rx [expr double(abs($dx)) /	$xscale]
		set dy [expr $yscale * $rx * $sign_dy]
	    }

	    if { $yr < $ymin ||	$yr > $ymax } then {
		if { $yr < $ymin } then	{
		    set	dy [expr $ymin - $y0]
		} else {
		    set	dy [expr $ymax - $y0]
		}
		set ry [expr double(abs($dy)) /	$yscale]
		set dx [expr $xscale * $ry * $sign_dx]
	    }
	}

    # Final box	coordinates

	set xr [expr $xl + $dx]
	set yr [expr $yl + $dy]

# zoom from center out,	preserving aspect ratio

    } else {

    # Get box lengths, adjusting downward if necessary to keep in bounds

	set dx [expr abs($x1 - $x0)]
	set dy [expr abs($y1 - $y0)]

	set xr [expr $x0 + $dx]
	set xl [expr $x0 - $dx]
	set yr [expr $y0 + $dy]
	set yl [expr $y0 - $dy]

	if { $xl < $xmin } then	{
	    set	dx [expr $x0 - $xmin]
	}
	if { $xr > $xmax } then	{
	    set	dx [expr $xmax - $x0]
	}
	if { $yl < $ymin } then	{
	    set	dy [expr $y0 - $ymin]
	}
	if { $yr > $ymax } then	{
	    set	dy [expr $ymax - $y0]
	}

    # Constant aspect ratio

	if { $zoomopts($this,0)	==  1 }	then {

	# Scale	factors	used to	maintain plot aspect ratio

	    set	xscale [expr $xmax - $xmin]
	    set	yscale [expr $ymax - $ymin]

	# Adjust box size for proper aspect ratio

	    set	rx [expr double($dx) / $xscale]
	    set	ry [expr double($dy) / $yscale]

	    if { $rx > $ry } then {
		set dy [expr $yscale * $rx]
	    } else {
		set dx [expr $xscale * $ry]
	    }

	    set	xr [expr $x0 + $dx]
	    set	xl [expr $x0 - $dx]
	    set	yr [expr $y0 + $dy]
	    set	yl [expr $y0 - $dy]

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

    # Final box	coordinates

	set xr [expr $x0 + $dx]
	set xl [expr $x0 - $dx]
	set yr [expr $y0 + $dy]
	set yl [expr $y0 - $dy]
    }

# Optional translation to relative device coordinates.

    if { $opt == 1 } then {
	set wxl	[expr "$xl / double($Lx)" ]
	set wxr	[expr "$xr / double($Lx)" ]
	set wyl	[expr "1.0 - $yr / double($Ly)"	]
	set wyr	[expr "1.0 - $yl / double($Ly)"	]

    } else {
	set wxr	$xl
	set wxl	$xr
	set wyr	$yl
	set wyl	$yr
    }

    return "$wxl $wyl $wxr $wyr"
}

#----------------------------------------------------------------------------
# zoom_mouse_draw
#
# Draws	zoom box in response to	mouse motion (with button held down).
#----------------------------------------------------------------------------

body Pltkwin::zoom_mouse_draw {wx0  wy0	wx1 wy1} {

    set	coords [zoom_coords $wx0    $wy0 $wx1 $wy1 0]

    [plwin] draw rect \
	[lindex	"$coords" 0] [lindex "$coords" 1] \
	[lindex	"$coords" 2] [lindex "$coords" 3]
}

#----------------------------------------------------------------------------
# zoom_mouse_end
#
# Performs actual zoom,	invoked	when user releases mouse button.
#----------------------------------------------------------------------------

body Pltkwin::zoom_mouse_end	{wx0 wy0 wx1 wy1}   {

# Finish rubber	band draw

    bind [plwin] <B1-ButtonRelease> {}
    bind [plwin] <B1-Motion> {}
    label_reset
    [plwin] draw end

# Select new plot region

    set	coords [zoom_coords $wx0    $wy0 $wx1 $wy1 1]

    view_zoom [lindex	"$coords" 0] [lindex "$coords" 1] \
	[lindex	"$coords" 2] [lindex "$coords" 3]
}

#----------------------------------------------------------------------------
# view_select
#
# Handles change of view into plot.
# Given	in relative plot window	coordinates.
#----------------------------------------------------------------------------

body Pltkwin::view_select {x0 y0 x1 y1}	{

# Adjust arguments to be in bounds and properly	ordered	(xl < xr, etc)

    set	xl [min	$x0 $x1]
    set	yl [min	$y0 $y1]
    set	xr [max	$x0 $x1]
    set	yr [max	$y0 $y1]

    set	xmin 0.
    set	ymin 0.
    set	xmax 1.
    set	ymax 1.

    set	xl [max	$xmin [min $xmax $xl]]
    set	yl [max	$ymin [min $ymax $yl]]
    set	xr [max	$xmin [min $xmax $xr]]
    set	yr [max	$ymin [min $ymax $yr]]

# Only create scrollbars if really needed.

    if {($xl ==	$xmin) && ($xr == $xmax)} \
    then {set hscroll 0} else {set hscroll 1}

    if {($yl ==	$xmin) && ($yr == $xmax)} \
    then {set vscroll 0} else {set vscroll 1}

    if { ! ($hscroll ||	$vscroll)} {return}

# Select plot region

    [plwin] view select	$xl $yl	$xr $yr

# Fix up view

    fixview $hscroll $vscroll
}

#----------------------------------------------------------------------------
# view_zoom
#
# Handles zoom.
# Given	in relative device coordinates.
#----------------------------------------------------------------------------

body Pltkwin::view_zoom	{x0 y0	x1 y1} {

    global xl xr yl yr

# Adjust arguments to be properly ordered (xl <	xr, etc)

    set	xl [min	$x0 $x1]
    set	yl [min	$y0 $y1]
    set	xr [max	$x0 $x1]
    set	yr [max	$y0 $y1]

# Check	for double-click (specified zoom region	less than a few	pixels
# wide).  In this case,	magnification is 2X in each direction, centered	at
# the mouse location.  At the boundary,	the magnification is determined
# by the distance to the boundary.

    set	stdzoom	0.5
    if { ($xr -	$xl < 0.02) && ($yr - $yl < 0.02) } then {
	set nxl	[expr $xl - 0.5	* $stdzoom]
	set nxr	[expr $xl + 0.5	* $stdzoom]
	if { $nxl < 0.0	} then {
	    set	nxl 0.0
	    set	nxr [expr 2.0 *	$xl]
	}
	if { $nxr > 1.0	} then {
	    set	nxr 1.0
	    set	nxl [expr 2.0 *	$xl - 1.0]
	}
	set xl $nxl
	set xr $nxr

	set nyl	[expr $yl - 0.5	* $stdzoom]
	set nyr	[expr $yl + 0.5	* $stdzoom]
	if { $nyl < 0.0	} then {
	    set	nyl 0.0
	    set	nyr [expr 2.0 *	$yl]
	}
	if { $nyr > 1.0	} then {
	    set	nyr 1.0
	    set	nyl [expr 2.0 *	$yl - 1.0]
	}
	set yl $nyl
	set yr $nyr
    }

# Adjust arguments to be in bounds (in case margins are	in effect).

    set	bounds [[plwin]	view bounds]
    set	xmin [lindex "$bounds" 0]
    set	ymin [lindex "$bounds" 1]
    set	xmax [lindex "$bounds" 2]
    set	ymax [lindex "$bounds" 3]

    set	xl [max	$xmin [min $xmax $xl]]
    set	yl [max	$ymin [min $ymax $yl]]
    set	xr [max	$xmin [min $xmax $xr]]
    set	yr [max	$ymin [min $ymax $yr]]

# Only create scrollbars if really needed.

    set	hscroll	[expr ($xl != $xmin) ||	($xr !=	$xmax)]
    set	vscroll	[expr ($yl != $ymin) ||	($yr !=	$ymax)]

    if { ! ($hscroll ||	$vscroll)} then	{
	[plwin]	redraw
	return
    }

# Select plot region

    [plwin] view zoom $xl $yl $xr $yr

# Fix up view

    fixview $hscroll $vscroll

# Add window to	zoom windows list

    global zidx	zidx_max zxl zyl zxr zyr

    incr zidx($this)
    set	zidx_max($this)	$zidx($this)

    set	coords [[plwin]	view]
    set	zxl($this,$zidx($this))	[lindex	"$coords"   0]
    set	zyl($this,$zidx($this))	[lindex	"$coords"   1]
    set	zxr($this,$zidx($this))	[lindex	"$coords"   2]
    set	zyr($this,$zidx($this))	[lindex	"$coords"   3]
}

#----------------------------------------------------------------------------
# zoom_back
#
# Traverses the	zoom windows list backward.
#----------------------------------------------------------------------------

body Pltkwin::zoom_back	{} {

    global zidx	zxl zyl	zxr zyr

    if { $zidx($this) == 0  } then return

    incr zidx($this) -1

    set	xl $zxl($this,$zidx($this))
    set	yl $zyl($this,$zidx($this))
    set	xr $zxr($this,$zidx($this))
    set	yr $zyr($this,$zidx($this))

# Select plot region

    [plwin] view select	$xl $yl	$xr $yr
}

#----------------------------------------------------------------------------
# zoom_forward
#
# Traverses the	zoom windows list forward.
#----------------------------------------------------------------------------

body Pltkwin::zoom_forward {} {

    global zidx	zidx_max zxl zyl zxr zyr

    if { $zidx_max($this) == 0	|| $zidx($this)	==  $zidx_max($this) }	then return

    incr zidx($this)

    set	xl $zxl($this,$zidx($this))
    set	yl $zyl($this,$zidx($this))
    set	xr $zxr($this,$zidx($this))
    set	yr $zyr($this,$zidx($this))

# Select plot region

    [plwin] view select	$xl $yl	$xr $yr
}

#----------------------------------------------------------------------------
# view_scroll
#
# Scrolls view incrementally.
# Similar to clicking on arrow at end of scrollbar (but	speed is user
# controllable).
#----------------------------------------------------------------------------

body Pltkwin::view_scroll {dx dy s} {
    global key_scroll_mag
    global key_scroll_speed

# Set up multiplication	factor

    set	mult $key_scroll_speed
    if { $s & 0x01 } then {
	set mult [expr $mult * $key_scroll_mag]
    }
    if { $s & 0x02 } then {
	set mult [expr $mult * $key_scroll_mag]
    }
    if { $s & 0x04 } then {
	set mult [expr $mult * $key_scroll_mag]
    }
    if { $s & 0x08 } then {
	set mult [expr $mult * $key_scroll_mag]
    }

# Now scroll

    if {($dx !=	0) && \
	    [winfo exists $itk_interior.hscroll] && [winfo  ismapped $itk_interior.hscroll] } then  {

	set dx [expr $dx * $mult]
	set first  [lindex [$itk_interior.hscroll get]	2]
	[plwin]	xscroll	[expr $first+$dx]
    }
    if {($dy !=	0) && \
	    [winfo exists $itk_interior.vscroll] && [winfo  ismapped $itk_interior.vscroll] } then  {

	set dy [expr $dy * $mult]
	set first  [lindex [$itk_interior.vscroll get]	2]
	[plwin]	yscroll	[expr $first+$dy]
    }
}

#----------------------------------------------------------------------------
# fixview
#
# Handles updates of scrollbars	& plot after view change.
#----------------------------------------------------------------------------

body Pltkwin::fixview {hscroll	vscroll} {

# Create scrollbars if they don't already exist.

    set	created_sb 0
    if { $hscroll && ! [winfo exists $itk_interior.hscroll] } then  {
	set created_sb 1
	scrollbar $itk_interior.hscroll	-relief	sunken	-orient	horiz \
	    -command "[plwin] xscroll"
	[plwin]	config	-xscroll "$itk_interior.hscroll	set"
    }
    if { $vscroll && ! [winfo exists $itk_interior.vscroll] } then  {
	set created_sb 1
	scrollbar $itk_interior.vscroll	-relief	sunken	\
	    -command "[plwin] yscroll"
	[plwin]	config	-yscroll "$itk_interior.vscroll	set"
    }

# When scrollbars are first created, it	may be necessary to unmap then map
# the plframe widget so	that it	has a chance to	initialize the scrollbars
# before they are mapped.

    if { $created_sb } then {
	pack unpack [plwin]
	pack append $itk_interior [plwin]   {left expand fill}
    }

# Map scrollbars if not	already	mapped.
# To get packing right,	need to	unmap then remap plot widget.
# Otherwise need to do explicit	redraw.

    if { ($hscroll && !	[winfo ismapped	$itk_interior.hscroll])	||  \
	 ($vscroll && !	[winfo ismapped	$itk_interior.vscroll])	} then	{

	update
	pack unpack [plwin]
	if { $hscroll }	then {
	    pack append	$itk_interior $itk_interior.hscroll {bottom fillx}
	}
	if { $vscroll }	then {
	    pack append	$itk_interior $itk_interior.vscroll {right filly}
	}
	pack append $itk_interior [plwin]   {expand fill}

    } else {
	[plwin]	redraw
    }
}

#----------------------------------------------------------------------------
# update_view
#
# Updates view.	 Results in scrollbars being added if they are appropriate.
# Does nothing if the plot window is unchanged from the	default.
#----------------------------------------------------------------------------

body Pltkwin::update_view {} {
    set	coords [[plwin]	view]

    set	xl [lindex "$coords" 0]
    set	yl [lindex "$coords" 1]
    set	xr [lindex "$coords" 2]
    set	yr [lindex "$coords" 3]

    view_select	$xl $yl	$xr $yr
}

#----------------------------------------------------------------------------
# status_msg
#
# Used for temporarily flashing	a status message in the	status bar.  Better
# than a dialog	because	it can be ignored and will go away on its own.
#----------------------------------------------------------------------------

body Pltkwin::status_msg {msg}	{
    label_set $msg
    after 2500 [code $this label_reset]
}

#----------------------------------------------------------------------------
# label_reset
#
# Resets message in status bar to the default.
#----------------------------------------------------------------------------

body Pltkwin::label_reset {} {
    $itk_component(lstat) configure -text " [string range $this	1 end]"
}

#----------------------------------------------------------------------------
# label_set
#
# Sets message in status bar.
#----------------------------------------------------------------------------

body Pltkwin::label_set	{msg} {
    $itk_component(lstat) configure -text " $msg"
}

#----------------------------------------------------------------------------
# dplink
#
# Initializes socket data link between widget and client code.
# In addition, as this is the last client/server connection needed, I
# disable further connections.
#----------------------------------------------------------------------------

body Pltkwin::dplink	{client} {

    global list_sock data_sock

    dp_Host +
    set	rv [dp_connect -server 0]
    set	list_sock [lindex $rv 0]
    set	data_port [lindex $rv 1]

    dp_RDO $client set data_port $data_port
    set	data_sock [lindex [dp_accept $list_sock] 0]
    [plwin] openlink socket $data_sock
    dp_Host -
}
