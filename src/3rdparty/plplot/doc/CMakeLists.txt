# Process documentation for the PLplot project.

# Process man pages in this subdirectory.

set(core_manpages pltek.1)

if(PLD_pstex)
  set(core_manpages ${core_manpages} pstex2eps.1)
endif(PLD_pstex)

if(PLD_plmeta)
  set(core_manpages ${core_manpages} plrender.1 plm2gif.1 plpr.1)
endif(PLD_plmeta)

set(tcl_manpages plserver.1 pltcl.1)

set(manpages ${core_manpages})
if (ENABLE_tcl)
  set(manpages ${manpages} ${tcl_manpages})
endif (ENABLE_tcl)

install(FILES ${manpages} DESTINATION ${MAN_DIR}/man1)

# Process the docbook documentation.
add_subdirectory(docbook)

# Process the doxygen documentation.

if(BUILD_DOX_DOC)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    @ONLY
    )

  # Attempt to collect the same source-code files that are specified
  # by the INPUT parameter in Doxyfile(.in).  Note, this includes
  # all configured files in the build tree because all source
  # subdirectories are processed by the top-level CMakeLists.txt file
  # before this doc subdirectory.  However, these dependencies will
  # miss at cmake time the files in the build tree that are generated
  # at run time.

  set(doxygen_prefixes  "*.c;*.cc;*.cxx;*.cpp;*.c++;*.d;*.java;*.h;*.py;*.f90;*.f")
  set(doxygen_directories
    ${CMAKE_SOURCE_DIR}/lib
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/drivers
    ${CMAKE_SOURCE_DIR}/bindings
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_BINARY_DIR}/lib
    ${CMAKE_BINARY_DIR}/bindings
    ${CMAKE_BINARY_DIR}/include
    )
  set(doxygen_globs )
  foreach(directory ${doxygen_directories})
    foreach(prefix ${doxygen_prefixes})
      list(APPEND doxygen_globs ${directory}/${prefix})
    endforeach(prefix ${doxygen_prefixes})
  endforeach(directory ${doxygen_directories})
  # Must append one specific configured file to be consistent with
  # the INPUT parameter in Doxyfile(.in).
  list(APPEND doxygen_globs ${CMAKE_BINARY_DIR}/plplot_config.h)
  #message(STATUS "DEBUG: doxygen_globs = ${doxygen_globs}")

  # RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} works (because that is the
  # [default] working directory for running the doxygen command) and makes the
  # resulting paths substantially shorter, but is actually not
  # necessary.
  file(GLOB_RECURSE doxygen_file_depends
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    ${doxygen_globs}
    )
  #message(STATUS "DEBUG: doxygen_file_depends = ${doxygen_file_depends}")

  # Notes: OUTPUT file is a directory so this insures "make clean"
  # removes that directory.
  add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/doxygen
    COMMAND ${DOXYGEN_EXECUTABLE}
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
    ${doxygen_file_depends}
    )
  add_custom_target(build_doxygen ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/doxygen
    )

  # Add some library dependencies that will insure all run-time
  # generated source files will be correctly processed by doxygen.
  # These won't have the proper file dependencies (see comment above)
  # so the doxygen documentation won't be remade if one of these
  # run-time files is regenerated, but source-file regeneration (which
  # would be caused by editing one of the prerequisites of these
  # generated source files) should be a rare case in practice.
  add_dependencies(build_doxygen PLPLOT::plplot)
  # Generated C code for the Tcl bindings.
  if(ENABLE_tcl)
    add_dependencies(build_doxygen PLPLOT::plplottcltk)
  endif(ENABLE_tcl)
  # Generated C (and C++ in the Octave case) code for the
  # swig-generated bindings.
  if(ENABLE_python)
    add_dependencies(build_doxygen PLPLOT::plplotc)
  endif(ENABLE_python)
  if(ENABLE_java)
    add_dependencies(build_doxygen PLPLOT::plplotjavac_wrap)
  endif(ENABLE_java)
  if(ENABLE_lua)
    add_dependencies(build_doxygen PLPLOT::plplotluac)
  endif(ENABLE_lua)
  if(ENABLE_octave)
    add_dependencies(build_doxygen PLPLOT::plplot_octave)
  endif(ENABLE_octave)
  # Generated automoc code for the plplotqt library.
  if(ENABLE_qt)
    add_dependencies(build_doxygen PLPLOT::plplotqt)
  endif(ENABLE_qt)
elseif(PREBUILT_DOX_DOC)
  # For prebuilt doxygen documentation copy from source tree
  # to build tree just as if that doxygen documentation had been
  # generated.
  if(CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
    add_custom_target(build_doxygen ALL)
  else(CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
    add_custom_target(build_doxygen ALL
      COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/doxygen ${CMAKE_CURRENT_BINARY_DIR}/doxygen
      )
  endif(CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
endif(BUILD_DOX_DOC)

if(BUILD_DOX_DOC OR PREBUILT_DOX_DOC)
  install(CODE "
message(STATUS \"Installing: \$ENV{DESTDIR}${DOC_DIR}/doxygen tree\")
execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_BINARY_DIR}/doxygen \$ENV{DESTDIR}${DOC_DIR}/doxygen)
")
  if(DEFINED WWW_USER AND DEFINED WWW_HOST AND DEFINED WWW_DIR)
    add_custom_target(
      www-install-doxygen
      ssh ${WWW_USER}@${WWW_HOST} mkdir -p ${WWW_DIR}/htdocs/doxygen
      COMMAND rsync -avq ${CMAKE_CURRENT_BINARY_DIR}/doxygen/ ${WWW_USER}@${WWW_HOST}:${WWW_DIR}/htdocs/doxygen
      )
    add_dependencies(www-install-doxygen build_doxygen)
  endif(DEFINED WWW_USER AND DEFINED WWW_HOST AND DEFINED WWW_DIR)
endif(BUILD_DOX_DOC OR PREBUILT_DOX_DOC)
