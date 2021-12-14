#!/bin/bash
# Script to check all our C/C++ API for backwards-incompatible changes
# using abi-compliance-checker.

# Check that you have abi-compliance-checker.pl on your PATH.

if [ ! -x "$(which abi-compliance-checker.pl)" ] ; then
    echo "You must put abi-compliance-checker.pl on your PATH"
    exit 1
fi

allowed_version=1.21.4
acc_version=$(abi-compliance-checker.pl -dumpversion)
if [ "$acc_version" != "$allowed_version" ] ; then
    echo "
Detected abi-compliance-checker.pl version = '$acc_version'.
This script only works with abi-compliance_checker.pl $allowed_version."
    exit 1
fi

# Boiler plate to obtain SOURCE_TREE
ORIGINAL_PATH="$(pwd)"
cd "$(dirname $0)"
# Absolute Path of the script
SCRIPT_PATH="$(pwd)"

# Assumption: top-level source tree is parent directory of where script
# is located.
SOURCE_TREE="$(dirname ${SCRIPT_PATH})"

cd $SOURCE_TREE

WC_VERSION=$(svnversion)
echo "WC_VERSION = $WC_VERSION"
svnversion | grep --silent '[^0-9]'
NON_NUMERIC_SVNVERSION_rc=$?
if [ $NON_NUMERIC_SVNVERSION_rc -eq 0 ] ; then
    echo "Warning: svnversion produces a $WC_VERSION result with non-numeric"
    echo "characters corresponding to a working copy that is different from the"
    echo "PLplot repository."
fi

# Build C/C++ library components of working copy.
rm -rf /tmp/plplot_wc
mkdir -p /tmp/plplot_wc/build /tmp/plplot_wc/install
cd /tmp/plplot_wc/build
echo "Configuring, building and installing the working copy of PLplot."
echo "This may take a while...."
cmake  "-DCMAKE_INSTALL_PREFIX=/tmp/plplot_wc/install" -DDEFAULT_NO_BINDINGS=ON -DENABLE_cxx=ON -DENABLE_qt=ON -DENABLE_tcl=ON -DENABLE_tk=ON -DENABLE_wxwidgets=ON -DDEFAULT_NO_DEVICES=ON -DPLD_extqt=ON -DPLD_wxwidgets=ON $SOURCE_TREE >& cmake.out
make -j4 VERBOSE=1 install >& make_install.out

# Configure abi-complicance-checker configuration file for working copy install.
sed -e 's?${VERSION}?'"r${WC_VERSION}?" -e 's?${PREFIX}?'"/tmp/plplot_wc/install?" -e 's?;/?\n/?g' < abi-compliance-checker.xml.template > plplot_wc.xml

# Obtain tarball version number for reference.
echo -n "Reference version number (5.9.6)? "
read REFERENCE_VERSION
if [ -z "$REFERENCE_VERSION" ] ; then
   REFERENCE_VERSION=5.9.6
fi
echo "REFERENCE_VERSION = $REFERENCE_VERSION"

# Download and gpg --verify and unpack that tarball in the /tmp directory.

cd /tmp

if [ ! -f plplot-$REFERENCE_VERSION.tar.gz ] ; then
    echo "Downloading the reference version of PLplot."
    echo "This may take a while...."
    wget -O plplot-$REFERENCE_VERSION.tar.gz "http://downloads.sourceforge.net/project/plplot/plplot/$REFERENCE_VERSION Source/plplot-$REFERENCE_VERSION.tar.gz?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fplplot%2Ffiles%2F&ts=1283877998&use_mirror=superb-sea2"
fi
if [ ! -f plplot-$REFERENCE_VERSION.tar.gz.asc ] ; then
    wget -O plplot-$REFERENCE_VERSION.tar.gz.asc "http://downloads.sourceforge.net/project/plplot/plplot/$REFERENCE_VERSION Source/plplot-$REFERENCE_VERSION.tar.gz.asc?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fplplot%2Ffiles%2F&ts=1283877998&use_mirror=superb-sea2"
fi

gpg --verify plplot-$REFERENCE_VERSION.tar.gz.asc

REFERENCE_SOURCE_TREE=/tmp/plplot-$REFERENCE_VERSION
rm -rf $REFERENCE_SOURCE_TREE
tar zxf plplot-$REFERENCE_VERSION.tar.gz

# Build C/C++ library components of reference version.
rm -rf /tmp/plplot_reference
mkdir -p /tmp/plplot_reference/build /tmp/plplot_reference/install
cd /tmp/plplot_reference/build
echo "Configuring, building and installing the reference version of PLplot."
echo "This may take a while...."
cmake  "-DCMAKE_INSTALL_PREFIX=/tmp/plplot_reference/install" -DDEFAULT_NO_BINDINGS=ON -DENABLE_cxx=ON -DENABLE_qt=ON -DENABLE_tcl=ON -DENABLE_tk=ON -DENABLE_wxwidgets=ON -DDEFAULT_NO_DEVICES=ON -DPLD_extqt=ON -DPLD_wxwidgets=ON $REFERENCE_SOURCE_TREE >& cmake.out
make -j4 VERBOSE=1 install >& make_install.out

# Configure abi-complicance-checker configuration file for reference install.
sed -e 's?${VERSION}?'"r${REFERENCE_VERSION}?" -e 's?${PREFIX}?'"/tmp/plplot_reference/install?" -e 's?;/?\n/?g' < /tmp/plplot_wc/build/abi-compliance-checker.xml.template > plplot_reference.xml

echo "Running abi-compliance-checker.pl."
echo "This may take a while...."

rm -rf /tmp/abi-compliance-checker
mkdir -p /tmp/abi-compliance-checker
cd /tmp/abi-compliance-checker
abi-compliance-checker.pl -l plplotd \
-d1 /tmp/plplot_reference/build/plplot_reference.xml \
-d2 /tmp/plplot_wc/build/plplot_wc.xml

echo "N.B. the above report file(s) are in the directory $PWD"
