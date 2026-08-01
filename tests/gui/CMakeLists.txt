project(gui_test C CXX)
aux_source_directory(./ SRCS_GUI_TESTS)
find_package(GTEST REQUIRED)
find_package(Pixman REQUIRED)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CDROID_DEPINCLUDES}
   ${GTEST_INCLUDE_DIRS}
   ${PIXMAN_INCLUDE_DIRS}
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_SOURCE_DIR}/src/gui
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/uriparser
)
link_directories(${CMAKE_BINARY_DIR}/lib)
add_executable(gui_test ${SRCS_GUI_TESTS})
# Pack the test assets (navigation graphs, etc.) into gui_test.pak. App auto-loads
# <exe-name>.pak (src/gui/core/app.cc), so @navigation/* and idgen IDs become reachable.
CreatePAK(gui_test ${PROJECT_SOURCE_DIR}/assets ${PROJECT_BINARY_DIR}/gui_test.pak ${PROJECT_SOURCE_DIR}/R.h)
target_link_libraries(gui_test PRIVATE ${PIXMAN_LIBRAIRES} ${CDROID_DEPLIBS} ${GTEST_LIBRARIES} cdroid tvhal)

install(TARGETS gui_test DESTINATION bin/tests)

