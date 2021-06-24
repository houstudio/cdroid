/* svg_parser.c: SAX-based parser for SVG documents

   Copyright (C) 2000 Eazel, Inc.
   Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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

   Author: Raph Levien <raph@artofcode.com>
*/

#include "svgint.h"


typedef struct svg_parser_map {
    char *name;
    svg_parser_cb_t cb;
} svg_parser_map_t;




static svg_status_t
_svg_parser_push_state (svg_parser_t *parser, const svg_parser_cb_t *cb)
{
    svg_parser_state_t *state;

    state = malloc (sizeof (svg_parser_state_t));
    if (state == NULL)
        return SVG_STATUS_NO_MEMORY;

    if (parser->state) {
        *state = *parser->state;
    } else {
        state->container_element = NULL;
        state->text = NULL;
    }

    state->cb = cb;

    state->next = parser->state;
    parser->state = state;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_pop_state (svg_parser_t *parser)
{
    svg_parser_state_t *old;

    if (parser->state == NULL)
        return SVG_STATUS_SUCCESS;

    old = parser->state;
    parser->state = parser->state->next;
    free (old);

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_new_svg_group_element (svg_parser_t *parser, svg_dom_node_t *node,
                                   svg_element_t **svg_group_element)
{
    svg_status_t status;
    svg_element_t *parent;

    parent = parser->state->container_element;

    status = _svg_element_create (SVG_ELEMENT_TYPE_SVG_GROUP, node, parent, parser->svg,
                                  svg_group_element);
    if (status)
        return status;

    if (parent)
        status = _svg_container_add_element (&parent->e.container, *svg_group_element);
    else
        parser->svg->root_element = *svg_group_element;

    parser->state->container_element = *svg_group_element;

    return status;
}

static svg_status_t
_svg_parser_new_leaf_element (svg_parser_t *parser, svg_dom_node_t *node,
                              svg_element_t **child_element, svg_element_type_t type)
{
    svg_status_t status;
    svg_element_t *new_element;

    status = _svg_element_create (type, node, parser->state->container_element, parser->svg,
                                  &new_element);
    if (status)
        return status;

    status = _svg_container_add_element (&parser->state->container_element->e.container,
                                         new_element);
    if (status) {
        _svg_element_destroy (new_element);
        return status;
    }

    *child_element = new_element;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_new_virtual_leaf_element (svg_parser_t *parser, svg_dom_node_t *node,
                                      svg_element_t **child_element, svg_element_type_t type)
{
    svg_status_t status;
    svg_element_t *new_element;

    status = _svg_create_virtual_element (type, node, parser->state->container_element, parser->svg,
                                          &new_element);
    if (status)
        return status;

    status = _svg_container_add_element (&parser->state->container_element->e.container,
                                         new_element);
    if (status) {
        _svg_element_destroy (new_element);
        return status;
    }

    *child_element = new_element;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_new_container_element (svg_parser_t *parser, svg_dom_node_t *node,
                                   svg_element_t **container_element, svg_element_type_t type)
{
    svg_status_t status;

    status = _svg_parser_new_leaf_element (parser, node, container_element, type);
    if (status)
        return status;

    parser->state->container_element = *container_element;

    return status;
}

static svg_status_t
_svg_parser_parse_anchor (svg_parser_t *parser, svg_dom_node_t *node,
                          svg_element_t **anchor_element)
{
    /* XXX: pretending anchor is a group */
    return _svg_parser_new_container_element (parser, node, anchor_element, SVG_ELEMENT_TYPE_GROUP);
}

static svg_status_t
_svg_parser_parse_svg (svg_parser_t *parser, svg_dom_node_t *node,
                       svg_element_t **svg_group_element)
{
    return _svg_parser_new_svg_group_element (parser, node, svg_group_element);
}

static svg_status_t
_svg_parser_parse_group (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **group_element)
{
    return _svg_parser_new_container_element (parser, node, group_element, SVG_ELEMENT_TYPE_GROUP);
}

static svg_status_t
_svg_parser_parse_defs (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **defs_element)
{
    return _svg_parser_new_container_element (parser, node, defs_element, SVG_ELEMENT_TYPE_DEFS);
}

static svg_status_t
_svg_parser_parse_use (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **use_element)
{
    return _svg_parser_new_container_element (parser, node, use_element, SVG_ELEMENT_TYPE_USE);
}

static svg_status_t
_svg_parser_parse_symbol (svg_parser_t *parser, svg_dom_node_t *node,
                          svg_element_t **symbol_element)
{
    return _svg_parser_new_container_element (parser, node, symbol_element,
                                              SVG_ELEMENT_TYPE_SYMBOL);
}

static svg_status_t
_svg_parser_parse_path (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **path_element)
{
    svg_status_t status;
    svg_element_t *new_element;

    status = _svg_parser_new_leaf_element (parser, node, &new_element, SVG_ELEMENT_TYPE_PATH);
    if (status)
        return status;

    new_element->e.path.type = SVG_PATH_TYPE_PATH;

    *path_element = new_element;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_line (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **line_element)
{
    return _svg_parser_new_leaf_element (parser, node, line_element, SVG_ELEMENT_TYPE_LINE);
}


static svg_status_t
_svg_parser_parse_rect (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **rect_element)
{
    return _svg_parser_new_leaf_element (parser, node, rect_element, SVG_ELEMENT_TYPE_RECT);
}

static svg_status_t
_svg_parser_parse_circle (svg_parser_t *parser, svg_dom_node_t *node,
                          svg_element_t **circle_element)
{
    return _svg_parser_new_leaf_element (parser, node, circle_element, SVG_ELEMENT_TYPE_CIRCLE);
}

static svg_status_t
_svg_parser_parse_ellipse (svg_parser_t *parser, svg_dom_node_t *node,
                           svg_element_t **ellipse_element)
{
    return _svg_parser_new_leaf_element (parser, node, ellipse_element, SVG_ELEMENT_TYPE_ELLIPSE);
}

static svg_status_t
_svg_parser_parse_polygon (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **path_element)
{
    svg_status_t status;
    svg_element_t *new_element;

    status = _svg_parser_new_leaf_element (parser, node, &new_element, SVG_ELEMENT_TYPE_PATH);
    if (status)
        return status;

    new_element->e.path.type = SVG_PATH_TYPE_POLYGON;

    *path_element = new_element;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_polyline (svg_parser_t *parser, svg_dom_node_t *node,
                            svg_element_t **path_element)
{
    svg_status_t status;
    svg_element_t *new_element;

    status = _svg_parser_new_leaf_element (parser, node, &new_element, SVG_ELEMENT_TYPE_PATH);
    if (status)
        return status;

    new_element->e.path.type = SVG_PATH_TYPE_POLYLINE;

    *path_element = new_element;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_text (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **text_element)
{
    svg_status_t status;

    status = _svg_parser_new_container_element (parser, node, text_element, SVG_ELEMENT_TYPE_TEXT);
    if (status)
        return status;

    parser->state->text = &(*text_element)->e.text;
    parser->state->text_tspan = NULL;
    parser->state->tspan = NULL;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_tspan (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **tspan_element)
{
    svg_status_t status;
    svg_text_t *text;

    text = parser->state->text;
    if (text == NULL)
        return SVG_STATUS_PARSE_ERROR;

    status = _svg_parser_new_leaf_element (parser, node, tspan_element, SVG_ELEMENT_TYPE_TSPAN);
    if (status)
        return status;

    parser->state->tspan = &(*tspan_element)->e.tspan;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_image (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **image_element)
{
    return _svg_parser_new_leaf_element (parser, node, image_element, SVG_ELEMENT_TYPE_IMAGE);
}

static svg_status_t
_svg_parser_parse_linear_gradient (svg_parser_t *parser, svg_dom_node_t *node,
                                   svg_element_t **gradient_element)
{
    svg_status_t status;

    status = _svg_parser_new_container_element (parser, node, gradient_element,
                                                SVG_ELEMENT_TYPE_GRADIENT);
    if (status)
        return status;

    _svg_gradient_set_type (&(*gradient_element)->e.gradient, SVG_GRADIENT_LINEAR);

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_radial_gradient (svg_parser_t *parser, svg_dom_node_t *node,
                                   svg_element_t **gradient_element)
{
    svg_status_t status;

    status = _svg_parser_new_container_element (parser, node, gradient_element,
                                                SVG_ELEMENT_TYPE_GRADIENT);
    if (status)
        return status;

    _svg_gradient_set_type (&(*gradient_element)->e.gradient, SVG_GRADIENT_RADIAL);

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_gradient_stop (svg_parser_t *parser, svg_dom_node_t *node,
                                 svg_element_t **stop_element)
{
    if (parser->state->container_element == NULL ||
        parser->state->container_element->type != SVG_ELEMENT_TYPE_GRADIENT)
    {
        return SVG_STATUS_PARSE_ERROR;
    }

    return _svg_parser_new_leaf_element (parser, node, stop_element,
                                         SVG_ELEMENT_TYPE_GRADIENT_STOP);
}

static svg_status_t
_svg_parser_parse_pattern (svg_parser_t *parser, svg_dom_node_t *node,
                           svg_element_t **pattern_element)
{
    svg_status_t status;

    status = _svg_parser_new_container_element (parser, node, pattern_element,
                                                SVG_ELEMENT_TYPE_PATTERN);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_clip_path (svg_parser_t *parser, svg_dom_node_t *node,
                             svg_element_t **clip_path_element)
{
    svg_status_t status;

    status = _svg_parser_new_container_element (parser, node, clip_path_element,
                                                SVG_ELEMENT_TYPE_CLIP_PATH);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_mask (svg_parser_t *parser, svg_dom_node_t *node, svg_element_t **mask_element)
{
    svg_status_t status;

    status = _svg_parser_new_container_element (parser, node, mask_element, SVG_ELEMENT_TYPE_MASK);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_marker (svg_parser_t *parser, svg_dom_node_t *node,
                          svg_element_t **marker_element)
{
    svg_status_t status;

    status = _svg_parser_new_container_element (parser, node, marker_element,
                                                SVG_ELEMENT_TYPE_MARKER);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_tspan_characters (svg_parser_t *parser, svg_dom_node_t *node, const char *ch,
                                    size_t len)
{
    svg_status_t status;
    svg_tspan_t *tspan;

    tspan = parser->state->tspan;
    if (tspan == NULL) {
        tspan = parser->state->text_tspan;
        if (tspan == NULL)
            return SVG_STATUS_PARSE_ERROR;
    }

    status = _svg_tspan_append_chars (tspan, ch, len);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_text_characters (svg_parser_t *parser, svg_dom_node_t *node, const char *ch,
                                   size_t len)
{
    svg_status_t status;
    svg_element_t *tspan_element;

    /* pretend the <text> character data is a <tspan> with character data */
    status = _svg_parser_new_virtual_leaf_element (parser, node->parent, &tspan_element,
                                                   SVG_ELEMENT_TYPE_TSPAN);
    if (status)
        return status;

    tspan_element->e.tspan.virtual_node = tspan_element->node;

    parser->state->text_tspan = &tspan_element->e.tspan;

    status = _svg_parser_parse_tspan_characters (parser, node, ch, len);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_gradient_end (svg_parser_t *parser, svg_element_t *gradient_element)
{
    svg_container_t *gradient_container = &gradient_element->e.container;
    svg_gradient_int_t *gradient = &gradient_element->e.gradient;
    svg_element_t *stop_element;
    svg_gradient_stop_t *stop;
    int i;
    svg_status_t status;

    for (i = 0; i < gradient_container->num_elements; i++) {
        stop_element = gradient_container->element[i];
        if (stop_element->type != SVG_ELEMENT_TYPE_GRADIENT_STOP)
            continue;

        stop = &stop_element->e.gradient_stop;

        status = _svg_gradient_stop_apply_style (stop, &stop_element->node->style);
        if (status)
            return status;

        status = _svg_gradient_add_stop (gradient, stop);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_parse_pattern_end (svg_parser_t *parser, svg_element_t *pattern_element)
{
    svg_container_t *pattern_container = &pattern_element->e.container;
    svg_pattern_int_t *pattern = &pattern_element->e.pattern;
    int i;

    /* remove inherited contents if this pattern element has it's own contents */
    if (pattern->num_inherited_children > 0 &&
        pattern->num_inherited_children < pattern_container->num_elements) {
        for (i = 0; i < pattern_container->num_elements; i++) {
            if (i < pattern->num_inherited_children) {
                _svg_element_destroy (pattern_container->element[i]);
            } else {
                pattern_container->element[i - pattern->num_inherited_children] =
                    pattern_container->element[i];
                pattern_container->element[i - pattern->num_inherited_children] = NULL;
            }
        }

        pattern_container->num_elements -= pattern->num_inherited_children;
        pattern->num_inherited_children = 0;
    }

    return SVG_STATUS_SUCCESS;
}


static const svg_parser_map_t SVG_PARSER_MAP[] = {
    {"a",               {_svg_parser_parse_anchor,              NULL, NULL }},
    {"svg",             {_svg_parser_parse_svg,                 NULL, NULL }},
    {"g",               {_svg_parser_parse_group,               NULL, NULL }},
    {"path",            {_svg_parser_parse_path,                NULL, NULL }},
    {"line",            {_svg_parser_parse_line,                NULL, NULL }},
    {"rect",            {_svg_parser_parse_rect,                NULL, NULL }},
    {"circle",          {_svg_parser_parse_circle,              NULL, NULL }},
    {"ellipse",         {_svg_parser_parse_ellipse,             NULL, NULL }},
    {"defs",            {_svg_parser_parse_defs,                NULL, NULL }},
    {"use",             {_svg_parser_parse_use,                 NULL, NULL }},
    {"symbol",          {_svg_parser_parse_symbol,              NULL, NULL }},
    {"polygon",         {_svg_parser_parse_polygon,             NULL, NULL }},
    {"polyline",        {_svg_parser_parse_polyline,            NULL, NULL }},
    {"text",            {_svg_parser_parse_text,
                        NULL,
                        _svg_parser_parse_text_characters }},
    {"tspan",           {_svg_parser_parse_tspan,
                        NULL,
                        _svg_parser_parse_tspan_characters }},
    {"image",           {_svg_parser_parse_image,               NULL, NULL }},
    {"linearGradient",  {_svg_parser_parse_linear_gradient,
                        _svg_parser_parse_gradient_end,
                        NULL }},
    {"radialGradient",  {_svg_parser_parse_radial_gradient,
                        _svg_parser_parse_gradient_end,
                        NULL }},
    {"stop",            {_svg_parser_parse_gradient_stop,       NULL, NULL }},
    {"pattern",         {_svg_parser_parse_pattern,
                        _svg_parser_parse_pattern_end,
                        NULL }},
    {"clipPath",        {_svg_parser_parse_clip_path,           NULL, NULL }},
    {"mask",            {_svg_parser_parse_mask,                NULL, NULL }},
    {"marker",          {_svg_parser_parse_marker,              NULL, NULL }},
};


static svg_status_t
_svg_parser_process_dom_node (svg_parser_t *parser, svg_dom_node_t *node);

static svg_status_t
_svg_parser_process_dom_element_node (svg_parser_t *parser, svg_dom_node_t *node)
{
    const char *style_str;
    const svg_parser_cb_t *cb;
    svg_element_t *element;
    int i;
    svg_dom_node_t *child, *next_sibling;
    svg_status_t status = SVG_STATUS_SUCCESS;
    int style_defer = 0;

    if (!node->is_deep_clone) {
        status = _svg_get_css_stylesheet_declarations (node, parser->stylesheet,
                                                       &node->css_properties,
                                                       &node->num_css_properties);
        if (status)
            return status;

        _svg_attribute_get_string (node->qattrs, "style", &style_str, NULL);
        if (style_str) {
            status = _svg_parse_inline_css (style_str, strlen (style_str),
                                            &node->inline_css_properties,
                                            &node->num_inline_css_properties);
            if (status)
                return _svg_parser_return_attr_error (parser, "style", status);
        }
    }

    status = _svg_style_apply_properties (node, parser->svg);
    if (status == SVG_STATUS_UNKNOWN_REF_ELEMENT)
        style_defer = 1; /* parsing will be deferred after _svg_element_apply_properties() call below */
    else if (status)
        return status;


    cb = NULL;
    for (i = 0; i < SVG_ARRAY_SIZE (SVG_PARSER_MAP); i++) {
        if (_svg_compare_qname_svg (node->qname, SVG_PARSER_MAP[i].name) == 0) {
            cb = &SVG_PARSER_MAP[i].cb;
            break;
        }
    }

    if (cb == NULL)
        return SVG_STATUS_SUCCESS;

    status = _svg_parser_push_state (parser, cb);
    if (status)
        return status;

    element = NULL;
    status = (cb->parse_element_start) (parser, node, &element);
    if (status)
        return status;

    if (element != NULL) {

        element->node = node;

        status = _svg_element_apply_properties (element);
        if (status == SVG_STATUS_UNKNOWN_REF_ELEMENT || style_defer) {
            status = _svg_parser_create_deferred_element (parser, element);
            if (status)
                return status;
            return _svg_parser_pop_state (parser);
        } else if (status) {
            return status;
        }

        if (element->id) {
            status = _svg_store_element_by_id (parser->svg, element);
            if (status)
                if (status != SVG_STATUS_DUPLICATE_ELEMENT_ID || !element->node->is_deep_clone)
                    return status;
        }
    }

    child = node->children;
    while (child != NULL) {
        next_sibling = child->next_sibling;

        status = _svg_parser_process_dom_node (parser, child);
        if (status)
            return status;

        child = next_sibling;
    }

    if (element != NULL && cb->parse_element_end != NULL) {
        status = (cb->parse_element_end) (parser, element);
        if (status)
            return status;
    }

    status = _svg_parser_pop_state (parser);

    return status;
}

static svg_status_t
_svg_parser_process_dom_character_node (svg_parser_t *parser, svg_dom_node_t *node)
{
    char *ch_data;
    size_t ch_data_offset, ch_data_len;
    svg_status_t status;

    if (!parser->state->cb->parse_characters)
        return SVG_STATUS_SUCCESS;

    if (node->ch == NULL)
        return SVG_STATUS_SUCCESS;

    status = _svg_normalize_character_data (node, &ch_data, &ch_data_offset, &ch_data_len);
    if (status)
        return status;

    if (ch_data_len == 0)
        return SVG_STATUS_SUCCESS;

    status =
        (parser->state->cb->parse_characters) (parser, node, ch_data + ch_data_offset, ch_data_len);

    free (ch_data);

    return status;
}

static svg_status_t
_svg_parser_process_dom_node (svg_parser_t *parser, svg_dom_node_t *node)
{
    svg_status_t status;

    if (_svg_dom_is_element_node (node))
        status = _svg_parser_process_dom_element_node (parser, node);
    else
        status = _svg_parser_process_dom_character_node (parser, node);

    if (status)
        return _svg_parser_return_node_error (parser, node, status);

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parser_process_deferred_element (svg_parser_t *parser,
                                      svg_parser_deferred_element_t *def_element)
{
    svg_parser_state_t *original_parser_state;
    svg_dom_node_t *child, *next_sibling;
    svg_status_t status = SVG_STATUS_SUCCESS;

    original_parser_state = parser->state;
    parser->state = &def_element->state;


    /* try apply style properties again */

    status = _svg_style_apply_properties (def_element->element->node, parser->svg);
    if (status)
        goto end;

    /* try apply properties again */

    status = _svg_element_apply_properties (def_element->element);
    if (status)
        goto end;


    /* worked this time ... finish processing element node */

    if (def_element->element->id != NULL) {
        status = _svg_store_element_by_id (parser->svg, def_element->element);
        if (status)
            if (status != SVG_STATUS_DUPLICATE_ELEMENT_ID ||
                !def_element->element->node->is_deep_clone)
            {
                goto end;
            }
    }


    child = def_element->element->node->children;
    while (child != NULL) {
        next_sibling = child->next_sibling;

        status = _svg_parser_process_dom_node (parser, child);
        if (status)
            goto end;

        child = next_sibling;
    }

    if (parser->state->cb->parse_element_end != NULL) {
        status = (parser->state->cb->parse_element_end) (parser, def_element->element);
        if (status)
            goto end;
    }

    status = SVG_STATUS_SUCCESS;

  end:
    parser->state = original_parser_state;
    return status;
}




void
_svg_parser_sax_start_element (void *closure, const xmlChar *xml_name, const xmlChar **xml_atts)
{
    svg_parser_t *parser = closure;
    svg_dom_node_t *node;
    svg_status_t status;
    svg_qname_t *qname = NULL;
    svg_qattrs_t *qattrs = NULL;
    long line_number;

    line_number = _svg_parser_get_line_number (parser);

    status = _svg_parser_copy_xml_name (&parser->dom, xml_name, &qname);
    if (status)
        goto error;

    status = _svg_parser_copy_xml_attributes (&parser->dom, qname->ns_index, xml_atts, &qattrs);
    if (status)
        goto error;

    status = _svg_dom_start_element (&parser->dom, qname, qattrs, &node);
    if (status)
        goto error;
    /* node now has ownership */
    qname = NULL;
    qattrs = NULL;

    if (parser->dom.root_node == node) {
        _svg_dom_node_set_document_uri (node, parser->svg->document_uri);
        if (node->base_uri == NULL)
            _svg_dom_node_set_base_uri (node, parser->svg->base_uri, 0);
    }

    _svg_dom_set_line_number (node, line_number);

    return;

  error:
    _svg_parser_set_node_error (parser, line_number, qname, status);

    if (qname != NULL)
        _svg_dom_destroy_qname (qname);
    if (qattrs != NULL)
        _svg_dom_destroy_qattrs (qattrs);

    parser->status = status;
    _svg_parser_stop (parser);
}

void
_svg_parser_sax_end_element (void *closure, const xmlChar *xml_name)
{
    svg_parser_t *parser = closure;
    svg_qname_t *qname = NULL;
    svg_dom_node_t *ch_node;
    svg_status_t status;
    long line_number;

    line_number = _svg_parser_get_line_number (parser);

    status = _svg_parser_copy_xml_name (&parser->dom, xml_name, &qname);
    if (status)
        goto error;

    ch_node = _svg_dom_get_current_character_node (&parser->dom);
    if (ch_node != NULL && ch_node->ch != NULL && _svg_compare_qname_svg (qname, "style") == 0) {
        status = _svg_parse_css_buffer (parser->stylesheet, parser->svg->base_uri,
                                        ch_node->ch, ch_node->len);
        if (status)
            goto error;
    }

    status = _svg_dom_end_element (&parser->dom, qname);
    if (status)
        goto error;
    qname = NULL;

    return;

  error:
    _svg_parser_set_node_error (parser, line_number, qname, status);

    if (qname != NULL)
        _svg_dom_destroy_qname (qname);

    parser->status = status;
    _svg_parser_stop (parser);
}

void
_svg_parser_sax_characters (void *closure, const xmlChar *ch_unsigned, int len)
{
    svg_parser_t *parser = closure;
    svg_dom_node_t *element_node;
    svg_dom_node_t *node;
    const svg_parser_cb_t *cb;
    int i;
    svg_status_t status;
    long line_number;

    line_number = _svg_parser_get_line_number (parser);

    element_node = _svg_dom_get_current_element_node (&parser->dom);
    if (element_node == NULL) {
        status = SVG_STATUS_PARSE_ERROR;
        goto error;
    }

    /* skip character data if not required */
    if (_svg_compare_qname_svg (element_node->qname, "style") != 0) {
        cb = NULL;
        for (i = 0; i < SVG_ARRAY_SIZE (SVG_PARSER_MAP); i++) {
            if (_svg_compare_qname_svg (element_node->qname, SVG_PARSER_MAP[i].name) == 0) {
                cb = &SVG_PARSER_MAP[i].cb;
                break;
            }
        }
        if (cb == NULL || cb->parse_characters == NULL)
            return;
    }


    status = _svg_dom_characters (&parser->dom, (const char *) ch_unsigned, len, &node);
    if (status)
        goto error;

    return;

  error:
    _svg_parser_set_node_error (parser, line_number, element_node->qname, status);

    parser->status = status;
    _svg_parser_stop (parser);
}

void
_svg_parser_sax_proc_instr (void *closure, const xmlChar *target_unsigned,
                            const xmlChar *data_unsigned)
{
    svg_parser_t *parser = closure;
    static const char *xml_prefix = "<?xml version=\"1.0\"?>\n<tmp ";
    static const char *xml_suffix = " />";
    const char *target = (const char *) target_unsigned;
    const char *data = (const char *) data_unsigned;
    char *xml_buffer;
    svg_dom_t dom;
    size_t len;
    const char *type;
    const char *href;
    svg_status_t status;
    svg_uri_t *uri = NULL;
    svg_uri_t *abs_uri = NULL;

    if (strcmp (target, "xml-stylesheet") != 0)
        return;

    /* use the xml parser to parse the processing instruction using a temp xml document */

    status = _svg_dom_init (&dom);
    if (status)
        return;

    len = strlen (xml_prefix) + strlen (data) + strlen (xml_suffix);
    xml_buffer = (char *) malloc (len + 1);
    if (xml_buffer == NULL)
        goto end;
    strcpy (xml_buffer, xml_prefix);
    strcat (xml_buffer, data);
    strcat (xml_buffer, xml_suffix);

    status = _svg_parser_parse_pseudo_proc_instr (&dom, xml_buffer, len);

    free (xml_buffer);

    if (status || dom.root_node == NULL)
        goto end;


    type = _svg_dom_get_local_attr_value (dom.root_node, "type");
    if (type == NULL || strcmp (type, "text/css") != 0)
        goto end;

    href = _svg_dom_get_local_attr_value (dom.root_node, "href");
    if (href == NULL || href[0] == '\0')
        goto end;


    status = _svg_create_uri (href, &uri);
    if (status)
        goto end;

    if (_svg_uri_is_relative (uri)) {
        status = _svg_uri_create_absolute (parser->svg->base_uri, uri, &abs_uri);
        if (status)
            goto end;
    } else {
        abs_uri = uri;
        uri = NULL;
    }

    status = _svg_parse_external_css_stylesheet (parser->stylesheet, abs_uri);
    if (status)
        goto end;
    /* XXX: set parser error if fails, or just warn? E.g. SVG_STATUS_FILE_NOT_FOUND */


  end:
    if (uri != NULL)
        _svg_destroy_uri (uri);
    if (abs_uri != NULL)
        _svg_destroy_uri (abs_uri);
    _svg_dom_deinit (&dom);
}

void
_svg_parser_pseudo_proc_instr_start_element (void *closure, const xmlChar *xml_name,
                                             const xmlChar **xml_atts)
{
    svg_dom_t *dom = closure;
    svg_dom_node_t *node;
    svg_status_t status;
    svg_qname_t *qname = NULL;
    svg_qattrs_t *qattrs = NULL;

    status = _svg_parser_copy_xml_name (dom, xml_name, &qname);
    if (status)
        goto error;

    status = _svg_parser_copy_xml_attributes (dom, qname->ns_index, xml_atts, &qattrs);
    if (status)
        goto error;

    status = _svg_dom_start_element (dom, qname, qattrs, &node);
    if (status)
        goto error;
    /* node now has ownership */
    qname = NULL;
    qattrs = NULL;

    return;

  error:
    if (qname != NULL)
        _svg_dom_destroy_qname (qname);
    if (qattrs != NULL)
        _svg_dom_destroy_qattrs (qattrs);
}

void
_svg_parser_set_error (svg_parser_t *parser, long line_number, svg_status_t error_status)
{
    _svg_set_error_line_number (parser->svg, line_number);
    _svg_set_error_status (parser->svg, error_status);
}

void
_svg_parser_set_node_error (svg_parser_t *parser,
                            long line_number, svg_qname_t *node_qname, svg_status_t error_status)
{
    if (node_qname != NULL)
        _svg_set_error_element_name (parser->svg, node_qname);
    else
        _svg_set_error_element_name (parser->svg, NULL);
    _svg_parser_set_error (parser, line_number, error_status);
}

void
_svg_parser_set_attr_error (svg_parser_t *parser,
                            const char *attr_local_name, svg_status_t error_status)
{
    _svg_set_error_attribute_name (parser->svg, attr_local_name);
    _svg_set_error_status (parser->svg, error_status);
}

svg_status_t
_svg_parser_return_error (svg_parser_t *parser, svg_status_t error_status)
{
    _svg_parser_set_error (parser, 0, error_status);
    return error_status;
}

svg_status_t
_svg_parser_return_node_error (svg_parser_t *parser,
                               svg_dom_node_t *node, svg_status_t error_status)
{
    svg_dom_node_t *parent_node;

    if (_svg_dom_is_element_node (node)) {
        _svg_parser_set_node_error (parser, node->line_number, node->qname, error_status);
    } else {
        parent_node = _svg_dom_get_parent (node);
        if (parent_node != NULL)
            _svg_parser_set_node_error (parser, node->line_number, parent_node->qname,
                                        error_status);
        else
            _svg_parser_set_node_error (parser, node->line_number, NULL, error_status);
    }

    return error_status;
}

svg_status_t
_svg_parser_return_attr_error (svg_parser_t *parser, const char *attr_local_name,
                               svg_status_t error_status)
{
    _svg_parser_set_attr_error (parser, attr_local_name, error_status);
    return error_status;
}

svg_status_t
_svg_parser_process_dom (svg_parser_t *parser)
{
    svg_status_t status;
    svg_parser_deferred_element_t *prev_def_element, *def_element, *next_def_element;
    int have_processed_one;

    if (parser->dom.current_node != NULL)
        return _svg_parser_return_error (parser, SVG_STATUS_PARSE_ERROR);

    if (parser->dom.root_node == NULL)
        return _svg_parser_return_error (parser, SVG_STATUS_EMPTY_DOCUMENT);

    if (_svg_compare_qname_svg (parser->dom.root_node->qname, "svg") != 0) {
        if (parser->dom.root_node->qname->ns_index == NO_NAMESPACE_INDEX &&
            strcmp (parser->dom.root_node->qname->local_name, "svg") == 0)
        {
            return _svg_parser_return_error (parser, SVG_STATUS_MISSING_SVG_NAMESPACE_DECL);
        }
        return _svg_parser_return_error (parser, SVG_STATUS_ROOT_ELEMENT_NOT_SVG);
    }


    status = _svg_parser_process_dom_node (parser, parser->dom.root_node);
    if (status)
        return status;

    do {
        have_processed_one = 0;
        prev_def_element = NULL;
        def_element = parser->deferred_elements;
        while (def_element != NULL) {

            status = _svg_parser_process_deferred_element (parser, def_element);
            if (status && status != SVG_STATUS_UNKNOWN_REF_ELEMENT)
                return _svg_parser_return_node_error (parser, def_element->element->node, status);

            next_def_element = def_element->next;

            if (status != SVG_STATUS_UNKNOWN_REF_ELEMENT) {
                if (prev_def_element == NULL)
                    parser->deferred_elements = next_def_element;
                else
                    prev_def_element->next = next_def_element;

                free (def_element);

                have_processed_one = 1;
            } else {
                prev_def_element = def_element;
            }

            def_element = next_def_element;
        }
    } while (have_processed_one);

    if (parser->deferred_elements != NULL) {
        def_element = parser->deferred_elements;
        while (def_element->next != NULL)
            def_element = def_element->next;

        return _svg_parser_return_node_error (parser, def_element->element->node,
                                              SVG_STATUS_UNKNOWN_REF_ELEMENT);
    }


    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_parser_create_deferred_element (svg_parser_t *parser, svg_element_t *element)
{
    svg_parser_deferred_element_t *def_element;

    def_element = (svg_parser_deferred_element_t *) malloc (sizeof (svg_parser_deferred_element_t));
    if (def_element == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (def_element, 0, sizeof (svg_parser_deferred_element_t));

    def_element->element = element;
    def_element->state = *parser->state;
    def_element->state.next = NULL;

    def_element->next = parser->deferred_elements;
    parser->deferred_elements = def_element;

    return SVG_STATUS_SUCCESS;
}

