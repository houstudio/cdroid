cmake_minimum_required (VERSION 3.8)
project(gui_test C CXX)

set(CMAKE_CXX_STANDARD 11)
find_package(PLPLOT)
find_package(CDROID)
find_package(Freetype)
include_directories(
   ${DEPS_DIR}/include
   ${DEPS_DIR}/include/gui
   ${DEPS_DIR}/include/porting
   ${DEPS_DIR}/include/cairo
   ${DEPS_DIR}/include/epg
   ${DEPS_DIR}/include/freetype2
   ${PLPLOT_INCLUDE_DIR}
   ${FREETYPE_INCLUDE_DIRS}
)
message("***CDROID_LIBRARIES=${CDROID_LIBRARIES}")
set(CMAKE_CXX_FLAGS "-Wl,--copy-dt-needed-entries")  #prevent error adding symbols: DSO missing from command line
link_directories(${DEPS_DIR}/lib)
file(GLOB ExamplesFileList  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cc")
add_custom_target(build_app_examples COMMENT A target that requires all the examples.)

if(NOT ENABLE_PLPLOT)
list(REMOVE_ITEM ExamplesFileList plot.cc)
endif()
add_definitions(--include cdtypes.h)
add_definitions(--include cdlog.h)
foreach(Example ${ExamplesFileList})
  message(STATUS "\tCreating build rule for ${Example}")
  # Get the name without extension or directory
  get_filename_component(ExampleName ${Example} NAME_WE)
  # Define example executable
  add_executable(${ExampleName} ${Example})
  # Link example against curlpp
  target_link_libraries(${ExampleName} cdroid )
  # make the meta target depend on this example.
  add_dependencies(build_app_examples ${ExampleName})
  install(TARGETS ${ExampleName} DESTINATION bin/examples)
endforeach(Example ${ExamplesFileList})

