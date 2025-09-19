cmake_minimum_required (VERSION 3.8)
project(tvhal C CXX)

if( CMAKE_SYSTEM_NAME MATCHES "Linux" )
    add_definitions(-DLINUX=1)
endif()

message("CDROID_CHIPSET=${CDROID_CHIPSET}")

add_subdirectory(${CDROID_CHIPSET})

file(GLOB KEYCMAPS   ${PROJECT_SOURCE_DIR}/${CDROID_CHIPSET}/*.kcm)
if( NOT(${CDROID_CHIPSET} MATCHES "x64") )
    file(GLOB KEYMAP ${PROJECT_SOURCE_DIR}/${CDROID_CHIPSET}/*.kl)
endif()

file(GLOB HAL_HEADERS  ${PROJECT_SOURCE_DIR}/include/*.h)
install(FILES ${HAL_HEADERS} common/shared_queue.h DESTINATION include/porting)
file(COPY ${HAL_HEADERS} common/shared_queue.h  DESTINATION ${CMAKE_BINARY_DIR}/include/porting)
file(COPY ${KEYCMAPS} ${KEYMAP} DESTINATION ${CMAKE_BINARY_DIR} PATTERN "*.k*")
