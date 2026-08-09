cmake_minimum_required (VERSION 3.22)
project(CDROID C CXX)

# Binary AXML mode: when ON, pakbuilder uses SDK framework resources (aapt2 -x)
# to produce binary AXML layouts + resources.arsc in the pak. Default OFF (text XML).
option(ENABLE_BINARY_XML "Use SDK framework resources + binary AXML in pak" OFF)
set(CDROID_AAPT2 "/opt/android-sdk/build-tools/36.0.0/aapt2" CACHE PATH "aapt2 binary")
set(CDROID_ANDROID_JAR "/opt/android-sdk/platforms/android-36/android.jar" CACHE PATH "android.jar")
set(CDROID_SDK_RES "${CMAKE_SOURCE_DIR}/src/gui/res" CACHE PATH "framework res dir (slim base, ex-gui/assets)")
set(CDROID_SDK_RES_FILTER "${CMAKE_SOURCE_DIR}/scripts/sdk_res_filter.json" CACHE FILEPATH "JSON filter for SDK res densities/locales")

# Enable ctest at the top level so registered tests (e.g. the standalone androidfw
# sub-library unit tests) are discoverable via `ctest` from the build root.
enable_testing()

set(CMAKE_POLICY_DEFAULT_CMP0012 NEW)
if(COMMAND cmake_policy)
   cmake_policy(SET CMP0003 NEW)
   cmake_policy(SET CMP0012 NEW)
   #make vcpkg happy:)
endif(COMMAND cmake_policy)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

#LeakSanitizer does not work under ptrace (strace, gdb ,valgrind,...)
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address -g")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

include(cmake/depchecks.cmake)
include(cmake/common_functions.cmake)
include(cmake/options.cmake)
include(cmake/check_cdroiddeps.cmake)

add_compile_options(-Wno-deprecated-declarations)

set(DISABLE_FORCE_DEBUG_POSTFIX ON)
if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG) 
endif()
if(CMAKE_HOST_SYSTEM_NAME MATCHES "Linux")
   add_link_options("LINKER:-rpath-link,${CMAKE_SOURCE_DIR}/src/porting/${CDROID_CHIPSET}/lib")
endif()
add_subdirectory(src/porting)
# androidfw must be defined before src/gui so cdroid can link it.
add_subdirectory(src/gui/androidfw)
add_subdirectory(src/gui)
add_subdirectory(src/modules)
add_subdirectory(src/3rdparty)

add_subdirectory(apps)

add_subdirectory(tests)
GetGitVersion(CDROID)
string(TIMESTAMP COMPILE_TIME "%Y%m%d_%H%M%S")
set(CPACK_PACKAGE_NAME "CDROID-${COMPILE_TIME}-${CDROID_COMMITID}")
set(CPACK_PACKAGE_VERSION ${CDROID_VERSION})
set(CPACK_GENERATOR "ZIP")  # Choose the desired generator (e.g., ZIP, TGZ, NSIS, etc.)
include(CPack)

if(BUILD_DOCS)
  include(cmake/build_doxygen.cmake)
  build_doxygen()
endif()

