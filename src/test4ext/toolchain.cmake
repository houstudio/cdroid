
SET(CMAKE_SYSTEM_NAME Linux)

SET(TOOLCHAIN_DIR /opt/nationalchip-toolchain/)
set(CMAKE_CROSSCOMPILING true)

SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_DIR}/bin/arm-nationalchip-linux-uclibcgnueabihf-c++)
SET(CMAKE_C_COMPILER ${TOOLCHAIN_DIR}/bin/arm-nationalchip-linux-uclibcgnueabihf-cc)
SET(CMAKE_FIND_ROOT_PATH ${TOOLCHAIN_DIR}/arm-nationalchip-linux-uclibcgnueabihf/sysroot)
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
