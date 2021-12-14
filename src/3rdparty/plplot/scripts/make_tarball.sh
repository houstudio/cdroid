#!/bin/bash

# This script prepares a distribution tarball directly from a local working directory
# of a git repository for the PLplot project.

# CAVEAT: The tarball will only include files in the working directory that
# are in the index or committed.  So use "git add" appropriately to test local
# changes before committing them.

# CAVEAT: this script silently and completely deletes a directory called
# /tmp/plplot-dist-prep

# Copyright (C) 2003, 2004  Rafael Laboissiere
# Copyright (C) 2006-2018 Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA

# Set JOBS to suitable value. 16 is correct for hardware such as the
# Ryzen 7 1700 system with 8 cpu cores and twice that number of
# hardware threads.  But parallel builds are unreliable on Cygwin and
# MinGW-w64/MSYS2 so for those two platforms you should
# always set JOBS=1.
JOBS=16

usage () {
    local prog=$(basename $0)
    echo "Usage: $prog [-n] \\"
    echo "          [-c [-i prefix] [-o cmake options]] \\"
    echo "       $prog -d"
    echo "       $prog -h"
    echo
    echo "Option -n prevents pre-building anything including the DocBook manual."
    echo "When option -c is given, the generated tarball is"
    echo "  unpacked, configured with cmake and built with make, and"
    echo "  the ctest (build-tree tests) is run afterward."
    echo "  If the -i prefix option is specified in addition to -c,"
    echo "  the configuration is done with the specified install prefix"
    echo "  and make install is run after ctest."
    echo "  WARNING: the prefix directory is completely removed before the"
    echo "  install so be careful what you specify for the -i option."
    echo "  If the -o cmake options option is specified in addition to -c,"
    echo "  those options are used for the configuration of the unpacked"
    echo "  tarball."
    echo "Option -d prints the default values."
    echo "Option -h prints this usage information."
    exit $1
}

DOC_ARG=${DOC_ARG:--DBUILD_DOC=ON}
# Just easier to always keep this temporary directory in the same location
GITTMPDIR=/tmp/plplot-dist-prep

# Put here extra cmake options that should always be used when
# generating a tarball.  The value below is required by Debian Testing
config_opt="-DUSE_INCRTCL_VERSION_4=ON"

# Put here extra make options (currently none since parallel builds
# are handled a different way).

print_defaults () {
    local v
    for v in DOC_ARG GIT_URL ; do
	eval "echo $v=\\\"\$$v\\\""
    done
    exit 0
}

do_check=no
prefix=""
PREBUILD_ARG="-DPREBUILD_DIST=ON"
PREBUILT_DOC_ARG="-DPREBUILT_DOC=ON"
PREBUILT_DOX_DOC_ARG="-DPREBUILT_DOX_DOC=ON"
do_prebuild_dist=yes

while getopts "cdhi:no:u:" option
do
    case $option in
	c) do_check=yes ;;
	d) print_defaults ;;
	h) usage 0 ;;
	i) test -n "$OPTARG" || usage 1 ; prefix=$OPTARG ;;
	n) DOC_ARG=;PREBUILD_ARG=;PREBUILT_DOC_ARG=;PREBUILT_DOX_DOC_ARG=;do_prebuild_dist=no ;;
	o) config_opt="$config_opt $OPTARG" ;;
	*) usage 1 ;;
    esac
done

cleanup ( ) {
    rm -rf $GITTMPDIR
}

# Find absolute PATH of script without using readlink (since readlink is
# not available on all platforms).  Followed advice at
# http://fritzthomas.com/open-source/linux/551-how-to-get-absolute-path-within-shell-script-part2/
ORIGINAL_PATH="$(pwd)"
cd "$(dirname $0)"
# Absolute Path of the script
SCRIPT_PATH="$(pwd)"
cd "${ORIGINAL_PATH}"

# Assumption: top-level source tree is parent directory of where script
# is located.
SOURCE_TREE="$(dirname ${SCRIPT_PATH})"

cleanup
mkdir $GITTMPDIR
cd $GITTMPDIR

# For checkout-index, trailing slash for prefix directory is important for some
# reason according to the man page.  Furthermore, it turns out
# that --prefix must be an absolute directory.
git --work-tree=$SOURCE_TREE --git-dir=$SOURCE_TREE/.git checkout-index --all --prefix=$GITTMPDIR/plplot_source/ \
    && mkdir build_dir \
    && cd build_dir \
    && cmake \
    ${PREBUILD_ARG} ${DOC_ARG} -DBUILD_DOX_DOC=ON \
    ../plplot_source >& cmake.out \
    && echo "Making distribution tarball.  This may take a while...." \
    && (if [ "$do_prebuild_dist" = "yes" ] ; then
    make VERBOSE=1 -j${JOBS} prebuild_dist >& make_prebuild_dist.out
    fi) \
	&& make VERBOSE=1 -j${JOBS} package_source >& make_package_source.out \
	&& TARBALL=$(ls plplot-*.tar.gz) \
	&& UNPACKED_TARBALL_DIR=${TARBALL%%.tar.gz} \
	&& mv $TARBALL .. \
	&& cd .. \
	&& echo "Finished creating distribution tarball: $TARBALL" \
	&& test "$do_check" = "yes" \
	&& echo "Checking distribution tarball.  This may take a while...." \
	&& tar xfz $TARBALL \
	&& mkdir ctest_build_dir \
	&& ( cd ctest_build_dir \
	&& cmake ${config_opt} -DCMAKE_INSTALL_PREFIX:PATH=${prefix} \
        ${PREBUILT_DOC_ARG} ${PREBUILT_DOX_DOC_ARG}  \
	-DBUILD_TEST=ON \
	../${UNPACKED_TARBALL_DIR} >& cmake.out \
	&& make VERBOSE=1 -j${JOBS} >& make.out \
	&& ctest -j${JOBS} >& ctest.out \
	&& test -n "$prefix" \
	&& rm -rf ${prefix} \
	&& make VERBOSE=1 -j${JOBS} install >& make_install.out )
