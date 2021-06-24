/* svg_pattern.c: Data structures for SVG pattern elements

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

   Author: Steven Kramer
*/

#include "svgint.h"


svg_status_t
_svg_pattern_init (svg_pattern_int_t *pattern, svg_element_t *pattern_element)
{
    svg_status_t status;
    svg_container_t *pattern_container = &pattern->container;
    svg_pattern_t *ext_pattern = &pattern->ext_pattern;

    status = _svg_container_init (pattern_container);
    if (status)
        return status;

    status = _svg_create_element_ref (pattern_element, &pattern->ext_pattern.element_ref);
    if (status)
        return status;

    ext_pattern->units = SVG_COORD_SPACE_UNITS_BBOX;
    ext_pattern->content_units = SVG_COORD_SPACE_UNITS_USER;

    _svg_length_init_unit (&ext_pattern->x, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&ext_pattern->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&ext_pattern->width, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&ext_pattern->height, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_VERTICAL);

    _svg_view_box_init (&pattern->view_box);

    _svg_transform_init (&ext_pattern->transform);

    ext_pattern->have_viewbox = 0;

    pattern->num_inherited_children = 0;

    pattern->enable_render = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_pattern_init_copy (svg_pattern_int_t *pattern, svg_element_t *pattern_element,
                        svg_pattern_int_t *other)
{
    svg_status_t status;
    svg_pattern_t *ext_pattern = &pattern->ext_pattern;
    svg_container_t *pattern_container = &pattern->container;
    svg_pattern_t *other_ext = &other->ext_pattern;
    svg_container_t *other_container = &other->container;

    status = _svg_container_init_copy (pattern_container, other_container);
    if (status)
        return status;

    status = _svg_create_element_ref (pattern_element, &ext_pattern->element_ref);
    if (status)
        return status;

    ext_pattern->units = other_ext->units;
    ext_pattern->content_units = other_ext->content_units;
    ext_pattern->x = other_ext->x;
    ext_pattern->y = other_ext->y;
    ext_pattern->width = other_ext->width;
    ext_pattern->height = other_ext->height;
    ext_pattern->have_viewbox = other_ext->have_viewbox;
    ext_pattern->transform = other_ext->transform;

    pattern->view_box = other->view_box;
    pattern->num_inherited_children = other->num_inherited_children;
    pattern->enable_render = other->enable_render;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_pattern_deinit (svg_pattern_int_t *pattern)
{
    svg_status_t status;

    status = _svg_container_deinit (&pattern->container);
    if (status)
        return status;

    if (pattern->ext_pattern.element_ref != NULL)
        _svg_destroy_element_ref (pattern->ext_pattern.element_ref);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_pattern_apply_attributes (svg_element_t *pattern_element, const svg_qattrs_t *attributes)
{
    svg_pattern_int_t *pattern = &pattern_element->e.pattern;
    svg_container_t *pattern_container = &pattern->container;
    svg_pattern_t *ext_pattern = &pattern->ext_pattern;
    const char *href;
    svg_element_t *prototype_element;
    svg_pattern_int_t *prototype = NULL;
    svg_pattern_t *ext_prototype = NULL;
    svg_container_t *prototype_container = NULL;
    const char *str;
    svg_status_t status;
    svg_length_t h_len, v_len;

    _svg_length_init_unit (&h_len, 0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&v_len, 0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_VERTICAL);


    _svg_style_reset_display (&pattern_element->node->style);

    _svg_attribute_get_string_ns (attributes, XLINK_NAMESPACE_INDEX, "href", &href, NULL);
    if (href != NULL) {
        status = _svg_resolve_element_href (pattern_element->svg,
                                            pattern_element->node->document_uri,
                                            pattern_element->node->base_uri,
                                            href, &prototype_element);
        if (status) {
            if (status == SVG_STATUS_UNKNOWN_REF_ELEMENT)
                return status;  /* don't set error here because next time round it could be resolved */
            else
                return _svg_element_return_property_error (pattern_element, "href", status);
        }

        if (prototype_element->type != SVG_ELEMENT_TYPE_PATTERN)
            return _svg_element_return_property_error (pattern_element, "href",
                                                       SVG_STATUS_INVALID_VALUE);

        prototype = &prototype_element->e.pattern;
        ext_prototype = &prototype->ext_pattern;
        prototype_container = &prototype->container;

        if (pattern_container->num_elements == 0) {
            _svg_container_deinit (pattern_container);

            status = _svg_container_init_copy (pattern_container, prototype_container);
            if (status)
                return _svg_element_return_property_error (pattern_element, "href", status);

            pattern->num_inherited_children = pattern_container->num_elements;
        }

        ext_pattern->units = ext_prototype->units;
        ext_pattern->content_units = ext_prototype->content_units;
        ext_pattern->x = ext_prototype->x;
        ext_pattern->y = ext_prototype->y;
        ext_pattern->width = ext_prototype->width;
        ext_pattern->height = ext_prototype->height;
        ext_pattern->have_viewbox = ext_prototype->have_viewbox;
        ext_pattern->transform = ext_prototype->transform;

        pattern->view_box = prototype->view_box;
    }

    status = _svg_attribute_get_string (attributes, "patternUnits", &str, "objectBoundingBox");
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND) {
        if (strcmp (str, "userSpaceOnUse") == 0)
            ext_pattern->units = SVG_COORD_SPACE_UNITS_USER;
        else if (strcmp (str, "objectBoundingBox") == 0)
            ext_pattern->units = SVG_COORD_SPACE_UNITS_BBOX;
        else
            return _svg_element_return_property_error (pattern_element, "patternUnits",
                                                       SVG_STATUS_INVALID_VALUE);
    }

    status = _svg_attribute_get_string (attributes, "patternContentUnits", &str, "userSpaceOnUse");
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND) {
        if (strcmp (str, "userSpaceOnUse") == 0)
            ext_pattern->content_units = SVG_COORD_SPACE_UNITS_USER;
        else if (strcmp (str, "objectBoundingBox") == 0)
            ext_pattern->content_units = SVG_COORD_SPACE_UNITS_BBOX;
        else
            return _svg_element_return_property_error (pattern_element, "patternContentUnits",
                                                       SVG_STATUS_INVALID_VALUE);
    }

    status = _svg_attribute_get_length (attributes, "x", &h_len, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (pattern_element, "x", status);
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        ext_pattern->x = h_len;

    status = _svg_attribute_get_length (attributes, "y", &v_len, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (pattern_element, "y", status);
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        ext_pattern->y = v_len;

    status = _svg_attribute_get_length (attributes, "width", &h_len, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (pattern_element, "width", status);
    if (h_len.value < 0)
        return _svg_element_return_property_error (pattern_element, "width",
                                                   SVG_STATUS_INVALID_VALUE);
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        ext_pattern->width = h_len;

    status = _svg_attribute_get_length (attributes, "height", &v_len, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (pattern_element, "height", status);
    if (v_len.value < 0)
        return _svg_element_return_property_error (pattern_element, "height",
                                                   SVG_STATUS_INVALID_VALUE);
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        ext_pattern->height = v_len;


    status = _svg_attribute_get_string (attributes, "patternTransform", &str, NULL);
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND) {
        status = _svg_transform_parse_str (&ext_pattern->transform, str);
        if (status)
            return _svg_element_return_property_error (pattern_element, "patternTransform", status);
    }


    status = _svg_attribute_get_string (attributes, "viewBox", &str, NULL);
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND) {
        status = _svg_parse_view_box (str, &pattern->view_box);
        if (status)
            return _svg_element_return_property_error (pattern_element, "viewBox", status);

        ext_pattern->have_viewbox = 1;
    }

    if (ext_pattern->have_viewbox) {
        _svg_attribute_get_string (attributes, "preserveAspectRatio", &str, "xMidYMid meet");
        status = _svg_view_box_parse_aspect_ratio (str, &pattern->view_box);
        if (status)
            return _svg_element_return_property_error (pattern_element, "preserveAspectRatio",
                                                       status);
    }

    return SVG_STATUS_SUCCESS;
}

int
_svg_pattern_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_container_peek_render (&element->e.pattern.container, engine);
}

svg_status_t
_svg_pattern_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_pattern_int_t *pattern = &element->e.pattern;
    svg_length_t zero_horiz_length, zero_vert_length;
    svg_status_t status;

    if (!pattern->enable_render)
        return SVG_STATUS_SUCCESS;

    _svg_length_init_unit (&zero_horiz_length, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&zero_vert_length, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_VERTICAL);


    status = _svg_engine_begin_group (engine, closure,
                                      _svg_style_get_opacity (&element->node->style),
                                      element->id, element->klass);
    if (status)
        return status;
    status = _svg_engine_set_viewport (engine, closure, &zero_horiz_length, &zero_vert_length,
                                       &pattern->ext_pattern.width, &pattern->ext_pattern.height);
    if (status)
        return status;

    if (!_svg_view_box_is_null (&pattern->view_box)) {
        status = _svg_engine_apply_view_box (engine, closure, &pattern->view_box,
                                             &pattern->ext_pattern.width,
                                             &pattern->ext_pattern.height);
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


    status = _svg_container_render (&pattern->container, engine, closure);
    if (status)
        return status;


    status =
        _svg_engine_end_group (engine, closure, _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

