/* svg_clip_path.c: Data structures for SVG clip path elements

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
_svg_clip_path_init (svg_clip_path_int_t *clip_path, svg_element_t *clip_path_element)
{
    svg_status_t status;

    status = _svg_container_init (&clip_path->container);
    if (status)
        return status;

    status = _svg_create_element_ref (clip_path_element, &clip_path->ext_clip_path.element_ref);
    if (status)
        return status;

    clip_path->ext_clip_path.units = SVG_COORD_SPACE_UNITS_USER;

    clip_path->enable_render = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_clip_path_init_copy (svg_clip_path_int_t *clip_path, svg_element_t *clip_path_element,
                          svg_clip_path_int_t *other)
{
    svg_status_t status;

    status = _svg_container_init_copy (&clip_path->container, &other->container);
    if (status)
        return status;

    status = _svg_create_element_ref (clip_path_element, &clip_path->ext_clip_path.element_ref);
    if (status)
        return status;

    clip_path->ext_clip_path.units = other->ext_clip_path.units;

    clip_path->enable_render = other->enable_render;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_clip_path_deinit (svg_clip_path_int_t *clip_path)
{
    _svg_container_deinit (&clip_path->container);

    if (clip_path->ext_clip_path.element_ref != NULL)
        _svg_destroy_element_ref (clip_path->ext_clip_path.element_ref);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_clip_path_apply_attributes (svg_element_t *clip_path_element, const svg_qattrs_t *attributes)
{
    svg_clip_path_int_t *clip_path = &clip_path_element->e.clip_path;
    const char *str;

    _svg_style_reset_display (&clip_path_element->node->style);

    _svg_attribute_get_string (attributes, "clipPathUnits", &str, "userSpaceOnUse");
    if (strcmp (str, "userSpaceOnUse") == 0)
        clip_path->ext_clip_path.units = SVG_COORD_SPACE_UNITS_USER;
    else if (strcmp (str, "objectBoundingBox") == 0)
        clip_path->ext_clip_path.units = SVG_COORD_SPACE_UNITS_BBOX;
    else
        return _svg_element_return_property_error (clip_path_element, "clipPathUnits",
                                                   SVG_STATUS_INVALID_VALUE);

    return SVG_STATUS_SUCCESS;
}

int
_svg_clip_path_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_container_peek_render (&element->e.clip_path.container, engine);
}

svg_status_t
_svg_clip_path_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_clip_path_int_t *clip_path = &element->e.clip_path;
    svg_status_t status;

    if (!clip_path->enable_render)
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



    status = _svg_container_render (&clip_path->container, engine, closure);
    if (status)
        return status;


    status = _svg_engine_end_group (engine, closure,
                                    _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

