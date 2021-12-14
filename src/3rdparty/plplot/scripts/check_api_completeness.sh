#!/bin/bash
# This Linux-only script creates a sorted list of our API from several
# independent locations within our source tree and finds all the
# inconsistencies between them.

# This script should be run from the top-level source tree.

# Prepare API list from include/plplot.h to be compared with all others.

# First add functions which are known to be part of our public API but without
# the usual "c_" prefix folderol.  N.B. All functions will be added this
# way if/when we drop that folderol.
echo "plGetCursor" >| /tmp/plplot_api.txt
# Be sure to remove officially deprecated functions later, but
# there are none now.
grep '^#define.*pl.*c_pl' include/plplot.h |\
    tr '\t' " " |\
    tr -s " " |\
    cut --delimiter=" " --fields=2 \
	>> /tmp/plplot_api.txt
sort -o /tmp/plplot_api.txt /tmp/plplot_api.txt


case $1 in
    docbook)
	# Prepare API list from doc/docbook/src/api.xml
	# and compare with previous
	echo "documentation API differences (if any)"
	grep '<sect1.*id="pl.*".*renderas="sect.*">$' doc/docbook/src/api.xml |\
	    cut --delimiter='"' --fields=2 |\
	    sort |\
	    diff -au /tmp/plplot_api.txt -
	;;

    swig)
	# Prepare API list from bindings/swig-support/plplotcapi.i
	# and compare with previous
	echo "swig API differences (if any)"
	# The grep -v stanzas get rid of some of the non-public API that
	# is exposed for the swig-generated bindings.

	grep '^pl.*(' bindings/swig-support/plplotcapi.i |\
	    cut --delimiter='(' --fields=1 |\
	    grep -v plAlloc2dGrid |\
	    grep -v plClearOpts |\
	    grep -v plFindCommand |\
	    grep -v plFindName |\
	    grep -v plFree2dGrid |\
	    grep -v plGetFlt |\
	    grep -v plGetInt |\
	    grep -v plGetName |\
	    grep -v plMergeOpts |\
	    grep -v plMinMax2dGrid |\
	    grep -v plOptUsage |\
	    grep -v plResetOpts |\
	    grep -v plSetUsage |\
	    grep -v plTranslateCursor |\
	    grep -v plgDevs |\
	    grep -v plgFileDevs |\
	    grep -v plsButtonEH |\
	    grep -v plsError |\
	    grep -v plsKeyEH |\
	    grep -v pl_cmd |\
	    grep -v pldid2pc |\
	    grep -v pldip2dc |\
	    grep -v plf2eval |\
	    grep -v plf2eval2 |\
	    grep -v plf2evalr |\
	    grep -v plfcont |\
	    grep -v plfshade |\
	    grep -v plgesc |\
	    grep -v plgfile |\
	    grep -v plsexit |\
	    grep -v plsfile |\
	    grep -v plsxwin |\
	    grep -v pltr0f |\
	    grep -v pltr2f |\
	    grep -v pltr2p |\
	    sort |\
	    diff -au /tmp/plplot_api.txt -
	;;

    java)
	# Prepare API list from bindings/java/PLStream.java
	# and compare with previous.
	echo "java API differences (if any)"
	# The grep -v '[A-Z]' stanza gets rid of some of the non-public API that
	# is exposed.
	grep 'plplotjavac.pl.*(' bindings/java/PLStream.java |\
	    cut --delimiter='(' --fields=1 |\
	    cut --delimiter='.' --fields=2 |\
	    sort -u |\
	    grep -v '[A-Z]' |\
	    diff -au /tmp/plplot_api.txt -
	;;

    fortran)
	# Prepare API list from the fortran binding and compare with previous.
	echo "fortran API differences (if any)"
	# We search for all occurrences of 'bind(c,name=' (with arbitrary blanks) to determine
	# C names that are interfaced by our Fortran binding and parse that result with
	# cut and sed to obtain the C name (without "c_" prefix if that occurs).
	# We then do a unique
	# sort to get rid of duplicates, and specifically exclude some added special
	# fortran functions.
	grep -h 'bind( *c *, *name *=' bindings/fortran/*.f90 |\
	    cut --delimiter="'" --field=2 |\
	    sed -e 's?c_??' |\
	    sort -u |\
	    grep -v _private |\
	    grep -v _null |\
	    grep -v '^plf2evalr' |\
	    grep -v '^plfcont' |\
	    grep -v '^plfvect' |\
	    grep -v '^pltr' |\
	    diff -au /tmp/plplot_api.txt -
	;;

    c++)
	# Prepare API list from bindings/c++/plstream.h
	# and compare with previous.
	echo "c++ API differences (if any)"
	grep 'plstream::' bindings/c++/plstream.cc |\
	    grep -v '//' |\
	    grep '(' |\
	    cut --delimiter='(' --fields=1 |\
	    cut --delimiter=':' --fields=3 |\
	    sed 's/[^ ]*/pl&/' |\
	    sort -u |\
	    grep -v '[A-Z]' |\
	    diff -au /tmp/plplot_api.txt -
	;;

    tcl)
	# Prepare API list from bindings/tcl/plapi.tpl
	echo "tcl API differences (if any)"
	( grep '^pltclcmd' bindings/tcl/plapi.tpl |\
		cut --delimiter=' ' --fields=2  && \
		grep '{"pl' bindings/tcl/tclAPI.c | \
		    cut --delimiter='"' --fields=2 \
	) | \
	    sort -u | \
	    diff -au /tmp/plplot_api.txt -
	;;

    all)
	$0 docbook
	$0 swig
	$0 java
	$0 fortran
	$0 c++
	$0 tcl
	;;

    *)
	echo "First argument was $1"
	echo "Instead, it must be one of the following:"
	echo "docbook, swig, java, fortran, c++, tcl or all"
	;;
esac
#rm /tmp/plplot_api.txt
