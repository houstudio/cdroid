# lib/nn/CMakeLists.txt for PLplot
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

if(PL_HAVE_QHULL)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/nndll.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/nndll.h
    )

  set(nn_LIB_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/nn.h
    ${CMAKE_CURRENT_BINARY_DIR}/nndll.h
    )

  include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )

  set(csironn_LIB_SRCS
    delaunay.c
    hash.c
    istack.c
    lpi.c
    nnai.c
    nnpi.c
    nncommon.c
    )

  if(HAS_LIBQHULL_INCLUDE)
    set_source_files_properties(delaunay.c
      PROPERTIES COMPILE_DEFINITIONS HAS_LIBQHULL_INCLUDE
      )
  endif(HAS_LIBQHULL_INCLUDE)

  set_source_files_properties(
    ${csironn_LIB_SRCS}
    PROPERTIES
    COMPILE_FLAGS "-I${QHULL_INCLUDE_DIRS}"
    )

  set(csironn_LINK_LIBRARIES ${QHULL_LIBRARIES})
  if(MATH_LIB)
    list(APPEND csironn_LINK_LIBRARIES ${MATH_LIB})
  endif(MATH_LIB)

  set(LIB_INSTALL_RPATH ${QHULL_RPATH})
  configure_library_build(csironn "" "${csironn_LIB_SRCS}" "${csironn_LINK_LIBRARIES}" "${QHULL_RPATH}")

  set(nn_DOCFILES
    README
    )
  install(FILES README DESTINATION ${DOC_DIR} RENAME README.nn)

  install(FILES ${nn_LIB_HDRS} DESTINATION ${INCLUDE_DIR})

endif(PL_HAVE_QHULL)
