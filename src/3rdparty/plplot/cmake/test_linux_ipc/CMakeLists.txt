# Top-level CMakeLists.txt for a simple project to test Linux IPC efficiency.

# MAINTENANCE
# Use same minimum version for all platforms as the non-Linux platform minimum adopted for
# the PLplot project.
cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)

project(test_linux_ipc C)

add_executable(pshm_write pshm_write.c)
add_executable(pshm_read pshm_read.c)
add_executable(pshm_unlink pshm_unlink.c)

set_target_properties(pshm_write pshm_read pshm_unlink
  PROPERTIES
  INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}"
  )

# Note that both pshm_write.c and phsm_read.c contain calls to shm_open
# which on most Unix systems requires an explicit link
# with the rt library.  However, Mac OS X is a Unix system that does
# not have the rt library and provides those functions instead as part
# of the ordinary C libraries that do not require explicit linking.
# So only define RT_LIB for the Unix but not Mac case.
if(UNIX AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  find_library(RT_LIB rt)
  if(RT_LIB)
    message(STATUS "RT_LIB = ${RT_LIB}")
  else(RT_LIB)
    message(STATUS "WARNING: rt library could not be found so that pshm_write and pshm_read may not link correctly")
    set(RT_LIB)
  endif(RT_LIB)
else(UNIX AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(RT_LIB)
endif(UNIX AND NOT CMAKE_SYSTEM_NAME STREQUAL "Darwin")

# The -pthread compiler option set below is not enough, and I have
# to add this library to the ones that are linked in.
find_library(PTHREAD_LIB pthread)

target_link_libraries(pshm_write ${RT_LIB} ${PTHREAD_LIB})
target_link_libraries(pshm_read ${RT_LIB} ${PTHREAD_LIB})
target_link_libraries(pshm_unlink ${RT_LIB} ${PTHREAD_LIB})

# Web advice says -pthread has to be a compiler option as well.
target_compile_options(pshm_write PRIVATE "-pthread")
target_compile_options(pshm_read PRIVATE "-pthread")
target_compile_options(pshm_unlink PRIVATE "-pthread")
