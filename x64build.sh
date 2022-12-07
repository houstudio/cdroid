if [ $# -gt 0 ]
then
   mkdir -p outx64r
   pushd outx64r
   TYPE=Release
else
   mkdir -p outx64
   pushd outx64
   TYPE=Debug
fi
export PKG_CONFIG_PATH=$HOME/vcpkg/installed/x64-linux-dynamic/lib/pkgconfig
export PKG_CONFIG_LIBDIR=$HOME/vcpkg/installed/x64-linux-dynamic/lib/pkgconfig
cmake -DCDROID_CHIPSET=x64  -DENABLE_RFB=OFF  \
    -DCMAKE_INSTALL_PREFIX=./ \
    -DCMAKE_BUILD_TYPE=${TYPE} \
    -DM_LIBRARY=m \
    ..
popd

