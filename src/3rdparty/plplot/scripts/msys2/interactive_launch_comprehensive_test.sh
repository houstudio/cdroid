# MinGW-w64/MSYS2:
# Run the interactive comprehensive tests and collect the output
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
# The option "-DCMAKE_CXX_FLAGS=-fabi-version=8" is used to silence a mismatch in ABI for wxWidgets
#

src=`dirname $0`/..

$src/scripts/comprehensive_test.sh --generator_string "MSYS Makefiles" --cmake_added_options "-DCMAKE_CXX_FLAGS=-fabi-version=8" --build_command /usr/bin/make --ctest_command ctest --do_test_noninteractive no
