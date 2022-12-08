cmake_minimum_required (VERSION 3.8)
project(uidemo1 C CXX)

set(CMAKE_CXX_STANDARD 11)

find_package(CDROID REQUIRED)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${DEPS_DIR}/include/porting
   ${DEPS_DIR}/include/epg
)
aux_source_directory(./ DEMO_SRCS)
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
link_directories(${DEPS_DIR}/lib)
add_executable(uidemo1 ${DEMO_SRCS})

CreatePO(${PROJECT_SOURCE_DIR} ${PROJECT_BINARY_DIR} uidemo1)

CreatePAK(uidemo1 ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/uidemo1.pak ${PROJECT_SOURCE_DIR}/R.h)

add_custom_target(uidemo1translate
  COMMAND python  ${CMAKE_SOURCE_DIR}/src/tools/po2json.py ${CMAKE_CURRENT_BINARY_DIR}/uidemo.po
  WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
  COMMENT "Translate strings resource..."
  )

message("CDROID_LIBRARIES=${CDROID_LIBRARIES} CDROID_INCLUDE_DIRS=${CDROID_INCLUDE_DIRS}")
target_link_libraries(uidemo1 PRIVATE ${CDROID_LIBRARIES})

install(TARGETS uidemo1 DESTINATION bin)

