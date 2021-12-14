# sed script specifically designed to parse the output of
# nm --defined-only bindings/fortran/libplplotfortran.so | sort --key=3 | tr '[:lower:]' '[:upper:]'
# to help generate a sorted ifort definition file.
# N.B. note the last stanza of that pipeline which
# converts everything to upper case.  That upper-case
# result is assumed below.

# Header comments, LIBRARY, and EXPORTS lines
# are generated independently by cmake.

# Drop every line that does not refer to a module symbol
/^.*__PLPLOT.*_MOD_/! d

# Drop additional specific lines
/^.*PLPLOT_PRIVATE_UTILITIES/ d
/^.*MATRIX_TO_C/ d
/^.*MOD___/ d

# Parse module symbols into form required by ifort .def file
s?.*__PLPLOT_DOUBLE_MOD_?PLPLOT_DOUBLE_mp_?
s?.*__PLPLOT_GRAPHICS_MOD_?PLPLOT_GRAPHICS_mp_?
s?.*__PLPLOT_SINGLE_MOD_?PLPLOT_SINGLE_mp_?
s?.*__PLPLOT_MOD_?PLPLOT_mp_?
