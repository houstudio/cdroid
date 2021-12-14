#!/bin/bash

# This script will run parity_bit_check on all files in the PLplot
# source tree (except those listed below) to discover which of those
# files have any character with the eighth (parity) bit set.

# Copyright (C) 2010-2016 Alan W. Irwin
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

which parity_bit_check >/dev/null
parity_bit_check_rc=$?
if [ "$parity_bit_check_rc" != 0 ] ; then
    echo "
This script only works when parity_bit_check is on the PATH.
To make this so please run

make parity_bit_check

in the build tree and put the absolute (not relative) location of the
utils subdirectory of that build tree where parity_bit_check has been
built by the above command on your PATH."
    exit 1
fi

# Find absolute PATH of script without using readlink (since readlink is
# not available on all platforms).  Followed advice at
# http://fritzthomas.com/open-source/linux/551-how-to-get-absolute-path-within-shell-script-part2/
ORIGINAL_PATH="$(pwd)"
cd "$(dirname $0)"
# Absolute Path of the script
SCRIPT_PATH="$(pwd)"

# Assumption: top-level source tree is parent directory of where script
# is located.
SOURCE_TREE="$(dirname ${SCRIPT_PATH})"

# Must be in top directory of git working directory or outside that
# working directory completely in order for "git checkout-index" to
# work.
cd $SOURCE_TREE

# Copy all files in working directory tree of local git repository of PLplot
rm -rf /tmp/parity_bit_check
git --work-tree=$SOURCE_TREE --git-dir=$SOURCE_TREE/.git checkout-index --all --prefix=/tmp/parity_bit_check/
cd /tmp/parity_bit_check
find -type f >| /tmp/temporary_source_tree_list
for FILE in $(grep -v -f "$SOURCE_TREE"/scripts/parity_bit_check.exclude /tmp/temporary_source_tree_list); do
    parity_bit_check <$FILE
    parity_bit_check_rc=$?
    if [ "$parity_bit_check_rc" -ne 0 ]; then
	printf "%s %x\n" $FILE $parity_bit_check_rc
    fi
done
rm -f /tmp/temporary_source_tree_list
