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

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/ssd202-mtitoolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DCDROID_CHIPSET="sigma" \
   -DCMAKE_BUILD_TYPE=${TYPE} \
   ..
popd
