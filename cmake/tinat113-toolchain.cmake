
SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR /opt/tinat113-arm-toolchain)
SET(CMAKE_CROSSCOMPILING true)
SET(VCPKG_MESON_CROSS_FILE ${VCPKG_ROOT_DIR}/scripts/toolchains/tinat113_meson_cross.txt)
SET($ENV{STAGING_DIR} /opt/tinat113-arm-toolchain/target)
SET(CMAKE_SYSTEM_PROCESSOR arm)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-openwrt-linux-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-openwrt-linux-gcc)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/arm-openwrt-linux-gcc)
#SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR})#/arm-openwrt-linux/)
