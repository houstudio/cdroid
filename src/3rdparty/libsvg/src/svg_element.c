/* svg_element.c: Data structures for SVG graphics elements

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
_svg_element_create (svg_element_type_t type, svg_dom_node_t *node, svg_element_t *parent,
                     svg_t *svg, svg_element_t **element)
{
    svg_element_t *new_element;
    svg_status_t status;

    new_element = malloc (sizeof (svg_element_t));
    if (new_element == NULL)
        return SVG_STATUS_NO_MEMORY;

    status = _svg_element_init (new_element, type, node, parent, svg);
    if (status) {
        free (new_element);
        return status;
    }

    *element = new_element;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_init (svg_element_t *element, svg_element_type_t type, svg_dom_node_t *node,
                   svg_element_t *parent, svg_t *svg)
{
    svg_status_t status;

    memset (element, 0, sizeof (svg_element_t));

    element->type = type;
    element->parent = parent;
    element->svg = svg;
    element->node = node;

    status = _svg_transform_init (&element->transform);
    if (status)
        return status;

    switch (type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
        status = _svg_svg_group_init (&element->e.svg_group);
        break;
    case SVG_ELEMENT_TYPE_DEFS:
    case SVG_ELEMENT_TYPE_GROUP:
        status = _svg_group_init (&element->e.group);
        break;
    case SVG_ELEMENT_TYPE_USE:
        status = _svg_use_init (&element->e.use);
        break;
    case SVG_ELEMENT_TYPE_SYMBOL:
        status = _svg_symbol_init (&element->e.symbol);
        break;
    case SVG_ELEMENT_TYPE_PATH:
        status = _svg_path_init (&element->e.path);
        break;
    case SVG_ELEMENT_TYPE_CIRCLE:
        status = _svg_circle_init (&element->e.circle);
        break;
    case SVG_ELEMENT_TYPE_ELLIPSE:
        status = _svg_ellipse_init (&element->e.ellipse);
        break;
    case SVG_ELEMENT_TYPE_LINE:
        status = _svg_line_init (&element->e.line);
        break;
    case SVG_ELEMENT_TYPE_RECT:
        status = _svg_rect_init (&element->e.rect);
        break;
    case SVG_ELEMENT_TYPE_TEXT:
        status = _svg_text_init (&element->e.text);
        break;
    case SVG_ELEMENT_TYPE_TSPAN:
        status = _svg_tspan_init (&element->e.tspan);
        break;
    case SVG_ELEMENT_TYPE_IMAGE:
        status = _svg_image_init (&element->e.image);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT:
        status = _svg_gradient_init (&element->e.gradient);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT_STOP:
        status = _svg_gradient_stop_init (&element->e.gradient_stop);
        break;
    case SVG_ELEMENT_TYPE_PATTERN:
        status = _svg_pattern_init (&element->e.pattern, element);
        break;
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        status = _svg_clip_path_init (&element->e.clip_path, element);
        break;
    case SVG_ELEMENT_TYPE_MASK:
        status = _svg_mask_init (&element->e.mask, element);
        break;
    case SVG_ELEMENT_TYPE_MARKER:
        status = _svg_marker_init (&element->e.marker, element);
        break;
    }
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_init_copy (svg_element_t *element, svg_element_t *other)
{
    svg_status_t status = SVG_STATUS_SUCCESS;

    memset (element, 0, sizeof (svg_element_t));

    element->type = other->type;
    element->parent = other->parent;
    element->svg = other->svg;
    element->node = other->node;
    if (other->id)
        element->id = strdup (other->id);
    if (other->klass)
        element->klass = strdup (other->klass);

    element->transform = other->transform;

    switch (other->type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
        status = _svg_svg_group_init_copy (&element->e.svg_group, &other->e.svg_group);
        break;
    case SVG_ELEMENT_TYPE_DEFS:
    case SVG_ELEMENT_TYPE_GROUP:
        status = _svg_group_init_copy (&element->e.group, &other->e.group);
        break;
    case SVG_ELEMENT_TYPE_USE:
        status = _svg_use_init_copy (&element->e.use, &other->e.use);
        break;
    case SVG_ELEMENT_TYPE_SYMBOL:
        status = _svg_symbol_init_copy (&element->e.symbol, &other->e.symbol);
        break;
    case SVG_ELEMENT_TYPE_PATH:
        status = _svg_path_init_copy (&element->e.path, &other->e.path);
        break;
    case SVG_ELEMENT_TYPE_CIRCLE:
        status = _svg_circle_init_copy (&element->e.circle, &other->e.circle);
        break;
    case SVG_ELEMENT_TYPE_ELLIPSE:
        status = _svg_ellipse_init_copy (&element->e.ellipse, &other->e.ellipse);
        break;
    case SVG_ELEMENT_TYPE_LINE:
        status = _svg_line_init_copy (&element->e.line, &other->e.line);
        break;
    case SVG_ELEMENT_TYPE_RECT:
        status = _svg_rect_init_copy (&element->e.rect, &other->e.rect);
        break;
    case SVG_ELEMENT_TYPE_TEXT:
        status = _svg_text_init_copy (&element->e.text, &other->e.text);
        break;
    case SVG_ELEMENT_TYPE_TSPAN:
        status = _svg_tspan_init_copy (&element->e.tspan, &other->e.tspan);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT:
        status = _svg_gradient_init_copy (&element->e.gradient, &other->e.gradient);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT_STOP:
        status = _svg_gradient_stop_init_copy (&element->e.gradient_stop, &other->e.gradient_stop);
        break;
    case SVG_ELEMENT_TYPE_PATTERN:
        status = _svg_pattern_init_copy (&element->e.pattern, element, &other->e.pattern);
        break;
    case SVG_ELEMENT_TYPE_IMAGE:
        status = _svg_image_init_copy (&element->e.image, &other->e.image);
        break;
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        status = _svg_clip_path_init_copy (&element->e.clip_path, element, &other->e.clip_path);
        break;
    case SVG_ELEMENT_TYPE_MASK:
        status = _svg_mask_init_copy (&element->e.mask, element, &other->e.mask);
        break;
    case SVG_ELEMENT_TYPE_MARKER:
        status = _svg_marker_init_copy (&element->e.marker, element, &other->e.marker);
        break;
    }
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_deinit (svg_element_t *element)
{
    svg_status_t status = SVG_STATUS_SUCCESS;

    if (element->id)
        free (element->id);
    if (element->klass)
        free (element->klass);

    switch (element->type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
        status = _svg_svg_group_deinit (&element->e.svg_group);
        break;
    case SVG_ELEMENT_TYPE_DEFS:
    case SVG_ELEMENT_TYPE_GROUP:
        status = _svg_group_deinit (&element->e.group);
        break;
    case SVG_ELEMENT_TYPE_USE:
        status = _svg_use_deinit (&element->e.use);
        break;
    case SVG_ELEMENT_TYPE_SYMBOL:
        status = _svg_symbol_deinit (&element->e.symbol);
        break;
    case SVG_ELEMENT_TYPE_PATH:
        status = _svg_path_deinit (&element->e.path);
        break;
    case SVG_ELEMENT_TYPE_CIRCLE:
    case SVG_ELEMENT_TYPE_ELLIPSE:
    case SVG_ELEMENT_TYPE_LINE:
    case SVG_ELEMENT_TYPE_RECT:
        status = SVG_STATUS_SUCCESS;
        break;
    case SVG_ELEMENT_TYPE_TEXT:
        status = _svg_text_deinit (&element->e.text);
        break;
    case SVG_ELEMENT_TYPE_TSPAN:
        status = _svg_tspan_deinit (&element->e.tspan);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT:
        status = _svg_gradient_deinit (&element->e.gradient);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT_STOP:
        status = _svg_gradient_stop_deinit (&element->e.gradient_stop);
        break;
    case SVG_ELEMENT_TYPE_PATTERN:
        status = _svg_pattern_deinit (&element->e.pattern);
        break;
    case SVG_ELEMENT_TYPE_IMAGE:
        status = _svg_image_deinit (&element->e.image);
        break;
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        status = _svg_clip_path_deinit (&element->e.clip_path);
        break;
    case SVG_ELEMENT_TYPE_MASK:
        status = _svg_mask_deinit (&element->e.mask);
        break;
    case SVG_ELEMENT_TYPE_MARKER:
        status = _svg_marker_deinit (&element->e.marker);
        break;
    }
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_clone (svg_element_t *other, svg_element_t **element)
{
    svg_element_t *new_element;
    svg_status_t status;

    new_element = malloc (sizeof (svg_element_t));
    if (new_element == NULL)
        return SVG_STATUS_NO_MEMORY;

    status = _svg_element_init_copy (new_element, other);
    if (status) {
        free (new_element);
        return status;
    }

    *element = new_element;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_destroy (svg_element_t *element)
{
    svg_status_t status;

    if (element == NULL)
        return SVG_STATUS_SUCCESS;

    status = _svg_element_deinit (element);

    free (element);

    return status;
}

svg_status_t
_svg_create_virtual_element (svg_element_type_t type, svg_dom_node_t *style_node,
                             svg_element_t *parent, svg_t *svg, svg_element_t **element)
{
    svg_dom_node_t *virtual_node;
    svg_status_t status;

    status = _svg_create_dom_node (ELEMENT_NODE_TYPE, &virtual_node);
    if (status)
        return status;

    virtual_node->parent = style_node;

    status = _svg_style_init_inherit (&virtual_node->style, &style_node->style);
    if (status) {
        _svg_destroy_dom_node (virtual_node);
        return status;
    }

    status = _svg_element_create (type, virtual_node, parent, svg, element);
    if (status) {
        _svg_destroy_dom_node (virtual_node);
        return status;
    }

    return SVG_STATUS_SUCCESS;
}

int
_svg_element_peek_render (svg_element_t *element, svg_render_engine_t *engine)
{
    switch (element->type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
        return _svg_svg_group_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_GROUP:
        return _svg_group_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_PATH:
        return _svg_path_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_CIRCLE:
        return _svg_circle_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_ELLIPSE:
        return _svg_ellipse_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_LINE:
        return _svg_line_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_RECT:
        return _svg_rect_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_TEXT:
        return _svg_text_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_IMAGE:
        return _svg_image_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_PATTERN:
        return _svg_pattern_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        return _svg_clip_path_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_MASK:
        return _svg_mask_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_USE:
        return _svg_use_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_MARKER:
        return _svg_marker_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_TSPAN:
        return _svg_tspan_peek_render (element, engine);
    case SVG_ELEMENT_TYPE_DEFS:
    case SVG_ELEMENT_TYPE_GRADIENT:
    case SVG_ELEMENT_TYPE_GRADIENT_STOP:
    case SVG_ELEMENT_TYPE_SYMBOL:
        return 0;
    }

    return 0;
}

svg_status_t
svg_element_render (svg_element_t *element, svg_render_engine_t *target_engine,
                    void *target_closure)
{
    svg_status_t status = SVG_STATUS_SUCCESS;
    svg_render_engine_t *engine;
    void *closure;

    if (element->svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;

    if (!_svg_element_peek_render (element, target_engine))
        return SVG_STATUS_SUCCESS;

    if (element->svg->trace != NULL) {
        status = _svg_trace_push_target_engine (element->svg->trace, element->node->document_uri,
                                                target_engine, target_closure);
        if (status)
            return status;

        _svg_trace_get_engine (element->svg->trace, &engine, &closure);

        _svg_trace_set_position_info (element->svg->trace, element->node->qname->local_name,
                                      element->node->line_number);
    } else {
        engine = target_engine;
        closure = target_closure;
    }


    if (!_svg_style_get_display (&element->node->style)) {
        status = SVG_STATUS_SUCCESS;
        goto end;
    }

    switch (element->type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
        status = _svg_svg_group_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_GROUP:
        status = _svg_group_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_PATH:
        status = _svg_path_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_CIRCLE:
        status = _svg_circle_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_ELLIPSE:
        status = _svg_ellipse_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_LINE:
        status = _svg_line_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_RECT:
        status = _svg_rect_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_TEXT:
        status = _svg_text_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_IMAGE:
        status = _svg_image_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_PATTERN:
        status = _svg_pattern_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        status = _svg_clip_path_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_MASK:
        status = _svg_mask_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_USE:
        status = _svg_use_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_MARKER:
        status = _svg_marker_render (element, engine, closure);
        break;
    case SVG_ELEMENT_TYPE_DEFS:
    case SVG_ELEMENT_TYPE_GRADIENT:
    case SVG_ELEMENT_TYPE_GRADIENT_STOP:
    case SVG_ELEMENT_TYPE_SYMBOL:
    case SVG_ELEMENT_TYPE_TSPAN:
        /* these elements are not rendered directly */
        status = SVG_STATUS_SUCCESS;
        break;
    }

    if (status) {
        _svg_set_error_status (element->svg, status);
        _svg_set_error_line_number (element->svg, element->node->line_number);
        _svg_set_error_element_name (element->svg, element->node->qname);
    }

  end:
    if (element->svg->trace != NULL)
        _svg_trace_pop_target_engine (element->svg->trace);

    return status;
}

svg_status_t
_svg_element_apply_properties (svg_element_t *element)
{
    svg_status_t status;
    const char *id;
    const char *klass;

    status = _svg_transform_apply_attributes (element, &element->transform, element->node->qattrs);
    if (status)
        return status;

    _svg_attribute_get_string (element->node->qattrs, "id", &id, NULL);
    if (id == NULL)
        _svg_attribute_get_string_ns (element->node->qattrs, XML_NAMESPACE_INDEX, "id", &id, NULL);
    if (id != NULL)
        element->id = strdup (id);

    _svg_attribute_get_string (element->node->qattrs, "class", &klass, NULL);
    if (klass != NULL)
        element->klass = strdup (klass);

    switch (element->type) {
    case SVG_ELEMENT_TYPE_SVG_GROUP:
        status = _svg_svg_group_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_GROUP:
        status = _svg_group_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_SYMBOL:
        status = _svg_symbol_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_USE:
        status = _svg_use_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_PATH:
        status = _svg_path_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_DEFS:
        break;
    case SVG_ELEMENT_TYPE_RECT:
        status = _svg_rect_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_CIRCLE:
        status = _svg_circle_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_ELLIPSE:
        status = _svg_ellipse_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_LINE:
        status = _svg_line_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_TEXT:
        status = _svg_text_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_TSPAN:
        status = _svg_tspan_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_IMAGE:
        status = _svg_image_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT:
        status = _svg_gradient_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_GRADIENT_STOP:
        status = _svg_gradient_stop_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_PATTERN:
        status = _svg_pattern_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        status = _svg_clip_path_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_MASK:
        status = _svg_mask_apply_attributes (element, element->node->qattrs);
        break;
    case SVG_ELEMENT_TYPE_MARKER:
        status = _svg_marker_apply_attributes (element, element->node->qattrs);
        break;
    }

    if (status) {
        if (status == SVG_STATUS_UNKNOWN_REF_ELEMENT) {
            /* clean-up to allow this function to be called again */
            if (element->id != NULL) {
                free (element->id);
                element->id = NULL;
            }
            if (element->klass != NULL) {
                free (element->klass);
                element->klass = NULL;
            }
        }

        return status;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_element_return_property_error (svg_element_t *element, const char *property_name,
                                    svg_status_t error_status)
{
    _svg_set_error_property_name (element->svg, property_name);
    _svg_set_error_status (element->svg, error_status);

    return error_status;
}

