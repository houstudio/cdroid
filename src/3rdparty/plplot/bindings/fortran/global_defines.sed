# sed script specifically designed to parse the #define statements in
# bindings/swig-support/plplotcapi.i into a file that can be used for
# global variable definitions for fortran.

# Insert text at the start:
1 i\
    ! Do not edit this generated file.  Instead, check its consistency\
    ! with the #defines in bindings/swig-support/plplotcapi.i using the\
    ! (Unix) target "check_fortran_parameters".  If that target reports an\
    ! inconsistency (via a cmp message) between the generated\
    ! plplot_parameters.inc_compare file in the build tree and\
    ! plplot_parameters.inc in the source tree, then copy\
    ! plplot_parameters.inc_compare on top of plplot_parameters.inc and\
    ! check in that result.\

# Drop every line that is not a #define
/^#define/! d

# Drop specific #defines from the file that are not available
# because of specific #if statements.
/OBJECT_DATA/ d
/^#define pltr_img/ d

# Translate trailing comment identifier into Fortran trailing comment.
/^#define/ s?//?!?
# Transform octal constants to Fortran (BOZ) form.
/^#define/ s? 0\([0-9][0-9]*\)? o'\1'?
# Transform hexadecimal constants to Fortran (BOZ) form.
/^#define/ s? 0[xX]\([0-9a-fA-F][0-9a-fA-F]*\)? z'\1'?
# Handle special case of z'80000000' which generates integer overflow
# for gfortran (and probably other Fortran compilers).
/^#define/ s? z'80000000'? ishft(1,31)?

# Parse what is left while preserving trailing comments.  Watch out
# for parentheses around value as well.
# Treat PL_NOTSET as a floating value and append special parameter lines for
# PL_PI and PL_TWOPI in this case as well.

/^#define/ s?^#define *\(PL_NOTSET\)[ (]*\([^ ]*\)[ )]*\(.*\)$?    real(kind=private_double), parameter :: \1 = \2.0_private_double\3\n    real(kind=private_double), parameter :: PL_PI = 3.1415926535897932384_private_double\n    real(kind=private_double), parameter :: PL_TWOPI = 2.0_private_double*PL_PI?

# Replace all hexadecimal BOZ constants by their decimal equivalents to maintain
# standards compliance rather than relying on an extension from the
# Fortran standards which allows BOZ constants in this context.
# (This extension is likely implemented by each different Fortran
# compiler, but you can never be sure.)

/^#define/ s?z'0*\([0-9]\)'?\1?
/^#define/ s?z'0*f'?15?
/^#define/ s?z'0*\([0-9]\)0'?\1*16?
/^#define/ s?z'0*\([0-9]\)00'?\1*16*16?
/^#define/ s?z'0*\([0-9]\)000'?\1*16*16*16?
/^#define/ s?z'0*\([0-9]\)0000'?\1*16*16*16*16?
# This last one is one more than we currently need.
/^#define/ s?z'0*\([0-9]\)00000'?\1*16*16*16*16*16?

# Note we could do the same transformation for octal BOZ constants, but we
# don't have any at present.


# Comment the following line so that we do not use a special form for BOZ constants.
#/^#define/ s?^#define *\([^ ]*\)[ (]*\([oz][^ ]*\)[ )]*\(.*\)$?    integer :: \1 \3\n      data \1 / \2 /?
# Look for pattern
#  ^#define
#  <blanks>
#  <anything but blanks> = \1
#  <blanks or left parentheses>
#  <anything but blanks>  = \2
#  <blanks or right parentheses>
#  <! followed by anything to EOL>
/^#define/ s?^#define *\([^ ]*\)[ (]*\([^ ]*\)[ )]*\(!.*\)$?    integer, parameter :: \1 = \2 \3?
# Look for pattern
#  ^#define
#  <blanks>
#  <anything but blanks> = \1
#  <blanks or left parentheses>
#  <anything but blanks>  = \2
#  <zero or more blanks to EOL>
/^#define/ s?^#define *\([^ ]*\)[ (]*\([^ ]*\)*$?    integer, parameter :: \1 = \2?
