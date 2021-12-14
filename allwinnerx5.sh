if [ $# -gt 0 ]
then
mkdir -p out-allwinnerr
pushd out-allwinnerr
TYPE=Release
else
mkdir -p out-allwinner
pushd out-allwinner
TYPE=Debug
fi

cmake  -DCMAKE_TOOLCHAIN_FILE=../cmake/allwinner-x5toolchain.cmake\
    -DCDROID_CHIPSET=allwinner  -DENABLE_RFB=ON  \
    -DCMAKE_INSTALL_PREFIX=./ \
    -DCMAKE_BUILD_TYPE=${TYPE} \
    ..
popd

if [  ! -d "src/gui" ]; then
  pushd deps/lib
  mv libcairo.a libcairo_static.a
  mv libjrtp.a libjrtplib-static.a
  mv libgui.a libgui_static.a
  mv libdvbepg.a libdvbepg_static.a
  mv libtvhal.a libtvhal_static.a
  popd
fi
