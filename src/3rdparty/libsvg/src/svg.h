/* svg.h: Public interface for libsvg

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

#ifndef _LIBSVG_SVG_H_
#define _LIBSVG_SVG_H_

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct svg svg_t;
typedef struct svg_element svg_element_t;
typedef struct svg_element_ref svg_element_ref_t;

typedef enum svg_status {
    SVG_STATUS_SUCCESS = 0,
    SVG_STATUS_NO_MEMORY,
    SVG_STATUS_IO_ERROR,
    SVG_STATUS_FILE_NOT_FOUND,
    SVG_STATUS_RESOURCE_NOT_FOUND,
    SVG_STATUS_INVALID_VALUE,
    SVG_STATUS_INVALID_BASE_URI,
    SVG_STATUS_DUPLICATE_ELEMENT_ID,
    SVG_STATUS_MISSING_REF_ELEMENT_ID,
    SVG_STATUS_RELATIVE_URI,
    SVG_STATUS_UNKNOWN_REF_ELEMENT,
    SVG_STATUS_WRONG_REF_ELEMENT_TYPE,
    SVG_STATUS_ROOT_ELEMENT_NOT_SVG,
    SVG_STATUS_MISSING_SVG_NAMESPACE_DECL,
    SVG_STATUS_EMPTY_DOCUMENT,
    SVG_STATUS_ATTRIBUTE_NOT_FOUND,
    SVG_STATUS_INVALID_CALL,
    SVG_STATUS_PARSE_ERROR,
    SVG_STATUS_XML_PARSE_ERROR,
    SVG_STATUS_XML_TAG_MISMATCH,
    SVG_STATUS_XML_UNBOUND_PREFIX,
    SVG_STATUS_XML_INVALID_TOKEN,
    SVG_STATUS_CSS_PARSE_ERROR,
    SVG_STATUS_NO_RELATIVE_URI,
    SVG_STATUS_EXTERNAL_DOCUMENT_ERROR,
    SVG_STATUS_INTERNAL_ERROR,
} svg_status_t;

typedef enum svg_length_unit {
    SVG_LENGTH_UNIT_CM,
    SVG_LENGTH_UNIT_EM,
    SVG_LENGTH_UNIT_EX,
    SVG_LENGTH_UNIT_IN,
    SVG_LENGTH_UNIT_MM,
    SVG_LENGTH_UNIT_PC,
    SVG_LENGTH_UNIT_PCT,
    SVG_LENGTH_UNIT_PT,
    SVG_LENGTH_UNIT_PX,
} svg_length_unit_t;

typedef enum svg_length_orientation {
    SVG_LENGTH_ORIENTATION_HORIZONTAL,
    SVG_LENGTH_ORIENTATION_VERTICAL,
    SVG_LENGTH_ORIENTATION_OTHER
} svg_length_orientation_t;

typedef struct svg_length {
    double value;
    svg_length_unit_t unit;
    svg_length_orientation_t orientation;
} svg_length_t;

typedef struct svg_length_context {
    double dpi;
    double font_size;
    double x_height;
    double viewport_width;
    double viewport_height;
    int bbox_user_units;
} svg_length_context_t;

typedef enum {
    SVG_PATH_OP_MOVE_TO,
    SVG_PATH_OP_LINE_TO,
    SVG_PATH_OP_CURVE_TO,
    SVG_PATH_OP_QUAD_CURVE_TO,
    SVG_PATH_OP_ARC_TO,
    SVG_PATH_OP_CLOSE_PATH
} svg_path_op_t;

typedef struct svg_path_move_to {
    double x;
    double y;
} svg_path_move_to_t;

typedef struct svg_path_line_to {
    double x;
    double y;
} svg_path_line_to_t;

typedef struct svg_path_curve_to {
    double x1;
    double y1;
    double x2;
    double y2;
    double x3;
    double y3;
} svg_path_curve_to_t;

typedef struct svg_path_quad_curve_to {
    double x1;
    double y1;
    double x2;
    double y2;
} svg_path_quad_curve_to_t;

typedef struct svg_path_arc_to {
    double rx;
    double ry;
    double x_axis_rotation;
    int large_arc_flag;
    int sweep_flag;
    double x;
    double y;
} svg_path_arc_to_t;

typedef struct svg_path {
    struct svg_path *next;
    svg_path_op_t op;
    union {
        svg_path_move_to_t move_to;
        svg_path_line_to_t line_to;
        svg_path_curve_to_t curve_to;
        svg_path_quad_curve_to_t quad_curve_to;
        svg_path_arc_to_t arc_to;
    } p;
} svg_path_t;

typedef struct svg_center_point_arc {
    double cx;
    double cy;
    double rx;
    double ry;
    double start_angle;
    double delta_angle;
} svg_center_point_arc_t;

typedef struct svg_transform {
    double xx;
    double yx;
    double xy;
    double yy;
    double x0;
    double y0;
} svg_transform_t;

typedef struct svg_color {
    int is_current_color;
    unsigned int rgb;
} svg_color_t;

typedef struct svg_rect {
    double x;
    double y;
    double width;
    double height;
} svg_rect_t;

typedef enum svg_preserve_aspect_ratio {
    SVG_PRESERVE_ASPECT_RATIO_UNKNOWN,
    SVG_PRESERVE_ASPECT_RATIO_NONE,
    SVG_PRESERVE_ASPECT_RATIO_XMINYMIN,
    SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN,
    SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN,
    SVG_PRESERVE_ASPECT_RATIO_XMINYMID,
    SVG_PRESERVE_ASPECT_RATIO_XMIDYMID,
    SVG_PRESERVE_ASPECT_RATIO_XMAXYMID,
    SVG_PRESERVE_ASPECT_RATIO_XMINYMAX,
    SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX,
    SVG_PRESERVE_ASPECT_RATIO_XMAXYMAX
} svg_preserve_aspect_ratio_t;

typedef enum svg_meet_or_slice {
    SVG_MEET_OR_SLICE_UNKNOWN,
    SVG_MEET_OR_SLICE_MEET,
    SVG_MEET_OR_SLICE_SLICE
} svg_meet_or_slice_t;

typedef struct svg_view_box {
    svg_rect_t box;
    svg_preserve_aspect_ratio_t aspect_ratio;
    svg_meet_or_slice_t meet_or_slice;
    int defer;
} svg_view_box_t;

typedef enum svg_fill_rule {
    SVG_FILL_RULE_NONZERO,
    SVG_FILL_RULE_EVEN_ODD
} svg_fill_rule_t;

typedef enum svg_font_style {
    SVG_FONT_STYLE_NORMAL,
    SVG_FONT_STYLE_ITALIC,
    SVG_FONT_STYLE_OBLIQUE
} svg_font_style_t;

typedef enum svg_stroke_line_cap {
    SVG_STROKE_LINE_CAP_BUTT,
    SVG_STROKE_LINE_CAP_ROUND,
    SVG_STROKE_LINE_CAP_SQUARE
} svg_stroke_line_cap_t;

typedef enum svg_stroke_line_join {
    SVG_STROKE_LINE_JOIN_BEVEL,
    SVG_STROKE_LINE_JOIN_MITER,
    SVG_STROKE_LINE_JOIN_ROUND
} svg_stroke_line_join_t;

typedef enum svg_text_anchor {
    SVG_TEXT_ANCHOR_START,
    SVG_TEXT_ANCHOR_MIDDLE,
    SVG_TEXT_ANCHOR_END
} svg_text_anchor_t;

typedef enum svg_coord_space_units {
    SVG_COORD_SPACE_UNITS_USER,
    SVG_COORD_SPACE_UNITS_BBOX
} svg_coord_space_units_t;

typedef enum svg_gradient_type_t {
    SVG_GRADIENT_LINEAR,
    SVG_GRADIENT_RADIAL
} svg_gradient_type_t;

typedef enum svg_gradient_spread {
    SVG_GRADIENT_SPREAD_PAD,
    SVG_GRADIENT_SPREAD_REPEAT,
    SVG_GRADIENT_SPREAD_REFLECT
} svg_gradient_spread_t;

typedef struct svg_gradient_stop {
    svg_color_t color;
    double offset;
    double opacity;
} svg_gradient_stop_t;

typedef struct svg_gradient {
    svg_gradient_type_t type;
    union {
        struct {
            svg_length_t x1;
            svg_length_t y1;
            svg_length_t x2;
            svg_length_t y2;
        } linear;
        struct {
            svg_length_t cx;
            svg_length_t cy;
            svg_length_t r;
            svg_length_t fx;
            svg_length_t fy;
        } radial;
    } u;
    svg_coord_space_units_t units;
    svg_gradient_spread_t spread;
    svg_transform_t transform;
    svg_gradient_stop_t *stops;
    int num_stops;
    int stops_size;
} svg_gradient_t;

typedef struct svg_pattern {
    svg_element_ref_t *element_ref;
    svg_coord_space_units_t units;
    svg_coord_space_units_t content_units;
    svg_length_t x;
    svg_length_t y;
    svg_length_t width;
    svg_length_t height;
    int have_viewbox;
    svg_transform_t transform;
} svg_pattern_t;

typedef enum svg_paint_type {
    SVG_PAINT_TYPE_NONE,
    SVG_PAINT_TYPE_COLOR,
    SVG_PAINT_TYPE_GRADIENT,
    SVG_PAINT_TYPE_PATTERN
} svg_paint_type_t;

typedef struct svg_paint {
    svg_paint_type_t type;
    union {
        svg_color_t color;
        svg_gradient_t *gradient;
        svg_pattern_t *pattern;
    } p;
} svg_paint_t;

typedef enum svg_clip_rule {
    SVG_CLIP_RULE_NONZERO,
    SVG_CLIP_RULE_EVEN_ODD
} svg_clip_rule_t;

typedef struct svg_clip_path {
    svg_element_ref_t *element_ref;
    svg_coord_space_units_t units;
} svg_clip_path_t;

typedef struct svg_mask {
    svg_element_ref_t *element_ref;
    svg_coord_space_units_t units;
    svg_coord_space_units_t content_units;
    svg_length_t x;
    svg_length_t y;
    svg_length_t width;
    svg_length_t height;
} svg_mask_t;

typedef struct svg_marker {
    svg_element_ref_t *element_ref;
    double angle;
    int auto_angle;
} svg_marker_t;



typedef struct svg_render_engine {
    /* hierarchy */
    svg_status_t (*begin_group) (void *closure, double opacity, const char *id, const char *klass);
    svg_status_t (*begin_element) (void *closure, const char *id, const char *klass);
    svg_status_t (*end_element) (void *closure);
    svg_status_t (*end_group) (void *closure, double opacity);
    /* transform */
    svg_status_t (*set_viewport) (void *closure, const svg_length_t *x, const svg_length_t *y,
                                  const svg_length_t *width, const svg_length_t *height);
    svg_status_t (*apply_view_box) (void *closure, const svg_view_box_t *view_box,
                                    const svg_length_t *width, const svg_length_t *height);
    svg_status_t (*viewport_clipping_path) (void *closure,
                                            const svg_length_t *top, const svg_length_t *right,
                                            const svg_length_t *bottom, const svg_length_t *left);
    svg_status_t (*transform) (void *closure, const svg_transform_t *transform);
    svg_status_t (*end_transform) (void *closure);
    /* style */
    svg_status_t (*set_clip_path) (void *closure, const svg_clip_path_t *clip_path);
    svg_status_t (*set_clip_rule) (void *closure, svg_clip_rule_t clip_rule);
    svg_status_t (*set_color) (void *closure, const svg_color_t *color);
    svg_status_t (*set_fill_opacity) (void *closure, double fill_opacity);
    svg_status_t (*set_fill_paint) (void *closure, const svg_paint_t *paint);
    svg_status_t (*set_fill_rule) (void *closure, svg_fill_rule_t fill_rule);
    svg_status_t (*set_font_family) (void *closure, const char *family);
    svg_status_t (*set_font_size) (void *closure, double size);
    svg_status_t (*set_font_style) (void *closure, svg_font_style_t font_style);
    svg_status_t (*set_font_weight) (void *closure, unsigned int font_weight);
    svg_status_t (*set_marker_end) (void *closure, const svg_marker_t *marker);
    svg_status_t (*set_marker_mid) (void *closure, const svg_marker_t *marker);
    svg_status_t (*set_marker_start) (void *closure, const svg_marker_t *marker);
    svg_status_t (*set_mask) (void *closure, const svg_mask_t *mask);
    svg_status_t (*set_opacity) (void *closure, double opacity);
    svg_status_t (*set_stroke_dash_array) (void *closure, const double *dash_array, int num_dashes);
    svg_status_t (*set_stroke_dash_offset) (void *closure, const svg_length_t *offset);
    svg_status_t (*set_stroke_line_cap) (void *closure, svg_stroke_line_cap_t line_cap);
    svg_status_t (*set_stroke_line_join) (void *closure, svg_stroke_line_join_t line_join);
    svg_status_t (*set_stroke_miter_limit) (void *closure, double limit);
    svg_status_t (*set_stroke_opacity) (void *closure, double stroke_opacity);
    svg_status_t (*set_stroke_paint) (void *closure, const svg_paint_t *paint);
    svg_status_t (*set_stroke_width) (void *closure, const svg_length_t *width);
    svg_status_t (*set_text_anchor) (void *closure, svg_text_anchor_t text_anchor);
    svg_status_t (*set_visibility) (void *closure, int visible);
    svg_status_t (*end_style) (void *closure);
    /* text positioning */
    svg_status_t (*text_advance_x) (void *closure, const char *utf8, double *advance);
    svg_status_t (*set_text_position_x) (void *closure, const svg_length_t *x);
    svg_status_t (*set_text_position_y) (void *closure, const svg_length_t *y);
    svg_status_t (*adjust_text_position) (void *closure,
                                          const svg_length_t *dx, const svg_length_t *dy);
    svg_status_t (*set_text_chunk_width) (void *closure, double width);
    /* drawing */
    svg_status_t (*render_line) (void *closure, const svg_length_t *x1, const svg_length_t *y1,
                                 const svg_length_t *x2, const svg_length_t *y2);
    svg_status_t (*render_path) (void *closure, const svg_path_t *path);
    svg_status_t (*render_circle) (void *closure, const svg_length_t *cx, const svg_length_t *cy,
                                   const svg_length_t *r);
    svg_status_t (*render_ellipse) (void *closure, const svg_length_t *cx, const svg_length_t *cy,
                                    const svg_length_t *rx, const svg_length_t *ry);
    svg_status_t (*render_rect) (void *closure, const svg_length_t *x, const svg_length_t *y,
                                 const svg_length_t *width, const svg_length_t *height,
                                 const svg_length_t *rx, const svg_length_t *ry);
    svg_status_t (*render_text) (void *closure, const char *utf8);
    svg_status_t (*render_image) (void *closure, const char *uri, int index,
                                  const svg_view_box_t *view_box_template,
                                  const svg_length_t *x, const svg_length_t *y,
                                  const svg_length_t *width, const svg_length_t *height);
    /* miscellaneous */
    svg_status_t (*measure_position) (void *closure, const svg_length_t *ix, const svg_length_t *iy,
                                      double *ox, double *oy);
    svg_status_t (*measure_font_size) (void *closure, const char *font_family,
                                       double parent_font_size, const svg_length_t *in_size,
                                       double *out_size);
} svg_render_engine_t;



svg_status_t
svg_create (svg_t **svg);

svg_status_t
svg_destroy (svg_t *svg);


svg_status_t
svg_trace_render_engine (svg_t *svg);

svg_status_t
svg_set_base_directory (svg_t *svg, const char *directory);

svg_status_t
svg_set_base_uri (svg_t *svg, const char *abs_uri);


svg_status_t
svg_parse (svg_t *svg, const char *uri_or_filename);

svg_status_t
svg_parse_file (svg_t *svg, FILE *file);

svg_status_t
svg_parse_buffer (svg_t *svg, const char *buf, size_t count);


svg_status_t
svg_parse_chunk_begin (svg_t *svg);

svg_status_t
svg_parse_chunk (svg_t *svg, const char *buf, size_t count);

svg_status_t
svg_parse_chunk_end (svg_t *svg);


svg_status_t
svg_render (svg_t *svg, svg_render_engine_t *engine, void *closure);

svg_status_t
svg_element_render (svg_element_t *element, svg_render_engine_t *engine, void *closure);

svg_status_t
svg_element_ref_render (svg_element_ref_t *element_ref, svg_render_engine_t *engine, void *closure);

svg_status_t
svg_marker_render (const svg_marker_t *marker, double stroke_width, svg_render_engine_t *engine,
                   void *closure);


double
svg_get_dpi (const svg_t *svg);

void
svg_get_size (const svg_t *svg, svg_length_t *width, svg_length_t *height);

void
svg_get_image_uris (const svg_t *svg, const char ***image_uris, int *num_images);



double
svg_convert_length_to_user_units (const svg_length_context_t *context, const svg_length_t *length);


unsigned int
svg_color_get_red (const svg_color_t *color);

unsigned int
svg_color_get_green (const svg_color_t *color);

unsigned int
svg_color_get_blue (const svg_color_t *color);



svg_status_t
svg_get_bgra_image (const char *uri, unsigned char **data, unsigned int *data_width,
                    unsigned int *data_height);

svg_status_t
svg_get_bgra_image_from_buffer (const char *media_type, const unsigned char *buffer,
                                size_t buffer_size, unsigned char **data, unsigned int *data_width,
                                unsigned int *data_height);

svg_status_t
svg_get_image_filename (const char *uri, char **filename, int *is_temp_copy);

int
svg_is_data_scheme_uri (const char *uri);

svg_status_t
svg_decode_data_scheme_uri (const char *uri, char **media_type, unsigned char **buffer,
                            size_t *buffer_size);

svg_status_t
svg_get_relative_uri (svg_t *svg, const char *uri, char **rel_uri);


void
svg_complete_image_viewbox (svg_view_box_t *view_box, double image_width, double image_height);

svg_status_t
svg_get_viewbox_transform (const svg_view_box_t *view_box, double viewport_width,
                           double viewport_height, svg_transform_t *transform);


int
svg_center_point_param_arc (double x1, double y1, const svg_path_arc_to_t *e_arc,
                            svg_center_point_arc_t *c_arc);


void
svg_legalize_radial_gradient_focal_point (double cx, double cy, double r, double fx_in,
                                          double fy_in, double *fx_out, double *fy_out);


svg_status_t
svg_get_error_status (const svg_t *svg);

svg_status_t
svg_get_external_error_status (const svg_t *svg);

const char *
svg_get_error_string (svg_status_t status);

long
svg_get_error_line_number (const svg_t *svg);

const char *
svg_get_error_element (const svg_t *svg);

const char *
svg_get_error_property (const svg_t *svg);

const char *
svg_get_error_attribute (const svg_t *svg);


#ifdef __cplusplus
}
#endif
#endif

