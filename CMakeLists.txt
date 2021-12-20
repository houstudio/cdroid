project(Miniwin C CXX)
cmake_minimum_required (VERSION 3.2)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 11)

include(cmake/common_functions.cmake)
include(cmake/options.cmake)

set(CAIRO_HAS_CDROID_SURFACE TRUE)
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_INSTALL_MANDIR ${CMAKE_BINARY_DIR}/man)
set(CMAKE_INSTALL_DEFAULT_PREFIX ${CMAKE_BINARY_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules)

if (EXISTS "${CMAKE_SOURCE_DIR}/src/gui/widget/view.h" )
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
  add_subdirectory(src/3rdparty)
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
  COMMAND tar -zcf ${CMAKE_BINARY_DIR}/appdevops.tar.gz *.sh CMakeLists.txt README.md apps src/porting src/modules  src/tools cmake deps tests 
      apps/samples --checkpoint=300 --checkpoint-action=dot --totals
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Creating Application develop environment ..."
)

# Packaging
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_NAME} Package.")

set(CPACK_PACKAGE_VERSION_MAJOR ${VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${VERSION_PATCH})
set(CPACK_PACKAGE_VERSION "1.0")#${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")

if (WIN32)
  set(CPACK_GENERATOR ZIP)
else()
  set(CPACK_GENERATOR TGZ)
endif()

set(CPACK_STRIP_FILES TRUE)

include(CPack)

