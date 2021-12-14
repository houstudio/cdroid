# Cygwin64:
# Run the interactive comprehensive tests and collect the output
#
# -----
# TO DO: set the variable "src" to point to the right directory containing the PLplot source.
# -----
#
# Note 1:
# It may be necessary to set environment variables like PATH and CMAKE_LIBRARY_PATH to help CMake
# find the correct components
#
echo NOTE:
echo Removing /usr/local/bin from the path!
export PATH=$(echo :$PATH: |sed -e s,:/usr/local/bin:,, -e s,^:,, -e s,:$,,)
echo $PATH
echo ""

X 1>dummy 2>&1 &
export DISPLAY=:0.0

hash -r

src=`dirname $0`

$src/../comprehensive_test.sh --do_submit_dashboard no --do_test_noninteractive no --build_command "make"

echo ""
echo NOTE: /usr/local/bin was not included in the build
