# bindings/c++/CMakeLists.txt
# Copyright (C) 2006 Andrew Ross
# Copyright (C) 2015-2019 Alan W. Irwin
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

if(ENABLE_cxx)

  set(plplotcxx_LIB_SRCS
    plstream.cc
    )

  set(plplotcxx_INSTALLED_HEADERS
    plstream.h
    )

  # Set the include path
  include_directories(
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/include
    )

  # plplotcxx depends on the plplot library.
  set(LIB_INSTALL_RPATH ${LIB_DIR})

  configure_library_build(plplotcxx "" "${plplotcxx_LIB_SRCS}" PLPLOT::plplot "${LIB_INSTALL_RPATH}")

  install(FILES
    ${plplotcxx_INSTALLED_HEADERS}
    DESTINATION ${INCLUDE_DIR}
    )

  # Configure pkg-config *.pc file corresponding to libplplotcxx
  pkg_config_file("c++" "C++" " C++ binding" "plplotcxx" "" "-lplplot")

endif(ENABLE_cxx)
