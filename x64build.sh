if [ $# -gt 0 ]
then
   mkdir -p outx64r
   pushd outx64r
   TYPE=Release
   export PKG_CONFIG_PATH=$HOME/vcpkg/installed/x64-linux-dynamic/lib/pkgconfig
   export PKG_CONFIG_LIBDIR=$HOME/vcpkg/installed/x64-linux-dynamic/lib/pkgconfig
else
   mkdir -p outx64
   pushd outx64
   TYPE=Debug
   export PKG_CONFIG_PATH=$HOME/vcpkg/installed/x64-linux-dynamic/debug/lib/pkgconfig
   export PKG_CONFIG_LIBDIR=$HOME/vcpkg/installed/x64-linux-dynamic/debug/lib/pkgconfig
fi
export PKG_CONFIG_PATH=${PWD}/lib/pkgconfig:${PKG_CONFIG_PATH}
export PKG_CONFIG_LIBDIR=${PWD}/lib/pkgconfig:${PKG_CONFIG_LIBDIR}
cmake -DCDROID_CHIPSET=x64  -DENABLE_RFB=OFF  \
    -DCMAKE_INSTALL_PREFIX=./ \
    -DCMAKE_BUILD_TYPE=${TYPE} \
    -DM_LIBRARY=m \
    ..
popd

