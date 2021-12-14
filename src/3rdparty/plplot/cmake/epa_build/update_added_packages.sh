#!/bin/bash
# Update build configurations of added packages that are described by
# add_packages.xml

rm -f add_packages.data add_packages.stderr
touch add_packages.data add_packages.stderr
# PACKAGE_LIST only contains the package names in add_packages.xml
# that do not depend on any other package in that file.  Thus,
# PACKAGE_LIST contains a list of "starting" packages and the
# dependencies of those should suck in the rest of the packages in
# add_packages.xml.
PACKAGE_LIST="libffi intltool pixman gperf libxslt ragel"
for PACKAGE in $PACKAGE_LIST; do
  ./gtk_transform.py $PACKAGE 1 <add_packages.xml 1>> add_packages.data 2>> add_packages.stderr
done
cmake -DFILENAME:FILEPATH=add_packages.data -P configure_epa.cmake
