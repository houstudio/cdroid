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
aux_source_directory(./ DEMO_SRCS)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(uidemo2 ${DEMO_SRCS})

CreatePAK(uidemo2 ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/uidemo2.pak ${PROJECT_SOURCE_DIR}/R.h)

set(RESOURCES_DIR "${PROJECT_SOURCE_DIR}/assets")
file(GLOB ASSET_FILES "${RESOURCES_DIR}/*")
source_group(TREE "${RESOURCES_DIR}" PREFIX "Resources" FILES ${ASSET_FILES})
set(ANDROID_RESOURCE_DIR "${RESOURCES_DIR}")
#android_add_resource_dir("${ANDROID_RESOURCE_DIR}")
set(CMAKE_ANDROID_ASSETS_DIRECTORIES ${ANDROID_RESOURCE_DIR})

target_link_libraries(uidemo2 cdroid)

install(TARGETS uidemo2 DESTINATION bin)

