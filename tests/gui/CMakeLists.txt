project(gui_test C CXX)
set(CMAKE_CXX_STANDARD 11)
aux_source_directory(./ SRCS_GUI_TESTS)

include_directories(
   ./
   ${DEPS_DIR}/include
   ${DEPS_DIR}/include/gui
   ${DEPS_DIR}/include/porting
   ${DEPS_DIR}/include/cairo
   ${DEPS_DIR}/include/epg
   ${DEPS_DIR}/include/freetype2
   ${DEPS_DIR}/include/harfbuzz
   ${DEPS_DIR}/include/fribidi
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/src
   ${DEPS_DIR}/include/uriparser
)
link_directories(${DEPS_DIR}/lib)
add_executable(gui_test ${SRCS_GUI_TESTS})

target_link_libraries(gui_test PRIVATE cdroid gtest)

install(TARGETS gui_test DESTINATION bin/tests)

