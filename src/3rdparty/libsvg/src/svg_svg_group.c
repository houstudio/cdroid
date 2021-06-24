/* svg_svg_group.c: Data structures for SVG svg elements

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
_svg_svg_group_init (svg_svg_group_t *svg_group)
{
    svg_status_t status;

    status = _svg_container_init (&svg_group->container);
    if (status)
        return status;

    _svg_length_init_unit (&svg_group->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&svg_group->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&svg_group->width, 100, SVG_LENGTH_UNIT_PCT,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&svg_group->height, 100, SVG_LENGTH_UNIT_PCT,
                           SVG_LENGTH_ORIENTATION_VERTICAL);

    _svg_view_box_init (&svg_group->view_box);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_svg_group_init_copy (svg_svg_group_t *svg_group, svg_svg_group_t *other)
{
    svg_status_t status;

    status = _svg_container_init_copy (&svg_group->container, &other->container);
    if (status)
        return status;

    svg_group->x = other->x;
    svg_group->y = other->y;
    svg_group->width = other->width;
    svg_group->height = other->height;
    svg_group->view_box = other->view_box;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_svg_group_deinit (svg_svg_group_t *svg_group)
{
    svg_status_t status;

    status = _svg_container_deinit (&svg_group->container);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_svg_group_apply_attributes (svg_element_t *svg_group_element, const svg_qattrs_t *attributes)
{
    svg_svg_group_t *svg_group = &svg_group_element->e.svg_group;
    const char *view_box_str, *aspect_ratio_str;
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "x", &svg_group->x, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (svg_group_element, "x", status);

    status = _svg_attribute_get_length (attributes, "y", &svg_group->y, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (svg_group_element, "y", status);

    status = _svg_attribute_get_length (attributes, "width", &svg_group->width, "100%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (svg_group_element, "width", status);
    if (svg_group->width.value < 0)
        return _svg_element_return_property_error (svg_group_element, "width",
                                                   SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_length (attributes, "height", &svg_group->height, "100%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (svg_group_element, "height", status);
    if (svg_group->height.value < 0)
        return _svg_element_return_property_error (svg_group_element, "height",
                                                   SVG_STATUS_INVALID_VALUE);

    _svg_attribute_get_string (attributes, "viewBox", &view_box_str, NULL);
    if (view_box_str != NULL) {
        status = _svg_parse_view_box (view_box_str, &svg_group->view_box);
        if (status)
            return _svg_element_return_property_error (svg_group_element, "viewBox", status);

        _svg_attribute_get_string (attributes, "preserveAspectRatio", &aspect_ratio_str,
                                   "xMidYMid meet");
        status = _svg_view_box_parse_aspect_ratio (aspect_ratio_str, &svg_group->view_box);
        if (status)
            return _svg_element_return_property_error (svg_group_element, "preserveAspectRatio",
                                                       status);
    }

    return SVG_STATUS_SUCCESS;
}

int
_svg_svg_group_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_container_peek_render (&element->e.svg_group.container, engine);
}

svg_status_t
_svg_svg_group_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_svg_group_t *svg_group = &element->e.svg_group;
    svg_status_t status;

    status = _svg_engine_begin_group (engine, closure,
                                      _svg_style_get_opacity (&element->node->style),
                                      element->id, element->klass);
    if (status)
        return status;


    status = _svg_engine_set_viewport (engine, closure, &svg_group->x, &svg_group->y,
                                       &svg_group->width, &svg_group->height);
    if (status)
        return status;

    if (!_svg_view_box_is_null (&svg_group->view_box)) {
        status = _svg_engine_apply_view_box (engine, closure, &svg_group->view_box,
                                             &svg_group->width, &svg_group->height);
        if (status)
            return status;
    }

    status = _svg_style_render_viewport_clipping_path (&element->node->style, engine, closure);
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


    status = _svg_container_render (&svg_group->container, engine, closure);
    if (status)
        return status;


    status =
        _svg_engine_end_group (engine, closure, _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_svg_group_get_size (svg_svg_group_t *svg_group, svg_length_t *width, svg_length_t *height)
{
    *width = svg_group->width;
    *height = svg_group->height;

    return SVG_STATUS_SUCCESS;
}

