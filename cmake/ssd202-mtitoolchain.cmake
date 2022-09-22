
SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR /home/swdev/common/gcc-arm-8.2-2018.08-x86_64-arm-linux-gnueabihf/)
set(CMAKE_CROSSCOMPILING true)

SET(CMAKE_SYSTEM_PROCESSOR arm)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/arm-linux-gnueabihf/libc/usr)
