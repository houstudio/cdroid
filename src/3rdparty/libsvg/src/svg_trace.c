/* svg_trace.c: Trace render engine calls

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


static void
_svg_trace_print_indent (int level)
{
    int i;

    for (i = 0; i < level; i++)
        printf ("  ");
}

static void
_svg_trace_print_length (const svg_length_t *length)
{
    printf ("%f", length->value);

    switch (length->unit) {
    case SVG_LENGTH_UNIT_CM:
        printf ("cm");
        break;
    case SVG_LENGTH_UNIT_EM:
        printf ("em");
        break;
    case SVG_LENGTH_UNIT_EX:
        printf ("ex");
        break;
    case SVG_LENGTH_UNIT_IN:
        printf ("in");
        break;
    case SVG_LENGTH_UNIT_MM:
        printf ("mm");
        break;
    case SVG_LENGTH_UNIT_PC:
        printf ("pc");
        break;
    case SVG_LENGTH_UNIT_PCT:
        printf ("%%");
        break;
    case SVG_LENGTH_UNIT_PT:
        printf ("pt");
        break;
    case SVG_LENGTH_UNIT_PX:
        printf ("px");
        break;
    }
}

static void
_svg_trace_print_transform (const svg_transform_t *transform)
{
    printf ("[%f %f %f %f %f %f]",
            transform->xx, transform->yx,
            transform->xy, transform->yy,
            transform->x0, transform->y0);
};

static void
_svg_trace_print_element_ref (const svg_element_ref_t *element_ref)
{
    if (element_ref->element->id != NULL) {
        if (_svg_uri_equals (element_ref->element->node->document_uri,
                             element_ref->element->svg->document_uri))
        {
            printf ("ref(");
            printf ("#%s", element_ref->element->id);
            printf (")");
        }
        else
        {
            printf ("ref(");
            _svg_uri_print (element_ref->element->node->document_uri);
            printf ("#%s", element_ref->element->id);
            printf (")");
        }
    } else {
        printf ("element");
    }
}

static void
_svg_trace_print_coord_space_units (svg_coord_space_units_t units)
{
    switch (units) {
    case SVG_COORD_SPACE_UNITS_USER:
        printf ("userSpaceOnUse");
        return;
    case SVG_COORD_SPACE_UNITS_BBOX:
        printf ("objectBoundingBox");
        return;
    }
}

static void
_svg_trace_print_color (const svg_color_t *color)
{
    if (color->is_current_color)
        printf ("currentColor");
    else
        printf ("#%02x%02x%02x", svg_color_get_red (color),
                svg_color_get_green (color), svg_color_get_blue (color));
}

static void
_svg_trace_print_gradient_stops (const svg_gradient_t *gradient)
{
    int i;

    printf ("{");
    for (i = 0; i < gradient->num_stops; i++) {
        if (i != 0)
            printf (", ");
        printf ("{");
        _svg_trace_print_color (&gradient->stops[i].color);
        printf (", %f, %f", gradient->stops[i].offset, gradient->stops[i].opacity);
        printf ("}");
    }
    printf ("}");
}

static void
_svg_trace_print_gradient (const svg_gradient_t *gradient)
{
    switch (gradient->type) {
    case SVG_GRADIENT_LINEAR:
        printf ("linear, ");
        printf ("{");
        _svg_trace_print_length (&gradient->u.linear.x1);
        printf (", ");
        _svg_trace_print_length (&gradient->u.linear.y1);
        printf (", ");
        _svg_trace_print_length (&gradient->u.linear.x2);
        printf (", ");
        _svg_trace_print_length (&gradient->u.linear.y2);
        printf ("}");
        break;
    case SVG_GRADIENT_RADIAL:
        printf ("radial, ");
        printf ("{");
        _svg_trace_print_length (&gradient->u.radial.cx);
        printf (", ");
        _svg_trace_print_length (&gradient->u.radial.cy);
        printf (", ");
        _svg_trace_print_length (&gradient->u.radial.r);
        printf (", ");
        _svg_trace_print_length (&gradient->u.radial.fx);
        printf (", ");
        _svg_trace_print_length (&gradient->u.radial.fy);
        printf ("}");
        break;
    }
    printf (", ");

    _svg_trace_print_coord_space_units (gradient->units);
    printf (", ");

    switch (gradient->spread) {
    case SVG_GRADIENT_SPREAD_PAD:
        printf ("pad, ");
        break;
    case SVG_GRADIENT_SPREAD_REPEAT:
        printf ("repeat, ");
        break;
    case SVG_GRADIENT_SPREAD_REFLECT:
        printf ("reflect, ");
        break;
    }

    _svg_trace_print_transform (&gradient->transform);
    printf (", ");

    _svg_trace_print_gradient_stops (gradient);
}

static void
_svg_trace_print_pattern (const svg_pattern_t *pattern)
{
    _svg_trace_print_element_ref (pattern->element_ref);
    printf (", ");

    _svg_trace_print_coord_space_units (pattern->units);
    printf (", ");

    _svg_trace_print_coord_space_units (pattern->content_units);
    printf (", ");

    printf ("{");
    _svg_trace_print_length (&pattern->x);
    printf (", ");
    _svg_trace_print_length (&pattern->y);
    printf ("}, ");
    printf ("{");
    _svg_trace_print_length (&pattern->width);
    printf (", ");
    _svg_trace_print_length (&pattern->height);
    printf ("}, ");

    printf ("%s, ", (pattern->have_viewbox ? "viewbox" : "NULL"));

    _svg_trace_print_transform (&pattern->transform);
}

static void
_svg_trace_print_paint (const svg_paint_t *paint)
{
    switch (paint->type) {
    case SVG_PAINT_TYPE_NONE:
        printf ("none");
        break;
    case SVG_PAINT_TYPE_COLOR:
        printf ("color, ");
        _svg_trace_print_color (&paint->p.color);
        break;
    case SVG_PAINT_TYPE_GRADIENT:
        printf ("gradient, ");
        _svg_trace_print_gradient (paint->p.gradient);
        break;
    case SVG_PAINT_TYPE_PATTERN:
        printf ("pattern, ");
        _svg_trace_print_pattern (paint->p.pattern);
        break;
    }
}

static void
_svg_trace_print_marker (const svg_marker_t *marker)
{
    _svg_trace_print_element_ref (marker->element_ref);
    printf (", ");

    if (marker->auto_angle)
        printf ("auto");
    else
        printf ("%fdeg", marker->angle);
}

static void
_svg_trace_print_view_box (const svg_view_box_t *view_box)
{
    printf ("{{%f, %f, %f, %f}, ",
            view_box->box.x, view_box->box.y, view_box->box.width, view_box->box.height);

    switch (view_box->aspect_ratio) {
    case SVG_PRESERVE_ASPECT_RATIO_XMINYMIN:
        printf ("xMinYMin");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN:
        printf ("xMidYMin");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN:
        printf ("xMaxYMin");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMINYMID:
        printf ("xMinYMid");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMIDYMID:
        printf ("xMidYMid");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMAXYMID:
        printf ("xMaxYMid");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMINYMAX:
        printf ("xMinYMax");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX:
        printf ("xMidYMax");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_XMAXYMAX:
        printf ("xMaxYMax");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_UNKNOWN:
        printf ("unknown");
        break;
    case SVG_PRESERVE_ASPECT_RATIO_NONE:
        printf ("none");
        break;
    }

    if (view_box->aspect_ratio != SVG_PRESERVE_ASPECT_RATIO_NONE &&
        view_box->aspect_ratio != SVG_PRESERVE_ASPECT_RATIO_UNKNOWN)
    {
        switch (view_box->meet_or_slice) {
        case SVG_MEET_OR_SLICE_MEET:
            printf (", meet");
            break;
        case SVG_MEET_OR_SLICE_SLICE:
            printf (", slice");
            break;
        case SVG_MEET_OR_SLICE_UNKNOWN:
            printf (", unknown");
            break;
        }
    }

    printf ("}");
}




static svg_status_t
_svg_trace_begin_group (void *closure, double opacity, const char *id, const char *klass)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("begin_group(%f, ", opacity);
    if (id != NULL)
        printf ("'%s', ", id);
    else
        printf ("NULL, ");
    if (klass != NULL)
        printf ("'%s'", klass);
    else
        printf ("NULL");
    printf (")");

    if (trace->node_name != NULL)
        printf ("  <%s, %ld>\n", trace->node_name, trace->line_number);
    else
        printf ("  <NULL, %ld>\n", trace->line_number);

    trace->level++;

    return _svg_engine_begin_group (trace->targets->target_engine, trace->targets->target_closure,
                                    opacity, id, klass);
}

static svg_status_t
_svg_trace_begin_element (void *closure, const char *id, const char *klass)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("begin_element(");
    if (id != NULL)
        printf ("'%s', ", id);
    else
        printf ("NULL, ");
    if (klass != NULL)
        printf ("'%s'", klass);
    else
        printf ("NULL");
    printf (")");

    if (trace->node_name != NULL)
        printf ("  <%s, %ld>\n", trace->node_name, trace->line_number);
    else
        printf ("  <NULL, %ld>\n", trace->line_number);

    trace->level++;

    return _svg_engine_begin_element (trace->targets->target_engine, trace->targets->target_closure,
                                      id, klass);
}

static svg_status_t
_svg_trace_end_element (void *closure)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    trace->level--;

    _svg_trace_print_indent (trace->level);

    printf ("end_element()\n");

    return _svg_engine_end_element (trace->targets->target_engine, trace->targets->target_closure);
}

static svg_status_t
_svg_trace_end_group (void *closure, double opacity)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    trace->level--;

    _svg_trace_print_indent (trace->level);

    printf ("end_group(%f)\n", opacity);

    return _svg_engine_end_group (trace->targets->target_engine, trace->targets->target_closure,
                                  opacity);
}

static svg_status_t
_svg_trace_set_viewport (void *closure, const svg_length_t *x, const svg_length_t *y,
                         const svg_length_t *width, const svg_length_t *height)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_viewport(");
    _svg_trace_print_length (x);
    printf (", ");
    _svg_trace_print_length (y);
    printf (", ");
    _svg_trace_print_length (width);
    printf (", ");
    _svg_trace_print_length (height);
    printf (")\n");

    return _svg_engine_set_viewport (trace->targets->target_engine, trace->targets->target_closure,
                                     x, y, width, height);
}

static svg_status_t
_svg_trace_apply_view_box (void *closure, const svg_view_box_t *view_box,
                           const svg_length_t *width, const svg_length_t *height)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("apply_view_box(");
    _svg_trace_print_view_box (view_box);
    printf (", ");
    _svg_trace_print_length (width);
    printf (", ");
    _svg_trace_print_length (height);
    printf (")\n");

    return _svg_engine_apply_view_box (trace->targets->target_engine,
                                       trace->targets->target_closure, view_box, width, height);
}

static svg_status_t
_svg_trace_viewport_clipping_path (void *closure,
                                   const svg_length_t *top, const svg_length_t *right,
                                   const svg_length_t *bottom, const svg_length_t *left)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("viewport_clipping_path(");
    _svg_trace_print_length (top);
    printf (", ");
    _svg_trace_print_length (right);
    printf (", ");
    _svg_trace_print_length (bottom);
    printf (", ");
    _svg_trace_print_length (left);
    printf (")\n");

    return _svg_engine_viewport_clipping_path (trace->targets->target_engine,
                                               trace->targets->target_closure, top, right, bottom,
                                               left);
}

static svg_status_t
_svg_trace_transform (void *closure, const svg_transform_t *transform)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("transform(");
    _svg_trace_print_transform (transform);
    printf (")\n");

    return _svg_engine_transform (trace->targets->target_engine, trace->targets->target_closure,
                                  transform);
}

static svg_status_t
_svg_trace_end_transform (void *closure)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    return _svg_engine_end_transform (trace->targets->target_engine,
                                      trace->targets->target_closure);
}

static svg_status_t
_svg_trace_set_clip_path (void *closure, const svg_clip_path_t *clip_path)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_clip_path(");
    if (clip_path != NULL) {
        _svg_trace_print_element_ref (clip_path->element_ref);
        printf (", ");
        _svg_trace_print_coord_space_units (clip_path->units);
    } else {
        printf ("NULL");
    }
    printf (")\n");

    return _svg_engine_set_clip_path (trace->targets->target_engine, trace->targets->target_closure,
                                      clip_path);
}

static svg_status_t
_svg_trace_set_clip_rule (void *closure, svg_clip_rule_t clip_rule)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_clip_rule(");
    switch (clip_rule) {
    case SVG_CLIP_RULE_NONZERO:
        printf ("non-zero");
        break;
    case SVG_CLIP_RULE_EVEN_ODD:
        printf ("even-odd");
        break;
    }
    printf (")\n");

    return _svg_engine_set_clip_rule (trace->targets->target_engine, trace->targets->target_closure,
                                      clip_rule);
}

static svg_status_t
_svg_trace_set_color (void *closure, const svg_color_t *color)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_color(");
    _svg_trace_print_color (color);
    printf (")\n");

    return _svg_engine_set_color (trace->targets->target_engine, trace->targets->target_closure,
                                  color);
}

static svg_status_t
_svg_trace_set_fill_opacity (void *closure, double fill_opacity)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_fill_opacity(%f)\n", fill_opacity);

    return _svg_engine_set_fill_opacity (trace->targets->target_engine,
                                         trace->targets->target_closure, fill_opacity);
}

static svg_status_t
_svg_trace_set_fill_paint (void *closure, const svg_paint_t *paint)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_fill_paint(");
    _svg_trace_print_paint (paint);
    printf (")\n");

    return _svg_engine_set_fill_paint (trace->targets->target_engine,
                                       trace->targets->target_closure, paint);
}

static svg_status_t
_svg_trace_set_fill_rule (void *closure, svg_fill_rule_t fill_rule)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_fill_rule(");
    switch (fill_rule) {
    case SVG_FILL_RULE_NONZERO:
        printf ("non-zero");
        break;
    case SVG_FILL_RULE_EVEN_ODD:
        printf ("even-odd");
        break;
    }
    printf (")\n");

    return _svg_engine_set_fill_rule (trace->targets->target_engine, trace->targets->target_closure,
                                      fill_rule);
}

static svg_status_t
_svg_trace_set_font_family (void *closure, const char *family)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_font_family(");
    if (family != NULL)
        printf ("'%s'", family);
    else
        printf ("NULL");
    printf (")\n");

    return _svg_engine_set_font_family (trace->targets->target_engine,
                                        trace->targets->target_closure, family);
}

static svg_status_t
_svg_trace_set_font_size (void *closure, double size)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_font_size(%f)\n", size);

    return _svg_engine_set_font_size (trace->targets->target_engine, trace->targets->target_closure,
                                      size);
}

static svg_status_t
_svg_trace_set_font_style (void *closure, svg_font_style_t font_style)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_font_style(");
    switch (font_style) {
    case SVG_FONT_STYLE_NORMAL:
        printf ("normal");
        break;
    case SVG_FONT_STYLE_ITALIC:
        printf ("italic");
        break;
    case SVG_FONT_STYLE_OBLIQUE:
        printf ("oblique");
        break;
    }
    printf (")\n");

    return _svg_engine_set_font_style (trace->targets->target_engine,
                                       trace->targets->target_closure, font_style);
}

static svg_status_t
_svg_trace_set_font_weight (void *closure, unsigned int font_weight)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_font_weight(%d)\n", font_weight);

    return _svg_engine_set_font_weight (trace->targets->target_engine,
                                        trace->targets->target_closure, font_weight);
}

static svg_status_t
_svg_trace_set_marker_end (void *closure, const svg_marker_t *marker)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_marker_end(");
    if (marker != NULL)
        _svg_trace_print_marker (marker);
    else
        printf ("NULL");
    printf (")\n");

    return _svg_engine_set_marker_end (trace->targets->target_engine,
                                       trace->targets->target_closure, marker);
}

static svg_status_t
_svg_trace_set_marker_mid (void *closure, const svg_marker_t *marker)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_marker_mid(");
    if (marker != NULL)
        _svg_trace_print_marker (marker);
    else
        printf ("NULL");
    printf (")\n");

    return _svg_engine_set_marker_mid (trace->targets->target_engine,
                                       trace->targets->target_closure, marker);
}

static svg_status_t
_svg_trace_set_marker_start (void *closure, const svg_marker_t *marker)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_marker_start(");
    if (marker != NULL)
        _svg_trace_print_marker (marker);
    else
        printf ("NULL");
    printf (")\n");

    return _svg_engine_set_marker_start (trace->targets->target_engine,
                                         trace->targets->target_closure, marker);
}

static svg_status_t
_svg_trace_set_mask (void *closure, const svg_mask_t *mask)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_mask(");
    if (mask != NULL) {
        _svg_trace_print_element_ref (mask->element_ref);
        printf (", ");
        _svg_trace_print_coord_space_units (mask->units);
        printf (", ");
        _svg_trace_print_coord_space_units (mask->content_units);
        printf (", {");
        _svg_trace_print_length (&mask->x);
        printf (", ");
        _svg_trace_print_length (&mask->y);
        printf ("}, {");
        _svg_trace_print_length (&mask->width);
        printf (", ");
        _svg_trace_print_length (&mask->height);
        printf ("}");
    } else {
        printf ("NULL");
    }
    printf (")\n");


    return _svg_engine_set_mask (trace->targets->target_engine, trace->targets->target_closure,
                                 mask);
}

static svg_status_t
_svg_trace_set_opacity (void *closure, double opacity)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_opacity(%f)\n", opacity);

    return _svg_engine_set_opacity (trace->targets->target_engine, trace->targets->target_closure,
                                    opacity);
}

static svg_status_t
_svg_trace_set_stroke_dash_array (void *closure, const double *dash_array, int num_dashes)
{
    svg_trace_t *trace = (svg_trace_t *) closure;
    int i;

    _svg_trace_print_indent (trace->level);

    if (dash_array == NULL) {
        printf ("set_stroke_dash_array(NULL)\n");
    } else {
        printf ("set_stroke_dash_array(");
        for (i = 0; i < num_dashes; i++) {
            if (i != 0)
                printf (", ");
            printf ("%f", dash_array[i]);
        }
        printf (")\n");
    }

    return _svg_engine_set_stroke_dash_array (trace->targets->target_engine,
                                              trace->targets->target_closure, dash_array,
                                              num_dashes);
}

static svg_status_t
_svg_trace_set_stroke_dash_offset (void *closure, const svg_length_t *offset)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_stroke_dash_offset(");
    _svg_trace_print_length (offset);
    printf (")\n");

    return _svg_engine_set_stroke_dash_offset (trace->targets->target_engine,
                                               trace->targets->target_closure, offset);
}

static svg_status_t
_svg_trace_set_stroke_line_cap (void *closure, svg_stroke_line_cap_t line_cap)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_stroke_line_cap(");
    switch (line_cap) {
    case SVG_STROKE_LINE_CAP_BUTT:
        printf ("butt");
        break;
    case SVG_STROKE_LINE_CAP_ROUND:
        printf ("round");
        break;
    case SVG_STROKE_LINE_CAP_SQUARE:
        printf ("square");
        break;
    }
    printf (")\n");

    return _svg_engine_set_stroke_line_cap (trace->targets->target_engine,
                                            trace->targets->target_closure, line_cap);
}

static svg_status_t
_svg_trace_set_stroke_line_join (void *closure, svg_stroke_line_join_t line_join)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_stroke_line_join(");
    switch (line_join) {
    case SVG_STROKE_LINE_JOIN_BEVEL:
        printf ("bevel");
        break;
    case SVG_STROKE_LINE_JOIN_MITER:
        printf ("miter");
        break;
    case SVG_STROKE_LINE_JOIN_ROUND:
        printf ("round");
        break;
    }
    printf (")\n");

    return _svg_engine_set_stroke_line_join (trace->targets->target_engine,
                                             trace->targets->target_closure, line_join);
}

static svg_status_t
_svg_trace_set_stroke_miter_limit (void *closure, double limit)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_stroke_miter_limit(%f)\n", limit);

    return _svg_engine_set_stroke_miter_limit (trace->targets->target_engine,
                                               trace->targets->target_closure, limit);
}

static svg_status_t
_svg_trace_set_stroke_opacity (void *closure, double stroke_opacity)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_stroke_opacity(%f)\n", stroke_opacity);

    return _svg_engine_set_stroke_opacity (trace->targets->target_engine,
                                           trace->targets->target_closure, stroke_opacity);
}

static svg_status_t
_svg_trace_set_stroke_paint (void *closure, const svg_paint_t *paint)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_stroke_paint(");
    _svg_trace_print_paint (paint);
    printf (")\n");

    return _svg_engine_set_stroke_paint (trace->targets->target_engine,
                                         trace->targets->target_closure, paint);
}

static svg_status_t
_svg_trace_set_stroke_width (void *closure, const svg_length_t *width)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_stroke_width(");
    _svg_trace_print_length (width);
    printf (")\n");

    return _svg_engine_set_stroke_width (trace->targets->target_engine,
                                         trace->targets->target_closure, width);
}

static svg_status_t
_svg_trace_set_text_anchor (void *closure, svg_text_anchor_t text_anchor)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_text_anchor(");
    switch (text_anchor) {
    case SVG_TEXT_ANCHOR_START:
        printf ("start");
        break;
    case SVG_TEXT_ANCHOR_MIDDLE:
        printf ("middle");
        break;
    case SVG_TEXT_ANCHOR_END:
        printf ("end");
        break;
    }
    printf (")\n");

    return _svg_engine_set_text_anchor (trace->targets->target_engine,
                                        trace->targets->target_closure, text_anchor);
}

static svg_status_t
_svg_trace_set_visibility (void *closure, int visible)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_visibility(%s)\n", (visible ? "visible" : "hidden"));

    return _svg_engine_set_visibility (trace->targets->target_engine,
                                       trace->targets->target_closure, visible);
}

static svg_status_t
_svg_trace_end_style (void *closure)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    return _svg_engine_end_style (trace->targets->target_engine, trace->targets->target_closure);
}

static svg_status_t
_svg_trace_text_advance_x (void *closure, const char *utf8, double *advance)
{
    svg_trace_t *trace = (svg_trace_t *) closure;
    svg_status_t status;

    _svg_trace_print_indent (trace->level);

    printf ("set_text_advance_x(");
    if (utf8 != NULL)
        printf ("'%s', ", utf8);
    else
        printf ("NULL, ");
    printf ("advance)");

    status =
        _svg_engine_text_advance_x (trace->targets->target_engine, trace->targets->target_closure,
                                    utf8, advance);

    printf (" -> advance = %f\n", *advance);

    return status;
}

static svg_status_t
_svg_trace_set_text_position_x (void *closure, const svg_length_t *x)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_text_position_x(");
    _svg_trace_print_length (x);
    printf (")\n");

    return _svg_engine_set_text_position_x (trace->targets->target_engine,
                                            trace->targets->target_closure, x);
}

static svg_status_t
_svg_trace_set_text_position_y (void *closure, const svg_length_t *y)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_text_position_y(");
    _svg_trace_print_length (y);
    printf (")\n");

    return _svg_engine_set_text_position_y (trace->targets->target_engine,
                                            trace->targets->target_closure, y);
}

static svg_status_t
_svg_trace_adjust_text_position (void *closure, const svg_length_t *dx, const svg_length_t *dy)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("adjust_text_position(");
    _svg_trace_print_length (dx);
    printf (", ");
    _svg_trace_print_length (dy);
    printf (")\n");

    return _svg_engine_adjust_text_position (trace->targets->target_engine,
                                             trace->targets->target_closure, dx, dy);
}

static svg_status_t
_svg_trace_set_text_chunk_width (void *closure, double width)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("set_text_chunk_width(%f)\n", width);

    return _svg_engine_set_text_chunk_width (trace->targets->target_engine,
                                             trace->targets->target_closure, width);
}

static svg_status_t
_svg_trace_render_line (void *closure, const svg_length_t *x1, const svg_length_t *y1,
                        const svg_length_t *x2, const svg_length_t *y2)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("render_line(");
    _svg_trace_print_length (x1);
    printf (", ");
    _svg_trace_print_length (y1);
    printf (", ");
    _svg_trace_print_length (x2);
    printf (", ");
    _svg_trace_print_length (y2);
    printf (")\n");

    return _svg_engine_render_line (trace->targets->target_engine, trace->targets->target_closure,
                                    x1, y1, x2, y2);
}

static svg_status_t
_svg_trace_render_path (void *closure, const svg_path_t *ext_path)
{
    svg_trace_t *trace = (svg_trace_t *) closure;
    const svg_path_t *path_element;

    _svg_trace_print_indent (trace->level);

    if (ext_path == NULL) {
        printf ("render_path(NULL)\n");
    } else {
        printf ("render_path(\n");

        trace->level++;

        path_element = ext_path;
        while (path_element != NULL) {

            _svg_trace_print_indent (trace->level);

            switch (path_element->op) {
            case SVG_PATH_OP_MOVE_TO:
                printf ("move_to {%f, %f}\n", path_element->p.move_to.x, path_element->p.move_to.y);
                break;
            case SVG_PATH_OP_LINE_TO:
                printf ("line_to {%f, %f}\n", path_element->p.line_to.x, path_element->p.line_to.y);
                break;
            case SVG_PATH_OP_CURVE_TO:
                printf ("curve_to {{%f, %f}, {%f, %f}, {%f, %f}}\n",
                        path_element->p.curve_to.x1, path_element->p.curve_to.y1,
                        path_element->p.curve_to.x2, path_element->p.curve_to.y2,
                        path_element->p.curve_to.x3, path_element->p.curve_to.y3);
                break;
            case SVG_PATH_OP_QUAD_CURVE_TO:
                printf ("curve_to {{%f, %f}, {%f, %f}}\n",
                        path_element->p.quad_curve_to.x1, path_element->p.quad_curve_to.y1,
                        path_element->p.quad_curve_to.x2, path_element->p.quad_curve_to.y2);
                break;
            case SVG_PATH_OP_ARC_TO:
                printf ("arc_to {{%f, %f}, %f, %s, %s, {%f, %f}}\n",
                        path_element->p.arc_to.rx, path_element->p.arc_to.ry,
                        path_element->p.arc_to.x_axis_rotation,
                        (path_element->p.arc_to.large_arc_flag ? "true" : "false"),
                        (path_element->p.arc_to.sweep_flag ? "true" : "false"),
                        path_element->p.arc_to.x, path_element->p.arc_to.y);
                break;
            case SVG_PATH_OP_CLOSE_PATH:
                printf ("close_path\n");
                break;
            }

            path_element = path_element->next;
        }

        trace->level--;

        _svg_trace_print_indent (trace->level);
        printf (")\n");
    }

    return _svg_engine_render_path (trace->targets->target_engine, trace->targets->target_closure,
                                    ext_path);
}

static svg_status_t
_svg_trace_render_circle (void *closure, const svg_length_t *cx, const svg_length_t *cy,
                          const svg_length_t *r)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("render_circle(");
    _svg_trace_print_length (cx);
    printf (", ");
    _svg_trace_print_length (cy);
    printf (", ");
    _svg_trace_print_length (r);
    printf (")\n");

    return _svg_engine_render_circle (trace->targets->target_engine, trace->targets->target_closure,
                                      cx, cy, r);
}

static svg_status_t
_svg_trace_render_ellipse (void *closure, const svg_length_t *cx, const svg_length_t *cy,
                           const svg_length_t *rx, const svg_length_t *ry)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("render_ellipse(");
    _svg_trace_print_length (cx);
    printf (", ");
    _svg_trace_print_length (cy);
    printf (", ");
    _svg_trace_print_length (rx);
    printf (", ");
    _svg_trace_print_length (ry);
    printf (")\n");

    return _svg_engine_render_ellipse (trace->targets->target_engine,
                                       trace->targets->target_closure, cx, cy, rx, ry);
}

static svg_status_t
_svg_trace_render_rect (void *closure, const svg_length_t *x, const svg_length_t *y,
                        const svg_length_t *width, const svg_length_t *height,
                        const svg_length_t *rx, const svg_length_t *ry)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("render_rect(");
    _svg_trace_print_length (x);
    printf (", ");
    _svg_trace_print_length (y);
    printf (", ");
    _svg_trace_print_length (width);
    printf (", ");
    _svg_trace_print_length (height);
    printf (", ");
    _svg_trace_print_length (rx);
    printf (", ");
    _svg_trace_print_length (ry);
    printf (")\n");

    return _svg_engine_render_rect (trace->targets->target_engine, trace->targets->target_closure,
                                    x, y, width, height, rx, ry);
}

static svg_status_t
_svg_trace_render_text (void *closure, const char *utf8)
{
    svg_trace_t *trace = (svg_trace_t *) closure;

    _svg_trace_print_indent (trace->level);

    printf ("render_text(");
    if (utf8 != NULL)
        printf ("'%s'", utf8);
    else
        printf ("NULL");
    printf (")\n");

    return _svg_engine_render_text (trace->targets->target_engine, trace->targets->target_closure,
                                    utf8);
}

static svg_status_t
_svg_trace_render_image (void *closure, const char *uri, int index,
                         const svg_view_box_t *view_box_template,
                         const svg_length_t *x, const svg_length_t *y,
                         const svg_length_t *width, const svg_length_t *height)
{
    svg_trace_t *trace = (svg_trace_t *) closure;
    int i;

    _svg_trace_print_indent (trace->level);

    printf ("render_image(");
    if (uri != NULL) {
        if (svg_is_data_scheme_uri (uri)) {
            printf ("'");
            for (i = 0; i < 64; i++) {
                if (uri[i] == '\0')
                    break;
                fputc ((unsigned char) uri[i], stdout);
            }
            if (uri[i] != '\0')
                printf ("...etc");
            printf ("', ");
        } else {
            printf ("'%s', ", uri);
        }
    } else {
        printf ("NULL, ");
    }
    _svg_trace_print_view_box (view_box_template);
    printf (", ");
    _svg_trace_print_length (x);
    printf (", ");
    _svg_trace_print_length (y);
    printf (", ");
    _svg_trace_print_length (width);
    printf (", ");
    _svg_trace_print_length (height);
    printf (")\n");

    return _svg_engine_render_image (trace->targets->target_engine, trace->targets->target_closure,
                                     uri, index, view_box_template, x, y, width, height);
}

static svg_status_t
_svg_trace_measure_position (void *closure, const svg_length_t *ix, const svg_length_t *iy,
                             double *ox, double *oy)
{
    svg_trace_t *trace = (svg_trace_t *) closure;
    svg_status_t status;

    _svg_trace_print_indent (trace->level);

    printf ("measure_position(");
    _svg_trace_print_length (ix);
    printf (", ");
    _svg_trace_print_length (iy);
    printf (", ox, oy)");

    status =
        _svg_engine_measure_position (trace->targets->target_engine, trace->targets->target_closure,
                                      ix, iy, ox, oy);

    printf (" -> ox = %f, oy = %f\n", *ox, *oy);

    return status;
}

static svg_status_t
_svg_trace_measure_font_size (void *closure, const char *font_family, double parent_font_size,
                              const svg_length_t *in_size, double *out_size)
{
    svg_trace_t *trace = (svg_trace_t *) closure;
    svg_status_t status;

    _svg_trace_print_indent (trace->level);

    printf ("measure_font_size(");
    if (font_family != NULL)
        printf ("'%s', ", font_family);
    else
        printf ("NULL, ");
    printf ("%f, ", parent_font_size);
    _svg_trace_print_length (in_size);
    printf (", out_size)");

    status =
        _svg_engine_measure_font_size (trace->targets->target_engine,
                                       trace->targets->target_closure, font_family,
                                       parent_font_size, in_size, out_size);

    printf (" -> out_size = %f\n", *out_size);

    return status;
}




static svg_render_engine_t SVG_TRACE_RENDER_ENGINE = {
    /* hierarchy */
    _svg_trace_begin_group,
    _svg_trace_begin_element,
    _svg_trace_end_element,
    _svg_trace_end_group,
    /* transform */
    _svg_trace_set_viewport,
    _svg_trace_apply_view_box,
    _svg_trace_viewport_clipping_path,
    _svg_trace_transform,
    _svg_trace_end_transform,
    /* style */
    _svg_trace_set_clip_path,
    _svg_trace_set_clip_rule,
    _svg_trace_set_color,
    _svg_trace_set_fill_opacity,
    _svg_trace_set_fill_paint,
    _svg_trace_set_fill_rule,
    _svg_trace_set_font_family,
    _svg_trace_set_font_size,
    _svg_trace_set_font_style,
    _svg_trace_set_font_weight,
    _svg_trace_set_marker_end,
    _svg_trace_set_marker_mid,
    _svg_trace_set_marker_start,
    _svg_trace_set_mask,
    _svg_trace_set_opacity,
    _svg_trace_set_stroke_dash_array,
    _svg_trace_set_stroke_dash_offset,
    _svg_trace_set_stroke_line_cap,
    _svg_trace_set_stroke_line_join,
    _svg_trace_set_stroke_miter_limit,
    _svg_trace_set_stroke_opacity,
    _svg_trace_set_stroke_paint,
    _svg_trace_set_stroke_width,
    _svg_trace_set_text_anchor,
    _svg_trace_set_visibility,
    _svg_trace_end_style,
    /* text positioning */
    _svg_trace_text_advance_x,
    _svg_trace_set_text_position_x,
    _svg_trace_set_text_position_y,
    _svg_trace_adjust_text_position,
    _svg_trace_set_text_chunk_width,
    /* drawing */
    _svg_trace_render_line,
    _svg_trace_render_path,
    _svg_trace_render_circle,
    _svg_trace_render_ellipse,
    _svg_trace_render_rect,
    _svg_trace_render_text,
    _svg_trace_render_image,
    /* miscellaneous */
    _svg_trace_measure_position,
    _svg_trace_measure_font_size,
};


static void
_svg_trace_init_engine (svg_trace_target_t *trace_target, svg_render_engine_t *target_engine)
{
    trace_target->trace_engine = SVG_TRACE_RENDER_ENGINE;

    if (target_engine->begin_group == NULL)
        trace_target->trace_engine.begin_group = NULL;
    if (target_engine->begin_element == NULL)
        trace_target->trace_engine.begin_element = NULL;
    if (target_engine->end_element == NULL)
        trace_target->trace_engine.end_element = NULL;
    if (target_engine->end_group == NULL)
        trace_target->trace_engine.end_group = NULL;
    if (target_engine->set_viewport == NULL)
        trace_target->trace_engine.set_viewport = NULL;
    if (target_engine->apply_view_box == NULL)
        trace_target->trace_engine.apply_view_box = NULL;
    if (target_engine->viewport_clipping_path == NULL)
        trace_target->trace_engine.viewport_clipping_path = NULL;
    if (target_engine->transform == NULL)
        trace_target->trace_engine.transform = NULL;
    if (target_engine->end_transform == NULL)
        trace_target->trace_engine.end_transform = NULL;
    if (target_engine->set_clip_path == NULL)
        trace_target->trace_engine.set_clip_path = NULL;
    if (target_engine->set_clip_rule == NULL)
        trace_target->trace_engine.set_clip_rule = NULL;
    if (target_engine->set_color == NULL)
        trace_target->trace_engine.set_color = NULL;
    if (target_engine->set_fill_opacity == NULL)
        trace_target->trace_engine.set_fill_opacity = NULL;
    if (target_engine->set_fill_paint == NULL)
        trace_target->trace_engine.set_fill_paint = NULL;
    if (target_engine->set_fill_rule == NULL)
        trace_target->trace_engine.set_fill_rule = NULL;
    if (target_engine->set_font_family == NULL)
        trace_target->trace_engine.set_font_family = NULL;
    if (target_engine->set_font_size == NULL)
        trace_target->trace_engine.set_font_size = NULL;
    if (target_engine->set_font_style == NULL)
        trace_target->trace_engine.set_font_style = NULL;
    if (target_engine->set_font_weight == NULL)
        trace_target->trace_engine.set_font_weight = NULL;
    if (target_engine->set_marker_end == NULL)
        trace_target->trace_engine.set_marker_end = NULL;
    if (target_engine->set_marker_mid == NULL)
        trace_target->trace_engine.set_marker_mid = NULL;
    if (target_engine->set_marker_start == NULL)
        trace_target->trace_engine.set_marker_start = NULL;
    if (target_engine->set_mask == NULL)
        trace_target->trace_engine.set_mask = NULL;
    if (target_engine->set_opacity == NULL)
        trace_target->trace_engine.set_opacity = NULL;
    if (target_engine->set_stroke_dash_array == NULL)
        trace_target->trace_engine.set_stroke_dash_array = NULL;
    if (target_engine->set_stroke_dash_offset == NULL)
        trace_target->trace_engine.set_stroke_dash_offset = NULL;
    if (target_engine->set_stroke_line_cap == NULL)
        trace_target->trace_engine.set_stroke_line_cap = NULL;
    if (target_engine->set_stroke_line_join == NULL)
        trace_target->trace_engine.set_stroke_line_join = NULL;
    if (target_engine->set_stroke_miter_limit == NULL)
        trace_target->trace_engine.set_stroke_miter_limit = NULL;
    if (target_engine->set_stroke_opacity == NULL)
        trace_target->trace_engine.set_stroke_opacity = NULL;
    if (target_engine->set_stroke_paint == NULL)
        trace_target->trace_engine.set_stroke_paint = NULL;
    if (target_engine->set_stroke_width == NULL)
        trace_target->trace_engine.set_stroke_width = NULL;
    if (target_engine->set_text_anchor == NULL)
        trace_target->trace_engine.set_text_anchor = NULL;
    if (target_engine->set_visibility == NULL)
        trace_target->trace_engine.set_visibility = NULL;
    if (target_engine->end_style == NULL)
        trace_target->trace_engine.end_style = NULL;
    if (target_engine->set_text_position_x == NULL)
        trace_target->trace_engine.set_text_position_x = NULL;
    if (target_engine->set_text_position_y == NULL)
        trace_target->trace_engine.set_text_position_y = NULL;
    if (target_engine->adjust_text_position == NULL)
        trace_target->trace_engine.adjust_text_position = NULL;
    if (target_engine->text_advance_x == NULL)
        trace_target->trace_engine.text_advance_x = NULL;
    if (target_engine->set_text_chunk_width == NULL)
        trace_target->trace_engine.set_text_chunk_width = NULL;
    if (target_engine->render_line == NULL)
        trace_target->trace_engine.render_line = NULL;
    if (target_engine->render_path == NULL)
        trace_target->trace_engine.render_path = NULL;
    if (target_engine->render_circle == NULL)
        trace_target->trace_engine.render_circle = NULL;
    if (target_engine->render_ellipse == NULL)
        trace_target->trace_engine.render_ellipse = NULL;
    if (target_engine->render_rect == NULL)
        trace_target->trace_engine.render_rect = NULL;
    if (target_engine->render_text == NULL)
        trace_target->trace_engine.render_text = NULL;
    if (target_engine->render_image == NULL)
        trace_target->trace_engine.render_image = NULL;
    if (target_engine->measure_position == NULL)
        trace_target->trace_engine.measure_position = NULL;
    if (target_engine->measure_font_size == NULL)
        trace_target->trace_engine.measure_font_size = NULL;
}



svg_status_t
_svg_create_trace (svg_trace_t **trace)
{
    svg_trace_t *new_trace;

    new_trace = (svg_trace_t *) malloc (sizeof (svg_trace_t));
    if (new_trace == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_trace, 0, sizeof (svg_trace_t));

    *trace = new_trace;

    return SVG_STATUS_SUCCESS;
}

void
_svg_destroy_trace (svg_trace_t *trace)
{
    if (trace == NULL)
        return;

    while (trace->targets != NULL)
        _svg_trace_pop_target_engine (trace);

    free (trace);
}

svg_status_t
_svg_trace_push_target_engine (svg_trace_t *trace, const svg_uri_t *document_uri,
                               svg_render_engine_t *engine, void *closure)
{
    svg_trace_target_t *new_target;

    /* closure != trace if the user application calls an api render() function
       otherwise it is an internal render function call and the trace already wraps
       the target render engine */

    if (closure == trace && trace->targets == NULL)
        return SVG_STATUS_INTERNAL_ERROR;


    new_target = (svg_trace_target_t *) malloc (sizeof (svg_trace_target_t));
    if (new_target == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_target, 0, sizeof (svg_trace_target_t));

    if (closure == trace) {
        new_target->target_engine = trace->targets->target_engine;
        new_target->target_closure = trace->targets->target_closure;
        _svg_trace_init_engine (new_target, trace->targets->target_engine);
    } else {
        new_target->target_engine = engine;
        new_target->target_closure = closure;
        _svg_trace_init_engine (new_target, engine);
    }

    new_target->document_uri = document_uri;
    if (trace->targets == NULL || !_svg_uri_equals (trace->targets->document_uri, document_uri)) {
        _svg_trace_print_indent (trace->level);
        printf ("in_document(");
        _svg_uri_print (document_uri);
        printf (")\n");
        trace->level++;
    }

    new_target->next = trace->targets;
    trace->targets = new_target;

    if (closure != trace) {
        new_target->new_render = 1;
        _svg_trace_print_indent (trace->level);
        printf ("begin_render\n");
        trace->level++;
    }

    return SVG_STATUS_SUCCESS;
}

void
_svg_trace_pop_target_engine (svg_trace_t *trace)
{
    svg_trace_target_t *old_target;

    if (trace->targets == NULL)
        return;

    old_target = trace->targets;
    trace->targets = trace->targets->next;

    if (old_target->new_render) {
        trace->level--;
        _svg_trace_print_indent (trace->level);
        printf ("end_render\n");
    }

    if (trace->targets == NULL ||
        !_svg_uri_equals (trace->targets->document_uri, old_target->document_uri))
    {
        trace->level--;
        _svg_trace_print_indent (trace->level);
        printf ("out_document(");
        _svg_uri_print (old_target->document_uri);
        printf (")\n");
    }

    free (old_target);
}

void
_svg_trace_set_position_info (svg_trace_t *trace, const char *node_name, long line_number)
{
    trace->node_name = node_name;
    trace->line_number = line_number;
}

void
_svg_trace_get_engine (svg_trace_t *trace, svg_render_engine_t **engine, void **closure)
{
    *engine = &trace->targets->trace_engine;
    *closure = trace;
}

