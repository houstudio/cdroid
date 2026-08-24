cmake_minimum_required (VERSION 3.8)
project(printerdemo C CXX)
set(CMAKE_CXX_STANDARD 14)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
)
# Explicit source list (aux_source_directory globs at configure time, so newly
# added page files would not be picked up without a manual reconfigure).
set(DEMO_SRCS
   main.cc
   printer_common.cc
   home_fragment.cc
   copy_fragment.cc
   scan_fragment.cc
   maintain_fragment.cc
   settings_fragment.cc
   about_fragment.cc
)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(printerdemo ${DEMO_SRCS})
CreatePAK(printerdemo ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/printerdemo.pak ${PROJECT_SOURCE_DIR}/R.h)
target_link_libraries(printerdemo PRIVATE cdroid)
install(TARGETS printerdemo DESTINATION bin)
