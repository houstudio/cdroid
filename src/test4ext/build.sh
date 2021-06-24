mkdir -p out
pushd out
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DNGL_CHIPSET="gx3213" \
   ..
#   -DUSE_RFB_GRAPH=ON \
popd
