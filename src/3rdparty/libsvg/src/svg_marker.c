/* svg_marker.c: Data structures for SVG marker elements

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
_svg_marker_render_group_start (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_marker_int_t *marker = &element->e.marker;
    svg_status_t status;
    svg_dom_node_t *virtual_node = NULL;
    svg_transform_t units_transform;

    status = _svg_create_dom_node (ELEMENT_NODE_TYPE, &virtual_node);
    if (status)
        return status;

    status = _svg_style_init_inherit (&virtual_node->style, &element->node->parent->style);
    if (status)
        goto fail;


    status = _svg_engine_begin_group (engine, closure, 1.0, NULL, NULL);
    if (status)
        goto fail;

    if (marker->units == SVG_MARKER_UNITS_STROKE_WIDTH) {
        status =
            _svg_transform_init_scale (&units_transform, marker->stroke_width,
                                       marker->stroke_width);
        if (status)
            goto fail;

        status = _svg_transform_render (&units_transform, engine, closure);
        if (status)
            goto fail;
    }

    status = _svg_engine_end_transform (engine, closure);
    if (status)
        goto fail;


    status = _svg_style_render (virtual_node, engine, closure);
    if (status)
        goto fail;

    _svg_destroy_dom_node (virtual_node);

    return SVG_STATUS_SUCCESS;

  fail:
    if (virtual_node != NULL)
        _svg_destroy_dom_node (virtual_node);

    return status;
}

static svg_status_t
_svg_marker_render_group_end (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_status_t status;

    status = _svg_engine_end_group (engine, closure, 1.0);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_marker_init (svg_marker_int_t *marker, svg_element_t *marker_element)
{
    svg_status_t status;
    svg_container_t *marker_container = &marker->container;

    status = _svg_container_init (marker_container);
    if (status)
        return status;

    status = _svg_create_element_ref (marker_element, &marker->ext_marker.element_ref);
    if (status)
        return status;

    _svg_length_init_unit (&marker->ref_x, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&marker->ref_y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    _svg_length_init_unit (&marker->width, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&marker->height, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    _svg_view_box_init (&marker->view_box);

    marker->stroke_width = 0.0;

    marker->enable_render = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_marker_init_copy (svg_marker_int_t *marker, svg_element_t *marker_element,
                       svg_marker_int_t *other)
{
    svg_status_t status;
    svg_marker_t *ext_marker = &marker->ext_marker;
    svg_container_t *marker_container = &marker->container;
    svg_marker_t *other_ext = &other->ext_marker;
    svg_container_t *other_container = &other->container;

    status = _svg_container_init_copy (marker_container, other_container);
    if (status)
        return status;

    status = _svg_create_element_ref (marker_element, &ext_marker->element_ref);
    if (status)
        return status;

    marker->ref_x = other->ref_x;
    marker->ref_y = other->ref_y;
    marker->units = other->units;
    marker->width = other->width;
    marker->height = other->height;
    marker->stroke_width = other->stroke_width;
    marker->view_box = other->view_box;

    ext_marker->angle = other_ext->angle;
    ext_marker->auto_angle = other_ext->auto_angle;

    marker->enable_render = other->enable_render;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_marker_deinit (svg_marker_int_t *marker)
{
    svg_status_t status;

    status = _svg_container_deinit (&marker->container);
    if (status)
        return status;

    if (marker->ext_marker.element_ref != NULL)
        _svg_destroy_element_ref (marker->ext_marker.element_ref);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_marker_apply_attributes (svg_element_t *marker_element, const svg_qattrs_t *attributes)
{
    svg_marker_int_t *marker = &marker_element->e.marker;
    svg_marker_t *ext_marker = &marker->ext_marker;
    const char *str;
    svg_status_t status;

    _svg_style_reset_display (&marker_element->node->style);


    status = _svg_attribute_get_string (attributes, "markerUnits", &str, "strokeWidth");
    if (strcmp (str, "userSpaceOnUse") == 0)
        marker->units = SVG_MARKER_UNITS_USER;
    else if (strcmp (str, "strokeWidth") == 0)
        marker->units = SVG_MARKER_UNITS_STROKE_WIDTH;
    else
        return _svg_element_return_property_error (marker_element, "markerUnits",
                                                   SVG_STATUS_INVALID_VALUE);

    status = _svg_attribute_get_length (attributes, "refX", &marker->ref_x, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (marker_element, "refX", status);

    status = _svg_attribute_get_length (attributes, "refY", &marker->ref_y, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (marker_element, "refY", status);

    status = _svg_attribute_get_length (attributes, "markerWidth", &marker->width, "3");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (marker_element, "markerWidth", status);

    status = _svg_attribute_get_length (attributes, "markerHeight", &marker->height, "3");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (marker_element, "markerHeight", status);

    _svg_attribute_get_string (attributes, "orient", &str, "0");
    if (strcmp (str, "auto") == 0) {
        ext_marker->auto_angle = 1;
    } else {
        status = _svg_str_parse_angle (str, &ext_marker->angle);
        if (status)
            return _svg_element_return_property_error (marker_element, "orient", status);

        ext_marker->auto_angle = 0;
    }

    _svg_attribute_get_string (attributes, "viewBox", &str, NULL);
    if (str != NULL) {
        status = _svg_parse_view_box (str, &marker->view_box);
        if (status)
            return _svg_element_return_property_error (marker_element, "viewBox", status);

        _svg_attribute_get_string (attributes, "preserveAspectRatio", &str, "xMidYMid meet");
        status = _svg_view_box_parse_aspect_ratio (str, &marker->view_box);
        if (status)
            return _svg_element_return_property_error (marker_element, "preserveAspectRatio",
                                                       status);
    }

    return SVG_STATUS_SUCCESS;
}

int
_svg_marker_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_container_peek_render (&element->e.marker.container, engine);
}

svg_status_t
_svg_marker_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_marker_int_t *marker = &element->e.marker;
    svg_status_t status;
    svg_length_t viewport_x, viewport_y;
    double ref_x, ref_y;
    double viewport_ref_x, viewport_ref_y;
    double marker_width, marker_height;
    svg_transform_t transform;

    if (element->svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;

    if (!marker->enable_render)
        return SVG_STATUS_SUCCESS;


    _svg_length_init_unit (&viewport_x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&viewport_y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);


    status = _svg_marker_render_group_start (element, engine, closure);
    if (status)
        return status;


    status = _svg_engine_measure_position (engine, closure,
                                           &marker->width, &marker->height,
                                           &marker_width, &marker_height);
    if (status)
        return status;

    status = _svg_engine_measure_position (engine, closure,
                                           &marker->ref_x, &marker->ref_y, &ref_x, &ref_y);
    if (status)
        return status;

    if (!_svg_view_box_is_null (&marker->view_box)) {
        if (marker->ref_x.unit == SVG_LENGTH_UNIT_PCT)
            ref_x = (marker->ref_x.value / 100.0) * marker->view_box.box.width;
        if (marker->ref_y.unit == SVG_LENGTH_UNIT_PCT)
            ref_y = (marker->ref_y.value / 100.0) * marker->view_box.box.height;

        status =
            _svg_view_box_transform (&marker->view_box, marker_width, marker_height, &transform);
        if (status)
            return status;

        status = _svg_transform_coordinate (&transform,
                                            ref_x, ref_y, &viewport_ref_x, &viewport_ref_y);
        if (status)
            return status;

        viewport_x.value = -viewport_ref_x;
        viewport_y.value = -viewport_ref_y;
    } else {
        _svg_length_init_unit (&viewport_x, -ref_x, SVG_LENGTH_UNIT_PX,
                               SVG_LENGTH_ORIENTATION_HORIZONTAL);
        _svg_length_init_unit (&viewport_y, -ref_y, SVG_LENGTH_UNIT_PX,
                               SVG_LENGTH_ORIENTATION_VERTICAL);
    }


    status = _svg_engine_begin_group (engine, closure,
                                      _svg_style_get_opacity (&element->node->style),
                                      element->id, element->klass);
    if (status)
        return status;

    status = _svg_engine_set_viewport (engine, closure,
                                       &viewport_x, &viewport_y, &marker->width, &marker->height);
    if (status)
        return status;

    if (!_svg_view_box_is_null (&marker->view_box)) {
        status = _svg_engine_apply_view_box (engine, closure,
                                             &marker->view_box, &marker->width, &marker->height);
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


    status = _svg_container_render (&marker->container, engine, closure);
    if (status)
        return status;


    status =
        _svg_engine_end_group (engine, closure, _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;


    status = _svg_marker_render_group_end (element, engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
svg_marker_render (const svg_marker_t *ext_marker, double stroke_width, svg_render_engine_t *engine,
                   void *closure)
{
    svg_marker_int_t *non_const_marker =
        (svg_marker_int_t *) & ext_marker->element_ref->element->e.marker;
    double original_stroke_width;
    svg_status_t status;

    original_stroke_width = non_const_marker->stroke_width;
    non_const_marker->stroke_width = stroke_width;

    status = svg_element_ref_render (non_const_marker->ext_marker.element_ref, engine, closure);

    non_const_marker->stroke_width = original_stroke_width;

    return status;
}

