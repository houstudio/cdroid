#----------------------------------------------------------------------------
# about w
#
# Create a top-level window that displays info for About.. menu item.
# Maurice LeBrun, IFS
# Adapted from mkStyles.tcl from widget demo.
#
# Arguments:
#    w -	Name to use for new top-level window.
#
# Note:
# There is a fundamental problem with the text widget, in that you can't
# get both decent word wrap and nice scrolling.  When proportional fonts are
# used (which are nicest for reading), the spacing comes out much different
# from that in the text file, leading to a very ragged right edge.  One way
# to try to fix this is to eliminate all <CR>'s from paragraphs and turn
# on word wrap with the "-wrap word" configuration option.  But this sucks
# because then scrolling becomes jerky, as a single line scroll moves the
# text by a whole paragraph!  (I tried it, it feels terrible.)  So, the only
# remaining thing to do is to format it by hand.  Ugh.
#----------------------------------------------------------------------------

proc about {{w .about}} {
    catch {destroy $w}
    toplevel $w
    dpos $w
    wm title $w "About PLPLOT"
    wm iconname $w "About"
    normal_text_setup $w 56

    insertWithTags $w.t {\

      This is the Tk output driver for PLplot, a cross-platform
software package for creating scientific plots. To help accomplish
that task it is organized as a core C library, language bindings for
that library, and device drivers which control how the plots are
presented in non-interactive and interactive plotting contexts.  The
PLplot core library can be used to create standard x-y plots, semi-log
plots, log-log plots, contour plots, 3D surface plots, mesh plots, bar
charts and pie charts. Multiple graphs (of the same or different
sizes) may be placed on a single page, and multiple pages are allowed
for those device formats that support them.  PLplot has core support
for Unicode. This means for our many Unicode-aware devices that plots
can be labelled using the enormous selection of Unicode mathematical
symbols. A large subset of our Unicode-aware devices also support
complex text layout (CTL) languages such as Arabic, Hebrew, and Indic
and Indic-derived CTL scripts such as Devanagari, Thai, Lao, and
Tibetan.  A number of compiled and interpreted languages have access
to PLplot, see our list of language bindings.  PLplot device drivers
support a number of different file formats for non-interactive
plotting and a number of different platforms that are suitable for
interactive plotting. It is easy to add new device drivers to PLplot
by writing a small number of device dependent routines.  PLplot is
free software primarily licensed under the LGPL.

Platforms: PLplot is currently known to work on Linux, Mac OS X, and
other Unices and Windows (2000, XP and Vista).  PLplot is built on
these platforms using our CMake-based build system.

Language Bindings: The core C PLplot library has language bindings in
Ada, C/C++/D, Fortran 77/90, Java, Lisp, Lua, OCaml, Octave,
Python, and Tcl/Tk.

File Devices: PLplot has devices that produce files in the CGM, GIF,
JPEG, LaTeX, PBM, PDF, PNG, PostScript, SVG, and Xfig formats.

GUI Devices: PLplot has devices that produce GUI's on the Gtk+,
PyQt, Qt, Tcl/Tk, wxWidgets, and X platforms.

      Tcl stands for ``tool command language'' and is pronounced ``tickle.''
Tk is an X11 toolkit that provides the Motif look and feel and is
implemented using the Tcl command language.  All of the fancy graphical
elements (buttons, menus, scrollbars, etc) in the user interface are
implemented via Tk with Tcl the glue that holds everything together.
The combination of Tcl and Tk is very powerful, providing easy access at
a high level to the workings of the graphical interface, which in turn
enables rapid prototyping and an unparalleled degree of flexibility.  The
PLPLOT Tk driver is built with these principles in mind, and provides
ways for the user to customize the interface (add buttons, menu entries,
callbacks to user code, etc).

For more information about PLplot (including complete documentation)
consult our website at http://plplot.org.

} normal
    $w.t configure -state disabled
    $w.t mark set insert 0.0
}
