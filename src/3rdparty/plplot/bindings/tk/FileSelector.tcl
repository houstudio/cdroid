# FileSelect
# ----------------------------------------------------------------------
# Implements a version of the OSF/Motif standard file selection dialog
# box using primitive widgets as the building blocks.
#
#
#   PUBLIC ATTRIBUTES:
#
#     -filterlabel ... label for filter entry, default "Filter"
#     -dirlabel ...... label for directory list, default "Directories"
#     -filelabel ..... label for file list, default "Files"
#     -selectlabel ... label for selection entry, default "Selection"
#     -dispfilter .... display filter, yes or no, default yes
#     -dispdir ....... display directory list, yes or no, default yes
#     -dispfile ...... display file list, yes or no, default yes
#     -dispselect .... display selection, yes or no, default yes
#     -filter ........ file list filter, defaults to "*"
#     -dir ........... initial directory, default to [pwd]
#     -title ......... window title text, default "Select File"
#     -full .......... display full file names, yes or no, default no
#
#     -width ......... width of filter/selection entry widgets in chars
#     -height ........ height of displayed list in lines
#
#   METHODS:
#
#     config ......... used to change public attributes
#     get ............ return the selection
#     activate ....... perform a grab, upon selection of ok(1) or cancel(0),
#                      return result.
#
#   USAGE:
#
#     FileSelect .fs -title "Test File Select" -full 0
#
#     if {[.fs activate]} {
#        puts stdout "OK >>==> [.fs get]"
#     } else {
#        puts stdout "Cancel"
#     }
#
#     .fs destroy
#
#   X11 OPTION DATABASE ATTRIBUTES
#
#     ...and the rest of the usual widget attributes
#
# ----------------------------------------------------------------------
#   AUTHOR:  Mark L. Ulferts          Phone: (214) 519-3947
#            DSC Communications Corp  E-mail: mulferts@spd.dsccc.com
# ----------------------------------------------------------------------


itcl_class FileSelect {

    # ------------------------------------------------------------------
    #  CONSTRUCTOR - create new scrolled text
    # ------------------------------------------------------------------
    constructor {config} {
	set class [$this info class]
	::rename $this $this-tmp-
	::toplevel $this -class $class
	::rename $this $this-win-
	::rename $this-tmp- $this

	#
	# Set the minimum size to 0,0 to allow both shrinkage and expansion.
	# Also, set the title if one has been given.
	#
	wm minsize $this 250 250
	wm title $this "$title"
	wm transient $this .
	wm withdraw $this

        #
	# Create an overall frame and separate frames for the filter,
	# lists, selection, and buttons.
	#
	set f [frame $this.fs]

	set fs(filterf) [frame $f.filterf]
	set fs(listf) [frame $f.listf]
	set fs(self) [frame $f.self]
	set fs(btnf) [frame $f.btnf -height 30]

	#
	# Create the label and entry widgets for the filter.  Turn off
	# the selection capability, at least from the visual aspect.
	#
	label $fs(filterf).label -text "$filterlabel"
	set fs(filter) [entry $fs(filterf).entry -relief sunken]
	$fs(filter) config -selectbackground \
		[lindex [$fs(filter) config -bg] 4] -selectborderwidth 0

	pack $fs(filterf).label -side top -anchor w
	pack $fs(filterf).entry -side bottom -fill x -expand yes

	#
	# Create directory list, scrollbar, and label for the directory
	# frame.  Make the list single select.
	#
	set fs(dirf) [frame $fs(listf).dirf]
	label $fs(dirf).label -text "$dirlabel"
	set fs(dirs) [listbox $fs(dirf).list -relief sunken \
		-yscroll "$fs(dirf).vscroll set" \
		-exportselection no]

	tk_listboxSingleSelect $fs(dirs)

	scrollbar $fs(dirf).vscroll -orient vertical -relief sunken \
		-command "$fs(dirf).list yview"

	pack $fs(dirf).label -side top -anchor w
	pack $fs(dirf).vscroll -side right -fill y
	pack $fs(dirf).list -side left -expand yes -fill both

	#
	# Create file list, scrollbar, and label for the file frame.
	# Again, make the list single select.
	#
        set fs(filef) [frame $fs(listf).filef]
	label $fs(filef).label -text "$filelabel"
	set fs(files) [listbox $fs(filef).list -relief sunken \
		-yscroll "$fs(filef).vscroll set" \
		-exportselection no]

	tk_listboxSingleSelect $fs(files)

	scrollbar $fs(filef).vscroll -orient vertical -relief sunken \
		-command "$fs(filef).list yview"

	pack $fs(filef).label -side top -anchor w
	pack $fs(filef).vscroll -side right -fill y
	pack $fs(filef).list -side left -expand yes -fill both

        #
        # Pack the directory and file lists based on the attributes
	# for displaying each list.  Add a filler frame between the
	# lists if both list are displayed.
        #
        frame $fs(listf).buf -width $_margin -borderwidth 0

        if {$dispdir} {pack $fs(dirf) -side left -fill both -expand yes}

	if {$dispdir && $dispfile} {pack $fs(listf).buf -side left}

	if {$dispfile} {pack $fs(filef) -side right -fill both -expand yes}

	#
	# Create the label and entry widgets for the selection frame. Turn
	# off the selection capability, at least from the visual aspect
	#
	label $fs(self).label -text "$selectlabel"
	set fs(select) [entry $fs(self).entry -relief sunken]
	$fs(select) config -selectbackground \
		[lindex [$fs(select) config -bg] 4] -selectborderwidth 0
	config -_selection "$dir/"

	pack $fs(self).label -side top -anchor w
	pack $fs(self).entry -side bottom -fill x -expand yes

	#
	# Add the separator and create the buttons in the button frame.
	# Each button is within a frame used to display as default.
	# The placer is used to locate the three buttons at relative
	# locations.
	#
	frame $f.line -height 2 -width 2 -borderwidth 1 -relief sunken

	frame $fs(btnf).okf -borderwidth 1
	set fs(okbtn) [button $fs(btnf).okf.ok -text OK -width 8]
	pack $fs(btnf).okf.ok -padx 2 -pady 2
	raise $fs(btnf).okf.ok

	frame $fs(btnf).ff -borderwidth 1
	set fs(filterbtn) [button $fs(btnf).ff.f -text Filter -width 8 \
		-command "$this _filtercmd"]
	pack $fs(btnf).ff.f -padx 2 -pady 2
	raise $fs(btnf).ff.f

	frame $fs(btnf).cf -borderwidth 1
	set fs(cancelbtn) [button $fs(btnf).cf.c -text Cancel -width 8]
	pack $fs(btnf).cf.c -padx 2 -pady 2
	raise $fs(btnf).cf.c

	place $fs(btnf).okf -relx 0 -rely 0.5 -anchor w
	place $fs(btnf).ff -relx 0.5 -rely 0.5 -anchor center
	place $fs(btnf).cf -relx 1 -rely 0.5 -anchor e

	#
	# Pack all the components of the file selection box.  The filter
	# and selection frames are packed based on the display attributes.
	#

        if {$dispfilter} {pack $fs(filterf) -fill x -padx $_margin -pady 5}

	pack $fs(listf) -fill both -padx $_margin -pady 5 -expand yes

	if {$dispselect} {pack $fs(self) -fill x -padx $_margin -pady 5}

	pack $f.line -fill x -pady 5
	pack $fs(btnf) -fill x -padx $_margin -pady 5

	pack $f -fill both -expand yes

	#
	# Set up the bindings for the list widgets. Single click in either
	# list executes a select method.  Double click for the both lists
	# selects the entry and then invokes the button callback.  Focus
	# events for the filter and select entry widgets control the default
	# button display, and return is mapped to the default button as well.
	#
        bind $fs(dirs) <1> "$this _selectdir %y"
        bind $fs(files) <1> "$this _selectfile %y"
        bind $fs(dirs) <Double-1> "$this _selectdir %y; $fs(filterbtn) invoke"
        bind $fs(files) <Double-1> "$this _selectfile %y; $fs(okbtn) invoke"

	bind $fs(filter) <FocusIn> "$this _defaultbtn filter"
	bind $fs(select) <FocusIn> "$this _defaultbtn ok"
	bind $fs(filter) <Return> "$fs(filterbtn) invoke"
	bind $fs(select) <Return> "$fs(okbtn) invoke"

	#
	# Explicitly handle configs that may have been ignored earlier.
	# Also, check to see if the user has specified, width, or height.
	# If not, use the default and config.
	#
	if {! [regexp {FileSelect::width} $config]} {
	    config -width $width
	}
	if {! [regexp {FileSelect::height} $config]} {
	    config -height $height
	}

	foreach attr $config {
	    config -$attr [set $attr]
	}

	#
	# Construction is complete.  Now set up the initial text for the
	# filter, selection, and both lists.  Finally, set the focus for
	# either the filter or select widget based on the display attribute.
	#
	_setfilter $dir $filter
	_setselection
	_filldirlist
	_fillfilelist

	if {$dispfilter} {
	    focus $fs(filter)
	} elseif {$dispselect} {
	    focus $fs(select)
	}
    }

    # ------------------------------------------------------------------
    #  METHOD:  config - used to change public attributes
    # ------------------------------------------------------------------
    method config {config} {}

    # ------------------------------------------------------------------
    #  DESTRUCTOR - destroy window containing widget
    # ------------------------------------------------------------------
    destructor {
	::rename $this-win- {}
	tk_destroy $this
    }

    # ------------------------------------------------------------------
    #  METHOD:  activate - Perform a grab operation, install the button
    #                      callbacks, and wait for the result.  Make sure
    #                      to reset the working directory back to the
    #                      original before returning the result.
    # ------------------------------------------------------------------
    method activate {} {
	global result

	set curwd "[pwd]"

	wm deiconify $this
	grab $this

	$fs(okbtn) config -command "$this _okcmd"
	$fs(cancelbtn) config -command "$this _cancelcmd"

	tkwait variable result($this)
	wm withdraw $this
	cd $curwd
	return $result($this)
    }

    # ------------------------------------------------------------------
    #  METHOD:  get - Return the selection.
    # ------------------------------------------------------------------
    method get {} {
	return $_selection
    }

    # ------------------------------------------------------------------
    #  METHOD:  _selectdir - Select the directory, set the filter to
    #                        the new directory.  Set the selection to the
    #                        new directory if the file list is not
    #                        displayed.  Mark the filter button as the
    #                        default.
    # ------------------------------------------------------------------
    method _selectdir {y} {
	$fs(dirs) select clear
	$fs(dirs) select to [$fs(dirs) nearest $y]

	set curwd "[pwd]"
	set seldir [$fs(dirs) get [$fs(dirs) curselection]]

	if {$seldir == "."} {
	    cd .
	} elseif {$seldir == ".."} {
	    cd ..
	} else {
	    cd $seldir
	}

	_setfilter "[pwd]"

	if {! $dispfile} {
	    _setselection "[pwd]"
	}

	cd $curwd

	_defaultbtn filter
    }

    # ------------------------------------------------------------------
    #  METHOD:  _selectfile - Select the file, set the selection to
    #                         the new file.  Mark the ok button as the
    #                         default.
    # ------------------------------------------------------------------
    method _selectfile {y} {
	$fs(files) select clear
	$fs(files) select to [$fs(files) nearest $y]

	config -_selection $dir/[$fs(files) get [$fs(files) curselection]]
	_setselection

	_defaultbtn ok
    }

    # ------------------------------------------------------------------
    #  METHOD:  _setfilter - Update the filter based on the parameters.
    #                        If the directory 'd' parameter is null, use
    #                        the 'dir' attribute.  If the file 'f'
    #                        parameter is null use the tail of the filter
    #                        entry text.
    # ------------------------------------------------------------------
    method _setfilter {{d ""} {f ""}} {
	if {$d == ""} {set d [file dirname [$fs(filter) get]]}
	if {$f == ""} {set f [file tail [$fs(filter) get]]}

	$fs(filter) delete 0 end
	$fs(filter) insert 0 "$d/$f"
    }

    # ------------------------------------------------------------------
    #  METHOD:  _setselection - Update the selection based on the
    #                           parameter.  If the file 'f' parameter is
    #                           null, use the 'selection' attribute.
    # ------------------------------------------------------------------
    method _setselection {{f ""}} {
	if {$f == ""} {set f $_selection}

	$fs(select) delete 0 end
	$fs(select) insert end "$f"
    }

    # ------------------------------------------------------------------
    #  METHOD:  _okcmd - Ok button callback.  Set the default button to
    #                    OK, and set the selection attribute to the
    #                    current entry widget text.  Undo the callbacks
    #                    for both the ok and cancel buttons.  Release
    #                    the grab and set the global result, which frees
    #                    the wait.
    # ------------------------------------------------------------------
    method _okcmd {} {
	global result

	_defaultbtn ok

	config -_selection [$fs(select) get]

	$fs(okbtn) config -command {}
	$fs(cancelbtn) config -command {}

	grab release $this
	set result($this) 1
    }

    # ------------------------------------------------------------------
    #  METHOD:  _filtercmd - Filter button callback.  Change directories
    #                        as needed, and set the dir, filter, and
    #                        selection attributes.  Change the filter and
    #                        selection text along with the list contents.
    #                        Mark the default button as filter.
    # ------------------------------------------------------------------
    method _filtercmd {} {
	set seldir [file dirname [$fs(filter) get]]

	cd $seldir

	config -dir "[pwd]"
	config -filter "[file tail [$fs(filter) get]]"
	config -_selection "$dir/"

	_setfilter
	_setselection
	_filldirlist
	_fillfilelist

	_defaultbtn filter
    }

    # ------------------------------------------------------------------
    #  METHOD:  _cancelcmd - Cancel button callback.  Set the default
    #                        button to cancel, undo the callbacks, and
    #                        release the grab and wait via the global
    #                        result variable.
    # ------------------------------------------------------------------
    method _cancelcmd {} {
	global result

	_defaultbtn cancel

	$fs(okbtn) config -command {}
	$fs(cancelbtn) config -command {}

	grab release $this
	set result($this) 0
    }

    # ------------------------------------------------------------------
    #  METHOD:  _filldirlist - Clear the directory list filling with the
    #                          results of an 'ls'.  Use the full attribute
    #                          to determine full file name insertion.
    #                          Select the first element if it exists.
    # ------------------------------------------------------------------
    method _filldirlist {} {
	$fs(dirs) delete 0 end

	foreach i [exec /bin/ls -a $dir] {
	    if {[file isdirectory $i]} {
		if {$full} {
		    $fs(dirs) insert end "$dir/$i"
		} else {
		    $fs(dirs) insert end [file tail $i]
		}
	    }
	}

	if {[$fs(dirs) size]} {
	    $fs(dirs) select clear
	    $fs(dirs) select to 0
	}
    }

    # ------------------------------------------------------------------
    #  METHOD:  _fillfilelist - Clear the file list filling with the
    #                           results of an 'glob'.  Use the full
    #                           attribute to determine full file name
    #                           insertion.  Select the first element if
    #                           it exists.
    # ------------------------------------------------------------------
    method _fillfilelist {} {
	$fs(files) delete 0 end

	foreach i [glob -nocomplain $dir/$filter] {
	    if {! [file isdirectory $i]} {
		if {$full} {
		    $fs(files) insert end $i
		} else {
		    $fs(files) insert end [file tail $i]
		}
	    }
	}

	if {[$fs(files) size]} {
	    $fs(files) select clear
	    $fs(files) select to 0
	}
    }

    # ------------------------------------------------------------------
    #  METHOD:  _defaultbtn - Sets the default button, either ok, filter
    #                         or cancel.  The focus is also adjusted.
    # ------------------------------------------------------------------
    method _defaultbtn {btn} {
	if {$btn == "ok"} {
	    $fs(btnf).okf config -relief sunken
	    $fs(btnf).ff config -relief flat
	    $fs(btnf).cf config -relief flat

	    focus $fs(select)
	} elseif {$btn == "filter"} {
	    $fs(btnf).okf config -relief flat
	    $fs(btnf).ff config -relief sunken
	    $fs(btnf).cf config -relief flat

	    focus $fs(filter)
	} elseif {$btn == "cancel"} {
	    $fs(btnf).okf config -relief flat
	    $fs(btnf).ff config -relief flat
	    $fs(btnf).cf config -relief sunken

	    focus none
	}
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE title - Set the window title.
    # ------------------------------------------------------------------
    public title "Select File" {
	if {[winfo exists $this]} {
	    wm title $this $title
	}
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE filterlabel - The label string above the filter widget.
    # ------------------------------------------------------------------
    public filterlabel "Filter" {
	if {[winfo exists $this]} {
	    $fs(filterf).label config -text $filterlabel
	}
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE filter - File list filter, defaults to "*".
    # ------------------------------------------------------------------
    public filter "*" {}

    # ------------------------------------------------------------------
    #  ATTRIBUTE dir - Initial directory, default to [pwd].
    # ------------------------------------------------------------------
    public dir "[pwd]" {
	cd $dir
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE dirlabel - Label for directory list, default "Directories"
    # ------------------------------------------------------------------
    public dirlabel "Directories" {
	if {[winfo exists $this]} {
	    $fs(dirf).label config -text $dirlabel
	}
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE filelabel - Label for file list, default "Files".
    # ------------------------------------------------------------------
    public filelabel "Files" {
	if {[winfo exists $this]} {
	    $fs(filef).label config -text $filelabel
	}
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE selectlabel - Label for selection entry, default
    #                          "Selection".
    # ------------------------------------------------------------------
    public selectlabel "Selection" {
	if {[winfo exists $this]} {
	    $fs(self).label config -text $selectlabel
	}
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE full - Display full file names, yes/no, default no.
    # ------------------------------------------------------------------
    public full 0 {}

    # ------------------------------------------------------------------
    #  ATTRIBUTE dispfilter - Display filter, yes or no, default yes
    # ------------------------------------------------------------------
    public dispfilter 1 {}

    # ------------------------------------------------------------------
    #  ATTRIBUTE dispdir - Display directory list, yes or no, default yes
    # ------------------------------------------------------------------
    public dispdir 1 {}

    # ------------------------------------------------------------------
    #  ATTRIBUTE dispfile - Display file list, yes or no, default yes
    # ------------------------------------------------------------------
    public dispfile 1 {}

    # ------------------------------------------------------------------
    #  ATTRIBUTE dispselect - Display selection, yes or no, default yes
    # ------------------------------------------------------------------
    public dispselect 1 {}

    # ------------------------------------------------------------------
    #  ATTRIBUTE width - Set the width of the selection and filter entry
    #                    widgets.
    # ------------------------------------------------------------------
    public width 50 {
	if {[winfo exists $this]} {
	    $fs(filter) config -width $width
	    $fs(select) config -width $width
	}
    }

    # ------------------------------------------------------------------
    #  ATTRIBUTE height - Set the height of the directory and file lists.
    # ------------------------------------------------------------------
    public height 10 {
	if {[winfo exists $this]} {
	    $fs(dirs) config -geometry [expr $width / 2]x$height
	    $fs(files) config -geometry [expr $width / 2]x$height
	}
    }

    # ------------------------------------------------------------------
    #  PRIVATE ATTRIBUTE _margin - Margin distance used in construction
    # ------------------------------------------------------------------
    public _margin 10 {}

    # ------------------------------------------------------------------
    #  PRIVATE ATTRIBUTE _selection - Current selection.
    # ------------------------------------------------------------------
    public _selection "[pwd]/" {}

    protected fs
}
