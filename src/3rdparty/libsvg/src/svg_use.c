/* svg_use.c: Data structures for SVG use elements

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
_svg_use_init (svg_use_t *use)
{
    svg_status_t status;

    status = _svg_group_init (&use->group);
    if (status)
        return status;

    _svg_length_init_unit (&use->x, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&use->y, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_use_init_copy (svg_use_t *use, svg_use_t *other)
{
    svg_status_t status;

    status = _svg_group_init_copy (&use->group, &other->group);
    if (status)
        return status;

    use->x = other->x;
    use->y = other->y;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_use_deinit (svg_use_t *use)
{
    svg_status_t status;

    status = _svg_group_deinit (&use->group);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_use_apply_attributes (svg_element_t *use_element, const svg_qattrs_t *attributes)
{
    svg_use_t *use = &use_element->e.use;
    svg_status_t status;
    const char *href;
    const char *width_str, *height_str;
    svg_element_t *ref_element;
    svg_dom_node_t *cloned_node;
    int have_width, have_height;


    _svg_attribute_get_string_ns (attributes, XLINK_NAMESPACE_INDEX, "href", &href, NULL);
    if (href == NULL)
        return _svg_element_return_property_error (use_element, "href", SVG_STATUS_INVALID_VALUE);

    status = _svg_resolve_element_href (use_element->svg, use_element->node->document_uri,
                                        use_element->node->base_uri, href, &ref_element);
    if (status) {
        if (status == SVG_STATUS_UNKNOWN_REF_ELEMENT)
            return status;   /* don't set error here because next time round it could be resolved */
        else
            return _svg_element_return_property_error (use_element, "href", status);
    }


    status = _svg_attribute_get_length (attributes, "x", &use->x, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (use_element, "x", status);

    status = _svg_attribute_get_length (attributes, "y", &use->y, "0");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (use_element, "y", status);


    status = _svg_attribute_get_string (attributes, "width", &width_str, "100%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (use_element, "width", status);
    have_width = (status == SVG_STATUS_SUCCESS);

    status = _svg_attribute_get_string (attributes, "height", &height_str, "100%");
    if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
        return _svg_element_return_property_error (use_element, "height", status);
    have_height = (status == SVG_STATUS_SUCCESS);


    status = _svg_dom_deep_clone_node (use_element->node, ref_element->node, &cloned_node);
    if (status)
        return status;

    cloned_node->override_display_property = 1;

    if (ref_element->type == SVG_ELEMENT_TYPE_SYMBOL) {
        status = _svg_dom_node_set_qname (cloned_node, SVG_NAMESPACE_INDEX, "svg");
        if (status)
            return status;

        status = _svg_dom_node_set_qattr (cloned_node, SVG_NAMESPACE_INDEX, "x", "0");
        if (status)
            return status;
        status = _svg_dom_node_set_qattr (cloned_node, SVG_NAMESPACE_INDEX, "y", "0");
        if (status)
            return status;
        status = _svg_dom_node_set_qattr (cloned_node, SVG_NAMESPACE_INDEX, "width", width_str);
        if (status)
            return status;
        status = _svg_dom_node_set_qattr (cloned_node, SVG_NAMESPACE_INDEX, "height", height_str);
        if (status)
            return status;
    } else if (ref_element->type == SVG_ELEMENT_TYPE_SVG_GROUP) {
        if (have_width) {
            status =
                _svg_dom_node_set_qattr (cloned_node, SVG_NAMESPACE_INDEX, "width", width_str);
            if (status)
                return status;
        }
        if (have_height) {
            status =
                _svg_dom_node_set_qattr (cloned_node, SVG_NAMESPACE_INDEX, "height", height_str);
            if (status)
                return status;
        }
    }


    return SVG_STATUS_SUCCESS;
}

int
_svg_use_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    return _svg_container_peek_render (&element->e.use.group.container, engine);
}

svg_status_t
_svg_use_render (svg_element_t *element, svg_render_engine_t *engine, void *closure)
{
    svg_use_t *use = &element->e.use;
    svg_transform_t transform;
    double x, y;
    svg_status_t status;

    status = _svg_engine_begin_group (engine, closure,
                                      _svg_style_get_opacity (&element->node->style),
                                      element->id, element->klass);
    if (status)
        return status;


    status = _svg_engine_measure_position (engine, closure, &use->x, &use->y, &x, &y);
    if (status)
        return status;

    _svg_transform_init_translate (&transform, x, y);
    _svg_transform_multiply_into_left (&transform, &element->transform);

    status = _svg_transform_render (&transform, engine, closure);
    if (status)
        return status;

    status = _svg_engine_end_transform (engine, closure);
    if (status)
        return status;


    status = _svg_style_render (element->node, engine, closure);
    if (status)
        return status;



    status = _svg_container_render (&use->group.container, engine, closure);
    if (status)
        return status;


    status =
        _svg_engine_end_group (engine, closure, _svg_style_get_opacity (&element->node->style));
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

