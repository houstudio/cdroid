cmake_minimum_required (VERSION 3.8)
project(uidemo2 C CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
  )
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
aux_source_directory(./ DEMO_SRCS)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(uidemo2 ${DEMO_SRCS})

CreatePAK(uidemo2 ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/uidemo2.pak ${PROJECT_SOURCE_DIR}/R.h)

target_link_libraries(uidemo2 cdroid)

install(TARGETS uidemo2 DESTINATION bin)

