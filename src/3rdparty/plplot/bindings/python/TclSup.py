# Tcl Support module.  For coping with Tcl-esque things in Python.

#from regsub import *
import re;

def TclList2Py(s):
    r = []
    if len(s) > 0:
        if s[0] == '{':
            # List elements enclosed in braces.
            j = 0
            itm = ""
            btwn_items = 1
            for i in range(len(s)):
                if btwn_items:
                    if s[i] == '{':
                        itm = ""
                        btwn_items = 0
                        continue

                if s[i] == '}':
                    # Finishing up an item.
                    r.append( itm )
                    btwn_items = 1
                    continue

                itm = itm + s[i]

        else:
            # List elements delimited by spaces
            r = re.split( s, ' ')

    return r
