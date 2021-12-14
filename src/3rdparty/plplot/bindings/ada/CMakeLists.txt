# bindings/ada/CMakeLists.txt
# Copyright (C) 2007-2019 Alan W. Irwin
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

if(ENABLE_ada)

  # Specification files containing, e.g., variables, types, and subprogramme (subprocedure) declarations.
  set(SPEC_SOURCE_LIST
    plplot.ads
    plplot_standard.ads
    plplot_thin.ads
    plplot_traditional.ads
    plplot_auxiliary.ads
    )

  # Body files containing code for the subprogrammes.  Only body files need to be compiled.  The
  # corresponding specification files will be found automatically.
  set(BODY_SOURCE_LIST
    plplot_standard.adb
    plplot_thin.adb
    plplot_traditional.adb
    plplot_auxiliary.adb
    )

  # These are Ada library information files built by gnat.  I am not
  # sure whether the name suffixes correspond to the *.adb or *.ads files
  # above or the union of them.  In any case, if any of the names change
  # above, then this list will probably have to be changed as well.)
  # N.B. the absolute location prefix of these files may have to be changed
  # in the future since this is a CMake internal.
  set(ALI_PREFIX
    ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/plplotada.dir
    )
  set(ALI_LIST
    ${ALI_PREFIX}/plplot_standard.ali
    ${ALI_PREFIX}/plplot_thin.ali
    ${ALI_PREFIX}/plplot_traditional.ali
    ${ALI_PREFIX}/plplot_auxiliary.ali
    )

  # I am pretty sure this is a workaround for a MinGW gnatmake bug
  # since the installation of these files is not needed for Linux.
  # However, without this workaround the standard examples build on
  # MinGW/MSYS platforms without problems (so gnatmake is happy if the
  # *.o files are not installed which is another indication we are
  # dealing with a bug here), but at run-time the ada examples
  # immediately return with a return code of 3.
  if(MINGW)
    list(APPEND ALI_LIST
      ${ALI_PREFIX}/plplot_standard.o
      ${ALI_PREFIX}/plplot_thin.o
      ${ALI_PREFIX}/plplot_traditional.o
      ${ALI_PREFIX}/plplot_auxiliary.o
      )
  endif(MINGW)

  set(plplotada_SPEC_SRCS)
  foreach(SPEC_FILE ${SPEC_SOURCE_LIST})
    list(APPEND plplotada_SPEC_SRCS
      ${CMAKE_CURRENT_SOURCE_DIR}/${SPEC_FILE}
      )
  endforeach(SPEC_FILE ${SPEC_SOURCE_LIST})

  set(plplotada_BODY_SRCS)
  foreach(BODY_FILE ${BODY_SOURCE_LIST})
    list(APPEND plplotada_BODY_SRCS
      ${CMAKE_CURRENT_SOURCE_DIR}/${BODY_FILE}
      )
  endforeach(BODY_FILE ${BODY_SOURCE_LIST})

  # plplotada depends on the plplot library.
  set(LIB_INSTALL_RPATH ${LIB_DIR})

  # target_link_libraries used in special way below so avoid using it inside configure_library_build.
  configure_library_build(plplotada "" "${plplotada_BODY_SRCS}" "" "${LIB_INSTALL_RPATH}")

  # Work around an issue in our CMake Ada language
  # support for MinGW/Windows.  FIXME.  This issue should be fixed
  # at the Ada language support level and not worked around here.

  # N.B. On the Cygwin platform the copy command below would fail
  # because on that platform a very different library naming
  # convention is used. Furthermore, it appears the naming convention
  # for the plplotada library on Cygwin is consistent, i.e.,
  # dll/cygplplot-13.dll and dll/cygplplotada-2.dll for the plplot and
  # plplotada libraries (presumably because UNIX is true on Cygwin).
  # Therefore, there is no need, in any case, for any
  # naming-convention workarounds for that platform.  Thus,
  # specifically exclude Cygwin below.

  if(USE_DLL_SUBDIRECTORY AND NOT CYGWIN)
    add_custom_command(
      TARGET plplotada POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy
      libplplotada.dll
      libplplotada.dll.a
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/dll
      VERBATIM
      )
  endif(USE_DLL_SUBDIRECTORY AND NOT CYGWIN)

  # Link to GNAT_LIB to avoid underlinking the plplotada library (which causes
  # link errors on at least the Cygwin platform), but use
  # the PRIVATE keyword (on all platforms) to avoid overlinking Ada applications
  # that link to plplotada.
  target_link_libraries(${WRITEABLE_TARGET}plplotada PRIVATE ${GNAT_LIB})

  # N.B. nm evidence shows that examples that use
  # callbacks (e.g., plfill in x15a.adb) have unresolved references
  # to c_plfill, etc. that require a public link to plplot
  # regardless of how NON_TRANSITIVE is set.
  target_link_libraries(${WRITEABLE_TARGET}plplotada PUBLIC PLPLOT::plplot)

  install(FILES ${plplotada_SPEC_SRCS} ${plplotada_BODY_SRCS}
    DESTINATION ${ADA_INCLUDE_DIR}
    )

  # Permissions of *.ali files in accordance with
  # http://people.debian.org/~lbrenta/debian-ada-policy.html
  install(FILES ${ALI_LIST}
    DESTINATION ${ADA_LIB_DIR}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
    )

  # Add generated .ali files to the list of additional files to be
  # removed by make clean
  SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${ALI_LIST}")

  # Configure pkg-config *.pc file corresponding to libplplotada
  # See remarks above concerning why plplot has to be public.
  pkg_config_file("ada" "Ada" " Ada binding" "plplotada" "" "${GNAT_LIB}" "-lplplot")

endif(ENABLE_ada)
