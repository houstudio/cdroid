# sed script specifically designed to parse the #define statements in
# bindings/swig-support/plplotcapi.i into a file that can be used for
# global variable definitions for tcl.

# Insert text at the start:
1 i\
// Do not edit this generated file.  Instead, check its consistency\
// with the #defines in bindings/swig-support/plplotcapi.i using the\
// (Unix) target "check_tcl_parameters".  If that target reports an\
// inconsistency (via a cmp message) between the generated\
// plplot_parameters.h_compare file in the build tree and\
// plplot_parameters.h in the source tree, then copy\
// plplot_parameters.h_compare on top of plplot_parameters.h and\
// check in that result.\
\
//  This file contains a function to set the various global variables\
//  used by PLplot\
\
static void set_plplot_parameters( Tcl_Interp *interp )\
{\
    Tcl_Eval( interp, "namespace eval ::PLPLOT { \\n"

# Append text at the end:
$ a\
        "}" );\
}
# Drop every line that is not a #define
/^#define/! d

# Drop specific #defines from the file that are not available
# because of specific #if statements.
/OBJECT_DATA/ d
/^#define pltr_img/ d

# Translate trailing comment identifier into # trailing comment.
/^#define/ s?//?#?

# Parse what is left while preserving trailing comments.  Watch out
# for parentheses around value as well.  Use the expr syntax to
# convert all hexadecimal constants to integers.
/^#define/ s?^#define *\([^ ]*\)[ (]*\([^ ]*\)[ )]*\(.*\)$?        "\3\\n"\n        "variable \1 [expr \2]\\n"?

# Append extra constants not #defined in plplotcapi.i after PL_NOTSET
# is processed.
/PL_NOTSET/ a\
        "\\n"\
        "variable PL_PI 3.1415926535897932384\\n"
