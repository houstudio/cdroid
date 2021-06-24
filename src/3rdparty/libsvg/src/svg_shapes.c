/* svg_shape.c: Data structures for rect, circle, ellipse and lines

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


static svg_status_t
_svg_shape_begin_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_status_t status;

    status = _svg_engine_begin_element (engine, closure, element->id, element->klass);
    if (status)
        return status;

    status = _svg_transform_render (&element->transform, engine, closure);
    if (status)
        return status;

    status = _svg_engine_end_transform (engine, closure);
    if (status)
        return status;

    status = _svg_style_render (element->node, engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_shape_end_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_status_t status;

    status = _svg_engine_end_element (engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}



svg_status_t
_svg_circle_init (svg_circle_t *circle)
{
    _svg_length_init_unit (&circle->cx, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&circle->cy, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&circle->r, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_OTHER);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_circle_init_copy (svg_circle_t *circle, svg_circle_t *other)
{
    *circle = *other;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_ellipse_init (svg_ellipse_t *ellipse)
{
    _svg_length_init_unit (&ellipse->cx, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&ellipse->cy, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&ellipse->rx, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&ellipse->ry, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_ellipse_init_copy (svg_ellipse_t *ellipse, svg_ellipse_t *other)
{
    *ellipse = *other;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_line_init (svg_line_t *line)
{
    _svg_length_init_unit (&line->x1, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&line->y1, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&line->x2, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&line->y2, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_line_init_copy (svg_line_t *line, svg_line_t *other)
{
    *line = *other;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_rect_init (svg_rect_element_t *rect)
{
    _svg_length_init_unit (&rect->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&rect->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&rect->width, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&rect->height, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&rect->rx, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&rect->ry, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_rect_init_copy (svg_rect_element_t *rect, svg_rect_element_t *other)
{
    *rect = *other;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_circle_apply_attributes (svg_element_t *circle_element, const svg_qattrs_t *attributes)
{
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "cx", &(circle_element->e.circle.cx), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (circle_element, "cx", status);

    status = _svg_attribute_get_length (attributes, "cy", &(circle_element->e.circle.cy), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (circle_element, "cy", status);

    status = _svg_attribute_get_length (attributes, "r", &(circle_element->e.circle.r), "0");
    if (status)
        return _svg_element_return_property_error (circle_element, "r", status);
    if (circle_element->e.circle.r.value < 0)
        return _svg_element_return_property_error (circle_element, "r", SVG_STATUS_INVALID_VALUE);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_ellipse_apply_attributes (svg_element_t *ellipse_element, const svg_qattrs_t *attributes)
{
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "cx", &(ellipse_element->e.ellipse.cx), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (ellipse_element, "cx", status);

    status = _svg_attribute_get_length (attributes, "cy", &(ellipse_element->e.ellipse.cy), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (ellipse_element, "cx", status);

    status = _svg_attribute_get_length (attributes, "rx", &(ellipse_element->e.ellipse.rx), "0");
    if (status)
        return _svg_element_return_property_error (ellipse_element, "rx", status);
    if (ellipse_element->e.ellipse.rx.value < 0)
        return _svg_element_return_property_error (ellipse_element, "rx", SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_length (attributes, "ry", &(ellipse_element->e.ellipse.ry), "0");
    if (status)
        return _svg_element_return_property_error (ellipse_element, "ry", status);
    if (ellipse_element->e.ellipse.ry.value < 0)
        return _svg_element_return_property_error (ellipse_element, "ry", SVG_STATUS_INVALID_VALUE);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_line_apply_attributes (svg_element_t *line_element, const svg_qattrs_t *attributes)
{
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "x1", &(line_element->e.line.x1), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (line_element, "x1", status);

    status = _svg_attribute_get_length (attributes, "y1", &(line_element->e.line.y1), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (line_element, "y1", status);

    status = _svg_attribute_get_length (attributes, "x2", &(line_element->e.line.x2), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (line_element, "x2", status);

    status = _svg_attribute_get_length (attributes, "y2", &(line_element->e.line.y2), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (line_element, "y2", status);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_rect_apply_attributes (svg_element_t *rect_element, const svg_qattrs_t *attributes)
{
    svg_status_t status;
    int has_rx = 0, has_ry = 0;

    status = _svg_attribute_get_length (attributes, "x", &(rect_element->e.rect.x), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (rect_element, "x", status);

    status = _svg_attribute_get_length (attributes, "y", &(rect_element->e.rect.y), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (rect_element, "y", status);

    status = _svg_attribute_get_length (attributes, "width", &(rect_element->e.rect.width), "0");
    if (status)
        return _svg_element_return_property_error (rect_element, "width", status);
    if (rect_element->e.rect.width.value < 0)
        return _svg_element_return_property_error (rect_element, "width", SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_length (attributes, "height", &(rect_element->e.rect.height), "0");
    if (status)
        return _svg_element_return_property_error (rect_element, "height", status);
    if (rect_element->e.rect.height.value < 0)
        return _svg_element_return_property_error (rect_element, "height",
                                                   SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_length (attributes, "rx", &(rect_element->e.rect.rx), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (rect_element, "rx", status);
    if (rect_element->e.rect.rx.value < 0)
        return _svg_element_return_property_error (rect_element, "rx", SVG_STATUS_INVALID_VALUE);
    if (status == SVG_STATUS_SUCCESS)
        has_rx = 1;

    status = _svg_attribute_get_length (attributes, "ry", &(rect_element->e.rect.ry), "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (rect_element, "ry", status);
    if (rect_element->e.rect.ry.value < 0)
        return _svg_element_return_property_error (rect_element, "ry", SVG_STATUS_INVALID_VALUE);
    if (status == SVG_STATUS_SUCCESS)
        has_ry = 1;

    if (has_rx || has_ry) {
        if (!has_rx)
            rect_element->e.rect.rx = rect_element->e.rect.ry;
        if (!has_ry)
            rect_element->e.rect.ry = rect_element->e.rect.rx;
    }

    /* it is left to the render engine to limit rx to half the width and
       ry to half the height */

    return SVG_STATUS_SUCCESS;
}

int
_svg_line_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_engine_support_render_line (engine);
}

svg_status_t
_svg_line_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_line_t *line = &element->e.line;
    svg_status_t status;

    status = _svg_shape_begin_render (element, engine, closure);
    if (status)
        return status;

    status = _svg_engine_render_line (engine, closure, &line->x1, &line->y1, &line->x2, &line->y2);
    if (status)
        return status;

    status = _svg_shape_end_render (element, engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

int
_svg_rect_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_engine_support_render_rect (engine);
}

svg_status_t
_svg_rect_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_rect_element_t *rect = &element->e.rect;
    svg_status_t status;

    status = _svg_shape_begin_render (element, engine, closure);
    if (status)
        return status;

    status = _svg_engine_render_rect (engine, closure, &rect->x, &rect->y,
                                      &rect->width, &rect->height, &rect->rx, &rect->ry);
    if (status)
        return status;


    status = _svg_shape_end_render (element, engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

int
_svg_circle_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_engine_support_render_circle (engine) && element->e.circle.r.value != 0;
}

svg_status_t
_svg_circle_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_circle_t *circle = &element->e.circle;
    svg_status_t status;

    if (circle->r.value == 0)
        return SVG_STATUS_SUCCESS;

    status = _svg_shape_begin_render (element, engine, closure);
    if (status)
        return status;


    status = _svg_engine_render_circle (engine, closure, &circle->cx, &circle->cy, &circle->r);
    if (status)
        return status;


    status = _svg_shape_end_render (element, engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

int
_svg_ellipse_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_engine_support_render_ellipse (engine) &&
        element->e.ellipse.rx.value != 0 && element->e.ellipse.ry.value != 0;
}

svg_status_t
_svg_ellipse_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_ellipse_t *ellipse = &element->e.ellipse;
    svg_status_t status;

    if (ellipse->rx.value == 0 || ellipse->ry.value == 0)
        return SVG_STATUS_SUCCESS;

    status = _svg_shape_begin_render (element, engine, closure);
    if (status)
        return status;


    status = _svg_engine_render_ellipse (engine, closure, &ellipse->cx, &ellipse->cy,
                                         &ellipse->rx, &ellipse->ry);
    if (status)
        return status;


    status = _svg_shape_end_render (element, engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

