if [ $# -gt 0 ]
then
  mkdir -p out-ssdr
  pushd out-ssdr
  TYPE=Release
  export PKG_CONFIG_PATH=$HOME/vcpkg/installed/arm-linux-dynamic/lib/pkgconfig
  export PKG_CONFIG_LIBDIR=$HOME/vcpkg/installed/arm-linux-dynamic/lib/pkgconfig
else
  mkdir -p out-ssd
  pushd out-ssd
  TYPE=Debug
  export PKG_CONFIG_PATH=$HOME/vcpkg/installed/arm-linux-dynamic/debug/lib/pkgconfig
  export PKG_CONFIG_LIBDIR=$HOME/vcpkg/installed/arm-linux-dynamic/debug/lib/pkgconfig
fi

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/ssd202-mtitoolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DCDROID_CHIPSET="sigma" \
   -DCMAKE_BUILD_TYPE=${TYPE} \
   ..
popd
