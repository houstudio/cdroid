SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR /opt/rk3506-arm-linux-gnueabihf)
SET(CMAKE_CROSSCOMPILING true)

SET(VCPKG_MESON_CROSS_FILE ${VCPKG_ROOT_DIR}/scripts/toolchains/rk3506_meson_cross.txt)
SET(ENV{"STAGING_DIR"} ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf)
SET(CMAKE_SYSTEM_PROCESSOR arm)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-gcc)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/arm-buildroot-linux-gnueabihf-gcc)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf)

IF(EXISTS ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot)
  SET(ENV{PKG_CONFIG_PATH} "${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot/usr/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
  SET(CMAKE_SYSROOT ${TOOLCHAIN_DIR}/arm-buildroot-linux-gnueabihf/sysroot)
ENDIF()