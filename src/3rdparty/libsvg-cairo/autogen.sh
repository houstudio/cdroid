#!/bin/sh
# Run this to generate all the initial makefiles, etc.

set -e

LIBTOOLIZE=${LIBTOOLIZE-libtoolize}
LIBTOOLIZE_FLAGS="--copy --force"
ACLOCAL=${ACLOCAL-aclocal}
AUTOHEADER=${AUTOHEADER-autoheader}
AUTOMAKE=${AUTOMAKE-automake}
AUTOMAKE_FLAGS="--add-missing"
AUTOCONF=${AUTOCONF-autoconf}

PKG_NAME=libsvg-cairo
ARGV0=$0

if test -z "$*"; then
  echo "$ARGV0:	Note: \`./configure' will be run with no arguments."
  echo "		If you wish to pass any to it, please specify them on the"
  echo "		\`$0' command line."
  echo
fi

do_cmd() {
    echo "$ARGV0: running \`$@'"
    $@
}

do_cmd $LIBTOOLIZE $LIBTOOLIZE_FLAGS

do_cmd $ACLOCAL $ACLOCAL_FLAGS

do_cmd $AUTOHEADER

do_cmd $AUTOMAKE $AUTOMAKE_FLAGS

do_cmd $AUTOCONF

do_cmd ./configure --enable-maintainer-mode $@ && echo "Now type \`make' to compile $PKG_NAME" || exit 1
