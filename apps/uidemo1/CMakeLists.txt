cmake_minimum_required (VERSION 3.8)
project(uidemo1 C CXX)

set(CMAKE_CXX_STANDARD 11)

#find_package(CDROID REQUIRED)
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
aux_source_directory(./ DEMO_SRCS)
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(uidemo1 ${DEMO_SRCS})

CreatePAK(uidemo1 ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/uidemo1.pak ${PROJECT_SOURCE_DIR}/R.h)

add_custom_target(uidemo1translate
  COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/uidemo.po
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Translate strings resource..."
  )

message("CDROID_LIBRARIES=${CDROID_LIBRARIES} CDROID_INCLUDE_DIRS=${CDROID_INCLUDE_DIRS}")
target_link_libraries(uidemo1 PRIVATE cdroid)

install(TARGETS uidemo1 DESTINATION bin)

