
SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR /opt/aarch64-linux-gnu)
set(CMAKE_CROSSCOMPILING true)

SET(CMAKE_SYSTEM_PROCESSOR aarch64)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-gcc)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu-gcc)
#SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/target/sysroot)
