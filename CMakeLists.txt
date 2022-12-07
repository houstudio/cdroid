project(CDROID C CXX)
cmake_minimum_required (VERSION 3.2)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(cmake/common_functions.cmake)
include(cmake/options.cmake)

set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_INSTALL_MANDIR ${CMAKE_PREFIX_PATH}/man)
set(CMAKE_INSTALL_PREFIX ${CMAKE_PREFIX_PATH})
set(DISABLE_FORCE_DEBUG_POSTFIX ON)

set(ENV{PKG_CONFIG_PATH} "${CMAKE_INSTALL_LIBDIR}/pkgconfig:$ENV{PKG_CONFIG_PATH}")

if (EXISTS "${CMAKE_SOURCE_DIR}/src/gui/view/view.h" )
   set(DEPS_DIR ${CMAKE_BINARY_DIR})
else()
   set(DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG) 
endif()
message("BuildType=${CMAKE_BUILD_TYPE} OPTIONAL_LIBS=${OPTIONAL_LIBS}")

add_subdirectory(src/porting)
if (EXISTS "${CMAKE_SOURCE_DIR}/src/gui" )
  add_subdirectory(src/gui)
  add_subdirectory(src/modules)
endif()
add_subdirectory(apps)

add_subdirectory(tests)

if(BUILD_DOCS)
  include(cmake/build_doxygen.cmake)
  build_doxygen()
endif()

add_custom_target(appdevops
  COMMAND rm -f ${CMAKE_BINARY_DIR}/appdevops.tar.gz
  COMMAND rm -rf ${CMAKE_SOURCE_DIR}/deps
  COMMAND mkdir ${CMAKE_SOURCE_DIR}/deps
  COMMAND cp -r ${CMAKE_BINARY_DIR}/include/ ${CMAKE_SOURCE_DIR}/deps
  COMMAND cp -r ${CMAKE_BINARY_DIR}/lib ${CMAKE_SOURCE_DIR}/deps
  COMMAND cp -r ${CMAKE_BINARY_DIR}/bin ${CMAKE_SOURCE_DIR}/deps
  COMMAND tar -zcf ${CMAKE_BINARY_DIR}/appdevops.tar.gz *.sh CMakeLists.txt README.md apps 
      scripts src/porting src/modules  cmake deps tests 
      apps/samples --checkpoint=300 --checkpoint-action=dot --totals
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Creating Application develop environment ..."
)

