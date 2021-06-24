if [ $# -gt 0 ]
then
mkdir -p out-ali3528r
pushd out-ali3528r
TYPE=Release
else
mkdir -p out-ali3528
pushd out-ali3528
TYPE=Debug
fi

cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/ali3528-mtitoolchain.cmake \
   -DCMAKE_INSTALL_PREFIX=./ \
   -DCDROID_CHIPSET="ali3528" \
   -DCMAKE_BUILD_TYPE=${TYPE} \
   ..
popd
