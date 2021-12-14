# src_executable/CMakeLists.txt

if(BUILD_TEST)
  # Setup compilation of nominal source file (see below) by including
  # directory where test_ada library spec (*.ads) files reside.
  if(CORE_BUILD)
    include_directories(
      ${CMAKE_SOURCE_DIR}/src_lib
      )
  else(CORE_BUILD)
    include_directories(
      ${ADA_INCLUDE_DIR}
      )
  endif(CORE_BUILD)

  # Note due to limitations of the Ada language support for CMake, the
  # nominal source file for add_executable is compiled (which is why a
  # real file has to be specified), but otherwise it is ignored except
  # for its .adb suffix which identifies the Ada language.  The actual
  # source file name for the internal gnatmake command that creates
  # the executable is constructed from <TARGET>.adb.  Since no
  # directory can be specified from this construction (don't ask), you
  # must specify the directory of <TARGET>.adb with an -aI option (see
  # below).
  add_executable(hello hello.adb)

  # The first -aI option is required to find <TARGET>.adb (see above
  # comment). The second -aI option gives access to the test_ada
  # library spec (*.ads) files. The -aL option gives access to the
  # test_ada library *.ali files.  (For the CORE_BUILD case, that
  # location currently works but is probably not guaranteed
  # indefinitely for future versions of CMake.)

  if(CORE_BUILD)
    set_target_properties(
      hello
      PROPERTIES
      LINK_FLAGS
      "\"-aI${CMAKE_CURRENT_SOURCE_DIR}\" \"-aI${CMAKE_SOURCE_DIR}/src_lib\" \"-aL${CMAKE_BINARY_DIR}/src_lib/CMakeFiles/hello_1.dir\""
      )
  else(CORE_BUILD)
    set_target_properties(
      hello
      PROPERTIES
      LINK_FLAGS
      "\"-aI${CMAKE_CURRENT_SOURCE_DIR}\" \"-aI${ADA_INCLUDE_DIR}\" \"-aL${ADA_LIB_DIR}\""
      )
  endif(CORE_BUILD)
  target_link_libraries(hello hello_1)

  # CMake assumes compilation results only in object files being generated.
  # However, gnatmake generates both object files and *.ali (Ada library
  # information) files so it doesn't intrinsically know how to clean those
  # additional *.ali files.
  # Here is a workaround for this fundamental CMake limitation.

  # Add generated .ali file and "extra" generated pair of .ali and .o files
  # to the list of additional files to be removed by make clean

  set_directory_properties(
    PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/hello.dir/hello.ali;${CMAKE_CURRENT_BINARY_DIR}/hello.ali;${CMAKE_CURRENT_BINARY_DIR}/hello.o"
    )

  # configure file to compare with output of hello executable.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/compare_hello.out
    "hello, world"
    )

  # Add test_noninteractive custom_target to compare hello executable output
  # with file configure above.
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hello.out
    COMMAND
    hello
    COMMAND
    ${CMP_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hello.out ${CMAKE_CURRENT_BINARY_DIR}/compare_hello.out
    DEPENDS
    hello
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    VERBATIM
    )
  add_custom_target(test_noninteractive
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/hello.out
    )

endif(BUILD_TEST)

if(CORE_BUILD)
  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/hello.adb
    DESTINATION ${DATA_DIR}/examples/ada
    )
endif(CORE_BUILD)
