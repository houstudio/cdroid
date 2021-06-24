/* svg_mask.c: Data structures for SVG mask elements

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
_svg_mask_init (svg_mask_int_t *mask, svg_element_t *mask_element)
{
    svg_status_t status;
    svg_mask_t *ext_mask = &mask->ext_mask;

    status = _svg_container_init (&mask->container);
    if (status)
        return status;

    status = _svg_create_element_ref (mask_element, &ext_mask->element_ref);
    if (status)
        return status;

    ext_mask->units = SVG_COORD_SPACE_UNITS_BBOX;
    ext_mask->content_units = SVG_COORD_SPACE_UNITS_USER;

    _svg_length_init_unit (&ext_mask->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&ext_mask->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&ext_mask->width, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&ext_mask->height, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_VERTICAL);

    mask->enable_render = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_mask_init_copy (svg_mask_int_t *mask, svg_element_t *mask_element, svg_mask_int_t *other)
{
    svg_status_t status;
    svg_mask_t *ext_mask = &mask->ext_mask;
    svg_mask_t *other_ext = &other->ext_mask;

    status = _svg_container_init_copy (&mask->container, &other->container);
    if (status)
        return status;

    status = _svg_create_element_ref (mask_element, &ext_mask->element_ref);
    if (status)
        return status;

    ext_mask->units = other_ext->units;
    ext_mask->content_units = other_ext->content_units;
    ext_mask->x = other_ext->x;
    ext_mask->y = other_ext->y;
    ext_mask->width = other_ext->width;
    ext_mask->height = other_ext->height;

    mask->enable_render = other->enable_render;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_mask_deinit (svg_mask_int_t *mask)
{
    _svg_container_deinit (&mask->container);

    if (mask->ext_mask.element_ref != NULL)
        _svg_destroy_element_ref (mask->ext_mask.element_ref);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_mask_apply_attributes (svg_element_t *mask_element, const svg_qattrs_t *attributes)
{
    svg_mask_int_t *mask = &mask_element->e.mask;
    svg_mask_t *ext_mask = &mask->ext_mask;
    const char *str;
    svg_status_t status;

    _svg_style_reset_display (&mask_element->node->style);

    status = _svg_attribute_get_string (attributes, "maskUnits", &str, "objectBoundingBox");
    if (strcmp (str, "userSpaceOnUse") == 0)
        ext_mask->units = SVG_COORD_SPACE_UNITS_USER;
    else if (strcmp (str, "objectBoundingBox") == 0)
        ext_mask->units = SVG_COORD_SPACE_UNITS_BBOX;
    else
        return _svg_element_return_property_error (mask_element, "maskUnits",
                                                   SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_string (attributes, "maskContentUnits", &str, "userSpaceOnUse");
    if (strcmp (str, "userSpaceOnUse") == 0)
        ext_mask->content_units = SVG_COORD_SPACE_UNITS_USER;
    else if (strcmp (str, "objectBoundingBox") == 0)
        ext_mask->content_units = SVG_COORD_SPACE_UNITS_BBOX;
    else
        return _svg_element_return_property_error (mask_element, "maskContentUnits",
                                                   SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_length (attributes, "x", &ext_mask->x, "-10%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (mask_element, "x", status);

    status = _svg_attribute_get_length (attributes, "y", &ext_mask->y, "-10%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (mask_element, "y", status);

    status = _svg_attribute_get_length (attributes, "width", &ext_mask->width, "120%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (mask_element, "width", status);

    status = _svg_attribute_get_length (attributes, "height", &ext_mask->height, "120%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (mask_element, "height", status);

    return SVG_STATUS_SUCCESS;
}

int
_svg_mask_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_container_peek_render (&element->e.mask.container, engine);
}

svg_status_t
_svg_mask_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_mask_int_t *mask = &element->e.mask;
    svg_status_t status;

    if (!mask->enable_render)
        return SVG_STATUS_SUCCESS;

    status = _svg_engine_begin_group (engine, closure,
                                      _svg_style_get_opacity (&element->node->style),
                                      element->id, element->klass);
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



    status = _svg_container_render (&mask->container, engine, closure);
    if (status)
        return status;


    status =
        _svg_engine_end_group (engine, closure, _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

