cmake_minimum_required (VERSION 3.8)
project(preferencedemo)

set(CMAKE_CXX_STANDARD 14)

# CDROID includes/libraries come from the parent scope (all apps/ projects
# configure under the cdroid tree; no find_package needed — see kaidu_ms7).
include_directories(
    ./
    ${CDROID_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
    ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_SOURCE_DIR}/src/net        # cdnet (network settings screen)
   ${CMAKE_SOURCE_DIR}/src/bluetooth  # cdblue (connected devices screen)
)

add_executable(preferencedemo main.cc)

# App pak: the settings preference XMLs + value arrays, compiled to binary
# AXML/arsc (aapt2, -I framework.apk + widgetex.apk) and loaded at runtime
# as preferencedemo.pak beside the binary.
CreatePAK(preferencedemo ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/preferencedemo.pak ${PROJECT_SOURCE_DIR}/R.h EMBED_EXE)

target_link_libraries(preferencedemo PRIVATE cdroid cdnet cdblue)
install(TARGETS preferencedemo DESTINATION bin)
