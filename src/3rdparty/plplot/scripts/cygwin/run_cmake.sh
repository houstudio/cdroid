# Cygwin64:
# Sample script to run CMake and make in a separate build directory
#
src=`pwd`
PATH=/usr/lib/lapack:$src/dll:/usr/x86-64-pc-cygwin/sys-root/usr/lib:$PATH

cmake ../plplot-git -G "Unix Makefiles" -DBUILD_TEST=ON -DENABLE_DYNDRIVERS=ON -DCMAKE_LIBRARY_PATH=/cygdrive/c/cygwin64/usr/x86-64-pc-cygwin/sys-root/usr/lib -DPLD_wincairo=OFF -DBUILD_DOC=OFF -DENABLE_octave=ON
make
