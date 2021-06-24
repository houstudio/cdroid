/* svg_group.c: Data structures for SVG group elements

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

#include "svgint.h"


svg_status_t
_svg_group_init (svg_group_t *group)
{
    svg_status_t status;

    status = _svg_container_init (&group->container);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_group_init_copy (svg_group_t *group, svg_group_t *other)
{
    svg_status_t status;

    status = _svg_container_init_copy (&group->container, &other->container);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_group_deinit (svg_group_t *group)
{
    svg_status_t status;

    status = _svg_container_deinit (&group->container);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_group_apply_attributes (svg_element_t *group_element, const svg_qattrs_t *attributes)
{
    return SVG_STATUS_SUCCESS;
}

int
_svg_group_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_container_peek_render (&element->e.group.container, engine);
}

svg_status_t
_svg_group_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_group_t *group = &element->e.group;
    svg_status_t status;

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



    status = _svg_container_render (&group->container, engine, closure);
    if (status)
        return status;


    status =
        _svg_engine_end_group (engine, closure, _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

