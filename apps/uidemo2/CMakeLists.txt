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
   ${CMAKE_BINARY_DIR}/include/cairo
   ${CMAKE_BINARY_DIR}/include/epg
   ${CMAKE_BINARY_DIR}/include/freetype2
  )
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
aux_source_directory(./ DEMO_SRCS)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(uidemo2 ${DEMO_SRCS})

CreatePO(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR} uidemo2)

CreatePAK(uidemo2 ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/uidemo2.pak ${PROJECT_SOURCE_DIR}/R.h)

add_custom_target(uidemotranslate
  COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/uidemo2.po
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Translate strings resource..."
  )

target_link_libraries(uidemo2 cdroid)

install(TARGETS uidemo2 DESTINATION bin)

