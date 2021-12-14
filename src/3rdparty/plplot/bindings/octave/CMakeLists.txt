# bindings/octave/CMakeLists.txt for PLplot
# Copyright (C) 2006-2019 Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Build and install octave interface to PLplot.
if(ENABLE_octave)

  add_subdirectory(PLplot)
  add_subdirectory(misc)

  # Install renamed documentation files.
  set(docfiles
    BUGS
    FGA
    INSTALL
    README
    ToDo
    USAGE
    )
  foreach(doc ${docfiles})
    install(FILES ${doc} DESTINATION ${DOC_DIR} RENAME ${doc}.octave)
  endforeach(doc ${docfiles})

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/octave_version.pl.in
    ${CMAKE_CURRENT_BINARY_DIR}/octave_version.pl
    @ONLY
    )
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/octaverc.in
    ${CMAKE_CURRENT_BINARY_DIR}/.octaverc
    @ONLY
    )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plplot_stub.m.in
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_stub.m
    @ONLY
    )

  set(octave_interface_INCLUDE_PATHS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/qsastime
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/lib/qsastime
    ${CMAKE_CURRENT_BINARY_DIR}
    ${OCTAVE_INCLUDE_PATH}
    ${CMAKE_SOURCE_DIR}/bindings/swig-support
    )
  include_directories(${octave_interface_INCLUDE_PATHS})
  # The -module option only used to workaround some bug in CMake swig
  # support for Octave which does not allow module name prefixes such
  # as ${WRITEABLE_TARGET} without specifying -module explicitly this
  # way.
  set(CMAKE_SWIG_FLAGS -module ${WRITEABLE_TARGET}plplot_octave -DSWIG_OCTAVE)
  set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

  set_source_files_properties(plplot_octave.i
    PROPERTIES
    SWIG_MODULE_NAME ${WRITEABLE_TARGET}plplot_octave
    CPLUSPLUS ON
    )

  set(SWIG_MODULE_plplot_octave_EXTRA_DEPS
    ${CMAKE_SOURCE_DIR}/bindings/swig-support/swig_documentation.i
    ${CMAKE_SOURCE_DIR}/bindings/swig-support/plplotcapi.i
    )

  # Configure build of swig-generated C++ wrapper.
  set(octave_tll_list
    PLPLOT::plplot
    "${OCTAVE_LIBRARIES}"
    "${OCTINTERP_LIBRARIES}"
    )

  # plplot_octave depends on the plplot library.
  set(LIB_INSTALL_RPATH ${LIB_DIR})
  # (Reasonable) assumption here is that OCTAVE_LIBRARIES and
  # OCTINTERP_LIBRARIES have the same path.
  # plplot_octave depends on the octave libraries.
  get_filename_component(OCTAVE_INSTALL_RPATH "${OCTAVE_LIBRARIES}" PATH)
  list(APPEND LIB_INSTALL_RPATH ${OCTAVE_INSTALL_RPATH})

  # Have to be specific about install permissions (likely because of the
  # ".oct" suffix set by UseSWIG.cmake and also demanded by octave
  # itself).  Use SO_PERMISSIONS to be consistent with permissions
  # used for other shared objects.
  configure_library_build(plplot_octave octave plplot_octave.i "${octave_tll_list}" "${LIB_INSTALL_RPATH}" ${OCTAVE_OCT_DIR} "${SO_PERMISSIONS}")

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_stub.m
    DESTINATION ${PLPLOT_OCTAVE_DIR}
    )

endif(ENABLE_octave)
