cmake_minimum_required (VERSION 3.8)
project(curldownload C CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(CURL REQUIRED)
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
   ${CURL_LIBRARIES}
)
aux_source_directory(./ CURLDLD_SRCS)
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(curldownload ${CURLDLD_SRCS})

#CreatePAK(w9 ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/w9.pak ${PROJECT_SOURCE_DIR}/R.h)

message("CDROID_LIBRARIES=${CDROID_LIBRARIES} CDROID_INCLUDE_DIRS=${CDROID_INCLUDE_DIRS}")
target_link_libraries(curldownload PRIVATE cdroid ${CURL_LIBRARIES})

install(TARGETS curldownload DESTINATION bin)

