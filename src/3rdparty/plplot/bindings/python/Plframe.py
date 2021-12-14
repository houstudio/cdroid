# Python 2 name is Tkinter, Python 3 name is tkinter (sigh).
import sys
if sys.version_info[:2] >= (3, 0):
    from tkinter import *
else:
    from Tkinter import *

import plplot as plplot_Plframe
from TclSup import *

from string import *

from math import *

CMD='cmd'

# Physically imported this function from Tkinter.py, b/c it evidently
# isn't made available via the above import statement.

def _flatten(tuple):
    res = ()
    for item in tuple:
        if type(item) in (TupleType, ListType):
            res = res + _flatten(item)
        elif item is not None:
            res = res + (item,)
    return res

#=============================================================================#
# class Plframe
#=============================================================================#

class Plframe(Widget):

    def __init__( self, master=None, cnf={}, **kw ):
        """Constructor.

        Initialize the Tk base class, and squirrel away the stream id,
        so that we will be able to ensure that future draw requests
        show up in the right place."""

        Widget.__init__( self, master, 'plframe', cnf, kw )

        self.strm = plplot_Plframe.plgstrm()

    def cmd( s, *args ):
        "Invoke a subcommand on the plframe widget."
        #apply( s.tk.call, (s._w, 'cmd',) + _flatten(args) )
        s.tk.call( s._w, 'cmd', *args)

    def info(s, what):
        return s.tk.call( s._w, 'info', what )

## Now implement the PLplot API.  For simple functions, can call
## straight to Tk, which is probably the most straightforward way, in
## the sense of making the Python/Tk widget look and act most like
## it's older brothplplot_Plframe.plot_Plframe. the Tcl version.  However, functions which rely
## on the Numeric extensions cannot work this way, so for those we
## call straight to the Python compiled interface to the PLplot API.

    def plenv( s, xmin, xmax, ymin, ymax, i, j ):
        s.cmd( 'plenv', xmin, xmax, ymin, ymax, i, j )

    def pleop(s):
        s.cmd( 'pleop' )

    def pllab( s, xlab, ylab, tlab ):
        s.cmd( 'pllab', xlab, ylab, tlab )

    def plline( s, x, y ):
        plsstrm( s.strm )
        plline( x, y )

    def plpoin( s, xs, ys, mark ):
        plsstrm( s.strm )
        plpoin( xs, ys, mark )

#=============================================================================#
# class PlXframe
#=============================================================================#

class PlXframe(Frame):

    """This class provides the same facilities as the Plframe, plus
    several additional facilities for advanced plot interaction, which
    are provided through menus and bindings."""

    def __init__( s, master=None, cnf={}, **kw ):
        """Constructor.

        Configure the widget database, and then set up the internal
        members.  Namely, a menubar and a contained Plframe."""

        Frame.__init__( s, master )

        s.setup_defaults()

        # We will be using the grid geometry manager to pack a plframe
        # widget along with some additional addornments.  The top row
        # (row 0) will contain the menubar.  The next row (row 1) will
        # hold the actual plframe, as well as the vertical scrollbar
        # when present..  And the last row (row 2) will hold the
        # horizontal scrollbar when present.  We want the menubar and
        # the horizontal scrollbar to use only the required space, and
        # we want the plframe to expand to fill any remaining space.
        # To accomplish this we set the weights for the top and bottom
        # row to zero, thus preventing them from expanding.

        s.rowconfigure( 0, weight=0, minsize=0 )
        s.rowconfigure( 1, weight=1, minsize=0 )
        s.rowconfigure( 2, weight=0, minsize=0 )

        # Similarly for the columns, we will use the left column (col
        # 0) for the plframe and the horizontal scrollbar, and the
        # right column (col 1) for the vertical scrollbar.  Moreover, the
        # menubar in the top row will span columns 0 and 1.  Again, we
        # want the rightmost column (col 0) to stay at whatever is the
        # minimum width to display its contents, and want the plframe
        # in the leftmost column to grow to fill all space.

        s.columnconfigure( 0, weight=1, minsize=0 )
        s.columnconfigure( 1, weight=0, minsize=0 )

        # Okay, now get the actual plframe widget so we can display
        # it.  And we pack it into its grid cell with all four sides
        # of the plframe sticking to the expandable size of the
        # enclosing grid cell, as arranged by the row,col weighting
        # scheme described above.

        s.plf = Plframe( s, kw )
        s.plf.grid( row=1, column=0, sticky=NSEW )

        s.build_menu_bar()

        s.strm = plplot_Plframe.plgstrm()

        s.setup_bindings()

        s.hscroll_exists = 0
        s.vscroll_exists = 0

## All this stuff is being based heavily on the Pltkwin.tcl thing by
## Vince, for itcl/itk (which in turn was based heavily on the
## plwidgets.tcl stuff by Maurice).

    def setup_defaults(s):
        #s.saveopt_dev = StringVar()
        s.saveopt_dev = "psc"
        #s.saveopt_dev.set( "psc" )

        #s.saveopt_file = IntVar()
        s.saveopt_file = 0
        #s.saveopt_file.set( 0 )

        # Now do the zoom support stuff.

        s.zidx = 0
        s.zxl = [ 0. ]; s.zyl = [ 0. ]
        s.zxr = [ 1. ]; s.zyr = [ 1. ]

    def setup_bindings(s):
        s.plf.bind( "<Any-KeyPress>", s.key_filter )
        s.plf.bind( "<Any-ButtonPress>", s.user_mouse )
        s.plf.bind( "<Any-Enter>", s.set_focus )

    def set_focus(s,e):
        # print("in set_focus")
        s.plf.focus()

    def build_menu_bar(s):
        "Create the menubar at the top of the PlXframe"

        s.ftop = Frame( s )
        s.ftop.grid( row=0, columnspan=2, sticky='ew' )

        s.ftop.plot = Menubutton( s.ftop, text="Plot", underline=0,
                                  relief=RAISED )
        s.ftop.plot.m = Menu( s.ftop.plot )
        s.ftop.plot["menu"] = s.ftop.plot.m

        s.ftop.plot.pack( side=LEFT )

        s.create_menu_print( s.ftop.plot.m )
        s.create_menu_save( s.ftop.plot.m )
        s.create_menu_orient( s.ftop.plot.m )
        s.create_menu_zoom( s.ftop.plot.m )
        s.create_menu_page( s.ftop.plot.m )
        s.create_menu_options( s.ftop.plot.m )
        s.create_menu_debug  ( s.ftop.plot.m )

        s.ftop.eop = Button( s.ftop, text="Clear",
                             command=s.clearpage )
        s.ftop.eop.pack( side=RIGHT )

        s.ftop.lstat = Label( s.ftop, anchor=W, relief=RAISED )
        s.ftop.lstat.pack( side=RIGHT, expand=1, fill=BOTH )

    def create_menu_print( s, m ):
        m.add( "command", label="Print", command=s.cmd_print )

    def create_menu_save( s, pmenu ):
        """Create the menu which lets us control the whole business of
        saving plots to disk."""

        m = Menu( pmenu )
        pmenu.add( "cascade", label='Save', menu=m )

        # Save - As

        m.add( "command", label="As", command=s.save_as )

        # Save - Again

        m.add( "command", label="Again", command=s.save_again,
               state=DISABLED )

        # Save - Close

        m.add( "command", label="Close", command=s.save_close,
               state=DISABLED )

        m.add( "separator" )

        # Save - Set device.. (another cascade)

        m.sdev = Menu( m )
        m.add( "cascade", label="Set device", menu=m.sdev )

        # Generate the device list in the "Save/Set device" widget
        # menu, by querying the plframe widget for the available
        # output devices (which are listed).

        devnames = s.plf.info( "devnames" )
        devkeys = s.plf.info( "devkeys" )

        # Oh no, these came back as Tcl lists.  Have to convert them
        # to Python lists.

        devnamlst = TclList2Py( devnames )
        devkeylst = TclList2Py( devkeys )

        #print("devnamlst = ", devnamlst)
        #print("devkeylst = ", devkeylst)
        #print("len(devnamlst) = ", len(devnamlst))

##      for i in range( len(devnamlst) ):
##          devnam = devnamlst[i]
##          devkey = devkeylst[i]
##
##          m.sdev.add( "radio", label=devnam, variable=s.saveopt_dev,
##                      value=devkey )

        # Now get the ball rolling by preinvoking one of these.
        # Default to the first one, whatever it is, but use psc
        # (Postscript color), if we can find it in the list of
        # available devices.

##      ivk = 1
##      for i in range( len(devnamlst) ):
##          if devkeylst[i] == "psc":
##              ivk = i+1
##              break
##
##      m.sdev.invoke( ivk )

        # Save - Set file type.. (another cascade)

        m.sfile = Menu( m )
        m.add( "cascade", label="Set file type", menu=m.sfile )

        m.sfile.add( "radio", label="Single file (one plot/file)",
                     variable=s.saveopt_file, value=0 )
        m.sfile.add( "radio", label="Archive file( many plots/file)",
                     variable=s.saveopt_file, value=1 )

        m.sfile.invoke( 1 )

    def create_menu_orient( s, pmenu ):
        m = Menu( pmenu )
        pmenu.add( "cascade", label='Orient', menu=m )

        m.config( postcommand=lambda o=s, x=m: o.update_orient(x) )

        # Orient - 0 degrees

        m.add( 'radio', label="0 degrees",
               command=lambda o=s: o.orient(0) )

        # Orient - 90 degrees

        m.add( 'radio', label="90 degrees",
               command=lambda o=s: o.orient(1) )

        # Orient - 180 degrees

        m.add( 'radio', label="180 degrees",
               command=lambda o=s: o.orient(2) )

        # Orient - 270 degrees

        m.add( 'radio', label="270 degrees",
               command=lambda o=s: o.orient(3) )

    def create_menu_zoom( s, pmenu ):
        m = Menu( pmenu )
        pmenu.add( "cascade", label='Zoom', menu=m )

        m.config( postcommand=lambda o=s, x=m: o.update_zoom(x) )

        # Zoom - select (by mouse)

        m.add( 'command', label="Select",
               command=lambda o=s: o.zoom_select() )

        # Zoom - back (go back 1 zoom level)

        m.add( 'command', label="Back",
               command=lambda o=s: o.zoom_back(),
               state='disabled' )

        # Zoom - forward (go forward 1 zoom level)

        m.add( 'command', label="Forward",
               command=lambda o=s: o.zoom_forward(),
               state='disabled' )

        # Zoom - enter bounds

        m.add( 'command', label="Enter bounds..",
               command=lambda o=s: o.zoom_enter() )

        # Zoom - reset

        m.add( 'command', label="Reset",
               command=lambda o=s: o.zoom_reset() )

        # Zoom - options (another cascade)

        zom = Menu( m )
        m.add( 'cascade', label="Options", menu=zom )

        s.zoomopts_asp = IntVar()
        s.zoomopts_sel = IntVar()

        zom.add( 'check', label="Preserve aspect ratio",
                 variable=s.zoomopts_asp )

        zom.add( 'separator' )

        zom.add( 'radio', label="Start from corner",
                 variable=s.zoomopts_sel, value=0 )

        zom.add( 'radio', label="Start from center",
                 variable=s.zoomopts_sel, value=1 )

        s.zoomopts_sel = 1
        s.zoomopts_asp = 1

        zom.invoke(1)
        zom.invoke(4)

    def create_menu_page( s, pmenu ):
        m = Menu( pmenu )
        pmenu.add( "cascade", label='Page', menu=m )

        # Page - enter bounds

        m.add( 'command', label="Setup..", command=s.page_enter )

        # Page - reset

        m.add( 'command', label="Reset", command=s.page_reset )

    def create_menu_options( s, pmenu ):
        m = Menu( pmenu )
        pmenu.add( 'cascade', label="Options", menu=m )

        m.add( 'command', label="Palette 0", command=s.plcmap0_edit )
        m.add( 'command', label="Palette 1", command=s.plcmap1_edit )

    def create_menu_debug( s, pmenu ):
        pmenu.add( "command", label="Debug PlXframe", command=s.debug )

## Now the commands needed to implement the menus.

    def key_filter( s, e ):
        """Process keystroke events, and parcel out to various control
        functions."""

        kn = e.keysym

        if kn == 'z':
            s.zoom_select()
        elif kn == 'b':
            s.zoom_back()
        elif kn == 'f':
            s.zoom_forward()
        elif kn == 'r':
            s.zoom_reset()
        else:
            pass
            print("Unknown keyname ", kn)

    def user_mouse( s, e ):
        print("in user_mouse")

## flash

    def cmd_print(s):
        s.label_set( "Printing plot..." )
        s.tk.call( s.plf._w, 'print' )
        # Should see about error trapping, like the itk widget does.

    def sucky_save(s):
        """A sucky save menu thing.  Needs to be enhanced to work like
        the one in Tcl/Itcl."""

        s.tk.call( s.plf._w, 'save', 'as', 'ps', 'xx.ps' )
        s.tk.call( s.plf._w, 'save', 'close' )
        print("Plot saved to xx.ps")

    def save_as(s): pass
    def save_again(s): pass
    def save_close(s): pass

    def update_zoom(s,m):
        """Configure zoom menu.

        Responsible for making sure zoom menu entries are normal or
        disabled as appropriate.  In particular, that 'Back' or 'Forward'
        are only displayed if it is possible to traverse the zoom windows
        list in that direction."""

        zframes = len( s.zxl )

        if s.zidx == 0:
            #print("disable back")
            m.entryconfig( 2, state=DISABLED )
        else:
            #print("enable back")
            m.entryconfig( 2, state=ACTIVE )

        if s.zidx == zframes-1:
            #print("disable forward")
            m.entryconfig( 3, state=DISABLED )
        else:
            #print("enable forward")
            m.entryconfig( 3, state=ACTIVE )

    def zoom_select(s):
        "Zooms plot in response to mouse selection."

        # In itk we save the existing binding so it can be restored.

        # Don't know how to save a binding for future use in Python/Tk.
##      s.def_button_cmd = s.plf.bind( "<ButtonPress>" )

##      global def_button_cmd zoomopts
##
##      set def_button_cmd [bind [plwin] <ButtonPress>]

        if s.zoomopts_sel == 0:
            s.label_set( "Click on one corner of zoom region." )
        elif s.zoomopts_sel == 1:
            s.label_set( "Click on center of zoom region." )

            s.plf.bind( "<ButtonPress>", s.zoom_start )

    def zoom_enter(s):
        print("zoom_enter")

##----------------------------------------------------------------------------
## zoom_reset
##
## Resets after zoom.
## Note that an explicit redraw is not necessary since the packer
## issues a resize after the scrollbars are unmapped.
##----------------------------------------------------------------------------

    def zoom_reset(s):

##      global def_button_cmd

        s.label_reset()
##      bind [plwin] <ButtonPress> $def_button_cmd

        s.tk.call( s.plf._w, 'view', 'reset' )

        if s.hscroll_exists and atoi( s.tk.call( 'winfo', 'ismapped', s.hscroll._w ) ):
            s.hscroll.grid_forget()

        if s.vscroll_exists and atoi( s.tk.call( 'winfo', 'ismapped', s.vscroll._w ) ):
            s.vscroll.grid_forget()

## Reset zoom windows list

        s.zidx = 0
        s.zxl = [ 0. ]; s.zyl = [ 0. ]
        s.zxr = [ 1. ]; s.zyr = [ 1. ]

    def update_orient(s,m):

        r = s.tk.call( s.plf._w, 'orient' )

        # Grr, this is a floating point string.  Must stand on our
        # heads to get an actual integer out of it.

        f = atof( r )
        fsi = "%.0f" % f
        i = atoi( fsi )

        n = i / 90
        n = i % 4

        m.invoke( n+1 )

    def orient(s, n):
        """Set the orientation of the plframe, but check to make sure
        we only do this if the new orientation is different from the
        old one."""

        oldori = s.tk.call( s.plf._w, 'orient' )
        oldf = atof( oldori )
        oldn = atoi( "%.0f" % oldf ) % 4

        if n != oldn:
            rots = "%d" % n
            s.tk.call( s.plf._w, 'orient', rots )

    def page_enter(s):
        print("in page_enter")

    def page_reset(s):
        print("in page_reset")

    def zoom_start( s, e ):
        "Starts plot zoom."

        s.wx = e.x
        s.wy = e.y

## Restore previous binding, but don't know how to do this in Python/Tk.
##      s.plf.bind( "<ButtonPress>", s.def_button_cmd )

##      global def_button_cmd
##
##      bind [plwin] <ButtonPress> $def_button_cmd

## Maybe what I should do for now is just remove the one we instlaled,
## but punt on restoring the prexisting binding.

        s.plf.bind( "<ButtonPress>", None )

## Hmpffff.  That didn't work...  Grrrrrr.

        s.label_set( "Select zoom region by dragging mouse, then release." )

        s.tk.call( s.plf._w, 'draw', 'init' )
        s.plf.bind( "<B1-Motion>", s.zoom_mouse_draw )
        s.plf.bind( "<B1-ButtonRelease>", s.zoom_mouse_end )

##----------------------------------------------------------------------------
## zoom_coords
##
## Transforms the initial and final mouse coordinates to either:
##
## opt =        0               device coordinates
## opt =        1               normalized device coordinates
##
## The global variable "zoomopts" is used to determine zoom behavior:
##
## zoomopts($this,0):
##      0       box     follows mouse movements exactly
##      1       box     follows mouse movements so that aspect ratio is preserved (default)
##
## zoomopts($this,1):
##      0       first and last points specified determine opposite corners
##              of zoom box.
##      1       box is centered about the first point clicked on,
##              perimeter follows mouse (default)
##
##----------------------------------------------------------------------------

    def zoom_coords( s, x0, y0, x1, y1, opt ):

        # Convert the integer input to float, prevents problems with
        # division.

        x0 = float(x0)
        y0 = float(y0)
        x1 = float(x1)
        y1 = float(y1)

        Lx = s.plf.winfo_width()
        Ly = s.plf.winfo_height()

        # Enforce boundaries in device coordinate space

        bounds = split( s.tk.call( s.plf._w, 'view', 'bounds' ) )
        xmin = Lx * atof( bounds[0] )
        ymin = Ly * atof( bounds[1] )
        xmax = Lx * atof( bounds[2] )
        ymax = Ly * atof( bounds[3] )

        x1 = max( xmin, min( xmax, x1 ) )
        y1 = max( ymin, min( ymax, y1 ) )

        # Two-corners zoom.

        if s.zoomopts_sel == 0:
            pass

##      if { $zoomopts($this,1) == 0 } then {

            # Get box lengths

            dx = x1 - x0
            dy = y1 - y0
##          set dx [expr $x1 - $x0]
##          set dy [expr $y1 - $y0]

            sign_dx = sign(dx)
            sign_dy = sign(dy)
##          set sign_dx [expr ($dx > 0) ? 1 : -1]
##          set sign_dy [expr ($dy > 0) ? 1 : -1]

            xl = x0
            yl = y0
##          set xl $x0
##          set yl $y0

            # Constant aspect ratio

            if s.zoomopts_asp == 1:
                pass
##          if { $zoomopts($this,0) == 1 } then {
##
##              # Scale factors used to maintain plot aspect ratio
##
##   set xscale [expr $xmax - $xmin]
##   set yscale [expr $ymax - $ymin]
##
##  # Adjust box size for proper aspect ratio
##
##   set rx [expr double(abs($dx)) / $xscale]
##   set ry [expr double(abs($dy)) / $yscale]
##
##   if { $rx > $ry } then {
##    set dy [expr $yscale * $rx * $sign_dy]
##   } else {
##    set dx [expr $xscale * $ry * $sign_dx]
##   }
##
##   set xr [expr $xl + $dx]
##   set yr [expr $yl + $dy]
##
##  # Now check again to see if in bounds, and adjust if not
##
##   if { $xr < $xmin || $xr > $xmax } then {
##    if { $xr < $xmin } then {
##     set dx [expr $xmin - $x0]
##    } else {
##     set dx [expr $xmax - $x0]
##    }
##    set rx [expr double(abs($dx)) / $xscale]
##    set dy [expr $yscale * $rx * $sign_dy]
##   }
##
##   if { $yr < $ymin || $yr > $ymax } then {
##    if { $yr < $ymin } then {
##     set dy [expr $ymin - $y0]
##    } else {
##     set dy [expr $ymax - $y0]
##    }
##    set ry [expr double(abs($dy)) / $yscale]
##    set dx [expr $xscale * $ry * $sign_dx]
##   }
##  }

            # Final box coordinates

            xr = xl + dx
            yr = yl + dy
##  set xr [expr $xl + $dx]
##  set yr [expr $yl + $dy]

### zoom from center out, preserving aspect ratio
        else:

            # Get box lengths, adjusting downward if necessary to keep
            # in bounds

            dx = abs( x1 - x0 )
            dy = abs( y1 - y0 )

            xr = x0 + dx;
            xl = x0 - dx
            yr = y0 + dy
            yl = y0 - dy

            if xl < xmin: dx = x0 - xmin
            if xr > xmax: dx = xmax - x0
            if yl < ymin: dy = y0 - ymin
            if yr > ymax: dy = ymax - y0

            # Constant aspect ratio

            if s.zoomopts_asp == 1:

                # Scale factors used to maintain plot aspect ratio

                xscale = xmax - xmin
                yscale = ymax - ymin

                # Adjust box size for proper aspect ratio

                rx = dx / xscale
                ry = dy / yscale

                if rx > ry:
                    dy = yscale * rx
                else:
                    dx = xscale * ry

                xr = x0 + dx
                xl = x0 - dx
                yr = y0 + dy
                yl = y0 - dy

                # Now check again to see if in bounds, and adjust
                # downward if not

                if xl < xmin:
                    dx = x0 - xmin
                    rx = dx / xscale
                    dy = yscale * rx
                if yr > ymax:
                    dx = xmax - x0
                    rx = dx / xscale
                    dy = yscale * rx
                if yl < ymin:
                    dy = y0 - ymin
                    ry = dy / yscale
                    dx = xscale * ry
                if yr > ymax:
                    dy = ymax - y0
                    ry = dy / yscale
                    dx = xscale * ry

            # Final box coordinates

            xr = x0 + dx
            xl = x0 - dx
            yr = y0 + dy
            yl = y0 - dy

## Optional translation to relative device coordinates.

        if opt == 1:
            wxl = xl / Lx
            wxr = xr / Lx
            wyl = 1.0 - float(yr) / Ly
            wyr = 1.0 - float(yl) / Ly
        else:
            wxr = xl
            wxl = xr
            wyr = yl
            wyl = yr

            return wxl, wyl, wxr, wyr

    def zoom_mouse_draw(s,e):
        "Draws zoom box in response to mouse motion (with button held down)."

        coords = s.zoom_coords( s.wx, s.wy, e.x, e.y, 0 )

        s.tk.call( s.plf._w, 'draw', 'rect',
                   coords[0], coords[1], coords[2], coords[3] )

    def zoom_mouse_end( s, e ):
        "Performs actual zoom, invoked when user releases mouse button."

        # Finish rubber band draw

        s.plf.bind( "<B1-ButtonRelease>" )
        s.plf.bind( "<B1-Motion>" )
##      bind [plwin] <B1-ButtonRelease> {}
##      bind [plwin] <B1-Motion> {}
        s.label_reset()
        s.tk.call( s.plf._w, 'draw', 'end' )

        # Select new plot region

        coords = s.zoom_coords( s.wx, s.wy, e.x, e.y, 1 )

        s.view_zoom( coords[0], coords[1], coords[2], coords[3] )

### Hmm, view_select is only called by update_view, which isn't called
### by anything...
##    def view_select( s, x0, y0, x1, y1 ):
##      """Handles change of view into plot.
##      Given in relative plot window coordinates."""
##
##      print("in view_select")
####body Pltkwin::view_select {x0 y0 x1 y1} {
##
#### Adjust arguments to be in bounds and properly ordered (xl < xr, etc)
##
####    set     xl [min $x0     $x1]
####    set     yl [min $y0     $y1]
####    set     xr [max $x0     $x1]
####    set     yr [max $y0     $y1]
##
##      xl = min( x0, x1 ); yl = min( y0, y1 )
##      xr = max( x0, x1 ); yr = max( y0, y1 )
##
####    set xmin 0.
####    set ymin 0.
####    set xmax 1.
####    set ymax 1.
##
##      xmin = 0.; ymin = 0.
##      xmax = 1.; ymax = 1.
##
####    set     xl [max $xmin [min $xmax $xl]]
####    set     yl [max $ymin [min $ymax $yl]]
####    set     xr [max $xmin [min $xmax $xr]]
####    set     yr [max $ymin [min $ymax $yr]]
##
##      xl = max( xmin, min( xmax, xl ) )
##      yl = max( ymin, min( ymax, yl ) )
##      xr = max( xmin, min( xmax, xr ) )
##      yr = max( ymin, min( ymax, yr ) )
##
#### Only create scrollbars if really needed.
##
####    if {($xl == $xmin) && ($xr == $xmax)} \
####    then {set hscroll 0} else {set hscroll 1}
####
####    if {($yl == $xmin) && ($yr == $xmax)} \
####    then {set vscroll 0} else {set vscroll 1}
####
####    if { ! ($hscroll || $vscroll)} {return}
##
##      if xl == xmin and xr == xmax:
##          hscroll = 0
##      else:
##          hscroll = 1
##
##      if yl == ymin and yr == ymax:
##          vscroll = 0
##      else:
##          vscroll = 1
##
##      if not (hscroll or vscroll):
##          return
##
#### Select plot region
##
####    [plwin] view select $xl $yl $xr $yr
##
##      s.tk.call( s.plf._w, 'view', 'select', xl, yl, xr, yr )
##
#### Fix up view
##
####    fixview $hscroll $vscroll
##      s.fixview( hscroll, vscroll )

    def view_zoom( s, x0, y0, x1, y1 ):
        "Handles zoom. Given in relative device coordinates."

## Adjust arguments to be properly ordered (xl < xr, etc)

        xl = min( x0, x1 )
        yl = min( y0, y1 )
        xr = max( x0, x1 )
        yr = max( y0, y1 )

## Check for double-click (specified zoom region less than a few
## pixels wide).  In this case, magnification is 2X in each direction,
## centered at the mouse location.  At the boundary, the magnification
## is determined by the distance to the boundary.

        stdzoom = .5
        if (xr - xl) < .02 and (yr - yl) < .02:
            nxl = xl - .5 * stdzoom
            nxr = xl + .5 * stdzoom
            if nxl < 0.:
                nxl = 0.
                nxr = 2. * xl
            if nxr > 1.:
                nxr = 1.
                nxl = 2. * xl - 1.
            xl = nxl
            xr = nxr

            nyl = yl - .5 * stdzoom
            nyr = yl + .5 * stdzoom
            if nyl < 0.:
                nyl = 0.
                nyr = 2. * yl
            if nyr > 1.:
                nyr = 1.
                nyl = 2. * yl - 1.
            yl = nyl
            yr = nyr

## Adjust arguments to be in bounds (in case margins are in effect).

        bounds = split( s.tk.call( s.plf._w, 'view', 'bounds' ) )
        xmin = atof( bounds[0] )
        ymin = atof( bounds[1] )
        xmax = atof( bounds[2] )
        ymax = atof( bounds[3] )

        xl = max( xmin, min( xmax, xl ) )
        yl = max( ymin, min( ymax, yl ) )
        xr = max( xmin, min( xmax, xr ) )
        yr = max( ymin, min( ymax, yr ) )

## Only create scrollbars if really needed.

        hscroll, vscroll = 0, 0
        if xl != xmin or xr != xmax: hscroll = 1
        if yl != ymin or yr != ymax: vscroll = 1

        if not (hscroll or yscroll):
            s.tk.call( s.plf._w, 'redraw' )
            return

## Select plot region

        s.tk.call( s.plf._w, 'view', 'zoom', xl, yl, xr, yr )

## Fix up view

        s.fixview( hscroll, vscroll )

## Add window to zoom windows list

        coords = split( s.tk.call( s.plf._w, 'view' ) )

        s.zidx = s.zidx + 1

        if s.zidx == len( s.zxl ):
            # Adding onto the end, no big deal
            s.zxl.append( atof( coords[0] ) )
            s.zyl.append( atof( coords[1] ) )
            s.zxr.append( atof( coords[2] ) )
            s.zyr.append( atof( coords[3] ) )
        else:
            # Adding into the middle...
            s.zxl[ s.zidx ] = atof( coords[0] )
            s.zyl[ s.zidx ] = atof( coords[1] )
            s.zxr[ s.zidx ] = atof( coords[2] )
            s.zyr[ s.zidx ] = atof( coords[3] )

            if s.zidx < len( s.zxl ) - 1:
                # Now chop off the end.
                s.zxl = s.zxl[0:s.zidx+1]
                s.zyl = s.zyl[0:s.zidx+1]
                s.zxr = s.zxr[0:s.zidx+1]
                s.zyr = s.zyr[0:s.zidx+1]

    def zoom_back(s):
        "Traverse the zoom windows list backward."

        if s.zidx > 0:
            s.zidx = s.zidx - 1

            xl = s.zxl[ s.zidx ]; yl = s.zyl[ s.zidx ]
            xr = s.zxr[ s.zidx ]; yr = s.zyr[ s.zidx ]

            # Select plot region

            s.tk.call( s.plf._w, 'view', 'select', xl, yl, xr, yr )

    def zoom_forward(s):
        "Traverse the zoom windows list forward."

        zframes = len( s.zxl )

        if zframes == 1 or s.zidx == zframes-1:
            return

        s.zidx = s.zidx + 1

        xl = s.zxl[ s.zidx ]; yl = s.zyl[ s.zidx ]
        xr = s.zxr[ s.zidx ]; yr = s.zyr[ s.zidx ]

        # Select plot region

        s.tk.call( s.plf._w, 'view', 'select', xl, yl, xr, yr )

    def view_scroll(s):
        print("in view_scroll")

    def fixview( s, hscroll, vscroll ):
        "Handles updates of scrollbars & plot after view change."

## Create scrollbars if they don't already exist.

        created_sb = 0
        if hscroll and not s.hscroll_exists:
            s.hscroll = Scrollbar( s, relief=SUNKEN, orient=HORIZONTAL )
            s.hscroll['command'] = ( s.plf._w, 'xscroll' )
            s.plf.config( xscroll=( s.hscroll, 'set' ) )
            s.hscroll_exists = 1
            created_sb = 1

        if vscroll and not s.vscroll_exists:
            s.vscroll = Scrollbar( s, relief=SUNKEN, orient=VERTICAL )
            s.vscroll['command'] = ( s.plf._w, 'yscroll' )
            s.plf.config( yscroll=( s.vscroll, 'set' ) )
            s.vscroll_exists = 1
            created_sb = 1

## When scrollbars are first created, it may be necessary to unmap
## then map the plframe widget so that it has a chance to initialize
## the scrollbars before they are mapped.

        if created_sb:
            s.plf.grid_forget()
            s.plf.grid( row=1, column=0, sticky='nsew' )

## Map scrollbars if not already mapped.  To get packing right, need
## to unmap then remap plot widget.  Otherwise need to do explicit
## redraw.

        if ( hscroll and not atoi( s.tk.call( 'winfo', 'ismapped', s.hscroll._w ) )
             or vscroll and not atoi( s.tk.call( 'winfo', 'ismapped', s.vscroll._w ) ) ) :
            s.update()
            s.plf.grid_forget()
            if hscroll:
                s.hscroll.grid( row=2, column=0, sticky='ew' )
            if vscroll:
                s.vscroll.grid( row=1, column=1, sticky='ns' )
                s.plf.grid( row=1, column=0, sticky='nsew' )
        else:
            s.tk.call( s.plf._w, 'redraw' )

## Hmmm.  Actually, "update_view" doesn't seem to be used by anything...
##    def update_view(s):
##      """Updates view.
##      Results in scrollbars being added if they are appropriate.
##      Does nothing if the plot window is unchanged from the default."""
##
##      print("in update_view")
####    set coords [[plwin] view]
####
####    set xl [lindex "$coords" 0]
####    set yl [lindex "$coords" 1]
####    set xr [lindex "$coords" 2]
####    set yr [lindex "$coords" 3]
####
####    view_select $xl $yl $xr $yr

    def status_msg(s,msg):
        s.label_set(msg)
        # schedule removal of the message with Tk "after"
        s.after( 2500, s.label_reset )

    def label_reset(s):
        s.ftop.lstat.config( text='' )

    def label_set(s,msg):
        s.ftop.lstat.config( text=msg )

    def plcmap0_edit(s):
        print("in plcmap0_edit")

    def plcmap1_edit(s):
        print("in plcmap1_edit")

    def clearpage(s):
        s.plf.setvar( 'wv', 1 )

## Now do the PLplot API.  Just vector these off to the contained
## Plframe widget.

# Define constants that are accessible from the API
# MAINTENANCE 2017-12
# These are taken from the "#define" section of bindings/swig-support/plplotcapi.i
# and transformed as follows:
# grep '^#define' bindings/swig-support/plplotcapi.i |sed -e '/#define / s?#define ?    ?' -e 's?//?#?g' -e 's? \([(0-9]\)? = \1?' | grep '=' >|/tmp/software
# and then that generated file was inserted here.
    PLESC_SET_RGB                    = 1  # obsolete
    PLESC_ALLOC_NCOL                 = 2  # obsolete
    PLESC_SET_LPB                    = 3  # obsolete
    PLESC_EXPOSE                     = 4  # handle window expose
    PLESC_RESIZE                     = 5  # handle window resize
    PLESC_REDRAW                     = 6  # handle window redraw
    PLESC_TEXT                       = 7  # switch to text screen
    PLESC_GRAPH                      = 8  # switch to graphics screen
    PLESC_FILL                       = 9  # fill polygon
    PLESC_DI                         = 10 # handle DI command
    PLESC_FLUSH                      = 11 # flush output
    PLESC_EH                         = 12 # handle Window events
    PLESC_GETC                       = 13 # get cursor position
    PLESC_SWIN                       = 14 # set window parameters
    PLESC_DOUBLEBUFFERING            = 15 # configure double buffering
    PLESC_XORMOD                     = 16 # set xor mode
    PLESC_SET_COMPRESSION            = 17 # AFR: set compression
    PLESC_CLEAR                      = 18 # RL: clear graphics region
    PLESC_DASH                       = 19 # RL: draw dashed line
    PLESC_HAS_TEXT                   = 20 # driver draws text
    PLESC_IMAGE                      = 21 # handle image
    PLESC_IMAGEOPS                   = 22 # plimage related operations
    PLESC_PL2DEVCOL                  = 23 # convert PLColor to device color
    PLESC_DEV2PLCOL                  = 24 # convert device color to PLColor
    PLESC_SETBGFG                    = 25 # set BG, FG colors
    PLESC_DEVINIT                    = 26 # alternate device initialization
    PLESC_GETBACKEND                 = 27 # get used backend of (wxWidgets) driver - no longer used
    PLESC_BEGIN_TEXT                 = 28 # get ready to draw a line of text
    PLESC_TEXT_CHAR                  = 29 # render a character of text
    PLESC_CONTROL_CHAR               = 30 # handle a text control character (super/subscript, etc.)
    PLESC_END_TEXT                   = 31 # finish a drawing a line of text
    PLESC_START_RASTERIZE            = 32 # start rasterized rendering
    PLESC_END_RASTERIZE              = 33 # end rasterized rendering
    PLESC_ARC                        = 34 # render an arc
    PLESC_GRADIENT                   = 35 # render a gradient
    PLESC_MODESET                    = 36 # set drawing mode
    PLESC_MODEGET                    = 37 # get drawing mode
    PLESC_FIXASPECT                  = 38 # set or unset fixing the aspect ratio of the plot
    PLESC_IMPORT_BUFFER              = 39 # set the contents of the buffer to a specified byte string
    PLESC_APPEND_BUFFER              = 40 # append the given byte string to the buffer
    PLESC_FLUSH_REMAINING_BUFFER     = 41 # flush the remaining buffer e.g. after new data was appended
    PLTEXT_FONTCHANGE                = 0  # font change in the text stream
    PLTEXT_SUPERSCRIPT               = 1  # superscript in the text stream
    PLTEXT_SUBSCRIPT                 = 2  # subscript in the text stream
    PLTEXT_BACKCHAR                  = 3  # back-char in the text stream
    PLTEXT_OVERLINE                  = 4  # toggle overline in the text stream
    PLTEXT_UNDERLINE                 = 5  # toggle underline in the text stream
    ZEROW2B                          = 1
    ZEROW2D                          = 2
    ONEW2B                           = 3
    ONEW2D                           = 4
    PLSWIN_DEVICE                    = 1      # device coordinates
    PLSWIN_WORLD                     = 2      # world coordinates
    PL_X_AXIS                        = 1      # The x-axis
    PL_Y_AXIS                        = 2      # The y-axis
    PL_Z_AXIS                        = 3      # The z-axis
    PL_OPT_ENABLED                   = 0x0001 # Obsolete
    PL_OPT_ARG                       = 0x0002 # Option has an argument
    PL_OPT_NODELETE                  = 0x0004 # Don't delete after processing
    PL_OPT_INVISIBLE                 = 0x0008 # Make invisible
    PL_OPT_DISABLED                  = 0x0010 # Processing is disabled
    PL_OPT_FUNC                      = 0x0100 # Call handler function
    PL_OPT_BOOL                      = 0x0200 # Set *var = 1
    PL_OPT_INT                       = 0x0400 # Set *var = atoi(optarg)
    PL_OPT_FLOAT                     = 0x0800 # Set *var = atof(optarg)
    PL_OPT_STRING                    = 0x1000 # Set var = optarg
    PL_PARSE_PARTIAL                 = 0x0000 # For backward compatibility
    PL_PARSE_FULL                    = 0x0001 # Process fully & exit if error
    PL_PARSE_QUIET                   = 0x0002 # Don't issue messages
    PL_PARSE_NODELETE                = 0x0004 # Don't delete options after
    PL_PARSE_SHOWALL                 = 0x0008 # Show invisible options
    PL_PARSE_OVERRIDE                = 0x0010 # Obsolete
    PL_PARSE_NOPROGRAM               = 0x0020 # Program name NOT in *argv[0]..
    PL_PARSE_NODASH                  = 0x0040 # Set if leading dash NOT required
    PL_PARSE_SKIP                    = 0x0080 # Skip over unrecognized args
    PL_FCI_MARK                      = 0x80000000
    PL_FCI_IMPOSSIBLE                = 0x00000000
    PL_FCI_HEXDIGIT_MASK             = 0xf
    PL_FCI_HEXPOWER_MASK             = 0x7
    PL_FCI_HEXPOWER_IMPOSSIBLE       = 0xf
    PL_FCI_FAMILY                    = 0x0
    PL_FCI_STYLE                     = 0x1
    PL_FCI_WEIGHT                    = 0x2
    PL_FCI_SANS                      = 0x0
    PL_FCI_SERIF                     = 0x1
    PL_FCI_MONO                      = 0x2
    PL_FCI_SCRIPT                    = 0x3
    PL_FCI_SYMBOL                    = 0x4
    PL_FCI_UPRIGHT                   = 0x0
    PL_FCI_ITALIC                    = 0x1
    PL_FCI_OBLIQUE                   = 0x2
    PL_FCI_MEDIUM                    = 0x0
    PL_FCI_BOLD                      = 0x1
    PL_MAXKEY                        = 16
    PL_MASK_SHIFT                    = 0x1    # ( 1 << 0 )
    PL_MASK_CAPS                     = 0x2    # ( 1 << 1 )
    PL_MASK_CONTROL                  = 0x4    # ( 1 << 2 )
    PL_MASK_ALT                      = 0x8    # ( 1 << 3 )
    PL_MASK_NUM                      = 0x10   # ( 1 << 4 )
    PL_MASK_ALTGR                    = 0x20   #  ( 1 << 5 )
    PL_MASK_WIN                      = 0x40   # ( 1 << 6 )
    PL_MASK_SCROLL                   = 0x80   # ( 1 << 7 )
    PL_MASK_BUTTON1                  = 0x100  # ( 1 << 8 )
    PL_MASK_BUTTON2                  = 0x200  # ( 1 << 9 )
    PL_MASK_BUTTON3                  = 0x400  # ( 1 << 10 )
    PL_MASK_BUTTON4                  = 0x800  # ( 1 << 11 )
    PL_MASK_BUTTON5                  = 0x1000 # ( 1 << 12 )
    PL_MAXWINDOWS                    = 64     # Max number of windows/page tracked
    PL_NOTSET                        = ( -42 )
    PLESC_DOUBLEBUFFERING_ENABLE     = 1
    PLESC_DOUBLEBUFFERING_DISABLE    = 2
    PLESC_DOUBLEBUFFERING_QUERY      = 3
    PL_BIN_DEFAULT                   = 0x0
    PL_BIN_CENTRED                   = 0x1
    PL_BIN_NOEXPAND                  = 0x2
    PL_BIN_NOEMPTY                   = 0x4
    GRID_CSA                         = 1 # Bivariate Cubic Spline approximation
    GRID_DTLI                        = 2 # Delaunay Triangulation Linear Interpolation
    GRID_NNI                         = 3 # Natural Neighbors Interpolation
    GRID_NNIDW                       = 4 # Nearest Neighbors Inverse Distance Weighted
    GRID_NNLI                        = 5 # Nearest Neighbors Linear Interpolation
    GRID_NNAIDW                      = 6 # Nearest Neighbors Around Inverse Distance Weighted
    PL_HIST_DEFAULT                  = 0x00
    PL_HIST_NOSCALING                = 0x01
    PL_HIST_IGNORE_OUTLIERS          = 0x02
    PL_HIST_NOEXPAND                 = 0x08
    PL_HIST_NOEMPTY                  = 0x10
    PL_POSITION_NULL                 = 0x0
    PL_POSITION_LEFT                 = 0x1
    PL_POSITION_RIGHT                = 0x2
    PL_POSITION_TOP                  = 0x4
    PL_POSITION_BOTTOM               = 0x8
    PL_POSITION_INSIDE               = 0x10
    PL_POSITION_OUTSIDE              = 0x20
    PL_POSITION_VIEWPORT             = 0x40
    PL_POSITION_SUBPAGE              = 0x80
    PL_LEGEND_NULL                   = 0x0
    PL_LEGEND_NONE                   = 0x1
    PL_LEGEND_COLOR_BOX              = 0x2
    PL_LEGEND_LINE                   = 0x4
    PL_LEGEND_SYMBOL                 = 0x8
    PL_LEGEND_TEXT_LEFT              = 0x10
    PL_LEGEND_BACKGROUND             = 0x20
    PL_LEGEND_BOUNDING_BOX           = 0x40
    PL_LEGEND_ROW_MAJOR              = 0x80
    PL_COLORBAR_NULL                 = 0x0
    PL_COLORBAR_LABEL_LEFT           = 0x1
    PL_COLORBAR_LABEL_RIGHT          = 0x2
    PL_COLORBAR_LABEL_TOP            = 0x4
    PL_COLORBAR_LABEL_BOTTOM         = 0x8
    PL_COLORBAR_IMAGE                = 0x10
    PL_COLORBAR_SHADE                = 0x20
    PL_COLORBAR_GRADIENT             = 0x40
    PL_COLORBAR_CAP_NONE             = 0x80
    PL_COLORBAR_CAP_LOW              = 0x100
    PL_COLORBAR_CAP_HIGH             = 0x200
    PL_COLORBAR_SHADE_LABEL          = 0x400
    PL_COLORBAR_ORIENT_RIGHT         = 0x800
    PL_COLORBAR_ORIENT_TOP           = 0x1000
    PL_COLORBAR_ORIENT_LEFT          = 0x2000
    PL_COLORBAR_ORIENT_BOTTOM        = 0x4000
    PL_COLORBAR_BACKGROUND           = 0x8000
    PL_COLORBAR_BOUNDING_BOX         = 0x10000
    PL_DRAWMODE_UNKNOWN              = 0x0
    PL_DRAWMODE_DEFAULT              = 0x1
    PL_DRAWMODE_REPLACE              = 0x2
    PL_DRAWMODE_XOR                  = 0x4
    DRAW_LINEX                       = 0x001 # draw lines parallel to the X axis
    DRAW_LINEY                       = 0x002 # draw lines parallel to the Y axis
    DRAW_LINEXY                      = 0x003 # draw lines parallel to both the X and Y axis
    MAG_COLOR                        = 0x004 # draw the mesh with a color dependent of the magnitude
    BASE_CONT                        = 0x008 # draw contour plot at bottom xy plane
    TOP_CONT                         = 0x010 # draw contour plot at top xy plane
    SURF_CONT                        = 0x020 # draw contour plot at surface
    DRAW_SIDES                       = 0x040 # draw sides
    FACETED                          = 0x080 # draw outline for each square that makes up the surface
    MESH                             = 0x100 # draw mesh
# End of constants.

    # Define pltr0, pltr1, and pltr2 as equivalent strings so they
    # can be turned back into callback functions by plcont, etc., in
    # plplot.py
    pltr0 = "pltr0"
    pltr1 = "pltr1"
    pltr2 = "pltr2"

    def cmd( s, *args ):
        "Invoke a subcommand on the plframe widget."
        #apply( s.tk.call, (s.plf._w, 'cmd',) + _flatten(args) )
        s.tk.call( s.plf._w, 'cmd', *args)

    def pl_setcontlabelformat(s, lexp, sigdig):
        s.cmd( 'pl_setcontlabelformat', lexp, sigdig )

    def pl_setcontlabelparam(s, offset, size, spacing, active):
        s.cmd( 'pl_setcontlabelparam', offset, size, spacing, active)

    def pladv( s, page ):
        s.cmd( 'pladv', page )

    def plarc( s, x, y, a, b, angle1, angle2, rotate, fill ):
        s.cmd( 'plarc', x, y, a, b, angle1, angle2, rotate, fill )

    def plaxes( s, x0, y0, xopt, xtick, nxsub, yopt, ytick, nysub ):
        s.cmd( 'plaxes', x0, y0, xopt, xtick, nxsub, yopt, ytick, nysub )

    def plbin(s): pass
    def plbop(s):
        s.cmd( 'plbop' )

    def plbox( s, xopt, xtick, nxsub, yopt, ytick, nysub ):
        s.cmd( 'plbox', xopt, xtick, nxsub, yopt, ytick, nysub )

    def plbox3( s, xopt, xlabel, xtick, nsubx,
                yopt, ylabel, ytick, nsuby,
                zopt, zlabel, ztick, nsubz ):
        s.cmd( 'plbox3',
               xopt, xlabel, xtick, nsubx,
               yopt, ylabel, ytick, nsuby,
               zopt, zlabel, ztick, nsubz )

    def plclear(s):
        s.cmd( 'plclear' )

    def plcol0( s, col0 ):
        s.cmd( 'plcol0', col0 )

    def plcol1( s, col1 ):
        s.cmd( 'plcol1', col1 )

    def plcolorbar( s, *args ):
        plplot_Plframe.plsstrm( s.strm )
        return plplot_Plframe.plcolorbar( *args )

    def plcont( s, *args ):
        plplot_Plframe.plsstrm( s.strm )
        #apply( plcont, args )
        plplot_Plframe.plcont(*args)

    def plfcont( s ): pass
    def plcpstream( s ): pass

    def plenv( s, xmin, xmax, ymin, ymax, i, j ):
        s.cmd( 'plenv', xmin, xmax, ymin, ymax, i, j )

    def pleop(s):
        s.cmd( 'pleop' )
        #print("should've waited here, but it didn't.")
        s.plf.setvar( 'wv', '0' )
        s.label_set( "Plotting paused ... (Hit Clear to continue)" )
        #print("preparing to wait for wv to change")
        s.plf.waitvar( 'wv' )
        #print("it changed.")
        s.label_reset()
        s.update()

    def plfill( s, x, y ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plfill( x, y )

    def plfont( s, ifnt ):
        s.cmd( 'plfont', ifnt )

    def plfontld( s, fnt ):
        s.cmd( 'plfontld', fnt )

    def plgcol0(s, i):
        plplot_Plframe.plsstrm( s.strm )
        return plplot_Plframe.plgcol0(i)

    def plgcolbg(s):
        plplot_Plframe.plsstrm( s.strm )
        return plplot_Plframe.plgcolbg()

    def plhist( s, data, datmin, datmax, nbin, oldwin ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plhist( data, datmin, datmax, nbin, oldwin )

    def plhlsrgb(namespace, h, l, s):
        plplot_Plframe.plsstrm( namespace.strm )
        return plplot_Plframe.plhlsrgb(h, l, s)

    def pljoin( s, x1, y1, x2, y2 ):
        s.cmd( 'pljoin', x1, y1, x2, y2 )

    def pllab( s, xlab, ylab, tlab ):
        s.cmd( 'pllab', xlab, ylab, tlab )

    def pllegend(s, opt, position, x, y, plot_width, bg_color, bb_color, bb_style, nrow, ncolumn, opt_array, text_offset, text_scale, text_spacing, test_justification, text_colors, text, box_colors, box_patterns, box_scales, box_line_widths, line_colors, line_styles, line_widths, symbol_colors, symbol_scales, symbol_numbers, symbols):
        plplot_Plframe.plsstrm( s.strm )
        return plplot_Plframe.pllegend(opt, position, x, y, plot_width, bg_color, bb_color, bb_style, nrow, ncolumn, opt_array, text_offset, text_scale, text_spacing, test_justification, text_colors, text, box_colors, box_patterns, box_scales, box_line_widths, line_colors, line_styles, line_widths, symbol_colors, symbol_scales, symbol_numbers, symbols)

    def pllightsource( s, x, y, z ):
        s.cmd( 'pllightsource', x, y, z )

    def plline( s, x, y ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plline( x, y )

    def plline3( s, x, y, z ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plline3( x, y, z )

    def pllsty( s, lin ):
        s.cmd( 'pllsty', lin )

    # map and merridians ommitted.

    def plmesh( s, x, y, z, opt ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plmesh( x, y, z, opt )

    def plmeshc( s, x, y, z, opt, clevel ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plmeshc( x, y, z, opt, clevel )

    def plmtex( s, side, disp, pos, just, text ):
        s.cmd( 'plmtex', side, disp, pos, just, text )

    def plot3d( s, x, y, z, opt, side ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plot3d( x, y, z, opt, side )

    def plpoin( s, xs, ys, mark ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plpoin( xs, ys, mark )

    def plpoin3( s, x, y, z, code ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plpoin3( x, y, z, code )

    def plpoly3( s, x, y, z, draw, ifcc ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plpoly3( x, y, z, draw, ifcc )

    def plprec( s, setp, prec ):
        s.cmd( 'plprec', setp, prec )

    def plpsty( s, patt ):
        s.cmd( 'plpsty', patt )

    def plptex( s, x, y, dx, dy, just, text ):
        s.cmd( 'plptex', x, y, dx, dy, just, text )

    def plreplot( s ):
        s.cmd( 'plreplot' )

    def plschr( s, dflt, scale ):
        s.cmd( 'plschr', dflt, scale )

    def plsmaj( s, dflt, scale ):
        s.cmd( 'plsmaj', dflt, scale )

    def plsmin( s, dflt, scale ):
        s.cmd( 'plsmin', dflt, scale )

    def plshade( s, *args ):
        plplot_Plframe.plsstrm( s.strm )
        #apply( plshade, args )
        plplot_Plframe.plshade(*args)

    def plshades( s, *args ):
        plplot_Plframe.plsstrm( s.strm )
        #apply( plshades, args )
        plplot_Plframe.plshades(*args)

    def plscmap0(s, r, g, b):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plscmap0(r, g, b)

    def plscmap0n(s, ncol1):
        s.cmd( 'plscmap0n', ncol1 )

    def plscmap1l(s, itype, intensity, coord1, coord2, coord3, *alt_hue_path):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plscmap1l(itype, intensity, coord1, coord2, coord3, *alt_hue_path)

    def plscmap1n(s, ncol1):
        s.cmd( 'plscmap1n', ncol1 )

    def plscol0( s, icol0, r, g, b ):
        s.cmd( 'plscol0', icol0, r, g, b )

    def plscol0a( s, icol0, r, g, b, a ):
        s.cmd( 'plscol0a', icol0, r, g, b, a )

    def plspal0(s, filename):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plspal0(filename)

    def plspal1(s, filename, interpolate):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plspal1(filename, interpolate)

    def plssub( s, nx, ny ):
        s.cmd( 'plssub', nx, ny )

    def plssym( s, dflt, scale ):
        s.cmd( 'plssym', dflt, scale )

    # plstar and plstart not relevant

    def plstring(s, x, y, string):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plstring(x, y, string)

    def plstring3(s, x, y, z, string):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plstring3(x, y, z, string)

    def plstyl( s, mark, space ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plstyl( mark, space )

    def plsurf3d(s, x, y, z, opt, clevel):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plsurf3d(x, y, z, opt, clevel)

    def plsurf3dl(s, x, y, z, opt, clevel, indexxmin, indexymin, indexymax):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plsurf3dl(x, y, z, opt, clevel,indexxmin, indexymin, indexymax)

    def plsvpa( s, xmin, xmax, ymin, ymax ):
        s.cmd( 'plsvpa', xmin, xmax, ymin, ymax )

    def plsxax( s, digmax, digits ):
        s.cmd( 'plsxax', digmax, digits )

    def plsyax( s, digmax, digits ):
        s.cmd( 'plsyax', digmax, digits )

    def plsym( s, x, y, code ):
        plplot_Plframe.plsstrm( s.strm )
        plplot_Plframe.plsym( x, y, code )

    def plszax( s, digmax, digits ):
        s.cmd( 'plszax', digmax, digits )

    def plvasp( s, aspect ):
        s.cmd( 'plvasp', aspect )

    def plvpas( s, xmin, xmax, ymin, ymax, aspect ):
        s.cmd( 'plvpas', xmin, xmax, ymin, ymax, aspect )

    def plvpor( s, xmin, xmax, ymin, ymax ):
        s.cmd( 'plvpor', xmin, xmax, ymin, ymax )

    def plvsta(s):
        s.cmd( 'plvsta' )

    def plw3d( s, basex, basey, height, xmin0,
               xmax0, ymin0, ymax0, zmin0, zmax0, alt, az):
        s.cmd( 'plw3d',
               basex, basey, height, xmin0,
               xmax0, ymin0, ymax0, zmin0, zmax0, alt, az)

    def plwidth( s, width ):
        s.cmd( 'plwidth', width )

    def plwind( s, xmin, xmax, ymin, ymax ):
        s.cmd( 'plwind', xmin, xmax, ymin, ymax )

    def debug(s):
        print("Debugging dump for PlXframe:")
        print("s.saveopt_dev = ", s.saveopt_dev)

## End of Plframe.py
