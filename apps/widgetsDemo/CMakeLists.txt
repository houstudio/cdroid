cmake_minimum_required (VERSION 3.8)
project(widgetsDemo C CXX)

set(CMAKE_CXX_STANDARD 14)

include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
)
aux_source_directory(./ DEMO_SRCS)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(widgetsDemo ${DEMO_SRCS})

CreatePAK(widgetsDemo ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/widgetsDemo.pak ${PROJECT_SOURCE_DIR}/R.h)

message("widgetsDemo: CDROID_LIBRARIES=${CDROID_LIBRARIES} CDROID_INCLUDE_DIRS=${CDROID_INCLUDE_DIRS}")
target_link_libraries(widgetsDemo PRIVATE cdroid)

install(TARGETS widgetsDemo DESTINATION bin)
