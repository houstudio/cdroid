project(CDROID C CXX)
cmake_minimum_required (VERSION 3.2)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 14)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)
include(cmake/common_functions.cmake)
include(cmake/options.cmake)

set(DISABLE_FORCE_DEBUG_POSTFIX ON)

if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG) 
endif()

add_subdirectory(src/porting)
add_subdirectory(src/gui)
add_subdirectory(src/modules)

add_subdirectory(apps)

add_subdirectory(tests)

include (InstallRequiredSystemLibraries)
#set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set (CPACK_COMPONENT_LIBRARIES_DEPENDS ${CDROID_DEPLIBS})
set (CPACK_PACKAGE_VERSION_MAJOR "1")
set (CPACK_PACKAGE_VERSION_MINOR "0")
include (CPack)

set(APPS ${CMAKE_INSTALL_PREFIX}/bin/uidemo2)  # paths to executables
set(DIRS "$ENV{HOME}/vcpkg/installed/arm-linux-dynamic"
	"${CMAKE_INSTALL_PREFIX}/lib")   # directories to search for prerequisites
INSTALL(CODE "
   include(BundleUtilities)
   fixup_bundle(\"${APPS}\"   \"\"   \"${DIRS}\")
   " COMPONENT Runtime)

if(BUILD_DOCS)
  include(cmake/build_doxygen.cmake)
  build_doxygen()
endif()

