cmake_minimum_required (VERSION 3.8)
project(w9 C CXX)

set(CMAKE_CXX_STANDARD 11)

include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/cairo
   ${CMAKE_BINARY_DIR}/include/epg
   ${CMAKE_BINARY_DIR}/include/freetype2
   
)
aux_source_directory(./ W9_SRCS)
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(w9 ${W9_SRCS})

CreatePAK(w9 ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/w9.pak ${PROJECT_SOURCE_DIR}/R.h)

message("CDROID_LIBRARIES=${CDROID_LIBRARIES} CDROID_INCLUDE_DIRS=${CDROID_INCLUDE_DIRS}")
target_link_libraries(w9 PRIVATE cdroid)

install(TARGETS uidemo1 DESTINATION bin)

