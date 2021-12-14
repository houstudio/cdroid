#----------------------------------------------------------------------------
# help_gui w
#
# Create a top-level window that displays info for On GUI.. help item.
# Maurice LeBrun, IFS
# Adapted from mkStyles.tcl from widget demo.
#
# Arguments:
#    w -	Name to use for new top-level window.
#----------------------------------------------------------------------------

proc help_gui {{w .help_gui}} {
    catch {destroy $w}
    toplevel $w
    dpos $w
    wm title $w "Help on GUI"
    wm iconname $w "help_gui"
    normal_text_setup $w

    insertWithTags $w.t {\
      The PLPLOT/TK user interface is constructed from TK widgets, using Tcl
as a (dynamic) specification language.  The Tcl scripts provided with PLPLOT
control the layout of the user interface, initialization of the widgets, and
some of the binding of actions to events.  Widgets, on the other hand, are
all coded in C for speed.

      The fundamental addition to PLPLOT to allow it to use Tcl/TK effectively
has been the creation of a PLPLOT widget, called a "plframe".  It was
originally based on the TK frame widget but has many enhancements, as well
as close ties to the underlying graphics library.  The plframe widget can
be created, mapped, and destroyed basically just like a TK frame widget.
In addition, it supports numerous widget commands.

      The default PLPLOT/TK interface looks like a window with a top-level
menu bar, containing any number of frames devoted to plotting.  These child
windows contain a bar with an end-of-plot indicator, a "Plot" menu, a forward
page button (also a back page button when plrender is being used), and a
region for issuing status or prompt messages (initially containing the main
window name of the application).  Scrollbars appear to the right and bottom
of the plframe when in zoom mode.  The actual plframe has no decoration, but
interacts with all of the surrounding widgets.  The plframe and surrounding
widgets should be considered as one entity.

      There are several ways in which the plframe widget can be utilized, but
the most usual is via selection of the TK driver when running an
application linked to PLPLOT.  The structure of the code then can be
illustrated as follows:


   Parent process	           Communication	    Child process
		                  channel

       User-code
	|				           plserver
   plplot TK driver					|
   Tcl interpreter <------- TK send -------> Tcl interpreter
	|	 				|
       data writer  -------- named pipe- -----> plframe widget
					       data reader
						|
					     child X window


      The user-code and the renderer (called } normal
    insertWithTags $w.t {plserver} bold
    insertWithTags $w.t {) form a client/server
relationship, and are separate processes.  Under typical usage, where the
user-code starts first, it forks (vfork on systems that support it) and execs
the renderer.  Tcl/TK communication (via X properties) is used between their
embedded interpreters to send commands back and forth.  An extended version
of Tcl called Tcl-DP allows sockets to be used for this, and the model will
eventually be extended to allow the user code and the renderer to be running
on different machines.  The existence of an interpreter in each process
(actually each plplot stream that uses the TK driver) allows for a very easy
and robust method of sending commands between processes.  The data goes
through via an alternate channel for maximum speed.

Also supported are some alternate configurations:

(a) the server creates widgets, then launches applications passing the
      name of the plframe widget to plot to.  You could launch multiple
      codes, with each plotting into a specific widget.

(b) the server creates widgets, then calls functions to do the actual
      plotting.  This is in fact the easiest configuration since all the
      work is done in one process, but has the disadvantage that the
      user interface goes "dead" while the calculation is proceeding.

      By offloading the GUI-building code into a simple script language, it
becomes easy to build menus, buttons, scrollbars, etc.  All of these are
configurable by the user, as well as the ability to map events (such as
button presses) to functions calls in your own program. Note: the
alternate configurations as well as the customization options are not yet
fully tested.

      Most of the currently supported operations of the PLPLOT/TK user
interface are self-explanatory, and can be understood with a bit of
experimentation.  Better documentation will eventually follow.  Some
points to remember:

	1. When the plframe widget is created, it grabs the input focus to
allow easy advancement to the next plot by hitting <Return> (note: TK focus
doesn't behave quite the same as under other X toolkits).  However it is
possible for it to lose the focus by bringing up help windows and such.  If
the user interface seems to be responsive, try moving the mouse cursor into
the window you want to receive the input.  Most widgets (including plframe)
will grab the focus when you move the cursor into their window.

	2. There are sometimes keyboard-based shortcuts.  The ``OK'' button
at the bottom of most dialogs can typically be selected by hitting <Return>.
You can move between multiple input entries in a dialog by hitting <Tab>.
The text widgets used in the help entries can be scrolled by using the
up and down arrow keys, page up or page down keys, space, backspace, and
delete keys.

	3. A good way to customize the resources (fonts, colors, etc) used
in the interface is to copy the plconfig.tcl file, modify it according to
taste, then put it into a directory where it will be found at startup.  Also
a tclIndex file must be created for it (``plserver -mkidx'' will do this) and
put in the same directory.  The Tcl interpreter searches directories for
commands in the following order:

	 user-specified directory(s)	(set by -auto_load argument)
	 Current directory
	 ${PLPLOT_DIR}/tcl
	 ${HOME}/tcl
	 INSTALL_DIR/tcl

where HOME and PLPLOT_DIR are environmentals, and INSTALL_DIR is
set during installation (typically /usr/local/plplot).

I'll probably think of more later..
} normal
    $w.t configure -state disabled
    $w.t mark set insert 0.0
}
