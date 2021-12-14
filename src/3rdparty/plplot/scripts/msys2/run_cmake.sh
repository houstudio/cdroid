# MinGW-w64/MSYS2:
# Sample script to run CMake and make in a separate build directory
#
# -----
# TO DO: set the variable "src" to point to the right directory containing the PLplot source.
# -----
#
# Note 1:
# The generator "MSYS Makefiles" for CMake is the easiest generator to use, though alternatives
# are possible.
#
# Note 2:
# You may want to catch all output in a file for checking any errors or warnings:
#
#     run_cmake.sh 1>run_cmake.out 2>&1
#
# Note 3:
# The option -DCMAKE_CXX_FLAGS="-fabi-version=8" is used to silence a mismatch in ABI for wxWidgets
#

src=`dirname $0`/../../

cmake $src -G "MSYS Makefiles" -DBUILD_TEST=ON -DCMAKE_CXX_FLAGS="-fabi-version=8"
make
