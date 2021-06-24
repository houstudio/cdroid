/* svg_render_engine.c: Wrapper functions for render engine callback struct

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

   Author: Philip de Nier
*/

#include "svgint.h"


svg_status_t
_svg_engine_begin_group (svg_render_engine_t *engine, void *closure, double opacity, const char *id,
                         const char *klass)
{
    if (engine->begin_group == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->begin_group) (closure, opacity, id, klass);
}

svg_status_t
_svg_engine_begin_element (svg_render_engine_t *engine, void *closure, const char *id,
                           const char *klass)
{
    if (engine->begin_element == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->begin_element) (closure, id, klass);
}

svg_status_t
_svg_engine_end_element (svg_render_engine_t *engine, void *closure)
{
    if (engine->end_element == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->end_element) (closure);
}

svg_status_t
_svg_engine_end_group (svg_render_engine_t *engine, void *closure, double opacity)
{
    if (engine->end_group == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->end_group) (closure, opacity);
}

svg_status_t
_svg_engine_set_viewport (svg_render_engine_t *engine, void *closure, const svg_length_t *x,
                          const svg_length_t *y, const svg_length_t *width,
                          const svg_length_t *height)
{
    if (engine->set_viewport == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_viewport) (closure, x, y, width, height);
}

svg_status_t
_svg_engine_apply_view_box (svg_render_engine_t *engine, void *closure,
                            const svg_view_box_t *view_box, const svg_length_t *width,
                            const svg_length_t *height)
{
    if (engine->apply_view_box == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->apply_view_box) (closure, view_box, width, height);
}

svg_status_t
_svg_engine_viewport_clipping_path (svg_render_engine_t *engine, void *closure,
                                    const svg_length_t *top, const svg_length_t *right,
                                    const svg_length_t *bottom, const svg_length_t *left)
{
    if (engine->viewport_clipping_path == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->viewport_clipping_path) (closure, top, right, bottom, left);
}

svg_status_t
_svg_engine_transform (svg_render_engine_t *engine, void *closure, const svg_transform_t *transform)
{
    if (engine->transform == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->transform) (closure, transform);
}

svg_status_t
_svg_engine_end_transform (svg_render_engine_t *engine, void *closure)
{
    if (engine->end_transform == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->end_transform) (closure);
}

svg_status_t
_svg_engine_set_clip_path (svg_render_engine_t *engine, void *closure,
                           const svg_clip_path_t *clip_path)
{
    if (engine->set_clip_path == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_clip_path) (closure, clip_path);
}

svg_status_t
_svg_engine_set_clip_rule (svg_render_engine_t *engine, void *closure, svg_clip_rule_t clip_rule)
{
    if (engine->set_clip_rule == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_clip_rule) (closure, clip_rule);
}

svg_status_t
_svg_engine_set_color (svg_render_engine_t *engine, void *closure, const svg_color_t *color)
{
    if (engine->set_color == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_color) (closure, color);
}

svg_status_t
_svg_engine_set_fill_opacity (svg_render_engine_t *engine, void *closure, double fill_opacity)
{
    if (engine->set_fill_opacity == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_fill_opacity) (closure, fill_opacity);
}

svg_status_t
_svg_engine_set_fill_paint (svg_render_engine_t *engine, void *closure, const svg_paint_t *paint)
{
    if (engine->set_fill_paint == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_fill_paint) (closure, paint);
}

svg_status_t
_svg_engine_set_fill_rule (svg_render_engine_t *engine, void *closure, svg_fill_rule_t fill_rule)
{
    if (engine->set_fill_rule == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_fill_rule) (closure, fill_rule);
}

svg_status_t
_svg_engine_set_font_family (svg_render_engine_t *engine, void *closure, const char *family)
{
    if (engine->set_font_family == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_font_family) (closure, family);
}

svg_status_t
_svg_engine_set_font_size (svg_render_engine_t *engine, void *closure, double size)
{
    if (engine->set_font_size == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_font_size) (closure, size);
}

svg_status_t
_svg_engine_set_font_style (svg_render_engine_t *engine,
                            void *closure, svg_font_style_t font_style)
{
    if (engine->set_font_style == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_font_style) (closure, font_style);
}

svg_status_t
_svg_engine_set_font_weight (svg_render_engine_t *engine, void *closure, unsigned int font_weight)
{
    if (engine->set_font_weight == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_font_weight) (closure, font_weight);
}

svg_status_t
_svg_engine_set_marker_end (svg_render_engine_t *engine,
                            void *closure, const svg_marker_t *marker)
{
    if (engine->set_marker_end == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_marker_end) (closure, marker);
}

svg_status_t
_svg_engine_set_marker_mid (svg_render_engine_t *engine,
                            void *closure, const svg_marker_t *marker)
{
    if (engine->set_marker_mid == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_marker_mid) (closure, marker);
}

svg_status_t
_svg_engine_set_marker_start (svg_render_engine_t *engine,
                              void *closure, const svg_marker_t *marker)
{
    if (engine->set_marker_start == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_marker_start) (closure, marker);
}

svg_status_t
_svg_engine_set_mask (svg_render_engine_t *engine, void *closure, const svg_mask_t *mask)
{
    if (engine->set_mask == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_mask) (closure, mask);
}

svg_status_t
_svg_engine_set_opacity (svg_render_engine_t *engine, void *closure, double opacity)
{
    if (engine->set_opacity == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_opacity) (closure, opacity);
}

svg_status_t
_svg_engine_set_stroke_dash_array (svg_render_engine_t *engine, void *closure,
                                   const double *dash_array, int num_dashes)
{
    if (engine->set_stroke_dash_array == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_dash_array) (closure, dash_array, num_dashes);
}

svg_status_t
_svg_engine_set_stroke_dash_offset (svg_render_engine_t *engine, void *closure,
                                    const svg_length_t *offset)
{
    if (engine->set_stroke_dash_offset == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_dash_offset) (closure, offset);
}

svg_status_t
_svg_engine_set_stroke_line_cap (svg_render_engine_t *engine, void *closure,
                                 svg_stroke_line_cap_t line_cap)
{
    if (engine->set_stroke_line_cap == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_line_cap) (closure, line_cap);
}

svg_status_t
_svg_engine_set_stroke_line_join (svg_render_engine_t *engine, void *closure,
                                  svg_stroke_line_join_t line_join)
{
    if (engine->set_stroke_line_join == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_line_join) (closure, line_join);
}

svg_status_t
_svg_engine_set_stroke_miter_limit (svg_render_engine_t *engine, void *closure, double limit)
{
    if (engine->set_stroke_miter_limit == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_miter_limit) (closure, limit);
}

svg_status_t
_svg_engine_set_stroke_opacity (svg_render_engine_t *engine, void *closure, double stroke_opacity)
{
    if (engine->set_stroke_opacity == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_opacity) (closure, stroke_opacity);
}

svg_status_t
_svg_engine_set_stroke_paint (svg_render_engine_t *engine, void *closure, const svg_paint_t *paint)
{
    if (engine->set_stroke_paint == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_paint) (closure, paint);
}

svg_status_t
_svg_engine_set_stroke_width (svg_render_engine_t *engine, void *closure, const svg_length_t *width)
{
    if (engine->set_stroke_width == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_stroke_width) (closure, width);
}

svg_status_t
_svg_engine_set_text_anchor (svg_render_engine_t *engine, void *closure,
                             svg_text_anchor_t text_anchor)
{
    if (engine->set_text_anchor == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_text_anchor) (closure, text_anchor);
}

svg_status_t
_svg_engine_set_visibility (svg_render_engine_t *engine, void *closure, int visible)
{
    if (engine->set_visibility == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_visibility) (closure, visible);
}

svg_status_t
_svg_engine_end_style (svg_render_engine_t *engine, void *closure)
{
    if (engine->end_style == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->end_style) (closure);
}

svg_status_t
_svg_engine_text_advance_x (svg_render_engine_t *engine, void *closure, const char *utf8,
                            double *advance)
{
    if (engine->text_advance_x == NULL) {
        *advance = 0;
        return SVG_STATUS_SUCCESS;
    }

    return (engine->text_advance_x) (closure, utf8, advance);
}

svg_status_t
_svg_engine_set_text_position_x (svg_render_engine_t *engine, void *closure, const svg_length_t *x)
{
    if (engine->set_text_position_x == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_text_position_x) (closure, x);
}

svg_status_t
_svg_engine_set_text_position_y (svg_render_engine_t *engine, void *closure, const svg_length_t *y)
{
    if (engine->set_text_position_y == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_text_position_y) (closure, y);
}

svg_status_t
_svg_engine_adjust_text_position (svg_render_engine_t *engine, void *closure,
                                  const svg_length_t *dx, const svg_length_t *dy)
{
    if (engine->adjust_text_position == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->adjust_text_position) (closure, dx, dy);
}

svg_status_t
_svg_engine_set_text_chunk_width (svg_render_engine_t *engine, void *closure, double width)
{
    if (engine->set_text_chunk_width == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->set_text_chunk_width) (closure, width);
}

svg_status_t
_svg_engine_render_line (svg_render_engine_t *engine, void *closure, const svg_length_t *x1,
                         const svg_length_t *y1, const svg_length_t *x2, const svg_length_t *y2)
{
    if (engine->render_line == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->render_line) (closure, x1, y1, x2, y2);
}

svg_status_t
_svg_engine_render_path (svg_render_engine_t *engine, void *closure, const svg_path_t *ext_path)
{
    if (engine->render_path == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->render_path) (closure, ext_path);
}

svg_status_t
_svg_engine_render_circle (svg_render_engine_t *engine, void *closure, const svg_length_t *cx,
                           const svg_length_t *cy, const svg_length_t *r)
{
    if (engine->render_circle == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->render_circle) (closure, cx, cy, r);
}

svg_status_t
_svg_engine_render_ellipse (svg_render_engine_t *engine, void *closure, const svg_length_t *cx,
                            const svg_length_t *cy, const svg_length_t *rx, const svg_length_t *ry)
{
    if (engine->render_ellipse == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->render_ellipse) (closure, cx, cy, rx, ry);
}

svg_status_t
_svg_engine_render_rect (svg_render_engine_t *engine, void *closure, const svg_length_t *x,
                         const svg_length_t *y, const svg_length_t *width,
                         const svg_length_t *height, const svg_length_t *rx, const svg_length_t *ry)
{
    if (engine->render_rect == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->render_rect) (closure, x, y, width, height, rx, ry);
}

svg_status_t
_svg_engine_render_text (svg_render_engine_t *engine, void *closure, const char *utf8)
{
    if (engine->render_text == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->render_text) (closure, utf8);
}

svg_status_t
_svg_engine_render_image (svg_render_engine_t *engine, void *closure, const char *uri, int index,
                          const svg_view_box_t *view_box_template, const svg_length_t *x,
                          const svg_length_t *y, const svg_length_t *width,
                          const svg_length_t *height)
{
    if (engine->render_image == NULL)
        return SVG_STATUS_SUCCESS;

    return (engine->render_image) (closure, uri, index, view_box_template, x, y, width, height);
}

svg_status_t
_svg_engine_measure_position (svg_render_engine_t *engine, void *closure, const svg_length_t *ix,
                              const svg_length_t *iy, double *ox, double *oy)
{
    if (engine->measure_position == NULL) {
        *ox = 0;
        *oy = 0;
        return SVG_STATUS_SUCCESS;
    }

    return (engine->measure_position) (closure, ix, iy, ox, oy);
}

svg_status_t
_svg_engine_measure_font_size (svg_render_engine_t *engine, void *closure, const char *font_family,
                               double parent_font_size, const svg_length_t *in_size,
                               double *out_size)
{
    if (engine->measure_font_size == NULL) {
        *out_size = 0;
        return SVG_STATUS_SUCCESS;
    }

    return (engine->measure_font_size) (closure, font_family, parent_font_size, in_size, out_size);
}

int
_svg_engine_support_render_line (svg_render_engine_t *engine)
{
    return engine->render_line != NULL;
}

int
_svg_engine_support_render_path (svg_render_engine_t *engine)
{
    return engine->render_path != NULL;
}

int
_svg_engine_support_render_circle (svg_render_engine_t *engine)
{
    return engine->render_circle != NULL;
}

int
_svg_engine_support_render_ellipse (svg_render_engine_t *engine)
{
    return engine->render_ellipse != NULL;
}

int
_svg_engine_support_render_rect (svg_render_engine_t *engine)
{
    return engine->render_rect != NULL;
}

int
_svg_engine_support_render_text (svg_render_engine_t *engine)
{
    return engine->render_text != NULL;
}

int
_svg_engine_support_render_image (svg_render_engine_t *engine)
{
    return engine->render_image != NULL;
}

