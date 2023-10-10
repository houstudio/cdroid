project(gui_test C CXX)
aux_source_directory(./ SRCS_GUI_TESTS)
find_package(GTEST REQUIRED)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${GTEST_INCLUDE_DIRS}
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_SOURCE_DIR}/src/gui
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/uriparser
)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(gui_test ${SRCS_GUI_TESTS})

target_link_libraries(gui_test PRIVATE ${CDROID_DEPLIBS} ${GTEST_LIBRARIES} cdroid tvhal)

install(TARGETS gui_test DESTINATION bin/tests)

