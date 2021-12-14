#!/bin/bash

# Find the git commit year range for each individual author that committed changes to a given file
# and emit those data in useful copyright format.

# Typical usage:

# <pathname of script> <filename> <comment_prefix>

# where <filename> is the name of the file whose git logs are being analyzed by this script,
# and <comment_prefix> is an optional comment prefix (which defaults to "//") to prefix
# every emitted copyright line.

# Copyright (C) 2019 Alan W. Irwin
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

filename="$1"
comment_prefix="$2"
if [ -z $comment_prefix ] ; then
    # Use C language comment prefix by default
    comment_prefix="//"
fi

emit_copyright () {
    filename="$1"
    comment_prefix="$2"

    # save old IFS for restoration at the end
    IFS_OLD=$IFS
    # Set IFS so that spaces in authors names will not be delimiters
    # in command substitution.
    IFS=$'\n'

    # --pretty=format used to display combination of specified attributes where
    # %an is author name and %ad is date, and you have complete control of strings
    # surrounding those attributes (if needed) which identify them to following grep
    for NAME in $(git log --follow --pretty=format:'%an' $filename |sort -u |sed -e 's?.*: ??' -e 's? <.*$??') ; do
	FIRST_DATE=$(git log --follow --date=short --pretty=format:'Author: %an DATE: %ad' $filename |grep "Author: $NAME" |tail -1 |sed -e 's?^.*: ??' -e 's?-[0-9][0-9]-[0-9][0-9]$??')
	LAST_DATE=$(git log --follow --date=short --pretty=format:'Author: %an DATE: %ad' $filename |grep "Author: $NAME" |head -1 |sed -e 's?^.*: ??' -e 's?-[0-9][0-9]-[0-9][0-9]$??')

	if [ "$FIRST_DATE" = "$LAST_DATE" ] ; then
	    echo "$comment_prefix Copyright (C) $FIRST_DATE $NAME"
	else
	    echo "$comment_prefix Copyright (C) $FIRST_DATE-$LAST_DATE $NAME"
	fi
    done
    # Restore IFS
    unset IFS
    IFS=$IFS_OLD
}

emit_copyright "$filename" "$comment_prefix" | sort
