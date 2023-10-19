
SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR /opt/mips-ingenic-toolchain-720)
#SET(TOOLCHAIN_DIR /opt/mips-ingenic-toolchain-1210)
SET(CMAKE_CROSSCOMPILING true)
SET(VCPKG_MESON_CROSS_FILE ${VCPKG_ROOT_DIR}/scripts/toolchains/ingeric_meson_cross.txt)

SET(CMAKE_SYSTEM_PROCESSOR mips)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/mips-linux-gnu-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/mips-linux-gnu-gcc)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/mips-linux-gnu-gcc)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/mips-linux-gnu/libc)
