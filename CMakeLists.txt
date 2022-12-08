project(CDROID C CXX)
cmake_minimum_required (VERSION 3.2)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 11)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
include(cmake/common_functions.cmake)
include(cmake/options.cmake)

set(DISABLE_FORCE_DEBUG_POSTFIX ON)

if (EXISTS "${CMAKE_SOURCE_DIR}/src/gui/view/view.h" )
   set(DEPS_DIR ${CMAKE_BINARY_DIR})
else()
   set(DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)
endif()

if (CMAKE_BUILD_TYPE STREQUAL Debug)
   add_definitions(-DDEBUG) 
endif()

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

