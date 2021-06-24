/* svg_container.c: Data structures for SVG container elements

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


static svg_status_t
_svg_container_grow_element_by (svg_container_t *container, int additional)
{
    svg_element_t **new_element;
    int old_size = container->element_size;
    int new_size = container->num_elements + additional;

    if (new_size <= container->element_size)
        return SVG_STATUS_SUCCESS;

    container->element_size = new_size;
    new_element = realloc (container->element, container->element_size * sizeof (svg_element_t *));

    if (new_element == NULL) {
        container->element_size = old_size;
        return SVG_STATUS_NO_MEMORY;
    }

    container->element = new_element;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_container_init (svg_container_t *container)
{
    container->element = NULL;
    container->num_elements = 0;
    container->element_size = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_container_init_copy (svg_container_t *container, svg_container_t *other)
{
    svg_status_t status;
    svg_element_t *clone;
    int i;

    container->element = NULL;
    container->num_elements = 0;
    container->element_size = 0;

    for (i = 0; i < other->num_elements; i++) {
        status = _svg_element_clone (other->element[i], &clone);
        if (status)
            return status;

        status = _svg_container_add_element (container, clone);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_container_deinit (svg_container_t *container)
{
    int i;

    for (i = 0; i < container->num_elements; i++)
        _svg_element_destroy (container->element[i]);

    free (container->element);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_container_add_element (svg_container_t *container, svg_element_t *element)
{
    svg_status_t status;

    if (container->num_elements >= container->element_size) {
        int additional = container->element_size ? container->element_size : 4;
        status = _svg_container_grow_element_by (container, additional);
        if (status)
            return status;
    }

    container->element[container->num_elements] = element;
    container->num_elements++;

    return SVG_STATUS_SUCCESS;
}

int
_svg_container_peek_render (svg_container_t *container, svg_render_engine_t *engine)
{
    int i;

    for (i = 0; i < container->num_elements; i++) {
        if (_svg_element_peek_render (container->element[i], engine))
            return 1;
    }

    return 0;
}

svg_status_t
_svg_container_render (svg_container_t *container, svg_render_engine_t *engine, void *closure)
{
    int i;
    svg_status_t status;

    for (i = 0; i < container->num_elements; i++) {
        status = svg_element_render (container->element[i], engine, closure);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

