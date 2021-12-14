# doc/docbook/src/CMakeLists.txt for PLplot
# Copyright (C) 2006 Andrew Ross
# Copyright (C) 2006-2018 Alan W. Irwin
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

if(ADD_SPECIAL_CONSISTENCY_CHECKING)
  # For this special case, must have access to a build of Hǎiliàng Wang's
  # "plplot-doc-check" application.  To build that tool (written in the Go language)
  # follow these simple directions (which worked fine on 2018-12-10):
  # Install golang package from your distro (I used the Debian Testing
  # version, golang version 2:1.10~5).
  # cd /home/software/golang_apps #(or wherever you like)
  # export GOPATH=$(pwd)
  # go get -d h12.io/go-plplot/c/plplot-doc-check
  # go build h12.io/go-plplot/c/plplot-doc-check
  ## The above creates the "plplot-doc-check" executable in $GOPATH
  ## Now put $GOPATH on your PATH and CMake should do the rest.
  find_program(API_XML_CONSISTENCY_CHECKER plplot-doc-check)
  if(API_XML_CONSISTENCY_CHECKER)
    add_custom_target(check_api_xml_consistency
      COMMAND ${CMAKE_COMMAND} -E echo "Check that doc/docbook/src/api.xml is consistent with include/plplot.h"
      COMMAND ${API_XML_CONSISTENCY_CHECKER} -castxml -header=${CMAKE_SOURCE_DIR}/include/plplot.h -incdir=${CMAKE_BINARY_DIR}/include -apixml=${CMAKE_SOURCE_DIR}/doc/docbook/src/api.xml
      )

    # This command no longer works because of all the huge api.xml and plplot.h changes
    # made for plplot-5.12.0.  So do not make it a dependency of check_all.
    # add_dependencies(check_all check_api_xml_consistency)

  else(API_XML_CONSISTENCY_CHECKER)
    message(STATUS "WARNING: cannot find the plplot-doc-check application so cannot check the consistency of api.xml with plplot.h")
  endif(API_XML_CONSISTENCY_CHECKER)
endif(ADD_SPECIAL_CONSISTENCY_CHECKING)

set(BINDIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin)
set(PLPLOTDOC plplotdoc)
if(NOT DEFINED HTML_EXT)
  set(HTML_EXT "html")
endif(NOT DEFINED HTML_EXT)
set(MANVOL "3plplot")

# For now, must set this (UTC) date by hand before each release.  This value affects
# only the documentation build so we set it here rather than in some more
# central place.
set(RELEASE_DATE "2019-06-01")

foreach(suffix info html print)
  if(suffix STREQUAL "print")
    set(inline_entities ${CMAKE_CURRENT_SOURCE_DIR}/inline-${suffix}_xsl.ent)
  else(suffix STREQUAL "print")
    set(inline_entities ${CMAKE_CURRENT_SOURCE_DIR}/inline-${suffix}.ent)
  endif(suffix STREQUAL "print")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/${PLPLOTDOC}.xml.in
    ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-${suffix}.xml
    )
endforeach(suffix info html print)

set(
  SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/ada.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/advanced.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/api.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/api-c.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/api-compat.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/api-fortran.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/api-internal.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/api-obsolete.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/bibliography.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/c.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/deploying.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/fortran.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/cplus.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/drivers.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/gui.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/intro.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/libraries.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/ocaml.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/python.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/simple.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/tcl.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/wish.xml
  ${CMAKE_CURRENT_SOURCE_DIR}/math.ent
  )

if(
    PREBUILT_DOC
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/static_built_files
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${INFO_MANIFEST}
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN_MANIFEST}
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${HTML_MANIFEST}
    )

  # Copy pre-built documentation (whose filenames are contained in the
  # files "static_built_files", ${INFO_MANIFEST}, ${MAN_MANIFEST}, and
  # ${HTML_MANIFEST}) in the source tree to the build tree just as if
  # it had been created there by the alternative BUILD_DOC=ON case.
  if(CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
    add_custom_target(copy_prebuiltdoc ALL)
  else(CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
      COMMAND cp
      `cat static_built_files ${INFO_MANIFEST} ${MAN_MANIFEST} ${HTML_MANIFEST}`
      ${CMAKE_BINARY_DIR}/doc/docbook/src
      WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/doc/docbook/src
      )
    add_custom_target(
      copy_prebuiltdoc ALL
      DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
      )
  endif(CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")

  if(ONSGMLS)
    set(VALIDATE_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/validate.out)

    # Custom target for validating xml file
    add_custom_target(validate
      COMMAND ${ONSGMLS} ${XML_DECL} ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-html.xml > ${VALIDATE_OUTPUT}
      VERBATIM
      )
    add_dependencies(validate copy_prebuiltdoc)
  endif(ONSGMLS)
else(
    PREBUILT_DOC
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/static_built_files
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${INFO_MANIFEST}
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN_MANIFEST}
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${HTML_MANIFEST}
    )

  # Include this file in what is copied back from build tree to source tree
  # for the custom top-level prebuild_dist target.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
    "static_built_files\n"
    )

  # The following files are defined as entities in the various forms
  # of the configured ${PLPLOTDOC}-*.xml files and are
  # used to help build the documentation.  Of course, these files are
  # required for validation as well.
  # N.B. if PERL_FOUND is false, then BUILD_DOC is false.

  # N.B. the DEPENDS below for plplot creates both a file
  # dependency (assuring the custom command is re-run whenever
  # libplplot is updated) and also a target dependency for
  # the corresponding docbook_plplot-symbols_txt custom target
  # that depends on the OUTPUT of this custom command.
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/plplot-symbols.txt
    COMMAND ${PERL_EXECUTABLE}
    ${BINDIR}/get-library-symbols.pl > ${CMAKE_CURRENT_BINARY_DIR}/plplot-symbols.txt $<TARGET_LINKER_FILE:PLPLOT::plplot> ${CMAKE_SOURCE_DIR}/include/plplot.h 3
    DEPENDS
    ${CMAKE_SOURCE_DIR}/include/plplot.h
    plplot
    )
  # For parallel builds to work must have all file dependency chains of
  # targets independent of each other.  Thus, since several targets
  # need plplot-symbols.txt, must create the target
  # docbook_plplot-symbols_txt and have the targets depend on that target
  # rather than have each target file depend directly on
  # ${CMAKE_CURRENT_BINARY_DIR}/plplot-symbols.txt.
  add_custom_target(
    docbook_plplot-symbols_txt
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/plplot-symbols.txt
    )
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
    "plplot-symbols.txt\n"
    )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/plplot-structs.txt
    COMMAND ${PERL_EXECUTABLE}
    ${BINDIR}/get-library-structs.pl > ${CMAKE_CURRENT_BINARY_DIR}/plplot-structs.txt ${CMAKE_SOURCE_DIR}/include/plplot.h
    DEPENDS ${CMAKE_SOURCE_DIR}/include/plplot.h
    )
  # For parallel builds to work must have all file dependency chains of
  # targets independent of each other.  Thus, since several targets
  # need plplot-structs.txt, must use target dependencies rather than
  # file dependencies to build this file.
  add_custom_target(
    docbook_plplot-structs_txt
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/plplot-structs.txt
    )
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
    "plplot-structs.txt\n"
    )

  # Custom command for validating xml file
  # This one different from alternative above used in PREBUILT_DOC case
  # because this one has dependencies which have to be built first.
  if(ONSGMLS)

    set(VALIDATE_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/validate.out)

    add_custom_target(validate
      COMMAND ${ONSGMLS} ${XML_DECL} ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-html.xml > ${VALIDATE_OUTPUT}
      VERBATIM
      )
    add_dependencies(validate
      docbook_plplot-symbols_txt
      docbook_plplot-structs_txt
      )
  else(ONSGMLS)
    add_custom_target(validate
      ${CMAKE_COMMAND} -E echo "Unable to validate xml file - missing dependencies"
      )
  endif(ONSGMLS)

  # N.B. if PERL_FOUND is false, then BUILD_DOC is false.
  if(BUILD_DOC)
    set(docbook_targets_LIST)
    if(BUILD_INFO)
      # Custom commands for building info version of documentation
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-clean.xml
	COMMAND ${PERL_EXECUTABLE}
	${BINDIR}/info-clean.pl ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-info.xml > ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-clean.xml
	DEPENDS
	${SOURCE_FILES}
	${BINDIR}/info-clean.pl
	${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-info.xml
	${CMAKE_CURRENT_SOURCE_DIR}/inline-info.ent
	)

      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.txml
	COMMAND ${CMAKE_COMMAND} -E echo "Note that no information is lost when \"too deep\" warning messages indicate"
	COMMAND ${CMAKE_COMMAND} -E echo "where flat organization has replaced hierarchical organization."
	COMMAND ${DB2X_XSLTPROC}
	-s texi -o ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.txml ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-clean.xml
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-clean.xml
	VERBATIM
	)

      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi
	COMMAND ${DB2X_TEXIXML} --encoding=utf-8 ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.txml
	COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi_sed
	COMMAND sed "s?The PLplot Plotting Library: (${PLPLOTDOC})?PLplot: (${PLPLOTDOC})?" < ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi > ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi_sed
	COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi
	COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi_sed ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.txml
	VERBATIM
	)

      set(INFO_TMPDIR ${CMAKE_CURRENT_BINARY_DIR}/info)

      file(MAKE_DIRECTORY ${INFO_TMPDIR})

      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${INFO_MANIFEST}
	COMMAND ${CMAKE_COMMAND}
	-E remove -f ${INFO_TMPDIR}/*
	COMMAND ${MAKEINFO}
	--force -o ${INFO_TMPDIR}/${PLPLOTDOC}.info ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi
	COMMAND ${CMAKE_COMMAND}
	-E chdir ${INFO_TMPDIR} ls |cat > ${CMAKE_CURRENT_BINARY_DIR}/${INFO_MANIFEST}
	COMMAND mv
	${INFO_TMPDIR}/* ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}.texi
	)

      add_custom_target(
	info ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${INFO_MANIFEST}
	)
      list(APPEND docbook_targets_LIST info)

      # These files used for more than one target.  Therefore must
      # use target dependencies rather than file dependencies to build
      # them.  Otherwise, parallel builds won't work.
      add_dependencies(info
	docbook_plplot-symbols_txt
	docbook_plplot-structs_txt
	)
    else(BUILD_INFO)
      add_custom_target(
	info ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${INFO_MANIFEST}
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/${INFO_MANIFEST}
	)
    endif(BUILD_INFO)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
      "${INFO_MANIFEST}\n"
      )

    if(BUILD_MAN)
      # Custom commands for building man version of documentation

      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MAN_MANIFEST}
	COMMAND ${PERL_EXECUTABLE}
	${CMAKE_CURRENT_BINARY_DIR}/../bin/api2man.pl ${PLPLOTDOC}-info.xml ${CMAKE_CURRENT_SOURCE_DIR}/api.xml ${MANVOL} > ${CMAKE_CURRENT_BINARY_DIR}/${MAN_MANIFEST}
	DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/../bin/api2man.pl.in
	${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-info.xml
	${CMAKE_CURRENT_SOURCE_DIR}/inline-info.ent
	${CMAKE_CURRENT_SOURCE_DIR}/api.xml
	)

      add_custom_target(
	man ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${MAN_MANIFEST}
	)
      list(APPEND docbook_targets_LIST man)
    else(BUILD_MAN)
      add_custom_target(
	man ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${MAN_MANIFEST}
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/${MAN_MANIFEST}
	)
    endif(BUILD_MAN)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
      "${MAN_MANIFEST}\n"
      )

    if(BUILD_HTML)
      # Custom commands for building html version of documentation
      set(STYLESHEET_HTML ${PLPLOTDOC}-html.xsl)
      set(STYLESHEET_HTML_IN ${STYLESHEET_HTML}.in)
      configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/${STYLESHEET_HTML_IN}
	${CMAKE_CURRENT_BINARY_DIR}/${STYLESHEET_HTML}
	@ONLY
	)

      configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/stylesheet.css.xsl.in
	${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
	)

      set(HTML_TMPDIR ${CMAKE_CURRENT_BINARY_DIR}/html)
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${HTML_MANIFEST}
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${HTML_TMPDIR}
	COMMAND ${XMLTO} -vv -x ${CMAKE_CURRENT_BINARY_DIR}/${STYLESHEET_HTML} -o ${HTML_TMPDIR} html ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-html.xml
	# Remove empty plplotdoc-html.proc file that is generated by xmlto,
        # but which we don't want in the html manifest.
	COMMAND ${CMAKE_COMMAND} -E remove ${HTML_TMPDIR}/plplotdoc-html.proc
	COMMAND ${CMAKE_COMMAND}
	-E chdir ${HTML_TMPDIR} ls |cat > ${CMAKE_CURRENT_BINARY_DIR}/${HTML_MANIFEST}
	COMMAND mv ${HTML_TMPDIR}/* ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${CMAKE_COMMAND} -E remove_directory ${HTML_TMPDIR}
	DEPENDS
	${SOURCE_FILES}
	${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-html.xml
	${CMAKE_CURRENT_SOURCE_DIR}/inline-html.ent
	${CMAKE_CURRENT_BINARY_DIR}/${STYLESHEET_HTML}
	)
      add_custom_target(
	html ALL
	DEPENDS
	${CMAKE_CURRENT_BINARY_DIR}/${HTML_MANIFEST}
	)
      list(APPEND docbook_targets_LIST html)
      # These files used for more than one target.  Therefore must
      # use target dependencies rather than file dependencies to build
      # them.  Otherwise, parallel builds won't work.
      add_dependencies(html
	docbook_plplot-symbols_txt
	docbook_plplot-structs_txt
	)
    else(BUILD_HTML)
      add_custom_target(
	html ALL
	COMMAND ${CMAKE_COMMAND} -E remove ${CMAKE_CURRENT_BINARY_DIR}/${HTML_MANIFEST}
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_BINARY_DIR}/${HTML_MANIFEST}
	)
    endif(BUILD_HTML)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
      "${HTML_MANIFEST}\n"
      )

    if(BUILD_PRINT)
      # Custom commands for building print version of documentation
      set(STYLESHEET_PRINT ${PLPLOTDOC}-print.xsl)
      set(STYLESHEET_PRINT_IN ${STYLESHEET_PRINT}.in)
      configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/${STYLESHEET_PRINT_IN}
	${CMAKE_CURRENT_BINARY_DIR}/${STYLESHEET_PRINT}
	@ONLY
	)

      if(DOCBOOK_DBLATEX_BACKEND)
	add_custom_command(
	  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.pdf
	  COMMAND ${DBLATEX} --xsl-user=${CMAKE_CURRENT_SOURCE_DIR}/dblatex_stylesheet.xsl --backend=xetex --type=pdf -o ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.pdf ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-print.xml
	  DEPENDS
	  ${SOURCE_FILES}
	  ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-print.xml
	  ${CMAKE_CURRENT_SOURCE_DIR}/inline-print_xsl.ent
	  ${CMAKE_CURRENT_SOURCE_DIR}/dblatex_stylesheet.xsl
          VERBATIM
	  )
	add_custom_target(
	  pdf_target ALL
	  DEPENDS
	  ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.pdf
	  )
	# These files used for more than one target.  Therefore must
	# use target dependencies rather than file dependencies to build
	# them.  Otherwise, parallel builds won't work.
	add_dependencies(pdf_target
	  docbook_plplot-symbols_txt
	  docbook_plplot-structs_txt
	  )

	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
	  "${BASE}.pdf\n"
	  )

	add_custom_target(
	  print ALL
	  )
	list(APPEND docbook_targets_LIST print)
	add_dependencies(print pdf_target)

	# At this stage could generate PostScript result from PDF
	# using pdf2ps, but do not do that because the result is big
	# (50 (!) times larger than corresponding pdf), takes a long
	# time to generate, and is really ugly looking.
      else(DOCBOOK_DBLATEX_BACKEND)
	add_custom_command(
	  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.ps.gz
	  # xmlto --with-fop currently (at least on Debian) craps out
          # if DESTDIR is specified so make sure it is NOT specified.
	  COMMAND env DESTDIR= ${XMLTO} -vv --with-fop -x ${CMAKE_CURRENT_BINARY_DIR}/${STYLESHEET_PRINT} ps ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-print.xml
	  COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/plplotdoc-print.ps ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.ps
	  COMMAND ${GZIP} -f ${BASE}.ps
	  DEPENDS
	  ${SOURCE_FILES}
	  ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-print.xml
	  ${CMAKE_CURRENT_SOURCE_DIR}/inline-print_xsl.ent
	  VERBATIM
	  )
	add_custom_target(
	  ps_target ALL
	  DEPENDS
	  ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.ps.gz
	  )
	# These files used for more than one target.  Therefore must
	# use target dependencies rather than file dependencies to build
	# them.  Otherwise, parallel builds won't work.
	add_dependencies(ps_target
	  docbook_plplot-symbols_txt
	  docbook_plplot-structs_txt
	  )
	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
	  "${BASE}.ps.gz\n"
	  )

	add_custom_command(
	  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.pdf
	  # xmlto --with-fop currently (at least on Debian) craps out
          # if DESTDIR is specified so make sure it is NOT specified.
	  COMMAND env DESTDIR= ${XMLTO} -vv --with-fop -x ${CMAKE_CURRENT_BINARY_DIR}/${STYLESHEET_PRINT} pdf ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-print.xml
	  COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/plplotdoc-print.pdf ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.pdf
	  DEPENDS
	  ${SOURCE_FILES}
	  ${CMAKE_CURRENT_BINARY_DIR}/${PLPLOTDOC}-print.xml
	  ${CMAKE_CURRENT_SOURCE_DIR}/inline-print_xsl.ent
          VERBATIM
	  )
	add_custom_target(
	  pdf_target ALL
	  DEPENDS
	  ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.pdf
	  )
	# These files used for more than one target.  Therefore must
	# use target dependencies rather than file dependencies to build
	# them.  Otherwise, parallel builds won't work.
	add_dependencies(pdf_target
	  docbook_plplot-symbols_txt
	  docbook_plplot-structs_txt
	  )

	file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/static_built_files
	  "${BASE}.pdf\n"
	  )

	add_custom_target(
	  print ALL
	  )
	list(APPEND docbook_targets_LIST print)
	add_dependencies(print ps_target pdf_target)
      endif(DOCBOOK_DBLATEX_BACKEND)

    endif(BUILD_PRINT)

    if(docbook_targets_LIST)
      add_custom_target(build_docbook_documentation ALL)
      add_dependencies(build_docbook_documentation ${docbook_targets_LIST})
    endif(docbook_targets_LIST)
  endif(BUILD_DOC)
endif(
  PREBUILT_DOC
  AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/static_built_files
  AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${INFO_MANIFEST}
  AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN_MANIFEST}
  AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${HTML_MANIFEST}
  )

########### install files ###############
# Where appropriate, use contents of manifest files rather than
# file(GLOB... for best consistency.

if(BUILD_INFO OR PREBUILT_DOC)
  install(CODE "
# Suppress empty element warning.
cmake_policy(SET CMP0007 NEW)
# Suppress warning about setting the above policy!
cmake_policy(SET CMP0011 NEW)

file(READ ${CMAKE_CURRENT_BINARY_DIR}/${INFO_MANIFEST} INFO_FILES)
# Convert to list with trailing empty element
string(REGEX REPLACE \"\n\" \";\" INFO_FILES \"\${INFO_FILES}\")
# Delete trailing empty element
list(REMOVE_ITEM INFO_FILES \"\")
foreach(file \${INFO_FILES})
  # INSTALL_DESTINATION signature automatically honors DESTDIR.
  file(INSTALL \"${CMAKE_CURRENT_BINARY_DIR}/\${file}\" DESTINATION ${INFO_DIR})
endforeach (file \"\${INFO_FILES}\")
"
    )
endif(BUILD_INFO OR PREBUILT_DOC)

if(BUILD_MAN OR PREBUILT_DOC)
  install(CODE "
# Suppress empty element warning.
cmake_policy(SET CMP0007 NEW)
# Suppress warning about setting the above policy!
cmake_policy(SET CMP0011 NEW)
file(READ ${CMAKE_CURRENT_BINARY_DIR}/${MAN_MANIFEST} MAN_FILES)
# Convert to list with trailing empty element
string(REGEX REPLACE \"\n\" \";\" MAN_FILES \"\${MAN_FILES}\")
# Delete trailing empty element
list(REMOVE_ITEM MAN_FILES \"\")
foreach(file \${MAN_FILES})
  # INSTALL_DESTINATION signature automatically honors DESTDIR.
  file(INSTALL \"${CMAKE_CURRENT_BINARY_DIR}/\${file}\" DESTINATION ${MAN_DIR}/man3)
endforeach (file \"\${MAN_FILES}\")
"
    )
endif(BUILD_MAN OR PREBUILT_DOC)

if(BUILD_HTML OR PREBUILT_DOC)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/stylesheet.css.xsl.in
    ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${HTML_MANIFEST} ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css DESTINATION ${DOC_DIR}/html )
  install(CODE "
# Suppress empty element warning.
cmake_policy(SET CMP0007 NEW)
# Suppress warning about setting the above policy!
cmake_policy(SET CMP0011 NEW)
file(READ ${CMAKE_CURRENT_BINARY_DIR}/${HTML_MANIFEST} HTML_FILES)
# Convert to list with trailing empty element
string(REGEX REPLACE \"\n\" \";\" HTML_FILES \"\${HTML_FILES}\")
# Delete trailing empty element
list(REMOVE_ITEM HTML_FILES \"\")
foreach(file \${HTML_FILES})
  # INSTALL_DESTINATION signature automatically honors DESTDIR.
  file(INSTALL \"${CMAKE_CURRENT_BINARY_DIR}/\${file}\" DESTINATION ${DOC_DIR}/html)
endforeach (file \"\${HTML_FILES}\")
"
    )
endif(BUILD_HTML OR PREBUILT_DOC)

if(BUILD_PRINT OR PREBUILT_DOC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${BASE}.pdf DESTINATION ${DOC_DIR})

endif(BUILD_PRINT OR PREBUILT_DOC)

# Everything below here has to do with the custom target www-install
if((BUILD_DOC OR PREBUILT_DOC) AND (WWW_USER AND WWW_GROUP AND WWW_HOST AND WWW_DIR))

  set(TARBALL tarball.tgz)
  set(HTML_TARBALL ${BASE_HTML}.tar.gz)
  set(INFO_TARBALL ${BASE_INFO}.tar.gz)
  set(MAN_TARBALL ${BASE_MAN}.tar.gz)

  set(WWW_USERAT ${WWW_USER}@)
  set(RCP_PREFIX ${WWW_USERAT}${WWW_HOST}:)
  set(RSH_COMMAND ${RSH} ${WWW_USERAT}${WWW_HOST})

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${INFO_TARBALL}
    COMMAND rm -rf ${BASE_INFO}
    COMMAND mkdir ${BASE_INFO}
    COMMAND cp `cat ${INFO_MANIFEST}` ${BASE_INFO}
    COMMAND tar cfz ${INFO_TARBALL} ${BASE_INFO}
    COMMAND rm -rf ${BASE_INFO}
    DEPENDS ${INFO_MANIFEST}
    )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${MAN_TARBALL}
    COMMAND rm -rf ${BASE_MAN}
    COMMAND mkdir ${BASE_MAN}
    COMMAND cp `cat ${MAN_MANIFEST}` ${BASE_MAN}
    COMMAND tar cfz ${MAN_TARBALL} ${BASE_MAN}
    COMMAND rm -rf ${BASE_MAN}
    DEPENDS ${MAN_MANIFEST}
    )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${HTML_TARBALL}
    COMMAND rm -rf ${BASE_HTML}
    COMMAND mkdir ${BASE_HTML}
    COMMAND cp `cat ${HTML_MANIFEST}` ${BASE_HTML}
    COMMAND cp ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css ${BASE_HTML}
    COMMAND tar cfz ${HTML_TARBALL} ${BASE_HTML}
    COMMAND rm -rf ${BASE_HTML}
    DEPENDS
    ${HTML_MANIFEST}
    ${CMAKE_CURRENT_BINARY_DIR}/stylesheet.css
    )

  set(tarball_list
    ${INFO_TARBALL}
    ${MAN_TARBALL}
    ${HTML_TARBALL}
    )

  set(tarball_depends_list
    ${CMAKE_CURRENT_BINARY_DIR}/${INFO_TARBALL}
    ${CMAKE_CURRENT_BINARY_DIR}/${MAN_TARBALL}
    ${CMAKE_CURRENT_BINARY_DIR}/${HTML_TARBALL}
    )

  if(BUILD_PRINT)
    list(APPEND tarball_list
      ${BASE}.pdf
      )
    list(APPEND tarball_depends_list
      ${BASE}.pdf
      )
  endif(BUILD_PRINT)

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${TARBALL}
    COMMAND tar cfz ${TARBALL}
    ${tarball_list}
    -C ${CMAKE_CURRENT_SOURCE_DIR}/.. COPYING
    DEPENDS
    ${tarball_depends_list}
    ${CMAKE_CURRENT_SOURCE_DIR}/../COPYING
    )

  add_custom_target(www-uninstall
    COMMAND ${RSH_COMMAND} rm -rf ${WWW_DIR}/htdocs/docbook-manual
    )

  add_custom_target(
    www-install
    COMMAND ${RSH_COMMAND} mkdir -p ${WWW_DIR}/htdocs/docbook-manual
    COMMAND ${RCP} ${CMAKE_CURRENT_BINARY_DIR}/${TARBALL} ${RCP_PREFIX}${WWW_DIR}/htdocs/docbook-manual
    COMMAND ${RSH_COMMAND} tar xfz ${WWW_DIR}/htdocs/docbook-manual/${TARBALL} -C ${WWW_DIR}/htdocs/docbook-manual
    COMMAND ${RSH_COMMAND} tar xfz ${WWW_DIR}/htdocs/docbook-manual/${HTML_TARBALL} -C ${WWW_DIR}/htdocs/docbook-manual
    COMMAND ${RSH_COMMAND} chgrp -R ${WWW_GROUP} ${WWW_DIR}/htdocs/docbook-manual
    COMMAND ${RSH_COMMAND} chmod -R g=u ${WWW_DIR}/htdocs/docbook-manual
    COMMAND ${RSH_COMMAND} rm -f ${WWW_DIR}/htdocs/docbook-manual/${TARBALL}
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/${TARBALL}
    )

  if(
      PREBUILT_DOC
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/static_built_files
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${INFO_MANIFEST}
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN_MANIFEST}
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${HTML_MANIFEST}
      )
    add_dependencies(www-install copy_prebuiltdoc www-uninstall)
  else(
      PREBUILT_DOC
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/static_built_files
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${INFO_MANIFEST}
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN_MANIFEST}
      AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${HTML_MANIFEST}
      )
    add_dependencies(www-install www-uninstall)
  endif(
    PREBUILT_DOC
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/static_built_files
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${INFO_MANIFEST}
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${MAN_MANIFEST}
    AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${HTML_MANIFEST}
    )
endif((BUILD_DOC OR PREBUILT_DOC) AND (WWW_USER AND WWW_GROUP AND WWW_HOST AND WWW_DIR))
