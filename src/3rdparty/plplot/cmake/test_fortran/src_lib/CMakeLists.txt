# src_lib/CMakeLists.txt

set(SOURCE_LIST
${CMAKE_CURRENT_SOURCE_DIR}/hello_1.f90
)

add_library(hello_1 ${SOURCE_LIST})

if(USE_RPATH)
  set_target_properties(
    hello_1
    PROPERTIES
    INSTALL_RPATH "${LIB_INSTALL_RPATH}"
    )
else(USE_RPATH)
  set_target_properties(
    hello_1
    PROPERTIES
    INSTALL_NAME_DIR "${LIB_DIR}"
    )
endif(USE_RPATH)

set_target_properties(hello_1
PROPERTIES
SOVERSION 0
VERSION 0.0
OUTPUT_NAME hello
POSITION_INDEPENDENT_CODE ON
)

install(TARGETS hello_1
  EXPORT export_test_fortran
  ARCHIVE DESTINATION ${LIB_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  RUNTIME DESTINATION ${BIN_DIR}
  )

# This exports information for every target for the whole build that
# has the EXPORT export_test_fortran signature for the
# install(TARGETS ...) command.  The only such target in this test_fortran
# project is the hello_1 target above.
install(EXPORT export_test_fortran DESTINATION ${LIB_DIR}/cmake/test_fortran)

# Install overall configuration file describing the above export
install(FILES test_fortranConfig.cmake DESTINATION ${LIB_DIR}/cmake/test_fortran)

# Install module file
install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/hello_module.mod
    DESTINATION ${FORTRAN_MOD_DIR}
    )


