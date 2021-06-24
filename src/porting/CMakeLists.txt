cmake_minimum_required (VERSION 3.2)
project(tvhal C CXX)

if( CMAKE_SYSTEM_NAME MATCHES "Linux" )
add_definitions(-DLINUX=1)
endif()

message("CDROID_CHIPSET=${CDROID_CHIPSET}  USE_RFB_GRAPH=${USE_RFB_GRAPH}")

set(SRCS_COMMON common/cdlog.cc)

include_directories(include common)

add_subdirectory(${CDROID_CHIPSET})

link_directories(${CMAKE_SOURCE_DIR}/deps/lib)

add_library(tvhal SHARED ${SRCS_COMMON} )
add_library(tvhal_static STATIC ${SRCS_COMMON} )

target_link_libraries(tvhal ${CDROID_CHIPSET})
target_link_libraries(tvhal_static ${CDROID_CHIPSET})
set_target_properties(tvhal_static PROPERTIES OUTPUT_NAME "tvhal")

file(GLOB HAL_HEADERS  ${PROJECT_SOURCE_DIR}/include/*.h)

file(GLOB KEYCMAPS   ${PROJECT_SOURCE_DIR}/${CDROID_CHIPSET}/*.kcm)
if( NOT(${CDROID_CHIPSET} MATCHES "x86") )
    file(GLOB KEYMAP   ${PROJECT_SOURCE_DIR}/${CDROID_CHIPSET}/*.kl)
endif()

install (TARGETS tvhal  DESTINATION lib)
install (TARGETS tvhal_static  DESTINATION lib)
install (FILES ${HAL_HEADERS} DESTINATION include/porting/)

file(COPY ${HAL_HEADERS} common/shared_queue.h  DESTINATION ${CMAKE_BINARY_DIR}/include/porting)
file(COPY ${KEYCMAPS} ${KEYMAP} DESTINATION ${CMAKE_BINARY_DIR} PATTERN "*.k*")

