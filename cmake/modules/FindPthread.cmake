set(PTHREAD_LIBRARIES "")
set(PTHREAD_INCLUDE_DIRS "")

set(PTHREAD_LIB_NAME "pthreadVC3")

if(MSVC)
    if(${CMAKE_CXX_FLAGS} MATCHES "/MD")
        set(PTHREAD_LIB_NAME "pthreadVC3")
    elseif(${CMAKE_CXX_FLAGS} MATCHES "/MT")
        set(PTHREAD_LIB_NAME "pthreadVCE3")
    elseif(${CMAKE_CXX_FLAGS} MATCHES "/ML")
        set(PTHREAD_LIB_NAME "pthreadVSE3")
    else()
        message("Unsupported Runtime Library option")
    endif()
else()
    set(PTHREAD_LIB_NAME "pthread")
endif()
pkg_check_modules(PC_ZLIB  zlib)
find_library(PTHREAD_LIBRARY NAMES ${PTHREAD_LIB_NAME}
	PATHS "${CMAKE_MODULE_PATH}"
	HINTS ${PC_ZLIB_LIBDIR} ${PC_ZLIB_LIBRARY_DIRS}
             PATH_SUFFIXES "lib" "lib64")

find_path(PTHREAD_INCLUDE_DIR NAMES pthread.h
	HINTS ${PC_ZLIB_INCLUDEDIR} ${PC_ZLIB_INCLUDE_DIRS}
          PATH_SUFFIXES "include"
          )

if(NOT PTHREAD_LIBRARY)
   #message(FATAL_ERROR "Could not find ${PTHREAD_LIB_NAME} library in ${CMAKE_MODULE_PATH}")
endif()

if(NOT PTHREAD_INCLUDE_DIR)
   #message(FATAL_ERROR "Could not find pthread.h in ${CMAKE_MODULE_PATH}")
endif()
set(PTHREAD_FOUND TRUE)
set(PTHREAD_LIBRARIES ${PTHREAD_LIBRARY})
set(PTHREAD_INCLUDE_DIRS ${PTHREAD_INCLUDE_DIR})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Pthread DEFAULT_MSG PTHREAD_FOUND PTHREAD_LIBRARIES PTHREAD_INCLUDE_DIRS)

mark_as_advanced(PTHREAD_FOUND PTHREAD_INCLUDE_DIR PTHREAD_LIBRARIES)
