/* svgint.h: Internal interfaces for libsvg

   Copyright (C) 2002 USC/Information Sciences Institute
   Copyright (C) 2009 Philip de Nier <philipn@users.sourceforge.net>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Carl Worth <cworth@isi.edu>
*/

#ifndef _LIBSVG_SVGINT_H_
#define _LIBSVG_SVGINT_H_

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdlib.h>

#include <expat.h>
#include "svg_hash.h"

typedef XML_Char xmlChar;
typedef XML_Parser svg_xml_parser_context_t;


#if !defined(_MSC_VER)
/* XXX: defines in svg_version.h are currently not used, but what can be done
to include svg_version.h.in in Windows builds */
#include "svg_version.h"
#endif
#include "svg.h"
#include "svg_ascii.h"


#define SVG_ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))


#define SVG_STYLE_FLAG_NONE                             0x0000000000000ULL
#define SVG_STYLE_FLAG_CLIP_RULE                        0x0000000000001ULL
#define SVG_STYLE_FLAG_COLOR                            0x0000000000002ULL
#define SVG_STYLE_FLAG_COLOR_INTERPOLATION              0x0000000000004ULL
#define SVG_STYLE_FLAG_COLOR_INTERPOLATION_FILTERS      0x0000000000008ULL
#define SVG_STYLE_FLAG_COLOR_PROFILE                    0x0000000000010ULL
#define SVG_STYLE_FLAG_COLOR_RENDERING                  0x0000000000020ULL
#define SVG_STYLE_FLAG_CURSOR                           0x0000000000040ULL
#define SVG_STYLE_FLAG_DIRECTION                        0x0000000000080ULL
#define SVG_STYLE_FLAG_DISPLAY                          0x0000000000100ULL
#define SVG_STYLE_FLAG_FILL_OPACITY                     0x0000000000200ULL
#define SVG_STYLE_FLAG_FILL_PAINT                       0x0000000000400ULL
#define SVG_STYLE_FLAG_FILL_RULE                        0x0000000000800ULL
#define SVG_STYLE_FLAG_FONT_FAMILY                      0x0000000001000ULL
#define SVG_STYLE_FLAG_FONT_SIZE                        0x0000000002000ULL
#define SVG_STYLE_FLAG_FONT_SIZE_ADJUST                 0x0000000004000ULL
#define SVG_STYLE_FLAG_FONT_STRETCH                     0x0000000008000ULL
#define SVG_STYLE_FLAG_FONT_STYLE                       0x0000000010000ULL
#define SVG_STYLE_FLAG_FONT_VARIANT                     0x0000000020000ULL
#define SVG_STYLE_FLAG_FONT_WEIGHT                      0x0000000040000ULL
#define SVG_STYLE_FLAG_GLYPH_ORIENTATION_HORIZONTAL     0x0000000080000ULL
#define SVG_STYLE_FLAG_GLYPH_ORIENTATION_VERTICAL       0x0000000100000ULL
#define SVG_STYLE_FLAG_IMAGE_RENDERING                  0x0000000200000ULL
#define SVG_STYLE_FLAG_KERNING                          0x0000000400000ULL
#define SVG_STYLE_FLAG_LETTER_SPACING                   0x0000000800000ULL
#define SVG_STYLE_FLAG_MARKER_END                       0x0000001000000ULL
#define SVG_STYLE_FLAG_MARKER_MID                       0x0000002000000ULL
#define SVG_STYLE_FLAG_MARKER_START                     0x0000004000000ULL
#define SVG_STYLE_FLAG_OPACITY                          0x0000008000000ULL
#define SVG_STYLE_FLAG_POINTER_EVENTS                   0x0000010000000ULL
#define SVG_STYLE_FLAG_SHAPE_RENDERING                  0x0000020000000ULL
#define SVG_STYLE_FLAG_STROKE_DASH_ARRAY                0x0000040000000ULL
#define SVG_STYLE_FLAG_STROKE_DASH_OFFSET               0x0000080000000ULL
#define SVG_STYLE_FLAG_STROKE_LINE_CAP                  0x0000100000000ULL
#define SVG_STYLE_FLAG_STROKE_LINE_JOIN                 0x0000200000000ULL
#define SVG_STYLE_FLAG_STROKE_MITER_LIMIT               0x0000400000000ULL
#define SVG_STYLE_FLAG_STROKE_OPACITY                   0x0000800000000ULL
#define SVG_STYLE_FLAG_STROKE_PAINT                     0x0001000000000ULL
#define SVG_STYLE_FLAG_STROKE_WIDTH                     0x0002000000000ULL
#define SVG_STYLE_FLAG_TEXT_ANCHOR                      0x0004000000000ULL
#define SVG_STYLE_FLAG_TEXT_RENDERING                   0x0008000000000ULL
#define SVG_STYLE_FLAG_VISIBILITY                       0x0010000000000ULL
#define SVG_STYLE_FLAG_WORD_SPACING                     0x0020000000000ULL
#define SVG_STYLE_FLAG_WRITING_MODE                     0x0040000000000ULL
#define SVG_STYLE_FLAG_OVERFLOW                         0x0080000000000ULL
#define SVG_STYLE_FLAG_CLIP                             0x0100000000000ULL
#define SVG_STYLE_FLAG_CLIP_PATH                        0x0200000000000ULL
#define SVG_STYLE_FLAG_MASK                             0x0400000000000ULL
#define SVG_STYLE_FLAG_STOP_COLOR                       0x0800000000000ULL
#define SVG_STYLE_FLAG_STOP_OPACITY                     0x1000000000000ULL



typedef enum svgint_status {
    SVGINT_STATUS_ARGS_EXHAUSTED = 1000,
    SVGINT_STATUS_IMAGE_NOT_PNG,
    SVGINT_STATUS_IMAGE_NOT_JPEG,
    SVGINT_STATUS_UNKNOWN_DOCUMENT,
} svgint_status_t;

typedef struct svg_pt {
    double x;
    double y;
} svg_pt_t;

typedef struct svg_uri svg_uri_t;

typedef struct svg_uri_ref {
    char *element_id;
    svg_uri_t *uri;
} svg_uri_ref_t;

typedef enum svg_path_type {
    SVG_PATH_TYPE_PATH,
    SVG_PATH_TYPE_POLYGON,
    SVG_PATH_TYPE_POLYLINE
} svg_path_type_t;

typedef struct svg_path_int {
    svg_path_type_t type;

    svg_path_t *ext_path;
    svg_path_t *last_ext_path;

    svg_pt_t last_move_pt;
    svg_pt_t current_pt;

    svg_pt_t reflected_cubic_pt;
    svg_pt_t reflected_quad_pt;
} svg_path_int_t;

typedef struct svg_style {
    uint64_t flags;

    int display;
    int visibility;

    double fill_opacity;
    svg_paint_t fill_paint;
    svg_fill_rule_t fill_rule;

    char *font_family;
    svg_length_t font_size;
    double computed_font_size;
    svg_font_style_t font_style;
    unsigned int font_weight;

    double opacity;

    double *stroke_dash_array;
    int num_dashes;
    svg_length_t stroke_dash_offset;

    svg_stroke_line_cap_t stroke_line_cap;
    svg_stroke_line_join_t stroke_line_join;
    double stroke_miter_limit;
    double stroke_opacity;
    svg_paint_t stroke_paint;
    svg_length_t stroke_width;

    svg_color_t color;
    svg_text_anchor_t text_anchor;

    svg_color_t stop_color;
    double stop_opacity;

    int overflow_hidden;
    svg_length_t clip_rect[4];

    const svg_clip_path_t *clip_path;
    svg_clip_rule_t clip_rule;

    const svg_mask_t *mask;

    const svg_marker_t *marker_start;
    const svg_marker_t *marker_mid;
    const svg_marker_t *marker_end;
} svg_style_t;

typedef struct svg_dom_node svg_dom_node_t;

typedef struct svg_container {
    svg_element_t **element;
    int num_elements;
    int element_size;
} svg_container_t;

typedef struct svg_group {
    svg_container_t container;
} svg_group_t;

typedef struct svg_tspan {
    char *chars;
    size_t len;
    svg_length_t x;
    int x_set;
    svg_length_t y;
    int y_set;
    svg_length_t dx;
    svg_length_t dy;
    svg_dom_node_t *virtual_node;
} svg_tspan_t;

typedef struct svg_text {
    svg_container_t container;
    svg_length_t x;
    svg_length_t y;
    svg_length_t dx;
    svg_length_t dy;
} svg_text_t;

typedef struct svg_ellipse {
    svg_length_t cx;
    svg_length_t cy;
    svg_length_t rx;
    svg_length_t ry;
} svg_ellipse_t;

typedef struct svg_circle {
    svg_length_t cx;
    svg_length_t cy;
    svg_length_t r;
} svg_circle_t;

typedef struct svg_line {
    svg_length_t x1;
    svg_length_t y1;
    svg_length_t x2;
    svg_length_t y2;
} svg_line_t;

typedef struct svg_rect_element {
    svg_length_t x;
    svg_length_t y;
    svg_length_t width;
    svg_length_t height;
    svg_length_t rx;
    svg_length_t ry;
} svg_rect_element_t;

typedef struct svg_image {
    svg_uri_t *uri;
    int index;
    svg_view_box_t view_box_template;
    svg_length_t x;
    svg_length_t y;
    svg_length_t width;
    svg_length_t height;
} svg_image_t;

typedef struct svg_gradient_int {
    svg_container_t container;
    svg_gradient_t ext_gradient;
    int inherited_stops;
} svg_gradient_int_t;

typedef struct svg_pattern_int {
    svg_container_t container;
    svg_pattern_t ext_pattern;
    svg_view_box_t view_box;
    int num_inherited_children;
    int enable_render;
} svg_pattern_int_t;

typedef struct svg_clip_path_int {
    svg_container_t container;
    svg_clip_path_t ext_clip_path;
    int enable_render;
} svg_clip_path_int_t;

typedef struct svg_mask_int {
    svg_container_t container;
    svg_mask_t ext_mask;
    int enable_render;
} svg_mask_int_t;

typedef struct svg_symbol {
    svg_container_t container;
    svg_view_box_t view_box;
} svg_symbol_t;

typedef struct svg_use {
    svg_group_t group;
    svg_length_t x;
    svg_length_t y;
} svg_use_t;

typedef struct svg_svg_group {
    svg_container_t container;
    svg_length_t x;
    svg_length_t y;
    svg_length_t width;
    svg_length_t height;
    svg_view_box_t view_box;
} svg_svg_group_t;

typedef enum svg_marker_units {
    SVG_MARKER_UNITS_USER,
    SVG_MARKER_UNITS_STROKE_WIDTH
} svg_marker_units_t;

typedef struct svg_marker_int {
    svg_container_t container;
    svg_marker_t ext_marker;
    svg_marker_units_t units;
    svg_length_t width, height;
    double stroke_width;
    svg_length_t ref_x, ref_y;
    svg_view_box_t view_box;
    int enable_render;
} svg_marker_int_t;

typedef enum svg_element_type {
    SVG_ELEMENT_TYPE_SVG_GROUP,
    SVG_ELEMENT_TYPE_GROUP,
    SVG_ELEMENT_TYPE_DEFS,
    SVG_ELEMENT_TYPE_USE,
    SVG_ELEMENT_TYPE_SYMBOL,
    SVG_ELEMENT_TYPE_PATH,
    SVG_ELEMENT_TYPE_CIRCLE,
    SVG_ELEMENT_TYPE_ELLIPSE,
    SVG_ELEMENT_TYPE_LINE,
    SVG_ELEMENT_TYPE_RECT,
    SVG_ELEMENT_TYPE_TEXT,
    SVG_ELEMENT_TYPE_TSPAN,
    SVG_ELEMENT_TYPE_GRADIENT,
    SVG_ELEMENT_TYPE_GRADIENT_STOP,
    SVG_ELEMENT_TYPE_PATTERN,
    SVG_ELEMENT_TYPE_IMAGE,
    SVG_ELEMENT_TYPE_CLIP_PATH,
    SVG_ELEMENT_TYPE_MASK,
    SVG_ELEMENT_TYPE_MARKER,
} svg_element_type_t;

struct svg_element {
    struct svg_element *parent;

    svg_element_type_t type;
    svg_t *svg;
    struct svg_dom_node *node;
    char *id;
    char *klass;
    svg_transform_t transform;

    union {
        svg_container_t container;
        svg_group_t group;
        svg_path_int_t path;
        svg_circle_t circle;
        svg_ellipse_t ellipse;
        svg_line_t line;
        svg_rect_element_t rect;
        svg_text_t text;
        svg_tspan_t tspan;
        svg_image_t image;
        svg_gradient_int_t gradient;
        svg_gradient_stop_t gradient_stop;
        svg_pattern_int_t pattern;
        svg_clip_path_int_t clip_path;
        svg_mask_int_t mask;
        svg_symbol_t symbol;
        svg_use_t use;
        svg_svg_group_t svg_group;
        svg_marker_int_t marker;
    } e;
};

struct svg_element_ref {
    svg_element_t *element;
    int inherit_referrer_properties;
};

typedef struct svg_css_stylesheet svg_css_stylesheet_t;

typedef struct svg_css_declaration {
    char *name;
    char *value;
    int important;
} svg_css_declaration_t;

typedef struct svg_dom svg_dom_t;

typedef enum {
    ELEMENT_NODE_TYPE,
    CHARACTER_NODE_TYPE
} svg_dom_node_type_t;

typedef struct {
    int ns_index;
    char *local_name;
} svg_qname_t;

typedef struct {
    svg_qname_t name;
    char *value;
} svg_qattr_t;

typedef struct {
    svg_qattr_t *atts;
    int num;
} svg_qattrs_t;

struct svg_dom_node {
    svg_dom_node_type_t type;
    long line_number;

    svg_dom_t *doc;

    int is_deep_clone;

    struct svg_dom_node *parent;
    struct svg_dom_node *next_sibling;
    struct svg_dom_node *prev_sibling;

    svg_qname_t *qname;
    int own_qname;
    svg_qattrs_t *qattrs;
    int own_qattrs;
    struct svg_dom_node *children;

    svg_css_declaration_t *css_properties;
    int num_css_properties;
    svg_css_declaration_t *inline_css_properties;
    int num_inline_css_properties;

    int override_display_property;

    svg_style_t style;

    svg_uri_t *base_uri;
    int own_base_uri;
    svg_uri_t *document_uri;    /* not owned (XXX: make const when funcs modified to use consts consistently) */

    int preserve_space;

    char *ch;                   /* not null terminated */
    size_t len;
};

typedef enum {
    NO_NAMESPACE_INDEX = 0,
    SVG_NAMESPACE_INDEX,
    XML_NAMESPACE_INDEX,
    XLINK_NAMESPACE_INDEX,
} svg_known_ns_id_t;

typedef struct svg_ns_map {
    char *uri;
    int index;
} svg_ns_map_t;

struct svg_dom {
    svg_dom_node_t *root_node;
    svg_ns_map_t *ns_map;
    int num_ns;
    svg_dom_node_t *current_node;
};

typedef svg_status_t (svg_resource_read_stream_cb_t) (void *closure,
                                                      const char *buffer, size_t buffer_size);

typedef struct svg_parser svg_parser_t;

typedef svg_status_t (svg_parser_parse_element_start_t) (svg_parser_t *parser,
                                                         svg_dom_node_t *node,
                                                         svg_element_t **element);

typedef svg_status_t (svg_parser_parse_element_end_t) (svg_parser_t *parser,
                                                       svg_element_t *element);

typedef svg_status_t (svg_parser_parse_characters_t) (svg_parser_t *parser,
                                                      svg_dom_node_t *node,
                                                      const char *ch, size_t len);

typedef struct svg_parser_cb {
    svg_parser_parse_element_start_t *parse_element_start;
    svg_parser_parse_element_end_t *parse_element_end;
    svg_parser_parse_characters_t *parse_characters;
} svg_parser_cb_t;

typedef struct svg_parser_state {
    const svg_parser_cb_t *cb;
    svg_element_t *container_element;
    svg_text_t *text;
    svg_tspan_t *text_tspan;
    svg_tspan_t *tspan;
    struct svg_parser_state *next;
} svg_parser_state_t;

typedef struct svg_parser_deferred_element {
    svg_element_t *element;
    svg_parser_state_t state;
    struct svg_parser_deferred_element *next;
} svg_parser_deferred_element_t;

struct svg_parser {
    svg_t *svg;
    svg_xml_parser_context_t ctxt;
    svg_dom_t dom;
    svg_css_stylesheet_t *stylesheet;
    svg_parser_deferred_element_t *deferred_elements;
    svg_parser_state_t *state;
    svg_xml_hash_table_t *entities;
    svg_status_t status;
};

typedef struct svg_trace_target {
    struct svg_trace_target *next;
    const svg_uri_t *document_uri;
    svg_render_engine_t *target_engine;
    void *target_closure;
    svg_render_engine_t trace_engine;
    int new_render;
} svg_trace_target_t;

typedef struct svg_trace {
    svg_trace_target_t *targets;
    int level;
    const char *node_name;
    long line_number;
} svg_trace_t;

typedef struct svg_error_info {
    char *element_name;
    char *attribute_name;
    char *property_name;
    long line_number;
    svg_status_t error_status;
    svg_status_t external_error_status;
} svg_error_info_t;

struct svg {
    double dpi;

    svg_uri_t *base_uri;
    int user_set_base_uri;
    svg_uri_t *document_uri;

    svg_element_t *root_element;

    svg_xml_hash_table_t *element_ids;

    char **image_uris;
    int num_images;

    svg_parser_t parser;

    svg_render_engine_t *engine;

    svg_trace_t *trace;
    int own_trace;

    svg_error_info_t error_info;

    struct svg *parent;
    struct svg *children;
    struct svg *next_sibling;
};




/* svg.c */

svg_status_t _svg_store_element_by_id (svg_t *svg, svg_element_t *element);

svg_element_t *_svg_fetch_element_by_id (svg_t *svg, const char *id);

svg_status_t
_svg_resolve_element_href (svg_t *svg, svg_uri_t *document_uri, svg_uri_t *base_uri,
                           const char *href, svg_element_t **element);

svg_status_t
_svg_resolve_element_url (svg_t *svg, svg_uri_t *document_uri, svg_uri_t *base_uri,
                          const char *url, svg_element_t **element);

svg_status_t
_svg_register_image_uri (svg_t *svg, svg_uri_t *uri, int *index);

const char *
_svg_get_image_uri (svg_t *svg, int index);

void
_svg_set_error_status (svg_t *svg, svg_status_t error_status);

void
_svg_set_external_error_status (svg_t *svg, svg_status_t error_status);

void
_svg_set_error_line_number (svg_t *svg, long line_number);

void
_svg_set_error_element_name (svg_t *svg, const svg_qname_t *element_name);

void
_svg_set_error_attribute_name (svg_t *svg, const char *attribute_name);

void
_svg_set_error_property_name (svg_t *svg, const char *property_name);

svg_status_t
_svg_return_error_status (svg_t *svg, svg_status_t error_status);

svg_status_t
_svg_externalize_status (svgint_status_t internal_status, svg_status_t default_status);


/* svg_attribute.c */

svg_status_t
_svg_attribute_get_double (const svg_qattrs_t *attributes, const char *name, double *value,
                           double default_value);

svg_status_t
_svg_attribute_get_string (const svg_qattrs_t *attributes, const char *name, const char **value,
                           const char *default_value);

svg_status_t
_svg_attribute_get_string_ns (const svg_qattrs_t *attributes, int ns_index,
                              const char *name, const char **value, const char *default_value);

svg_status_t
_svg_attribute_get_length (const svg_qattrs_t *attributes, const char *name, svg_length_t *value,
                           const char *default_value);


/* svg_clip_path.c */

svg_status_t
_svg_clip_path_init (svg_clip_path_int_t *clip_path, svg_element_t *clip_path_element);

svg_status_t
_svg_clip_path_init_copy (svg_clip_path_int_t *clip_path, svg_element_t *clip_path_element,
                          svg_clip_path_int_t *other);

svg_status_t
_svg_clip_path_deinit (svg_clip_path_int_t *clip_path);

svg_status_t
_svg_clip_path_apply_attributes (svg_element_t *clip_path_element, const svg_qattrs_t *attributes);

int
 _svg_clip_path_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_clip_path_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_color.c */

svg_status_t
_svg_color_init_rgb (svg_color_t *color, unsigned int r, unsigned int g, unsigned int b);

svg_status_t
_svg_color_init_from_str (svg_color_t *color, const char *str);


/* svg_container.c */

svg_status_t
_svg_container_init (svg_container_t *container);

svg_status_t
_svg_container_init_copy (svg_container_t *container, svg_container_t *other);

svg_status_t
_svg_container_deinit (svg_container_t *container);

svg_status_t
_svg_container_add_element (svg_container_t *container, svg_element_t *element);

int
 _svg_container_peek_render (svg_container_t *container, svg_render_engine_t *engine);

svg_status_t
_svg_container_render (svg_container_t *container, svg_render_engine_t *engine, void *closure);


/* svg_css_style.c */

svg_status_t
_svg_create_css_stylesheet (svg_css_stylesheet_t **stylesheet);

void
_svg_destroy_css_stylesheet (svg_css_stylesheet_t *stylesheet);

svg_status_t
_svg_parse_css_buffer (svg_css_stylesheet_t *stylesheet,
                       svg_uri_t *base_uri, const char *buffer, size_t count);

svg_status_t
_svg_parse_external_css_stylesheet (svg_css_stylesheet_t *stylesheet, svg_uri_t *uri);

svg_status_t
_svg_parse_inline_css (const char *data, size_t count,
                       svg_css_declaration_t **declarations, int *num_declarations);

svg_status_t
_svg_get_css_stylesheet_declarations (svg_dom_node_t *node, svg_css_stylesheet_t *stylesheet,
                                      svg_css_declaration_t **declarations, int *num_declarations);

void
_svg_destroy_css_declarations (svg_css_declaration_t *declarations, int num_declarations);


/* svg_dom.c */

svg_status_t
_svg_dom_init (svg_dom_t *dom);

void
_svg_dom_deinit (svg_dom_t *dom);

svg_status_t
_svg_dom_create_qname (int ns_index, const char *local_name, svg_qname_t **qname);

svg_status_t
_svg_dom_init_qname (svg_qname_t *qname, int ns_index, const char *local_name);

void
_svg_dom_deinit_qname (svg_qname_t *qname);

void
_svg_dom_destroy_qname (svg_qname_t *qname);

svg_status_t
_svg_dom_create_qattrs (int num_qattrs, svg_qattrs_t **qattrs);

svg_status_t
_svg_dom_init_qattr (svg_qattr_t *qattr, int ns_index, const char *local_name, const char *value);

void
_svg_dom_deinit_qattr (svg_qattr_t *qattr);

void
_svg_dom_destroy_qattrs (svg_qattrs_t *qattrs);

int
_svg_compare_qname (svg_qname_t *left, svg_qname_t *right);

int
_svg_compare_qname_2 (svg_qname_t *left, int ns_index, const char *local_name);

int
_svg_compare_qname_svg (svg_qname_t *left, const char *svg_local_name);

svg_status_t
_svg_dom_register_uri (svg_dom_t *dom, const char *uri, int *index);

svg_status_t
_svg_create_dom_node (svg_dom_node_type_t type, svg_dom_node_t **node);

svg_status_t
_svg_destroy_dom_node (svg_dom_node_t *node);

svg_status_t
_svg_dom_start_element (svg_dom_t *dom, svg_qname_t *qname, svg_qattrs_t *qattrs,
                        svg_dom_node_t **node);

svg_status_t
_svg_dom_end_element (svg_dom_t *dom, svg_qname_t *qname);

svg_status_t
_svg_dom_characters (svg_dom_t *dom, const char *ch, size_t len, svg_dom_node_t **node);

svg_status_t
_svg_dom_deep_clone_node (svg_dom_node_t *parent, svg_dom_node_t *from_node, svg_dom_node_t **clone);

svg_status_t
_svg_dom_node_set_qname (svg_dom_node_t *node, int ns_index, const char *local_name);

svg_status_t
_svg_dom_node_set_qattr (svg_dom_node_t *node, int ns_index, const char *local_name,
                          const char *value);

void
_svg_dom_set_line_number (svg_dom_node_t *node, long line_number);

void
_svg_dom_node_set_base_uri (svg_dom_node_t *node, svg_uri_t *base_uri, int own_base_uri);

void
_svg_dom_node_set_document_uri (svg_dom_node_t *node, svg_uri_t *document_uri);

svg_dom_node_t *
_svg_dom_get_current_element_node (svg_dom_t *node);

svg_dom_node_t *
_svg_dom_get_current_character_node (svg_dom_t *node);

svg_dom_node_t *
_svg_dom_get_parent (svg_dom_node_t *node);

svg_dom_node_t *
_svg_dom_get_first_child (svg_dom_node_t *node);

svg_dom_node_t *
_svg_dom_get_next_sibling (svg_dom_node_t *node);

svg_dom_node_t *
_svg_dom_get_prev_sibling (svg_dom_node_t *node);

int
_svg_dom_is_element_node (svg_dom_node_t *node);

const char *
_svg_dom_get_node_local_name (svg_dom_node_t *node);

const char *
_svg_dom_get_local_attr_value (svg_dom_node_t *node, const char *local_name);

svg_status_t
_svg_normalize_character_data (svg_dom_node_t *node, char **ch_data, size_t *ch_data_offset,
                               size_t *ch_data_len);


/* svg_element.c */

svg_status_t
_svg_element_create (svg_element_type_t type, svg_dom_node_t *node, svg_element_t *parent,
                     svg_t *svg, svg_element_t **element);

svg_status_t
_svg_element_init (svg_element_t *element, svg_element_type_t type, svg_dom_node_t *node,
                   svg_element_t *parent, svg_t *svg);

svg_status_t
_svg_element_init_copy (svg_element_t *element, svg_element_t *other);

svg_status_t
_svg_element_deinit (svg_element_t *element);

svg_status_t
_svg_element_destroy (svg_element_t *element);

svg_status_t
_svg_element_clone (svg_element_t *other, svg_element_t **element);

svg_status_t
_svg_create_virtual_element (svg_element_type_t type, svg_dom_node_t *style_node,
                             svg_element_t *parent, svg_t *svg, svg_element_t **element);

int
_svg_element_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_element_apply_properties (svg_element_t *element);

svg_status_t
_svg_element_return_property_error (svg_element_t *element, const char *property_name,
                                    svg_status_t error_status);


/* svg_element_ref.c */

svg_status_t
_svg_create_element_ref (svg_element_t *element, svg_element_ref_t **element_ref);

void
_svg_destroy_element_ref (svg_element_ref_t *element_ref);


/* svg_gradient.c */

svg_status_t
_svg_gradient_stop_init (svg_gradient_stop_t *stop);

svg_status_t
_svg_gradient_stop_init_copy (svg_gradient_stop_t *stop, svg_gradient_stop_t *other);

svg_status_t
_svg_gradient_stop_deinit (svg_gradient_stop_t *stop);

svg_status_t
_svg_gradient_stop_apply_attributes (svg_element_t *stop, const svg_qattrs_t *attributes);

svg_status_t
_svg_gradient_stop_apply_style (svg_gradient_stop_t *stop, svg_style_t *style);

svg_status_t
_svg_gradient_init (svg_gradient_int_t *gradient);

svg_status_t
_svg_gradient_init_copy (svg_gradient_int_t *gradient, svg_gradient_int_t *other);

svg_status_t
_svg_gradient_deinit (svg_gradient_int_t *gradient);

svg_status_t
_svg_gradient_set_type (svg_gradient_int_t *gradient, svg_gradient_type_t type);

svg_status_t
_svg_gradient_add_stop (svg_gradient_int_t *gradient, svg_gradient_stop_t *stop);

svg_status_t
_svg_gradient_apply_attributes (svg_element_t *gradient_element, const svg_qattrs_t *attributes);


/* svg_group.c */

svg_status_t
_svg_group_init (svg_group_t *group);

svg_status_t
_svg_group_init_copy (svg_group_t *group, svg_group_t *other);

svg_status_t
_svg_group_deinit (svg_group_t *group);

svg_status_t
_svg_group_add_element (svg_group_t *group, svg_element_t *element);

svg_status_t
_svg_group_apply_attributes (svg_element_t *group_element, const svg_qattrs_t *attributes);

int
_svg_group_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_group_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_image.c */

svg_status_t
_svg_image_init (svg_image_t *image);

svg_status_t
_svg_image_init_copy (svg_image_t *image, svg_image_t *other);

svg_status_t
_svg_image_deinit (svg_image_t *image);

svg_status_t
_svg_image_apply_attributes (svg_element_t *image_element, const svg_qattrs_t *attributes);

int
_svg_image_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_image_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_length.c */

svg_status_t
_svg_length_init (svg_length_t *length, double value);

svg_status_t
_svg_length_init_unit (svg_length_t *length, double value, svg_length_unit_t unit,
                       svg_length_orientation_t orientation);

svg_status_t
_svg_length_init_from_str (svg_length_t *length, const char *str);

svg_status_t
_svg_length_init_from_array_str (svg_length_t *length, const char *str, const char **after_unit);


/* svg_marker.c */

svg_status_t
_svg_marker_init (svg_marker_int_t *marker, svg_element_t *marker_element);

svg_status_t
_svg_marker_init_copy (svg_marker_int_t *marker, svg_element_t *marker_element,
                       svg_marker_int_t *other);

svg_status_t
_svg_marker_deinit (svg_marker_int_t *marker);

svg_status_t
_svg_marker_apply_attributes (svg_element_t *marker_element, const svg_qattrs_t *attributes);

int
_svg_marker_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_marker_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_mask.c */

svg_status_t
_svg_mask_init (svg_mask_int_t *mask, svg_element_t *mask_element);

svg_status_t
_svg_mask_init_copy (svg_mask_int_t *mask, svg_element_t *mask_element, svg_mask_int_t *other);

svg_status_t
_svg_mask_deinit (svg_mask_int_t *mask);

svg_status_t
_svg_mask_apply_attributes (svg_element_t *mask_element, const svg_qattrs_t *attributes);

int
_svg_mask_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_mask_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_paint.c */

svg_status_t
_svg_paint_init (svg_paint_t *paint, svg_t *svg, svg_uri_t *document_uri, svg_uri_t *base_uri,
                 const char *str);


/* svg_parser.c */

svg_status_t
_svg_parser_init (svg_parser_t *parser, svg_t *svg);

svg_status_t
_svg_parser_deinit (svg_parser_t *parser);

svg_status_t
_svg_parser_begin_dom_parsing (svg_parser_t *parser);

svg_status_t
_svg_parser_parse_chunk (svg_parser_t *parser, const char *buf, size_t count);

svg_status_t
_svg_parser_end_dom_parsing (svg_parser_t *parser);

svg_status_t
_svg_parser_stop (svg_parser_t *parser);

svg_status_t
_svg_parser_parse_pseudo_proc_instr (svg_dom_t *dom, const char *buf, size_t count);

long
_svg_parser_get_line_number (svg_parser_t *parser);

void
_svg_parser_set_error (svg_parser_t *parser, long line_number, svg_status_t error_status);

void
_svg_parser_set_node_error (svg_parser_t *parser, long line_number, svg_qname_t *node_qname,
                            svg_status_t error_status);

void
_svg_parser_set_attr_error (svg_parser_t *parser, const char *attr_local_name,
                            svg_status_t error_status);

svg_status_t
_svg_parser_return_error (svg_parser_t *parser, svg_status_t error_status);

svg_status_t
_svg_parser_return_node_error (svg_parser_t *parser, svg_dom_node_t *node,
                               svg_status_t error_status);

svg_status_t
_svg_parser_return_attr_error (svg_parser_t *parser, const char *attr_local_name,
                               svg_status_t error_status);

svg_status_t
_svg_parser_process_dom (svg_parser_t *parser);

svg_status_t
_svg_parser_create_deferred_element (svg_parser_t *parser, svg_element_t *element);

svg_status_t
_svg_parser_copy_xml_name (svg_dom_t *dom, const xmlChar *xml_name, svg_qname_t **qname);

svg_status_t
_svg_parser_copy_xml_attributes (svg_dom_t *dom, int element_ns_index, const xmlChar **xml_atts,
                                 svg_qattrs_t **qattrs);

void
_svg_parser_sax_start_element (void *closure, const xmlChar *xml_name, const xmlChar **xml_atts);

void
_svg_parser_sax_end_element (void *closure, const xmlChar *xml_name);

void
_svg_parser_sax_characters (void *closure, const xmlChar *ch, int len);

void
_svg_parser_sax_proc_instr (void *closure, const xmlChar *target, const xmlChar *data);

void
_svg_parser_pseudo_proc_instr_start_element (void *closure, const xmlChar *xml_name,
                                             const xmlChar **xml_atts);


/* svg_path.c */

svg_status_t
_svg_path_init (svg_path_int_t *path);

svg_status_t
_svg_path_init_copy (svg_path_int_t *path, svg_path_int_t *other);

svg_status_t
_svg_path_deinit (svg_path_int_t *path);

int
_svg_path_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_path_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_path_apply_attributes (svg_element_t *path_element, const svg_qattrs_t *attributes);


/* svg_pattern.c */

svg_status_t
_svg_pattern_init (svg_pattern_int_t *pattern, svg_element_t *element);

svg_status_t
_svg_pattern_init_copy (svg_pattern_int_t *pattern, svg_element_t *pattern_element,
                        svg_pattern_int_t *other);

svg_status_t
_svg_pattern_deinit (svg_pattern_int_t *pattern);

svg_status_t
_svg_pattern_apply_attributes (svg_element_t *pattern_element, const svg_qattrs_t *attributes);

int
_svg_pattern_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_pattern_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_render_engine.c */

svg_status_t
_svg_engine_begin_group (svg_render_engine_t *engine, void *closure, double opacity,
                         const char *id, const char *klass);

svg_status_t
_svg_engine_begin_element (svg_render_engine_t *engine, void *closure,
                           const char *id, const char *klass);

svg_status_t
_svg_engine_end_element (svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_engine_end_group (svg_render_engine_t *engine, void *closure, double opacity);

svg_status_t
_svg_engine_set_viewport (svg_render_engine_t *engine, void *closure,
                          const svg_length_t *x, const svg_length_t *y,
                          const svg_length_t *width, const svg_length_t *height);

svg_status_t
_svg_engine_apply_view_box (svg_render_engine_t *engine, void *closure,
                            const svg_view_box_t *view_box,
                            const svg_length_t *width, const svg_length_t *height);

svg_status_t
_svg_engine_viewport_clipping_path (svg_render_engine_t *engine, void *closure,
                                    const svg_length_t *top, const svg_length_t *right,
                                    const svg_length_t *bottom, const svg_length_t *left);

svg_status_t
_svg_engine_transform (svg_render_engine_t *engine, void *closure,
                       const svg_transform_t *transform);

svg_status_t
_svg_engine_end_transform (svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_engine_set_clip_path (svg_render_engine_t *engine, void *closure,
                           const svg_clip_path_t *clip_path);

svg_status_t
_svg_engine_set_clip_rule (svg_render_engine_t *engine, void *closure, svg_clip_rule_t clip_rule);

svg_status_t
_svg_engine_set_color (svg_render_engine_t *engine, void *closure, const svg_color_t *color);

svg_status_t
_svg_engine_set_fill_opacity (svg_render_engine_t *engine, void *closure, double fill_opacity);

svg_status_t
_svg_engine_set_fill_paint (svg_render_engine_t *engine, void *closure, const svg_paint_t *paint);

svg_status_t
_svg_engine_set_fill_rule (svg_render_engine_t *engine, void *closure, svg_fill_rule_t fill_rule);

svg_status_t
_svg_engine_set_font_family (svg_render_engine_t *engine, void *closure, const char *family);

svg_status_t
_svg_engine_set_font_size (svg_render_engine_t *engine, void *closure, double size);

svg_status_t
_svg_engine_set_font_style (svg_render_engine_t *engine, void *closure,
                            svg_font_style_t font_style);

svg_status_t
_svg_engine_set_font_weight (svg_render_engine_t *engine, void *closure, unsigned int font_weight);

svg_status_t
_svg_engine_set_marker_end (svg_render_engine_t *engine, void *closure, const svg_marker_t *marker);

svg_status_t
_svg_engine_set_marker_mid (svg_render_engine_t *engine, void *closure, const svg_marker_t *marker);

svg_status_t
_svg_engine_set_marker_start (svg_render_engine_t *engine, void *closure,
                              const svg_marker_t *marker);

svg_status_t
_svg_engine_set_mask (svg_render_engine_t *engine, void *closure, const svg_mask_t *mask);

svg_status_t
_svg_engine_set_opacity (svg_render_engine_t *engine, void *closure, double opacity);

svg_status_t
_svg_engine_set_stroke_dash_array (svg_render_engine_t *engine, void *closure,
                                   const double *dash_array, int num_dashes);

svg_status_t
_svg_engine_set_stroke_dash_offset (svg_render_engine_t *engine, void *closure,
                                    const svg_length_t *offset);

svg_status_t
_svg_engine_set_stroke_line_cap (svg_render_engine_t *engine, void *closure,
                                 svg_stroke_line_cap_t line_cap);

svg_status_t
_svg_engine_set_stroke_line_join (svg_render_engine_t *engine, void *closure,
                                  svg_stroke_line_join_t line_join);

svg_status_t
_svg_engine_set_stroke_miter_limit (svg_render_engine_t *engine, void *closure, double limit);

svg_status_t
_svg_engine_set_stroke_opacity (svg_render_engine_t *engine, void *closure, double stroke_opacity);

svg_status_t
_svg_engine_set_stroke_paint (svg_render_engine_t *engine, void *closure, const svg_paint_t *paint);

svg_status_t
_svg_engine_set_stroke_width (svg_render_engine_t *engine, void *closure,
                              const svg_length_t *width);

svg_status_t
_svg_engine_set_text_anchor (svg_render_engine_t *engine, void *closure,
                             svg_text_anchor_t text_anchor);

svg_status_t
_svg_engine_set_visibility (svg_render_engine_t *engine, void *closure, int visible);

svg_status_t
_svg_engine_end_style (svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_engine_text_advance_x (svg_render_engine_t *engine, void *closure,
                            const char *utf8, double *advance);

svg_status_t
_svg_engine_set_text_position_x (svg_render_engine_t *engine, void *closure, const svg_length_t *x);

svg_status_t
_svg_engine_set_text_position_y (svg_render_engine_t *engine, void *closure, const svg_length_t *y);

svg_status_t
_svg_engine_adjust_text_position (svg_render_engine_t *engine, void *closure,
                                  const svg_length_t *dx, const svg_length_t *dy);

svg_status_t
_svg_engine_set_text_chunk_width (svg_render_engine_t *engine, void *closure, double width);

svg_status_t
_svg_engine_render_line (svg_render_engine_t *engine, void *closure,
                         const svg_length_t *x1, const svg_length_t *y1,
                         const svg_length_t *x2, const svg_length_t *y2);

svg_status_t
_svg_engine_render_path (svg_render_engine_t *engine, void *closure, const svg_path_t *ext_path);

svg_status_t
_svg_engine_render_circle (svg_render_engine_t *engine, void *closure,
                           const svg_length_t *cx, const svg_length_t *cy, const svg_length_t *r);

svg_status_t
_svg_engine_render_ellipse (svg_render_engine_t *engine, void *closure,
                            const svg_length_t *cx,const svg_length_t *cy,
                            const svg_length_t *rx, const svg_length_t *ry);

svg_status_t
_svg_engine_render_rect (svg_render_engine_t *engine, void *closure,
                         const svg_length_t *x, const svg_length_t *y,
                         const svg_length_t *width, const svg_length_t *height,
                         const svg_length_t *rx, const svg_length_t *ry);

svg_status_t
_svg_engine_render_text (svg_render_engine_t *engine, void *closure, const char *utf8);

svg_status_t
_svg_engine_render_image (svg_render_engine_t *engine, void *closure,
                          const char *uri, int index,
                          const svg_view_box_t *view_box_template,
                          const svg_length_t *x, const svg_length_t *y,
                          const svg_length_t *width, const svg_length_t *height);

svg_status_t
_svg_engine_measure_position (svg_render_engine_t *engine, void *closure,
                              const svg_length_t *ix, const svg_length_t *iy,
                              double *ox, double *oy);

svg_status_t
_svg_engine_measure_font_size (svg_render_engine_t *engine, void *closure,
                               const char *font_family, double parent_font_size,
                               const svg_length_t *in_size, double *out_size);

int
_svg_engine_support_render_line (svg_render_engine_t *engine);

int
_svg_engine_support_render_path (svg_render_engine_t *engine);

int
_svg_engine_support_render_circle (svg_render_engine_t *engine);

int
_svg_engine_support_render_ellipse (svg_render_engine_t *engine);

int
_svg_engine_support_render_rect (svg_render_engine_t *engine);

int
_svg_engine_support_render_text (svg_render_engine_t *engine);

int
_svg_engine_support_render_image (svg_render_engine_t *engine);

int
_svg_engine_support_render_image_buffer (svg_render_engine_t *engine);


/* svg_resource.c */

svg_status_t
_svg_resource_read_stream (svg_uri_t *uri, void *cb_closure,
                           svg_resource_read_stream_cb_t *read_cb);

svg_status_t
_svg_resource_read_stream_to_buffer (svg_uri_t *uri, char **buffer, size_t *buffer_size,
                                     size_t max_buffer_size);

svg_status_t
_svg_resource_get_access_filename (svg_uri_t *uri, char **filename, int *is_temp_copy);


/* svg_shapes.c */

svg_status_t
_svg_circle_init (svg_circle_t *circle);

svg_status_t
_svg_circle_init_copy (svg_circle_t *circle, svg_circle_t *other);

svg_status_t
_svg_ellipse_init (svg_ellipse_t *ellipse);

svg_status_t
_svg_ellipse_init_copy (svg_ellipse_t *ellipse, svg_ellipse_t *other);

svg_status_t
_svg_line_init (svg_line_t *line);

svg_status_t
_svg_line_init_copy (svg_line_t *line, svg_line_t *other);

svg_status_t
_svg_rect_init (svg_rect_element_t *rect);

svg_status_t
_svg_rect_init_copy (svg_rect_element_t *path, svg_rect_element_t *other);

svg_status_t
_svg_circle_apply_attributes (svg_element_t *circle_element, const svg_qattrs_t *attributes);

svg_status_t
_svg_ellipse_apply_attributes (svg_element_t *ellipse_element, const svg_qattrs_t *attributes);

svg_status_t
_svg_line_apply_attributes (svg_element_t *line_element, const svg_qattrs_t *attributes);

svg_status_t
_svg_rect_apply_attributes (svg_element_t *rect_element, const svg_qattrs_t *attributes);

int
_svg_circle_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_circle_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);

int
_svg_ellipse_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_ellipse_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);

int
_svg_line_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_line_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);

int
_svg_rect_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_rect_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_str.c */

void
_svg_str_skip_space (const char **str);

void
_svg_str_skip_char (const char **str, char c);

void
_svg_str_skip_space_or_char (const char **str, char c);

svgint_status_t
_svg_str_parse_csv_doubles (const char *str, double *value, int num_values, const char **end);

svg_status_t
_svg_str_parse_all_csv_doubles (const char *str, double **value, int *num_values, const char **end);

svg_status_t
_svg_str_parse_angle (const char *str, double *angle_degrees);


/* svg_style.c */

svg_status_t
_svg_style_init_empty (svg_style_t *style);

svg_status_t
_svg_style_init_copy (svg_style_t *style, svg_style_t *other);

svg_status_t
_svg_style_init_inherit (svg_style_t *style, svg_style_t *parent_style);

svg_status_t
_svg_style_deinit (svg_style_t *style);

void
_svg_style_reset_display (svg_style_t *style);

svg_status_t
_svg_style_render (svg_dom_node_t *node, svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_style_render_viewport_clipping_path (svg_style_t *style,
                                          svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_style_apply_properties (svg_dom_node_t *node, svg_t *svg);

double
_svg_style_get_opacity (svg_style_t *style);

int
_svg_style_get_display (svg_style_t *style);


/* svg_svg_group.c */

svg_status_t
_svg_svg_group_init (svg_svg_group_t *svg_group);

svg_status_t
_svg_svg_group_init_copy (svg_svg_group_t *svg_group, svg_svg_group_t *other);

svg_status_t
_svg_svg_group_deinit (svg_svg_group_t *svg_group);

svg_status_t
_svg_svg_group_apply_attributes (svg_element_t *svg_group_element, const svg_qattrs_t *attributes);

int
_svg_svg_group_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_svg_group_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_svg_group_get_size (svg_svg_group_t *svg_group, svg_length_t *width, svg_length_t *height);


/* svg_symbol.c */

svg_status_t
_svg_symbol_init (svg_symbol_t *symbol);

svg_status_t
_svg_symbol_init_copy (svg_symbol_t *symbol, svg_symbol_t *other);

svg_status_t
_svg_symbol_deinit (svg_symbol_t *symbol);

svg_status_t
_svg_symbol_apply_attributes (svg_element_t *symbol_element, const svg_qattrs_t *attributes);


/* svg_text.c */

svg_status_t
_svg_tspan_init (svg_tspan_t *tspan);

svg_status_t
_svg_tspan_init_copy (svg_tspan_t *tspan, svg_tspan_t *other);

svg_status_t
_svg_tspan_deinit (svg_tspan_t *tspan);

svg_status_t
_svg_tspan_append_chars (svg_tspan_t *tspan, const char *chars, size_t len);

svg_status_t
_svg_tspan_apply_attributes (svg_element_t *tspan_element, const svg_qattrs_t *attributes);

int
_svg_tspan_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_text_init (svg_text_t *text);

svg_status_t
_svg_text_init_copy (svg_text_t *text, svg_text_t *other);

svg_status_t
_svg_text_deinit (svg_text_t *text);

svg_status_t
_svg_text_append_chars (svg_text_t *text, const char *chars, int len);

int
_svg_text_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_text_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_text_apply_attributes (svg_element_t *text_element, const svg_qattrs_t *attributes);


/* svg_trace.c */

svg_status_t
_svg_create_trace (svg_trace_t **trace);

void
_svg_destroy_trace (svg_trace_t *trace);

svg_status_t
_svg_trace_push_target_engine (svg_trace_t *trace, const svg_uri_t *document_uri,
                               svg_render_engine_t *engine, void *closure);

void
_svg_trace_pop_target_engine (svg_trace_t *trace);

void
_svg_trace_set_position_info (svg_trace_t *trace, const char *node_name, long line_number);

void
_svg_trace_get_engine (svg_trace_t *trace, svg_render_engine_t **engine, void **closure);


/* svg_transform.c */

svg_status_t
_svg_transform_init (svg_transform_t *transform);

svg_status_t
_svg_transform_init_matrix (svg_transform_t *transform,
                            double xx, double yx,
                            double xy, double yy,
                            double x0, double y0);

svg_status_t
_svg_transform_init_translate (svg_transform_t *transform, double tx, double ty);

svg_status_t
_svg_transform_init_scale (svg_transform_t *transform, double sx, double sy);

svg_status_t
_svg_transform_init_rotate (svg_transform_t *transform, double angle_degrees);

svg_status_t
_svg_transform_init_skew_x (svg_transform_t *transform, double angle_degrees);

svg_status_t
_svg_transform_init_skew_y (svg_transform_t *transform, double angle_degrees);

svg_status_t
_svg_transform_add_translate (svg_transform_t *transform, double tx, double ty);

svg_status_t
_svg_transform_add_scale_uniform (svg_transform_t *transform, double s);

svg_status_t
_svg_transform_add_scale (svg_transform_t *transform, double sx, double sy);

svg_status_t
_svg_transform_parse_str (svg_transform_t *transform, const char *str);

svg_status_t
_svg_transform_add_rotate (svg_transform_t *transform, double angle_degrees);

svg_status_t
_svg_transform_add_skew_x (svg_transform_t *transform, double angle_degrees);

svg_status_t
_svg_transform_add_skew_y (svg_transform_t *transform, double angle_degrees);

svg_status_t
_svg_transform_coordinate (svg_transform_t *transform,
                           double x_in, double y_in, double *x_out, double *y_out);

svg_status_t
_svg_transform_multiply_into_left (svg_transform_t *t1, const svg_transform_t *t2);

svg_status_t
_svg_transform_multiply_into_right (const svg_transform_t *t1, svg_transform_t *t2);

svg_status_t
_svg_transform_render (svg_transform_t *transform, svg_render_engine_t *engine, void *closure);

svg_status_t
_svg_transform_apply_attributes (svg_element_t *element, svg_transform_t *transform,
                                 const svg_qattrs_t *attributes);


/* svg_uri.c */

svg_status_t
_svg_create_uri (const char *uri_str, svg_uri_t **uri);

svg_status_t
_svg_create_file_uri (const char *filename, svg_uri_t **uri);

svg_status_t
_svg_create_directory_uri (const char *directory, svg_uri_t **uri);

void
_svg_destroy_uri (svg_uri_t *uri);

svg_status_t
_svg_uri_clone (svg_uri_t *other, svg_uri_t **uri);

svg_status_t
_svg_uri_create_absolute (svg_uri_t *base_uri, svg_uri_t *uri, svg_uri_t **abs_uri);

svg_status_t
_svg_uri_create_relative (svg_uri_t *base_uri, svg_uri_t *uri, svg_uri_t **rel_uri);

int
_svg_uri_equals (const svg_uri_t *left, const svg_uri_t *right);

int
_svg_uri_is_data_scheme (const svg_uri_t *uri);

int
_svg_uri_is_file (const svg_uri_t *uri);

int
_svg_uri_is_relative (const svg_uri_t *uri);

int
_svg_uri_is_empty_path (const svg_uri_t *uri);

int
_svg_string_is_abs_filename (const char *uri_or_filename);

svg_status_t
_svg_uri_to_string (const svg_uri_t *uri, char **uri_str);

svg_status_t
_svg_uri_to_filename (const svg_uri_t *uri, char **filename);

const char *
_svg_uri_media_type (const svg_uri_t *uri);

void
_svg_uri_data (const svg_uri_t *uri, const char **data, int *base64);

svg_status_t
_svg_unescape_uri_string (char *uri_str, size_t *len);

void
_svg_uri_print (const svg_uri_t *uri);


/* svg_uri_reference.c */

svg_status_t
_svg_create_uri_ref (const char *uri_ref_str, svg_uri_t *base_uri, svg_uri_t *document_uri,
                     svg_uri_ref_t **uri_ref);

svg_status_t
_svg_create_uri_ref_url (const char *attribute, svg_uri_t *base_uri, svg_uri_t *document_uri,
                         svg_uri_ref_t **uri_ref);

void
_svg_destroy_uri_ref (svg_uri_ref_t *uri_ref);

int
_svg_is_uri_ref_url (const char *attribute);

void
_svg_uri_ref_print (svg_uri_ref_t *uri_ref);


/* svg_use.c */

svg_status_t
_svg_use_init (svg_use_t *use);

svg_status_t
_svg_use_init_copy (svg_use_t *use, svg_use_t *other);

svg_status_t
_svg_use_deinit (svg_use_t *use);

svg_status_t
_svg_use_apply_attributes (svg_element_t *use_element, const svg_qattrs_t *attributes);

int
_svg_use_peek_render (svg_element_t *element, svg_render_engine_t *engine);

svg_status_t
_svg_use_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);


/* svg_view_box.c */

svg_status_t
_svg_view_box_init (svg_view_box_t *view_box);

int
_svg_view_box_is_null (svg_view_box_t *view_box);

svg_status_t
_svg_parse_view_box (const char *view_box_str, svg_view_box_t *view_box);

svg_status_t
_svg_view_box_parse_aspect_ratio (const char *aspect_ratio_str, svg_view_box_t *view_box);

svg_status_t
_svg_view_box_transform (const svg_view_box_t *view_box,
                         double viewport_width, double viewport_height, svg_transform_t *transform);

#endif
