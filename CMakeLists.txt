project(CDROID C CXX)
cmake_minimum_required (VERSION 3.2)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 14)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
include(cmake/common_functions.cmake)
include(cmake/options.cmake)

if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
   #make vcpkg happy:)
endif(COMMAND cmake_policy)

set(DISABLE_FORCE_DEBUG_POSTFIX ON)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG) 
endif()
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
   add_link_options("LINKER:-rpath-link,${CMAKE_SOURCE_DIR}/src/porting/${CDROID_CHIPSET}/lib")
endif()
add_subdirectory(src/porting)
add_subdirectory(src/gui)
add_subdirectory(src/modules)
add_subdirectory(src/3rdparty/pinyin)

add_subdirectory(apps)

add_subdirectory(tests)

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_BRANCH_NAME
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
string(TIMESTAMP COMPILE_TIME "%Y%m%d_%H%M%S")
set(CPACK_PACKAGE_NAME "CDROID-${COMPILE_TIME}-${GIT_COMMIT_HASH}")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_GENERATOR "ZIP")  # Choose the desired generator (e.g., ZIP, TGZ, NSIS, etc.)
include(CPack)

if(BUILD_DOCS)
  include(cmake/build_doxygen.cmake)
  build_doxygen()
endif()

