lib_LTLIBRARIES = libsvg-cairo-@LIBSVG_CAIRO_MAJORMINOR@.la
library_includedir = $(includedir)/libsvg-cairo-@LIBSVG_CAIRO_MAJORMINOR@
nobase_library_include_HEADERS = svg-cairo.h

libsvg_cairo_@LIBSVG_CAIRO_MAJORMINOR@_la_SOURCES = \
	svg_cairo.c \
	svg-cairo.h \
	svg-cairo-internal.h \
	svg_cairo_sprintf_alloc.c \
	svg_cairo_state.c

libsvg_cairo_@LIBSVG_CAIRO_MAJORMINOR@_la_LDFLAGS = -version-info @VERSION_INFO@

INCLUDES = $(WARN_CFLAGS) $(PANGOCAIRO_CFLAGS) $(LIBSVG_CAIRO_CFLAGS) $(X_CFLAGS)

libsvg_cairo_@LIBSVG_CAIRO_MAJORMINOR@_la_LIBADD = $(PANGOCAIRO_LIBS) $(LIBSVG_CAIRO_LIBS) $(X_LIBS)
