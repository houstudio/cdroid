# lib/csa/CMakeLists.txt for PLplot
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

if(WITH_CSA)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/csadll.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/csadll.h
    )

  set(csa_LIB_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/csa.h
    ${CMAKE_CURRENT_BINARY_DIR}/csadll.h
    )

  include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  )

  set(csirocsa_LIB_SRCS
    csa.c
    )

  if(CMAKE_C_COMPILER MATCHES ".*gcc.*")
    # Certain versions of the gcc compiler (e.g., gcc 7.3.0-21 from
    # Debian Buster) produce libcsirocsa code that segfaults at run
    # time if the -finline-functions compiler option is used directly
    # or as the result of the -O3 option.  So for gcc append
    # the -fno-inline-functions option.
    message(STATUS "For csa.c use gcc -fno-inline-functions option to avoid segfaults")
    set_source_files_properties(csa.c PROPERTIES COMPILE_FLAGS "-fno-inline-functions")
  endif(CMAKE_C_COMPILER MATCHES ".*gcc.*")

  configure_library_build(csirocsa "" "${csirocsa_LIB_SRCS}" "${MATH_LIB}" "")

  set(csa_DOCFILES
    README
    )
  install(FILES README DESTINATION ${DOC_DIR} RENAME README.csa)
  install(FILES ${csa_LIB_HDRS} DESTINATION ${INCLUDE_DIR})

endif(WITH_CSA)
