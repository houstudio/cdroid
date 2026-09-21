cmake_minimum_required (VERSION 3.8)
project(weardemos C CXX)
set(CMAKE_CXX_STANDARD 14)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
)
# Explicit source list — one file per demo activity, mirroring the upstream
# SupportWearDemos layout (one Activity per demo).
set(DEMO_SRCS
   main.cc
   simplewearablerecyclerdemo.cc
   simplerecyclerdemo.cc
   simplenestedscrollviewdemo.cc
   wearableswitchdemo.cc
   circularprogresslayoutdemo.cc
   roundeddrawabledemo.cc
   alertdialogdemo.cc
   confirmationoverlaydemo.cc
)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(weardemos ${DEMO_SRCS})
CreatePAK(weardemos ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/weardemos.pak ${PROJECT_SOURCE_DIR}/R.h)
target_link_libraries(weardemos PRIVATE cdroid)
install(TARGETS weardemos DESTINATION bin)
