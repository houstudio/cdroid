lib_LTLIBRARIES = libsvg.la
include_HEADERS = svg.h

libsvg_la_SOURCES = \
	svg.c \
	svg.h \
	svgint.h \
	svg_ascii.h \
	svg_ascii.c \
	svg_attribute.c \
	svg_clip_path.c \
	svg_color.c \
	svg_container.c \
	svg_css_style.c \
	svg_dom.c \
	svg_element.c \
	svg_element_ref.c \
	svg_gradient.c \
	svg_group.c \
	svg_hash.c \
	svg_hash.h \
	svg_image.c \
	svg_length.c \
	svg_marker.c \
	svg_mask.c \
	svg_paint.c \
	svg_parser.c \
	svg_parser_expat.c \
	svg_pattern.c \
	svg_path.c \
	svg_render_engine.c \
	svg_resource.c \
	svg_shapes.c \
	svg_str.c \
	svg_style.c \
	svg_symbol.c \
	svg_svg_group.c \
	svg_text.c \
	svg_trace.c \
	svg_transform.c \
	svg_uri.c \
	svg_uri_reference.c \
	svg_use.c \
	svg_view_box.c \
	svg_version.h \
	win32_compat.h \
	$(LIBSVG_EXTRA_SOURCES)

libsvg_la_LDFLAGS = -version-info @VERSION_INFO@

INCLUDES = $(LIBSVG_CFLAGS) $(WARN_CFLAGS)

libsvg_la_LIBADD = $(LIBSVG_LIBS)

