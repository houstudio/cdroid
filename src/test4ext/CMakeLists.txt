cmake_minimum_required (VERSION 3.2)
project(3rdtests C CXX)
set(CMAKE_CXX_STANDARD 11)
include_directories(
   ./
   ./porting/inc
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/porting
)

add_subdirectory(porting)
link_directories(${HAL_EXTRA_LIBDIRS})
message("HAL_EXTRA_LIBS=${HAL_EXTRA_LIBS} \r\nHAL_EXTRA_LIBDIRS=${HAL_EXTRA_LIBDIRS}")

add_executable(3rdtests main.cc) 


target_link_libraries(3rdtests tvhal_static )

install (TARGETS 3rdtests  DESTINATION bin)
