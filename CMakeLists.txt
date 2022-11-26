project(CDROID C CXX)
cmake_minimum_required (VERSION 3.2)
set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
set(CMAKE_CXX_STANDARD 11)

include(cmake/common_functions.cmake)
include(cmake/options.cmake)

message("CMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR} CMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_INCLUDEDIR} "
	"CMAKE_INSTALL_DATAROOTDIR=${CMAKE_INSTALL_DATAROOTDIR} CMAKE_INSTALL_DATADIR=${CMAKE_INSTALL_DATADIR} "
	"CMAKE_INSTALL_LIBDIR=${CMAKE_INSTALL_LIBDIR} CMAKE_INSTALL_BINDIR=${CMAKE_INSTALL_BINDIR} ")
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR})
set(CMAKE_INSTALL_MANDIR ${CMAKE_PREFIX_PATH}/man)
set(CMAKE_INSTALL_PREFIX ${CMAKE_PREFIX_PATH})
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(DISABLE_FORCE_DEBUG_POSTFIX ON)
#set(CMAKE_INSTALL_DATAROOTDIR ${CMAKE_PREFIX_PATH}/data)
#set(CMAKE_INSTALL_DATADIR ${CMAKE_PREFIX_PATH}/data CACHE PATH "General install location")
#set(CMAKE_INSTALL_LIBDIR ${CMAKE_BINARY_DIR}/lib)
#set(CMAKE_INSTALL_INCLUDEDIR ${CMAKE_PREFIX_PATH}/include)
#set(CMAKE_INSTALL_BINDIR ${CMAKE_PREFIX_PATH}/bin)
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
  add_subdirectory(src/3rdparty)
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

