# Plwindow.tcl
# Vince Darley
# 1997-1998
# Geoffrey Furnish
# 9 May 1994
#
# @> [incr Tcl] interface to PLplot
#
###############################################################################

option add *Plwindow.width 250 widgetDefault
option add *Plwindow.height 250 widgetDefault

package require Plplotter
eval package require $pl_itk_package_name
#
# Usual options.
#
itk::usual Plwindow {
    keep -background -cursor -foreground \
      -plotbackground -xhairs -doublebuffer -width -height
}

itcl::class Plwindow {
    inherit itk::Widget

    constructor {args} {
	itk_component add plwin {
	    plframe $itk_interior.plwin -relief sunken
	} {
	    rename -background -plotbackground plotBackground Background

	    keep -width -height -xhairs -doublebuffer
	}
	grid $itk_component(plwin) -row 1 -column 0 -sticky nsew
	grid rowconfigure $itk_interior 1 -weight 1 -minsize 0

	eval itk_initialize $args
    }
    destructor {
    }

    method cmd {args} {
	uplevel 1 $itk_interior.plwin cmd $args
    }

    method plcont {data clev} {
	upvar $data d
	upvar $clev c
	cmd plcont d c
    }

    method plenv {xmin xmax ymin ymax just axis} {
	cmd plenv $xmin $xmax $ymin $ymax $just $axis
    }

    method pllab {xlab ylab toplab} {
	cmd pllab $xlab $ylab $toplab
    }

    method plline {n x y} {
	cmd plline $n $x $y
    }

    method plpoin {n x y code} {
	cmd plpoin $n $x $y $code
    }

    method plshade {data xmin xmax ymin ymax sh_min sh_max sh_col} {
	cmd plshade $data $xmin $xmax $ymin $ymax \
	  $sh_min $sh_max 1 $sh_col
    }

    public variable name
}









