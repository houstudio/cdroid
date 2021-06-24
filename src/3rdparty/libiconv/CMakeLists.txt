# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Drahoš
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
# Please note that the package source code is licensed under its own license.

project ( libiconv C )
cmake_minimum_required ( VERSION 2.8 )
include ( cmake/dist.cmake )
include ( configure )

# Options
option ( ENABLE_EXTRA "Enable a few rarely used encodings" OFF)
option ( ENABLE_NLS "Translation of program messages to the user's native language is requested" OFF)
#option ( ENABLE_RELOCATABLE "The package shall run at any location in the file system" ON )

# iconv.h
set ( USE_MBSTATE_T 1 )
set ( BROKEN_WCHAR_H 0 )
set ( HAVE_WCHAR_T 0)

# setup default include directories 
include_directories(${PROJECT_BINARY_DIR}/include)

configure_file ( config.h.cmake ${PROJECT_BINARY_DIR}/include/config.h )

iconv_configure_file (LIBRARY_NAME iconv 
  INPUT include/iconv.h.build.in 
  OUTPUT ${PROJECT_BINARY_DIR}/include/iconv.h )

iconv_configure_file (LIBRARY_NAME iconv 
  INPUT srclib/alloca.in.h 
  OUTPUT ${PROJECT_BINARY_DIR}/srclib/alloca.h )

iconv_configure_file (LIBRARY_NAME iconv 
  INPUT include/iconv.h.build.in 
  OUTPUT ${PROJECT_BINARY_DIR}/include/iconv.h )

iconv_configure_file (LIBRARY_NAME iconv 
  INPUT srclib/unistd.in.h 
  OUTPUT ${PROJECT_BINARY_DIR}/srclib/unistd.h @ONLY)

iconv_configure_file (LIBRARY_NAME iconv 
  INPUT srclib/unitypes.in.h 
  OUTPUT ${PROJECT_BINARY_DIR}/srclib/unitypes.h )

iconv_configure_file (LIBRARY_NAME charset
  INPUT libcharset/include/libcharset.h.in 
  OUTPUT ${PROJECT_BINARY_DIR}/include/libcharset.h )

iconv_configure_file (LIBRARY_NAME charset
  INPUT libcharset/include/localcharset.h.build.in 
  OUTPUT ${PROJECT_BINARY_DIR}/include/localcharset.h )

iconv_configure_file (LIBRARY_NAME iconv
  INPUT srclib/uniwidth.in.h
  OUTPUT ${PROJECT_BINARY_DIR}/srclib/uniwidth.h)

# Dirty fix for MinGW
if ( MINGW )
  add_definitions ( -DELOOP=0 -DHAVE_DECL_STRERROR_R=0 )
  configure_file ( srclib/alloca.in.h ${PROJECT_SOURCE_DIR}/srclib/alloca.h )
endif ()

include_directories ( ${PROJECT_SOURCE_DIR}  include srclib)
add_definitions ( -Dset_relocation_prefix=libcharset_set_relocation_prefix -Drelocate=libcharset_relocate -DHAVE_CONFIG_H -DINSTALLPREFIX=NULL -DNO_XMALLOC -DBUILDING_LIBCHARSET -DINSTALLDIR="" -DLIBDIR="" -DENABLE_RELOCATABLE=1 -DIN_LIBRARY -DENABLE_EXTRA=1)

# libcharset
set ( SRC_LIBCHARSET
  libcharset/lib/localcharset.c
  libcharset/lib/relocatable.c
)

####  preloadable_libiconv.so  ###
add_library (preloadable_libiconv lib/iconv.c libcharset/lib/localcharset.c lib/relocatable.c)
target_compile_definitions(preloadable_libiconv PRIVATE -DLIBICONV_PLUG)
set_target_properties(preloadable_libiconv PROPERTIES PREFIX "")
##################################

add_library ( charset ${SRC_LIBCHARSET} )
if ( NOT MSVC )
  target_compile_options (charset PRIVATE -DBUILDING_DLL)
endif()
target_compile_options (charset PRIVATE -DBUILDING_LIBCHARSET)

if ( FALSE AND NOT MSVC )
  # libicrt
  set ( SRC_LIBICRT 
    srclib/allocator.c
    srclib/areadlink.c
    srclib/careadlinkat.c
    srclib/malloca.c
    srclib/progname.c
    srclib/safe-read.c
    srclib/uniwidth/width.c
    srclib/xmalloc.c
    srclib/xstrdup.c
    srclib/xreadlink.c
    srclib/canonicalize-lgpl.c
    srclib/error.c
    srclib/lstat.c
    srclib/readlink.c
    srclib/stat.c
    srclib/strerror.c
    srclib/strerror-override.c  
  )
  #include_directories(${PROJECT_BINARY_DIR}/srclib/)
  add_library ( icrt STATIC ${SRC_LIBICRT} )
endif()
# libiconv
set ( SRC_LIBICONV
  lib/iconv.c
)

add_library ( iconv ${SRC_LIBICONV} )
target_link_libraries ( iconv charset )
set_target_properties ( iconv PROPERTIES COMPILE_FLAGS -DBUILDING_LIBICONV)

if(FALSE AND NOT MSVC)
  add_executable ( iconvcli src/iconv_no_i18n.c )
  target_link_libraries ( iconvcli iconv icrt charset )
  set_target_properties ( iconvcli PROPERTIES OUTPUT_NAME iconv )

  install_executable ( iconvcli )
endif()

# fix library naming for Visual Studio compilers
if(MSVC)
  set_target_properties ( iconv   PROPERTIES OUTPUT_NAME "libiconv" )
  set_target_properties ( charset PROPERTIES OUTPUT_NAME "libcharset" )
endif()

install_library ( iconv charset preloadable_libiconv)
install_header ( ${PROJECT_BINARY_DIR}/include/iconv.h )
install_header ( ${PROJECT_BINARY_DIR}/include/libcharset.h )
install_header ( ${PROJECT_BINARY_DIR}/include/localcharset.h )
install_data ( README AUTHORS COPYING )

file(COPY ${PROJECT_BINARY_DIR}/include/iconv.h 
          ${PROJECT_BINARY_DIR}/include/libcharset.h 
          ${PROJECT_BINARY_DIR}/include/localcharset.h
	  DESTINATION ${CMAKE_BINARY_DIR}/include)
