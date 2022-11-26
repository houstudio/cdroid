if [ $# -gt 0 ]
then
mkdir -p out-ssdr
pushd out-ssdr
TYPE=Release
else
mkdir -p out-ssd
pushd out-ssd
TYPE=Debug
fi

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/ssd202-mtitoolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DCDROID_CHIPSET="sigma" \
   -DCMAKE_BUILD_TYPE=${TYPE} \
   ..
popd
