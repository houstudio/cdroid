## Copyright (c) 2009  Openismus GmbH  <http://www.openismus.com/>
##
## This file is part of cairomm.
##
## cairomm is free software: you can redistribute it and/or modify it
## under the terms of the GNU Lesser General Public License as published
## by the Free Software Foundation, either version 2.1 of the License,
## or (at your option) any later version.
##
## cairomm is distributed in the hope that it will be useful, but
## WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
## See the GNU Lesser General Public License for more details.
##
## You should have received a copy of the GNU Lesser General Public License
## along with this library.  If not, see <http://www.gnu.org/licenses/>.

include $(srcdir)/filelist.am

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir) -I. $(CAIROMM_CFLAGS)
AM_CXXFLAGS = $(CAIROMM_WXXFLAGS)

lib_LTLIBRARIES = libcairomm-1.16.la

libcairomm_1_16_la_SOURCES = $(cairomm_cc) $(cairomm_private_h)
libcairomm_1_16_la_LDFLAGS = -no-undefined -version-info $(CAIROMM_SO_VERSION)
libcairomm_1_16_la_LIBADD  = $(CAIROMM_LIBS)

cairomm_includedir = $(includedir)/$(CAIROMM_MODULE_NAME)/cairomm
cairomm_include_HEADERS = $(cairomm_public_h)
