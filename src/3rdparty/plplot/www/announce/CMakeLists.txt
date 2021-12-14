# www/announce/CMakeLists.txt for PLplot
###
### Process this file with cmake to produce Makefile
###
# Copyright (C) 2008-2018 Alan W. Irwin
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

# Create rules to generate announcments that are in DocBook-XML form.

set(URL http://plplot.org)
set(XML_DCL /usr/share/sgml/declaration/xml.dcl)
set(PWD "${CMAKE_BINARY_DIR}/www")


set(generated_RELEASE_LIST
  5.3.0
  5.3.1
  )

set(generated_announcements_target_LIST)
set(validation_target_LIST)
foreach(generated_RELEASE ${generated_RELEASE_LIST})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.html
    COMMAND xmlto -m ${CMAKE_CURRENT_SOURCE_DIR}/html.xsl xhtml-nochunks ${CMAKE_CURRENT_SOURCE_DIR}/announce-plplot-${generated_RELEASE}.xml
    DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/html.xsl
    ${CMAKE_CURRENT_SOURCE_DIR}/announce-plplot-${generated_RELEASE}.xml
    )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.xhtml
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.html
    ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.xhtml
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.html
    )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.txt
    COMMAND lynx -force_html -dump -width=80 ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.xhtml | sed \"s,file://localhost${PWD},${URL},\" > ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.txt
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.xhtml
    )
  list(
    APPEND generated_announcements_target_LIST
    ${CMAKE_CURRENT_BINARY_DIR}/announce-plplot-${generated_RELEASE}.txt
    )

  add_custom_target(
    validate_announcement-${generated_RELEASE}
    COMMAND onsgmls ${XML_DCL} ${CMAKE_CURRENT_SOURCE_DIR}/announce-plplot-${generated_RELEASE}.xml > /dev/null
    )
  list(
    APPEND
    validation_target_LIST
    validate_announcement-${generated_RELEASE}
    )

endforeach(generated_RELEASE ${generated_RELEASE_LIST})

add_custom_target(
  generate_announcements
  DEPENDS ${generated_announcements_target_LIST}
  )

add_custom_target(validate_announcements)
add_dependencies(validate_announcements ${validation_target_LIST})
