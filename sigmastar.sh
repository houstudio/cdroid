if [ $# -gt 0 ]
then
mkdir -p out-ssd202r
pushd out-ssd202r
TYPE=Release
else
mkdir -p out-ssd202
pushd out-ssd202
TYPE=Debug
fi
export PKG_CONFIG_PATH=$HOME/vcpkg/installed/arm-linux-dynamic/lib/pkgconfig
export PKG_CONFIG_LIBDIR=$HOME/vcpkg/installed/arm-linux-dynamic/lib/pkgconfig
cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/ssd202-mtitoolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DCDROID_CHIPSET="sigma" \
   -DCMAKE_BUILD_TYPE=${TYPE} \
   ..
popd
