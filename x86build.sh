if [ $# -gt 0 ]
then
mkdir -p out-x86r
pushd out-x86r
TYPE=Release
else
mkdir -p out-x86
pushd out-x86
TYPE=Debug
fi

cmake -DCDROID_CHIPSET=x86  -DENABLE_RFB=ON  \
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
