cmake_minimum_required (VERSION 3.8)
project(uidemo C CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(CDroid REQUIRED)
include_directories(
   ./
   ${CDROID_INCLUDE_DIR}
   ${DEPS_DIR}/include
   ${DEPS_DIR}/include/gui
   ${DEPS_DIR}/include/porting
   ${DEPS_DIR}/include/cairo
   ${DEPS_DIR}/include/epg
   ${DEPS_DIR}/include/freetype2
)
aux_source_directory(./ DEMO_SRCS)
link_directories(${DEPS_DIR}/lib)
add_executable(uidemo ${DEMO_SRCS} ./R.h)

CreatePO(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR} uidemo)

CreatePAK(uidemo ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/uidemo.pak ${PROJECT_SOURCE_DIR}/R.h)

add_custom_target(uidemotranslate
  COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/uidemo.po
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Translate strings resource..."
  )


target_link_libraries(uidemo gui id3 tvhal)

install(TARGETS uidemo DESTINATION bin)

