# www/CMakeLists.txt for PLplot
###
### Process this file with cmake to produce Makefile
###
# Copyright (C) 2008 Alan W. Irwin
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

# Configure documentation file sizes.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/documentation.php.in
  ${CMAKE_CURRENT_BINARY_DIR}/documentation.php @ONLY
  )

# Configure style files
add_subdirectory(css)

# Set up Makefile rules to generate announcements and validate their DocBook-XML source.
add_subdirectory(announce)

# Copy base website files to a staging area in the build tree.

set(staging_FILES)

# Create the staging directory tree at cmake time.
file(MAKE_DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/staging/
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/css
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/img
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/js
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/announce
  )

# Note documentation.php configured as above.
set(toplevel_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/corefunctions.php
  ${CMAKE_CURRENT_SOURCE_DIR}/credits.php
  ${CMAKE_CURRENT_SOURCE_DIR}/development.php
  ${CMAKE_CURRENT_SOURCE_DIR}/downloads.php
  ${CMAKE_CURRENT_SOURCE_DIR}/examples.php
  ${CMAKE_CURRENT_SOURCE_DIR}/index.php
  ${CMAKE_CURRENT_SOURCE_DIR}/simplepie.inc
  ${CMAKE_CURRENT_BINARY_DIR}/documentation.php
  )

foreach(_file ${toplevel_FILES})
  get_filename_component(_filename ${_file} NAME)
  add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/${_filename}
  # Use cp -p to preserve dates/minimize rsync activity.
  COMMAND cp -p
  ${_file}
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/${_filename}
  DEPENDS ${_file}
  )
  list(APPEND staging_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/${_filename}
    )
endforeach(_file ${toplevel_FILES})

# Note style files configure by CMake above in the css (build) directory.
set(css_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/css/closelabel.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/css/loading.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/css/nextlabel.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/css/prevlabel.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/css/slimbox.css
  ${CMAKE_CURRENT_BINARY_DIR}/css/style_liquid.css
  ${CMAKE_CURRENT_BINARY_DIR}/css/style_static.css
  )

foreach(_file ${css_FILES})
  get_filename_component(_filename ${_file} NAME)
  add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/css/${_filename}
  # Use cp -p to preserve dates/minimize rsync activity.
  COMMAND cp -p
  ${_file}
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/css/${_filename}
  DEPENDS ${_file}
  )
  list(APPEND staging_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/css/${_filename}
    )
endforeach(_file ${css_FILES})

set(img_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/img/arrow.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/img/bg.jpg
  ${CMAKE_CURRENT_SOURCE_DIR}/img/content_bg.jpg
  ${CMAKE_CURRENT_SOURCE_DIR}/img/drop_shadow.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/img/header.jpg
  ${CMAKE_CURRENT_SOURCE_DIR}/img/menu_bg.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/img/menu_hover_bg.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/img/menu_selected_bg.gif
  ${CMAKE_CURRENT_SOURCE_DIR}/img/plplot_logo.jpg
  )

foreach(_file ${img_FILES})
  get_filename_component(_filename ${_file} NAME)
  add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/img/${_filename}
  # Use cp -p to preserve dates/minimize rsync activity.
  COMMAND cp -p
  ${_file}
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/img/${_filename}
  DEPENDS ${_file}
  )
  list(APPEND staging_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/img/${_filename}
    )
endforeach(_file ${img_FILES})

set(js_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/js/slimbox.js
  )

foreach(_file ${js_FILES})
  get_filename_component(_filename ${_file} NAME)
  add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/js/${_filename}
  # Use cp -p to preserve dates/minimize rsync activity.
  COMMAND cp -p
  ${_file}
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/js/${_filename}
  DEPENDS ${_file}
  )
  list(APPEND staging_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/js/${_filename}
    )
endforeach(_file ${js_FILES})

# The end of this list needs maintenance for each new release announcement
# that is generated (assuming we start making formal release announcements
# again).
set(announce_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/announce/ChangeLog-5.2.1-5.3.0
  ${CMAKE_CURRENT_SOURCE_DIR}/announce/ChangeLog-5.3.0-5.3.1
  ${CMAKE_CURRENT_SOURCE_DIR}/announce/ChangeLog-5.3.1-5.3.2
  ${CMAKE_CURRENT_SOURCE_DIR}/announce/announce-plplot-5.1.0.html
  ${CMAKE_CURRENT_SOURCE_DIR}/announce/announce-plplot-5.2.1.html
  ${CMAKE_CURRENT_SOURCE_DIR}/announce/announce.css
  ${CMAKE_CURRENT_BINARY_DIR}/announce/announce-plplot-5.3.0.xhtml
  ${CMAKE_CURRENT_BINARY_DIR}/announce/announce-plplot-5.3.0.txt
  ${CMAKE_CURRENT_BINARY_DIR}/announce/announce-plplot-5.3.1.xhtml
  ${CMAKE_CURRENT_BINARY_DIR}/announce/announce-plplot-5.3.1.txt
  )

# This has to be handled differently because some of these files created
# with the announce subdirectory generate_announcements target.
set(staging_announce_FILES)
foreach(_file ${announce_FILES})
  get_filename_component(_filename ${_file} NAME)
  add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/announce/${_filename}
  # Use cp -p to preserve dates/minimize rsync activity.
  COMMAND cp -p
  ${_file}
  ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/announce/${_filename}
  DEPENDS ${_file}
  )
  list(APPEND staging_announce_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/announce/${_filename}
    )
endforeach(_file ${announce_FILES})

add_custom_target(
  create_staging_announce
  DEPENDS ${staging_announce_FILES}
  )
add_dependencies(create_staging_announce generate_announcements)

add_custom_target(
  create_staging
  DEPENDS ${staging_FILES}
  )
add_dependencies(create_staging create_staging_announce)

if(DEFINED WWW_USER AND DEFINED WWW_HOST AND DEFINED WWW_DIR)
  add_custom_target(
    www-install-base
    ssh ${WWW_USER}@${WWW_HOST} mkdir -p ${WWW_DIR}/htdocs
    COMMAND rsync -avq ${CMAKE_CURRENT_BINARY_DIR}/staging/htdocs/ ${WWW_USER}@${WWW_HOST}:${WWW_DIR}/htdocs
    )
  add_dependencies(www-install-base create_staging)
endif(DEFINED WWW_USER AND DEFINED WWW_HOST AND DEFINED WWW_DIR)
