# cmake/modules/docbook.cmake
#
# Copyright (C) 2006  Andrew Ross
# Copyright (C) 2006-2018  Alan W. Irwin
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
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Configuration for the building of DocBook documentation

option(BUILD_DOC "Enable build of DocBook documentation" OFF)

if(NOT PERL_FOUND)
  set(
  BUILD_DOC OFF CACHE INTERNAL
  "Enable build of DocBook documentation"
  )
endif(NOT PERL_FOUND)

option(PREBUILT_DOC "Assume documentation is already built and present in doc/docbooks/src, ready to be installed. This option is useful for package maintainers" OFF)

if(BUILD_DOC AND PREBUILT_DOC)
  message(FATAL_ERROR "Options BUILD_DOC and PREBUILT_DOC are logically exclusive and must not be set simultaneously. Giving up.")
endif(BUILD_DOC AND PREBUILT_DOC)

# Website installation.

set(WWW_USER "" CACHE STRING "User name at WWW host")
set(WWW_GROUP "plplot" CACHE STRING "Group name at WWW host")
set(WWW_HOST "shell1.sourceforge.net" CACHE STRING "Host name at WWW host")
set(
WWW_DIR "/home/groups/p/pl/plplot/htdocs/docbook-manual"
CACHE STRING "Dir name at WWW host"
)
set(RSH "ssh" CACHE STRING "Remote shell command")
set(RCP "scp" CACHE STRING "Remote copy command")
# Website of the PLplot project
set(PLPLOT_WEBSITE "http://plplot.org" CACHE STRING "PLplot web site")

# Required for validation regardless of whether BUILD_DOC is set
set(XML_DECL "/usr/share/xml/declaration/xml.dcl" CACHE STRING "Full path to xml.dcl file.")
set(DOCBOOK_DTD_PUBID "-//OASIS//DTD DocBook XML V4.5//EN")
set(DOCBOOK_DTD_SYSID "http://www.oasis-open.org/docbook/xml/4.5/docbookx.dtd")

find_program(ONSGMLS onsgmls)
find_program(ENV_FOR_ONSGMLS env)

if(ONSGMLS AND ENV_FOR_ONSGMLS)
  message(STATUS "validate target will be available to check for syntax issues in the PLplot DocBook documentation using ${ENV_FOR_ONSGMLS} SP_CHARSET_FIXED=yes SP_ENCODING=xml ${ONSGMLS}.")
  # Note, these environment variables have to be set to use onsgml with
  # UTF-8 documents, see https://bugzilla.redhat.com/show_bug.cgi?id=66179
  set(ONSGMLS ${ENV_FOR_ONSGMLS} SP_CHARSET_FIXED=yes SP_ENCODING=xml ${ONSGMLS})
else(ONSGMLS AND ENV_FOR_ONSGMLS)
  message(STATUS "WARNING: validate target will not be available to check for syntax issues in the PLplot DocBook documentation because onsgmls (or env) was not found.")
  set(ONSGMLS NOTFOUND)
endif(ONSGMLS AND ENV_FOR_ONSGMLS)

# Check for required programs and perl libraries.
if(BUILD_DOC)

  # Configure the info backend which is implemented using perl and XML/XSLT tools.
  find_program(DB2X_TEXIXML db2x_texixml)
  if(NOT DB2X_TEXIXML)
    message(STATUS "WARNING: db2x_texixml not found")
  endif(NOT DB2X_TEXIXML)
  find_program(DB2X_XSLTPROC db2x_xsltproc)
  if(NOT DB2X_XSLTPROC)
    message(STATUS "WARNING: db2x_xsltproc not found")
  endif(NOT DB2X_XSLTPROC)
  find_program(MAKEINFO makeinfo)
  if(NOT MAKEINFO)
    message(STATUS "WARNING: makeinfo not found")
  endif(NOT MAKEINFO)
  if(PERL_FOUND AND DB2X_TEXIXML AND DB2X_XSLTPROC AND MAKEINFO)
    set(BUILD_INFO ON)
  else(PERL_FOUND AND DB2X_TEXIXML AND DB2X_XSLTPROC AND MAKEINFO)
    set(BUILD_INFO OFF)
    message(STATUS
    "WARNING: Not building info documentation - "
    "required programs are missing"
    )
  endif(PERL_FOUND AND DB2X_TEXIXML AND DB2X_XSLTPROC AND MAKEINFO)

  # Configure the man backend which is implemented using perl and XML tools
  if(PERL_FOUND AND PERL_XML_PARSER AND PERL_XML_DOM)
    set(BUILD_MAN ON)
  else(PERL_FOUND AND PERL_XML_PARSER AND PERL_XML_DOM)
    set(BUILD_MAN OFF)
    message(STATUS
    "WARNING: Not building man documentation - "
    "required programs are missing"
    )
  endif(PERL_FOUND AND PERL_XML_PARSER AND PERL_XML_DOM)

  # Configure the html backend which is implemented using xmlto.
  # The xml tool does its own warnings at run time when components are
  # missing so don't bother with looking for missing components at
  # cmake time.
  find_program(XMLTO xmlto)
  if(NOT XMLTO)
    message(STATUS "WARNING: xmlto not found")
  endif(NOT XMLTO)

  if(XMLTO)
    set(BUILD_HTML ON)
  else(XMLTO)
    message(STATUS
      "WARNING: xmlto script is not installed so not building html documentation"
      )
    set(BUILD_HTML OFF)
  endif(XMLTO)

  option(DOCBOOK_DBLATEX_BACKEND "Use \"dblatex --backend=xetex\" XML/XSLT backend tool to generate our print documentation from DocBook source" ON)


  if(DOCBOOK_DBLATEX_BACKEND)
    # For this default case configure the PDF-only print backend using dblatex and xelatex.
    find_program(DBLATEX dblatex)
    if(DBLATEX)
      set(BUILD_PRINT ON)
    else(DBLATEX)
      message(STATUS "WARNING: dblatex not found so not building print documentation")
      set(BUILD_PRINT OFF)
    endif(DBLATEX)

    if(BUILD_PRINT)
      # Note that the "dblatex --backend=xetex" command must have
      # some sort of xetex or xelatex python dependency.  I
      # haven't tracked down those details, but it is likely that
      # the xelatex is called as part of that processing (or if
      # not some dependency of xelatex is called) so if xelatex is
      # present it is likely that "dblatex --backend=xetex" will
      # work.
      find_program(XELATEX xelatex)
      if(NOT XELATEX)
	message(STATUS "WARNING: xelatex not found so not building print documentation")
	set(BUILD_PRINT OFF)
      endif(NOT XELATEX)
    endif(BUILD_PRINT)

  else(DOCBOOK_DBLATEX_BACKEND)
    # For this non-default case configure the PDF and compressed PostScript print backend
    # using xmlto and gzip.
    if(XMLTO)
      set(BUILD_PRINT ON)
    else(XMLTO)
      message(STATUS
	"WARNING: xmlto script is not installed so not building print documentation"
	)
      set(BUILD_PRINT OFF)
    endif(XMLTO)
    if(BUILD_PRINT)
      find_program(GZIP gzip)
      if(NOT GZIP)
	message(STATUS "WARNING: gzip not found so not building print documentation")
	set(BUILD_PRINT OFF)
      endif(NOT GZIP)
    endif(BUILD_PRINT)
  endif(DOCBOOK_DBLATEX_BACKEND)
endif(BUILD_DOC)

# The "BASE" variables needed for www/documentation.php.in configuration and
# in doc/docbook/src/CMakeLists.txt.
# The "MANIFEST" variables needed in top-level CMakeLists.txt and
# in doc/docbook/src/CMakeLists.txt.
if(BUILD_DOC OR PREBUILT_DOC)
  set(BASE "${PACKAGE}-${PLPLOT_VERSION}")
  set(INFO_MANIFEST "INFO-MANIFEST")
  set(BASE_INFO "${PACKAGE}-info-${PLPLOT_VERSION}")
  set(MAN_MANIFEST "MAN-MANIFEST")
  set(BASE_MAN "${PACKAGE}-man-${PLPLOT_VERSION}")
  set(HTML_MANIFEST "HTML-MANIFEST")
  set(BASE_HTML "${PACKAGE}-html-${PLPLOT_VERSION}")
endif(BUILD_DOC OR PREBUILT_DOC)
