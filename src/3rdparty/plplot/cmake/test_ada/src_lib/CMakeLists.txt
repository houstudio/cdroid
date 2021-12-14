# src_lib/CMakeLists.txt

set(SOURCE_LIST
${CMAKE_CURRENT_SOURCE_DIR}/hello_1.adb
${CMAKE_CURRENT_SOURCE_DIR}/hello_1.ads
)

# These are Ada library information files built by gnat.  I am not
# sure whether the name suffixes correspond to the *.adb or *.ads files
# above or the union of them.  In any case, if any of the names change
# above, then this list will probably have to be changed as well.)

# N.B. the absolute location prefix of these files may have to be changed
# in the future since this is currently a CMake internal.
set(ALI_PREFIX
${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/hello_1.dir
)

# This variable references the Ada library information file for the library.
# and is useful for cleaning and/or installing the *.ali files.

set(ALI_LIST
${ALI_PREFIX}/hello_1.ali
)

# CMake assumes compilation results only in object files being generated.
# However, gnatmake generates both object files and *.ali (Ada library
# information) files so it doesn't intrinsically know how to clean those
# additional *.ali files.
# Here is a workaround for this fundamental CMake limitation.

# Add generated .ali files to the list of additional files to be
# removed by make clean

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${ALI_LIST}")

add_library(hello_1 ${SOURCE_LIST})

# Link to GNAT_LIB to avoid underlinking the hello_1 library (which causes
# link errors on at least the Cygwin platform), but use
# the PRIVATE keyword (on all platforms) to avoid overlinking Ada applications
# that link to hello_1.
target_link_libraries(hello_1 PRIVATE ${GNAT_LIB})

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

install(FILES ${SOURCE_LIST}
  DESTINATION ${ADA_INCLUDE_DIR}
  )

# Permissions of *.ali files in accordance with
# http://people.debian.org/~lbrenta/debian-ada-policy.html
install(FILES ${ALI_LIST}
  DESTINATION ${ADA_LIB_DIR}
  PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
  )

install(TARGETS hello_1
  EXPORT export_test_ada
  ARCHIVE DESTINATION ${LIB_DIR}
  LIBRARY DESTINATION ${LIB_DIR}
  RUNTIME DESTINATION ${BIN_DIR}
  )

# This exports information for every target for the whole build that
# has the EXPORT export_test_ada signature for the
# install(TARGETS ...) command.  The only such target in this test_ada
# project is the hello_1 target above.
install(EXPORT export_test_ada DESTINATION ${LIB_DIR}/cmake/test_ada)

# Install overall configuration file describing the above export
install(FILES test_adaConfig.cmake DESTINATION ${LIB_DIR}/cmake/test_ada)