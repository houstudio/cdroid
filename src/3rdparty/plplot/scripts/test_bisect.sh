#!/bin/sh
# test script to generate an error for bisection purposes.
# Expect substantial updates to this script for each new error to be
# bisected.
#
# This particular case is used to generate the example 15 python rendering
# error in the build tree.

rm -rf \
/tmp/bisect_test_build_dir \
/tmp/bisect_test_install \
/tmp/bisect_test_install_build_dir

mkdir \
/tmp/bisect_test_build_dir \
/tmp/bisect_test_install_build_dir
SOURCE_DIR=`pwd`
cd /tmp/bisect_test_build_dir
cmake -DCMAKE_INSTALL_PREFIX=/tmp/bisect_test_install \
-DDEFAULT_NO_BINDINGS=ON -DDEFAULT_NO_DEVICES=ON -DBUILD_TEST=ON \
-DENABLE_tcl=ON -DENABLE_tk=ON -DPLD_xwin=ON -DPLD_tk=ON \
$SOURCE_DIR >& cmake.out
#make -j4 install >& make_install.out
#cd /tmp/bisect_test_install_build_dir
#cmake /tmp/bisect_test_install/share/plplot5.9.5/examples >& cmake.out
#make -j4 test_extXdrawable
#######
#make -j4 x17c
#make -j4 cairo
#examples/c/x17c -dev xcairo -np
#######
#make -j4 _plplotcmodule
#make python_examples xwin
#examples/python/x15 -dev xwin
#######
# Reduce number of examples run by test_tk_standard_examples to speed everything
# up.
cd examples/tk
patch < /tmp/speedup_patch
cd ../..
make -j4 test_tk_standard_examples

result=
while [ "$result" != "before" -a  "$result" != "after" ]; do
    echo -n "before/after? "
    read result
    if [ "$result" = "before" ]; then
	exit 0
    fi
    if [ "$result" = "after" ]; then
	exit 1
    fi
done
