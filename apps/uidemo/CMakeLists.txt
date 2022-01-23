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
add_custom_target(uidemopo
  COMMAND touch ${PROJECT_BINARY_DIR}/uidemo.po
  COMMAND xgettext -d uidemo -j -c -p${PROJECT_BINARY_DIR} -kTEXT ${DEMO_SRCS}
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
  COMMENT "xgettext:Make MultiLanguage files (.po) for files in DIR:${PROJECT_SOURCE_DIR}"
)


CreatePAK(uidemo ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/uidemo.pak ${PROJECT_SOURCE_DIR}/R.h)

add_custom_target(uidemotranslate
  COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/uidemo.po
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Translate strings resource..."
  )

macro(makepopak tgname arg)
 add_custom_target(${tgname}
   COMMAND zip -r -0  ${PROJECT_BINARY_DIR}/${arg}.pak *
   COMMAND touch ${PROJECT_BINARY_DIR}/${arg}.po
   #COMMAND xgettext  -d ${arg} -j -c -p${PROJECT_BINARY_DIR} -kTEXT ${DEMO_SRCS}
   WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/assets
   COMMENT "=====xgettext:Make MultiLanguage files (.po) and resource package(pak)"
 )
endmacro()
makepopak(uidemodummy uidemo)
add_dependencies(uidemo uidemodummy)

target_link_libraries(uidemo gui id3 tvhal)

install(TARGETS uidemo DESTINATION bin)

