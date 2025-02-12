cmake_minimum_required (VERSION 3.8)
project(gui_test C CXX)

set(CMAKE_CXX_STANDARD 14)
find_package(PLPLOT)
find_package(CDROID)
find_package(Freetype)
include_directories(
   ./
   ${CDROID_INCLUDE_DIRS}
   ${CAIRO_INCLUDE_DIRS}
   ${FONTCONFIG_INCLUDE_DIRS}
   ${CMAKE_BINARY_DIR}/include
   ${CMAKE_BINARY_DIR}/include/gui
   ${CMAKE_BINARY_DIR}/include/porting
   ${PLPLOT_INCLUDE_DIR}
   ${CAIRO_INCLUDE_DIRS}
   ${FREETYPE_INCLUDE_DIRS}
)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
set(CMAKE_CXX_FLAGS "-Wl,--copy-dt-needed-entries")  #prevent error adding symbols: DSO missing from command line
endif()
link_directories(${CMAKE_BINARY_DIR}/lib)
file(GLOB ExamplesFileList  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc")
add_custom_target(build_app_examples COMMENT A target that requires all the examples.)

if(NOT ENABLE_PLPLOT)
   list(REMOVE_ITEM ExamplesFileList plot.cc)
endif()
#add_definitions(--include cdtypes.h)
#add_definitions(--include cdlog.h)
#add_definitions(--include core/stdpatch.h)
link_directories({CMAKE_BINARY_DIR}/src/gui/Debug)
if(BUILD_EXAMPLES)
foreach(Example ${ExamplesFileList})
  message(STATUS "\tCreating build rule for ${Example}")
  get_filename_component(ExampleName ${Example} NAME_WE)
  # Define example executable
  add_executable(${ExampleName} ${Example})
  # Link example against curlpp
  target_link_libraries(${ExampleName} cdroid)
  # make the meta target depend on this example.
  add_dependencies(build_app_examples ${ExampleName})
  install(TARGETS ${ExampleName} DESTINATION bin/examples)
endforeach(Example ${ExamplesFileList})
endif(BUILD_EXAMPLES)
