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

find_library(PTHREAD_LIBRARY NAMES ${PTHREAD_LIB_NAME}
             PATHS "${CMAKE_INSTALL_PREFIX}/lib" 
             PATH_SUFFIXES "lib" "lib64"
             NO_DEFAULT_PATH)

find_path(PTHREAD_INCLUDE_DIR NAMES pthread.h
          PATHS "${CMAKE_INSTALL_PREFIX}/include"
          PATH_SUFFIXES "include"
          NO_DEFAULT_PATH)

if(NOT PTHREAD_LIBRARY)
    message(FATAL_ERROR "Could not find ${PTHREAD_LIB_NAME} library")
endif()

if(NOT PTHREAD_INCLUDE_DIR)
    message(FATAL_ERROR "Could not find pthread.h")
endif()

set(PTHREAD_LIBRARIES ${PTHREAD_LIBRARY})
set(PTHREAD_INCLUDE_DIRS ${PTHREAD_INCLUDE_DIR})

include(FindPackageHandleStandardVars)
find_package_handle_standard_args(PThread DEFAULT_MSG PTHREAD_LIBRARIES PTHREAD_INCLUDE_DIRS)

mark_as_advanced(PTHREAD_INCLUDE_DIR PTHREAD_LIBRARIES)
