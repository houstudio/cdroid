/* svg_text.c: Data structures for SVG text elements

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
_svg_prepare_tspan (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_status_t status;

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
_svg_text_width (svg_text_t *text, double *width, int start_element, svg_render_engine_t *engine,
                 void *closure)
{
    int e;
    double advance;
    double dx, dy;
    double total_advance = 0;
    svg_element_t *tspan_element;
    svg_tspan_t *tspan;
    svg_status_t status;

    for (e = start_element; e < text->container.num_elements; e++) {
        tspan_element = text->container.element[e];
        tspan = &tspan_element->e.tspan;

        if (e != start_element && (tspan->x_set || tspan->y_set))
            break;

        if (e == 0 && (text->dx.value != 0 || text->dy.value != 0)) {
            status = _svg_engine_measure_position (engine, closure, &text->dx, &text->dy, &dx, &dy);
            if (status)
                return status;

            total_advance += dx;
        }
        if (tspan->dx.value != 0 || tspan->dy.value != 0) {
            status = _svg_engine_measure_position (engine, closure, &tspan->dx, &tspan->dy,
                                                   &dx, &dy);
            if (status)
                return status;

            total_advance += dx;
        }

        if (tspan->chars == NULL)
            continue;


        status =
            _svg_engine_begin_element (engine, closure, tspan_element->id, tspan_element->klass);
        if (status)
            return status;

        status = _svg_prepare_tspan (tspan_element, engine, closure);
        if (status)
            return status;

        status = _svg_engine_text_advance_x (engine, closure, tspan->chars, &advance);
        if (status)
            return status;

        status = _svg_engine_end_element (engine, closure);
        if (status)
            return status;


        total_advance += advance;
    }

    *width = total_advance;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_tspan_init (svg_tspan_t *tspan)
{
    tspan->chars = NULL;
    tspan->len = 0;

    _svg_length_init_unit (&tspan->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&tspan->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    _svg_length_init_unit (&tspan->dx, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&tspan->dy, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    tspan->x_set = 0;
    tspan->y_set = 0;

    tspan->virtual_node = NULL;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_tspan_init_copy (svg_tspan_t *tspan, svg_tspan_t *other)
{
    *tspan = *other;
    tspan->chars = NULL;
    tspan->virtual_node = NULL;

    if (other->chars) {
        tspan->chars = strdup (other->chars);
        if (tspan->chars == NULL)
            return SVG_STATUS_NO_MEMORY;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_tspan_deinit (svg_tspan_t *tspan)
{
    svg_status_t status;

    if (tspan->chars != NULL) {
        free (tspan->chars);
        tspan->chars = NULL;
    }

    if (tspan->virtual_node != NULL) {
        status = _svg_destroy_dom_node (tspan->virtual_node);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_tspan_append_chars (svg_tspan_t *tspan, const char *chars, size_t len)
{
    char *new_chars;

    if (chars == NULL || len == 0)
        return SVG_STATUS_SUCCESS;

    new_chars = realloc (tspan->chars, tspan->len + len + 1);
    if (new_chars == NULL)
        return SVG_STATUS_NO_MEMORY;

    tspan->chars = new_chars;

    if (tspan->len == 0)
        tspan->chars[0] = '\0';
    strncat (tspan->chars, chars, len);

    tspan->len += len;
    tspan->chars[tspan->len] = '\0';

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_tspan_apply_attributes (svg_element_t *tspan_element, const svg_qattrs_t *attributes)
{
    svg_tspan_t *tspan = &tspan_element->e.tspan;
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "x", &tspan->x, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (tspan_element, "x", status);
    tspan->x_set = (status == SVG_STATUS_SUCCESS);

    status = _svg_attribute_get_length (attributes, "y", &tspan->y, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (tspan_element, "y", status);
    tspan->y_set = (status == SVG_STATUS_SUCCESS);

    status = _svg_attribute_get_length (attributes, "dx", &tspan->dx, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (tspan_element, "dx", status);

    status = _svg_attribute_get_length (attributes, "dy", &tspan->dy, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (tspan_element, "dy", status);


    return SVG_STATUS_SUCCESS;
}

int
_svg_tspan_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return element->e.tspan.len > 0;
}

svg_status_t
_svg_text_init (svg_text_t *text)
{
    svg_status_t status;

    status = _svg_container_init (&text->container);
    if (status)
        return status;

    _svg_length_init_unit (&text->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&text->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    _svg_length_init_unit (&text->dx, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&text->dy, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_text_init_copy (svg_text_t *text, svg_text_t *other)
{
    svg_status_t status;

    status = _svg_container_init_copy (&text->container, &other->container);
    if (status)
        return status;

    text->x = other->x;
    text->y = other->y;
    text->dx = other->dx;
    text->dy = other->dy;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_text_deinit (svg_text_t *text)
{
    svg_status_t status;

    status = _svg_container_deinit (&text->container);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_text_apply_attributes (svg_element_t *text_element, const svg_qattrs_t *attributes)
{
    svg_text_t *text = &text_element->e.text;
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "x", &text->x, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (text_element, "x", status);

    status = _svg_attribute_get_length (attributes, "y", &text->y, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (text_element, "y", status);

    status = _svg_attribute_get_length (attributes, "dx", &text->dx, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (text_element, "dx", status);

    status = _svg_attribute_get_length (attributes, "dy", &text->dy, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (text_element, "dy", status);


    return SVG_STATUS_SUCCESS;
}

int
_svg_text_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_engine_support_render_text (engine) &&
        _svg_container_peek_render (&element->e.text.container, engine);
}

svg_status_t
_svg_text_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_text_t *text = &element->e.text;
    int e;
    double width = 0;
    svg_element_t *tspan_element;
    svg_tspan_t *tspan;
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


    for (e = 0; e < text->container.num_elements; e++) {
        tspan_element = text->container.element[e];
        tspan = &tspan_element->e.tspan;

        if (e == 0 || tspan->x_set || tspan->y_set) {
            status = _svg_text_width (text, &width, e, engine, closure);
            if (status)
                return status;
        }


        status =
            _svg_engine_begin_element (engine, closure, tspan_element->id, tspan_element->klass);
        if (status)
            return status;

        status = _svg_prepare_tspan (tspan_element, engine, closure);
        if (status)
            return status;

        if (e == 0 || tspan->x_set || tspan->y_set) {
            if (tspan->x_set) {
                status = _svg_engine_set_text_position_x (engine, closure, &tspan->x);
                if (status)
                    return status;
            } else if (e == 0) {
                status = _svg_engine_set_text_position_x (engine, closure, &text->x);
                if (status)
                    return status;
            }
            if (tspan->y_set) {
                status = _svg_engine_set_text_position_y (engine, closure, &tspan->y);
                if (status)
                    return status;
            } else if (e == 0) {
                status = _svg_engine_set_text_position_y (engine, closure, &text->y);
                if (status)
                    return status;
            }

            status = _svg_engine_set_text_chunk_width (engine, closure, width);
            if (status)
                return status;
        }

        if (e == 0 && (text->dx.value != 0 || text->dy.value != 0)) {
            status = _svg_engine_adjust_text_position (engine, closure, &text->dx, &text->dy);
            if (status)
                return status;
        }
        if (tspan->dx.value != 0 || tspan->dy.value != 0) {
            status = _svg_engine_adjust_text_position (engine, closure, &tspan->dx, &tspan->dy);
            if (status)
                return status;
        }

        status = _svg_engine_render_text (engine, closure, tspan->chars);
        if (status)
            return status;

        status = _svg_engine_end_element (engine, closure);
        if (status)
            return status;
    }


    status =
        _svg_engine_end_group (engine, closure, _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;


    return SVG_STATUS_SUCCESS;
}

