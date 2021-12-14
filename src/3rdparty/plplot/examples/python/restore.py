# Restore PLplot environment to defaults to eliminate
# Cross-talk between examples run under the same environment.
def main(w):
    w.plcol0(1)
    # Plots such as example 1 with multiple subpages per page
    # need some method to restore initial state with just
    # one subpage per page.  But the next two do not work.
    # and if you drop pladv, then the screen does not get
    # cleared between plots.  So address this issue later!
    # w.pladv(0)
    # Do plots with 1 subwindow per page
    # w.plssub(1, 1)
