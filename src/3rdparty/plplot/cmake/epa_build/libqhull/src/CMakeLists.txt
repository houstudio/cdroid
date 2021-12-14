# Configurable items here.
option(USE_RPATH "Use -rpath when linking libraries, executables" ON)

# Change before each release whenever there has been an API change of any
# kind.
set(qhull_SOVERSION 5)
set(qhull_VERSION ${qhull_SOVERSION}.0.0)

# libraries are all shared by default
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Use static build for now on Windows for the
# "Ninga" generator case since that case appears to
# be broken for the shared library build
# _AND_ using the same name kernel ("qhull") for the library
# and executable.
if(WIN32 AND CMAKE_GENERATOR STREQUAL "Ninja")
  message(STATUS "Force static library build for qhull as temporary workaround for Ninja bug.")
  set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build shared libraries" FORCE)
endif(WIN32 AND CMAKE_GENERATOR STREQUAL "Ninja")

# some extra flags for Borland C++ Compiler 5.5
if(BORLAND)
  add_definitions(-A -w-)
endif(BORLAND)

# Order object files by frequency of execution.  Small files at end.
set(
  qhull_src
  rboxlib.c
  user.c
  global.c
  stat.c
  io.c
  geom2.c
  poly2.c
  merge.c
  libqhull.c
  geom.c
  poly.c
  qset.c
  mem.c
  usermem.c
  userprintf.c
  random.c
  )

# header files
set(
  qhull_hdr
  geom.h
  io.h
  libqhull.h
  mem.h
  merge.h
  poly.h
  qhull.h
  qhull_a.h
  qset.h
  random.h
  stat.h
  user.h
  )

add_library(
  qhull
  ${qhull_src}
  )

# Symbol visibility support.
if(BUILD_SHARED_LIBS)
  set_target_properties(
    qhull
    PROPERTIES
    COMPILE_DEFINITIONS "USINGDLL"
    )
endif(BUILD_SHARED_LIBS)

# find math library
if(NOT WIN32)
  find_library(MATH_LIB NAMES m PATHS /usr/local/lib /usr/lib)
  if(NOT MATH_LIB)
    message(FATAL_ERROR "Cannot find required math library")
  endif(NOT MATH_LIB)
  target_link_libraries(
    qhull
    ${MATH_LIB}
    )
endif(NOT WIN32)

# qh_QHpointer=1 is the value used by Debian packagers for qhull_2003.1
# but comment it out since it causes segfaults for PLplot for qhull_2010.1,
# and not setting the value is the default which is presumably the value
# best tested by the qhull developers.
set_target_properties(
  qhull
  PROPERTIES
  SOVERSION ${qhull_SOVERSION}
  VERSION ${qhull_VERSION}
#  COMPILE_DEFINITIONS qh_QHpointer=1
  )

if(UNIX)
  if(APPLE)
    set_target_properties(qhull PROPERTIES
      INSTALL_NAME_DIR "${CMAKE_INSTALL_PREFIX}/lib")
  else(APPLE)
    set_target_properties(qhull PROPERTIES
      INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib"
      INSTALL_RPATH_USE_LINK_PATH TRUE
      BUILD_WITH_INSTALL_RPATH FALSE)
  endif(APPLE)
endif(UNIX)

install(
  TARGETS qhull
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  )


# samples
set(qhullcmd_SOURCES  unix.c)
set(rbox_SOURCES      rbox.c)
set(qconvex_SOURCES   qconvex.c)
set(qdelaunay_SOURCES qdelaun.c)
set(qvoronoi_SOURCES  qvoronoi.c)
set(qhalf_SOURCES     qhalf.c)

set(
  TARGET_LIST
  qhullcmd rbox qconvex qdelaunay qvoronoi qhalf
  )


foreach(TARGET ${TARGET_LIST})
  add_executable(${TARGET} ${${TARGET}_SOURCES})
  target_link_libraries(${TARGET} qhull)
  if(USE_RPATH)
    set_target_properties(
      ${TARGET}
      PROPERTIES
      INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib
      INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/bin
      )
  else(USE_RPATH)
    set_target_properties(
      ${TARGET}
      PROPERTIES
      INSTALL_NAME_DIR ${CMAKE_INSTALL_PREFIX}/bin
      )
  endif(USE_RPATH)
  install(TARGETS ${TARGET} DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
endforeach(TARGET ${TARGET_LIST})

set_property(TARGET qhullcmd PROPERTY OUTPUT_NAME "qhull")


# install header
install(
  FILES ${qhull_hdr}
  DESTINATION include/qhull
  )
