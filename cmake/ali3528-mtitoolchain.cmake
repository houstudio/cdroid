
SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR $ENV{HOME}/toolchain_mti)
set(CMAKE_CROSSCOMPILING true)

SET(CMAKE_SYSTEM_PROCESSOR mips)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/host/usr/bin/mips-mti-linux-gnu-g++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/host/usr/bin/mips-mti-linux-gnu-gcc)
SET(CMAKE_ASM_COMPILER ${TOOLCHAIN_DIR}/host/usr/bin/mips-mti-linux-gnu-gcc)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/target/sysroot)
