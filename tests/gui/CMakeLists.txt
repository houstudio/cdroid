project(gui_test C CXX)
set(CMAKE_CXX_STANDARD 11)
aux_source_directory(./ SRCS_GUI_TESTS)
find_package(GTEST REQUIRED)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${GTEST_INCLUDE_DIRS}
   ${DEPS_DIR}/include/porting
   ${DEPS_DIR}/include
   ${DEPS_DIR}/include/freetype2
   ${DEPS_DIR}/include/harfbuzz
   ${DEPS_DIR}/include/fribidi
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg/src
   ${CMAKE_SOURCE_DIR}/src/3rdparty/libsvg-cairo/src
   ${DEPS_DIR}/include/uriparser
)
link_directories(${DEPS_DIR}/lib)
add_executable(gui_test ${SRCS_GUI_TESTS})

target_link_libraries(gui_test PRIVATE ${CDROID_DEPLIBS} ${CDROID_LIBRARIES} ${GTEST_LIBRARIES} tvhal)

install(TARGETS gui_test DESTINATION bin/tests)

