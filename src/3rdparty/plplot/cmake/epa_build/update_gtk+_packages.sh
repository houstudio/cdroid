#!/bin/bash
# Update build configurations of gtk+ packages that are described by
# GTK+ jhbuild build configuration
# Note the patch
# below drops wayland and tests for the gtk+ package which should
# greatly reduce the number of packages that are configured as direct
# and indirect dependencies of gtk+.

# Chose this latest version (as of August 2013) because it probably has
# improved build and dependency instructions compared to previous versions.
# For example, the glib version (2.37.4) is known to solve a build issue
# that I ran into for earlier glib versions.
export GTK_VERSION=3.9.4
rm -rf $GTK_VERSION
mkdir $GTK_VERSION
cd $GTK_VERSION
# This downloads all the relevant xml build configuration files
# for the given version of GTK
wget -nd  ftp://ftp.acc.umu.se/pub/GNOME/teams/releng/$GTK_VERSION/*
cd ..
./gtk_xml_recursive_process.py \
$GTK_VERSION/gnome-apps-$GTK_VERSION.modules \
>| gtk_packages_$GTK_VERSION.xml

rm -rf $GTK_VERSION

# Patch this result to correct errors I have discovered in the jhbuild
# configuration or else to use new versions of packages.

patch --no-backup-if-mismatch <patch_gtk_packages.xml

# To create the schema for gtk_packages.xml use
trang -I xml gtk_packages_$GTK_VERSION.xml gtk_packages_$GTK_VERSION.rnc

# That resulting schema file is quite helpful in figuring out
# how to process gtk_packages_$GTK_VERSION.xml as below.

# Transform the xml form to a form that can be used by a cmake script.
# Note, there will be some information messages sent to stderr by this
# script that typically relate to good packages (ones that can be
# transformed by this script) and bad packages. Currently the list of
# bad packages are confined just to those which are completely missing
# from the jhbuild data for gtk.  The first command-line argument is
# the starting package name and the second command-line argument is a
# control variable where the least significant bit is ON for following (hard)
# "dependencies" dependencies, the next least significant bit is ON
# for following (soft) "suggests", dependencies, and the next least
# significant bit is ON for following (would be nice) "after"
# dependencies.  Currently I use a command variable of 1 to keep
# the number of packages configured for building gtk+ and
# (hard) dependencies to a minimum.
./gtk_transform.py "gtk+" 1 <gtk_packages_$GTK_VERSION.xml 1>| gtk+_packages.data 2>|gtk+_packages.stderr

# Finally to actually generate build configurations for build_packages run
# the following command.

cmake -DFILENAME:FILEPATH=gtk+_packages.data -P configure_epa.cmake

# Patch generated configuration files.  This patch file contains
# additional changes that cannot be done via a patch to the *.xml file.
# Typically, these changes are hand edits which are tested then committed.
# So typically the patch is created by rerunning the above cmake
# command then using "git diff" >| configured_gtk+.patch" to generate
# the reverse form of the patch to change the result created by the
# above cmake command into the git-committed form which is done with
# the following patch command.

patch --reverse --no-backup-if-mismatch -p0 <configured_gtk+.patch
