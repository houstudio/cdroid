# Initialisation.  This file is automatically found and sourced by
# either Pltcl_Init or Pltk_Init.  If it cannot be found or sourced, those
# procedures will throw an error.

# This file is sourced by the Plplot-Tcl Init function
# It should be used for any general purpose initialization
# There is no need to source lots of other files, since
# auto-loading should be used for that purpose

# Turn this on if you can't figure out if this is being sourced.
#puts "In plplot.tcl"

# When plplot is initialized dynamically from Tk, it will source
# this file.
lappend auto_path $pllibrary
set env(PL_LIB) $pllibrary
# Set up defaults (auto-loaded from the auto_path)
if {![catch {package present Tk}]} {
    catch {pldefaults}
}
