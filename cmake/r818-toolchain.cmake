
SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR /opt/r818-toolchain/gcc/linux-x86/aarch64/toolchain-sunxi-glibc/toolchain)
set(CMAKE_CROSSCOMPILING true)

SET(CMAKE_SYSTEM_PROCESSOR aarch64)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-gcc)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-openwrt-linux-gcc)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})
