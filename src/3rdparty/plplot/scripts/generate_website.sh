#!/bin/bash

# Set JOBS to suitable value. 16 is correct for hardware such as the
# Ryzen 7 1700 system with 8 cpu cores and twice that number of
# hardware threads.  But parallel builds are unreliable on Cygwin and
# MinGW-w64/MSYS2 so for those two platforms you should
# always set JOBS=1.
JOBS=16

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

echo "Generate and upload all PLplot website components to a computer host that is accessible with ssh."
echo -n "Valid login name on ssh-accessible host? "
read USERNAME
echo -n "Valid group name on ssh-accessible host? "
read GROUPNAME
echo -n "ssh-accessible hostname? "
read HOSTNAME
echo "The following directory prefix on '$HOSTNAME' will be removed and then replaced by the complete PLplot website so be careful with what you specify"
echo -n "Expendable directory prefix on '$HOSTNAME'? "
read WEBSITE_PREFIX
echo ""
echo "Summary:"
echo "USERNAME = $USERNAME"
echo "GROUPNAME = $GROUPNAME"
echo "HOSTNAME = $HOSTNAME"
echo "WEBSITE_PREFIX = $WEBSITE_PREFIX"
echo ""
echo "Last warning: if you specify 'yes' below, then the '$WEBSITE_PREFIX' directory on the remote host, '$HOSTNAME', will be removed and then replaced."
ANSWER=
while [ "$ANSWER" != "yes" -a "$ANSWER" != "no" ] ; do
    echo -n "Continue (yes/no)? "
    read ANSWER
done
if [ "$ANSWER" = "no" ] ; then
    echo "Immediate exit specified!"
    exit
fi

# Remove $WEBSITE_PREFIX to insure absolutely clean result.
echo ""
echo "Completely remove old website files."
ssh $USERNAME@$HOSTNAME "rm -rf $WEBSITE_PREFIX; mkdir -p $WEBSITE_PREFIX; chmod u=rwx,g=rwxs $WEBSITE_PREFIX"

# Get down to the work of generating the components of the PLplot website
# and uploading them to the $WEBSITE_PREFIX directory on $HOSTNAME.

# Build a local version of the documentation.

# N.B. this build puts some documentation results into the source tree so
# we use a throwaway source tree for this.

rm -rf /tmp/plplotdoc
mkdir -p /tmp/plplotdoc/build
cd /tmp/plplotdoc
echo ""
echo "Use git checkout-index to copy working directory from staged or committed"
echo "changes of current branch of $SOURCE_TREE to /tmp/plplotdoc/plplot_source"
# For checkout-index, trailing slash for prefix directory is important for some
# reason according to the man page.  Furthermore, it turns out
# that --prefix must be an absolute directory.
git --work-tree=$SOURCE_TREE --git-dir=$SOURCE_TREE/.git checkout-index --all --prefix=/tmp/plplotdoc/plplot_source/
echo ""
echo "Configure and build PLplot documentation.  This may take a while depending on your cpu speed...."
cd /tmp/plplotdoc/build

# Do NOT exclude all bindings and devices since some of the bindings are dependencies of the
# doxygen build and some of the bindings depend on certain devices being enabled.
cmake \
    -DWWW_USER=$USERNAME \
    -DWWW_GROUP=$GROUPNAME \
    -DWWW_HOST=$HOSTNAME \
    -DWWW_DIR=$WEBSITE_PREFIX \
    -DPREBUILD_DIST=ON \
    -DBUILD_DOC=ON \
    -DBUILD_DOX_DOC=ON \
    ../plplot_source \
    >& cmake.out
make VERBOSE=1 -j$JOBS prebuild_dist >& make_prebuild.out

echo ""
echo "Install the configured base part of the website to $WEBSITE_PREFIX on $HOSTNAME."
cd /tmp/plplotdoc/build
make VERBOSE=1 -j$JOBS www-install-base >& make_www-install-base.out

echo ""
echo "Install the just-generated documentation to $WEBSITE_PREFIX/htdocs/docbook-manual on $HOSTNAME."
# This command completely removes WWW_DIR/htdocs/docbook-manual on $HOSTNAME
# so be careful how you specify the above -DWWW_DIR option.
cd /tmp/plplotdoc/build
make VERBOSE=1 -j$JOBS www-install >& make_www-install.out
make VERBOSE=1 -j$JOBS www-install-doxygen >& make_www-install-doxygen.out

echo ""
echo "Build PLplot, PLplot examples, and screenshots of those examples.  This may take a while depending on your cpu speed...."

# N.B. this command completely removes WWW_DIR/htdocs/examples-data
# on $HOSTNAME so be careful how you specify WWW_DIR!

cd /tmp/plplotdoc/plplot_source
WWW_USER=$USERNAME \
    WWW_GROUP=$GROUPNAME \
    WWW_HOST=$HOSTNAME \
    WWW_DIR=$WEBSITE_PREFIX \
    scripts/htdocs-gen_plot-examples.sh >& htdocs_gen.out

# If all the above works like it should, and $HOSTNAME has apache and PHP
# installed properly, you should be able to browse the
# resulting complete website.  AWI verified this by specifying
# irwin, irwin, merlin (his local machine), and /home/irwin/public_html/plplot
# for USERNAME, GROUPNAME, HOSTNAME, and WEBSITE_PREFIX for this script
# and browsing http://merlin/~irwin/plplot/htdocs/ afterward.

# Once you are satisfied with the local version of the website, then
# upload it to the official PLplot SourceForge site using the rsync
# procedure documented in README.Release_Manager_Cookbook.
