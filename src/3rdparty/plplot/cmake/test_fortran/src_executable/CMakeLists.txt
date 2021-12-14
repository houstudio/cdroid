# src_executable/CMakeLists.txt

if(BUILD_TEST)

  if(CORE_BUILD)
    include_directories(${CMAKE_BINARY_DIR}/src_lib)
  else(CORE_BUILD)
    include_directories(${FORTRAN_MOD_DIR})
  endif(CORE_BUILD)

  add_executable(hello hello.f90)

  target_link_libraries(hello hello_1)

  # configure file to compare with output of hello executable.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/compare_hello.out
    "hello, world"
    )

  # Add test_noninteractive custom_target to compare hello executable output
  # with file configure above.
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/hello.out
    COMMAND
    hello > ${CMAKE_CURRENT_BINARY_DIR}/hello.out
    COMMAND
    ${CMP_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/hello.out ${CMAKE_CURRENT_BINARY_DIR}/compare_hello.out
    DEPENDS
    hello
    VERBATIM
    )
  add_custom_target(test_noninteractive
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/hello.out
    )

endif(BUILD_TEST)

if(CORE_BUILD)
  install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    ${CMAKE_CURRENT_SOURCE_DIR}/hello.f90
    DESTINATION ${DATA_DIR}/examples/fortran
    )
endif(CORE_BUILD)
