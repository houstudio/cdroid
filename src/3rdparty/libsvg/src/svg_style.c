/* svg_style.c: Data structure for holding SVG style properties

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


typedef struct svg_style_parse_map {
    const char *name;
    svg_status_t (*parse) (svg_dom_node_t *node, svg_t *svg, const char *value);
    const char *default_value;
} svg_style_parse_map_t;




static svg_status_t
_svg_style_return_property_error (svg_t *svg, const char *property_name, svg_status_t error_status)
{
    _svg_set_error_property_name (svg, property_name);
    _svg_set_error_status (svg, error_status);

    return error_status;
}

static svg_status_t
_svg_style_str_to_opacity (const char *str, double *ret)
{
    const char *end_ptr;
    double opacity;

    opacity = _svg_ascii_strtod (str, &end_ptr);

    if (end_ptr == str)
        return SVG_STATUS_PARSE_ERROR;

    if (end_ptr && end_ptr[0] == '%')
        opacity *= 0.01;

    if (opacity < 0)
        opacity = 0;
    if (opacity > 1)
        opacity = 1;

    *ret = opacity;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_color (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.color = node->parent->style.color;
    } else {
        status = _svg_color_init_from_str (&node->style.color, str);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_COLOR;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_fill_opacity (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.fill_opacity = node->parent->style.fill_opacity;
    } else {
        status = _svg_style_str_to_opacity (str, &node->style.fill_opacity);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_FILL_OPACITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_fill_paint (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.fill_paint = node->parent->style.fill_paint;
    } else {
        status =
            _svg_paint_init (&node->style.fill_paint, svg, node->document_uri, node->base_uri, str);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_FILL_PAINT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_fill_rule (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.fill_rule = node->parent->style.fill_rule;
    } else if (strcmp (str, "evenodd") == 0) {
        node->style.fill_rule = SVG_FILL_RULE_EVEN_ODD;
    } else if (strcmp (str, "nonzero") == 0) {
        node->style.fill_rule = SVG_FILL_RULE_NONZERO;
    } else {
        return SVG_STATUS_PARSE_ERROR;
    }

    node->style.flags |= SVG_STYLE_FLAG_FILL_RULE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_font_family (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    char *new_font_family;


    if (strcmp (str, "inherit") == 0) {
        if (node->parent == NULL) {
            node->style.flags |= SVG_STYLE_FLAG_FONT_FAMILY;
            return SVG_STATUS_SUCCESS;
        }

        new_font_family = strdup (node->parent->style.font_family);
    } else {
        new_font_family = strdup (str);
    }
    if (new_font_family == NULL)
        return SVG_STATUS_NO_MEMORY;

    /* XXX: check format */

    if (node->style.font_family != NULL)
        free (node->style.font_family);
    node->style.font_family = new_font_family;

    node->style.flags |= SVG_STYLE_FLAG_FONT_FAMILY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_font_size (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL) {
            node->style.font_size = node->parent->style.font_size;
            node->style.computed_font_size = node->parent->style.computed_font_size;
        }
    } else {
        status = _svg_length_init_from_str (&node->style.font_size, str);
        if (status)
            return status;

        node->style.computed_font_size = -1;
    }

    /* XXX: absolute and relative font sizes */

    node->style.flags |= SVG_STYLE_FLAG_FONT_SIZE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_font_style (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.font_style = node->parent->style.font_style;
    } else if (strcmp (str, "normal") == 0) {
        node->style.font_style = SVG_FONT_STYLE_NORMAL;
    } else if (strcmp (str, "italic") == 0) {
        node->style.font_style = SVG_FONT_STYLE_ITALIC;
    } else if (strcmp (str, "oblique") == 0) {
        node->style.font_style = SVG_FONT_STYLE_OBLIQUE;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_FONT_STYLE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_font_weight (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    const char *end;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.font_weight = node->parent->style.font_weight;
    } else if (strcmp (str, "normal") == 0) {
        node->style.font_weight = 400;
    } else if (strcmp (str, "bold") == 0) {
        node->style.font_weight = 700;
    } else if (strcmp (str, "lighter") == 0) {
        node->style.font_weight -= 100;
    } else if (strcmp (str, "bolder") == 0) {
        node->style.font_weight += 100;
    } else {
        node->style.font_weight = (unsigned int) _svg_ascii_strtod (str, &end);
        if (end == str)
            return SVG_STATUS_PARSE_ERROR;
    }

    if (node->style.font_weight < 100)
        node->style.font_weight = 100;
    if (node->style.font_weight > 900)
        node->style.font_weight = 900;

    node->style.flags |= SVG_STYLE_FLAG_FONT_WEIGHT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_opacity (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.opacity = node->parent->style.opacity;
    } else {
        status = _svg_style_str_to_opacity (str, &node->style.opacity);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_OPACITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_dash_array (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;
    double *new_dash_array;
    const char *end;
    int i, j;

    if (strcmp (str, "inherit") == 0 && node->parent == NULL) {
        node->style.flags |= SVG_STYLE_FLAG_STROKE_DASH_ARRAY;
        return SVG_STATUS_SUCCESS;
    }

    if (node->style.stroke_dash_array != NULL) {
        free (node->style.stroke_dash_array);
        node->style.stroke_dash_array = NULL;
    }
    node->style.num_dashes = 0;
    /* XXX: should only free if the new array was created successfully */

    if (strcmp (str, "inherit") == 0) {
        if (node->parent->style.num_dashes > 0) {
            new_dash_array = (double *) malloc (node->parent->style.num_dashes * sizeof (double));
            if (new_dash_array == NULL)
                return SVG_STATUS_NO_MEMORY;
            memcpy (new_dash_array, node->parent->style.stroke_dash_array,
                    node->parent->style.num_dashes * sizeof (double));
        } else {
            new_dash_array = NULL;
        }

        node->style.stroke_dash_array = new_dash_array;
        node->style.num_dashes = node->parent->style.num_dashes;
    } else if (strcmp (str, "none") != 0) {
        status =
            _svg_str_parse_all_csv_doubles (str, &node->style.stroke_dash_array,
                                            &node->style.num_dashes, &end);
        if (status)
            return status;

        if (node->style.num_dashes % 2) {
            node->style.num_dashes *= 2;

            new_dash_array =
                realloc (node->style.stroke_dash_array, node->style.num_dashes * sizeof (double));
            if (new_dash_array == NULL)
                return SVG_STATUS_NO_MEMORY;
            node->style.stroke_dash_array = new_dash_array;

            for (i = 0, j = node->style.num_dashes / 2; j < node->style.num_dashes; i++, j++)
                node->style.stroke_dash_array[j] = node->style.stroke_dash_array[i];
        }
    }

    /* XXX: dash array should <length>s and not doubles */

    node->style.flags |= SVG_STYLE_FLAG_STROKE_DASH_ARRAY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_dash_offset (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stroke_dash_offset = node->parent->style.stroke_dash_offset;
    } else {
        status = _svg_length_init_from_str (&node->style.stroke_dash_offset, str);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_STROKE_DASH_OFFSET;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_line_cap (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stroke_line_cap = node->parent->style.stroke_line_cap;
    } else if (strcmp (str, "butt") == 0) {
        node->style.stroke_line_cap = SVG_STROKE_LINE_CAP_BUTT;
    } else if (strcmp (str, "round") == 0) {
        node->style.stroke_line_cap = SVG_STROKE_LINE_CAP_ROUND;
    } else if (strcmp (str, "square") == 0) {
        node->style.stroke_line_cap = SVG_STROKE_LINE_CAP_SQUARE;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_STROKE_LINE_CAP;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_line_join (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stroke_line_join = node->parent->style.stroke_line_join;
    } else if (strcmp (str, "miter") == 0) {
        node->style.stroke_line_join = SVG_STROKE_LINE_JOIN_MITER;
    } else if (strcmp (str, "round") == 0) {
        node->style.stroke_line_join = SVG_STROKE_LINE_JOIN_ROUND;
    } else if (strcmp (str, "bevel") == 0) {
        node->style.stroke_line_join = SVG_STROKE_LINE_JOIN_BEVEL;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_STROKE_LINE_JOIN;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_miter_limit (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    const char *end;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stroke_miter_limit = node->parent->style.stroke_miter_limit;
    } else {
        node->style.stroke_miter_limit = _svg_ascii_strtod (str, &end);
        if (end == str)
            return SVG_STATUS_PARSE_ERROR;
    }

    node->style.flags |= SVG_STYLE_FLAG_STROKE_MITER_LIMIT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_opacity (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stroke_opacity = node->parent->style.stroke_opacity;
    } else {
        status = _svg_style_str_to_opacity (str, &node->style.stroke_opacity);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_STROKE_OPACITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_paint (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stroke_paint = node->parent->style.stroke_paint;
    } else {
        status = _svg_paint_init (&node->style.stroke_paint, svg, node->document_uri,
                                  node->base_uri, str);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_STROKE_PAINT;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stroke_width (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stroke_width = node->parent->style.stroke_width;
    } else {
        status = _svg_length_init_from_str (&node->style.stroke_width, str);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_STROKE_WIDTH;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_text_anchor (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.text_anchor = node->parent->style.text_anchor;
    } else if (strcmp (str, "start") == 0) {
        node->style.text_anchor = SVG_TEXT_ANCHOR_START;
    } else if (strcmp (str, "middle") == 0) {
        node->style.text_anchor = SVG_TEXT_ANCHOR_MIDDLE;
    } else if (strcmp (str, "end") == 0) {
        node->style.text_anchor = SVG_TEXT_ANCHOR_END;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_TEXT_ANCHOR;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_visibility (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.visibility = node->parent->style.visibility;
    } else if (strcmp (str, "hidden") == 0 || strcmp (str, "collapse") == 0) {
        node->style.visibility = 0;
    } else if (strcmp (str, "visible") == 0) {
        node->style.visibility = 1;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_VISIBILITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_display (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.display = node->parent->style.display;
    } else if (strcmp (str, "none") == 0) {
        node->style.display = 0;
    } else if (strcmp (str, "inline") == 0 || strcmp (str, "block") == 0 ||
                strcmp (str, "list-item") == 0 || strcmp (str, "run-in") == 0 ||
                strcmp (str, "compact") == 0 || strcmp (str, "marker") == 0 ||
                strcmp (str, "table") == 0 || strcmp (str, "inline-table") == 0 ||
                strcmp (str, "table-row-group") == 0 || strcmp (str, "table-header-group") == 0 ||
                strcmp (str, "table-footer-group") == 0 || strcmp (str, "table-row") == 0 ||
                strcmp (str, "table-column-group") == 0 || strcmp (str, "table-column") == 0 ||
                strcmp (str, "table-cell") == 0 || strcmp (str, "table-caption") == 0) {
        node->style.display = 1;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_DISPLAY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stop_color (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stop_color = node->parent->style.stop_color;
    } else {
        status = _svg_color_init_from_str (&node->style.stop_color, str);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_STOP_COLOR;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_stop_opacity (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.stop_opacity = node->parent->style.stop_opacity;
    } else {
        status = _svg_style_str_to_opacity (str, &node->style.stop_opacity);
        if (status)
            return status;
    }

    node->style.flags |= SVG_STYLE_FLAG_STOP_OPACITY;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_overflow (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.overflow_hidden = node->parent->style.overflow_hidden;
    } else if (strcmp (str, "hidden") == 0 || strcmp (str, "scroll") == 0) {
        node->style.overflow_hidden = 1;
    } else if (strcmp (str, "visible") == 0 || strcmp (str, "auto") == 0) {
        node->style.overflow_hidden = 0;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_OVERFLOW;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_clip_rect_length (const char *start, svg_length_t *length, const char **end)
{
    const char *s = start;
    svg_status_t status;

    _svg_str_skip_space (&s);

    if (strncmp (s, "auto", 4) == 0) {
        _svg_length_init_from_str (length, "0px");
        *end = s + 4;
    } else {
        status = _svg_length_init_from_array_str (length, s, end);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_clip (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    const char *s;
    const char *end;
    int i;
    svg_status_t status;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL) {
            for (i = 0; i < 4; i++)
                node->style.clip_rect[i] = node->parent->style.clip_rect[i];
        }
    } else if (strcmp (str, "auto") == 0) {
        for (i = 0; i < 4; i++)
            _svg_length_init_from_str (&node->style.clip_rect[i], "0px");
    } else {
        /* XXX: move to svg_str? */

        s = str;
        if (strncmp (s, "rect(", 5) != 0)
            return SVG_STATUS_PARSE_ERROR;
        end = s + 5;

        s = end;
        for (i = 0; i < 4; i++) {
            status = _svg_style_parse_clip_rect_length (s, &node->style.clip_rect[i], &end);
            if (status)
                return status;

            s = end;
            _svg_str_skip_space_or_char (&s, ',');
        }
    }

    node->style.flags |= SVG_STYLE_FLAG_CLIP;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_clip_path (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;
    svg_element_t *element;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.clip_path = node->parent->style.clip_path;
    } else if (strcmp (str, "none") == 0) {
        node->style.clip_path = NULL;
    } else {
        if (!_svg_is_uri_ref_url (str))
            return SVG_STATUS_INVALID_VALUE;

        status = _svg_resolve_element_url (svg, node->document_uri, node->base_uri, str, &element);
        if (status)
            return status;

        if (element->type != SVG_ELEMENT_TYPE_CLIP_PATH)
            return SVG_STATUS_WRONG_REF_ELEMENT_TYPE;

        node->style.clip_path = &element->e.clip_path.ext_clip_path;
    }

    node->style.flags |= SVG_STYLE_FLAG_CLIP_PATH;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_clip_rule (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.clip_rule = node->parent->style.clip_rule;
    } else if (strcmp (str, "evenodd") == 0) {
        node->style.clip_rule = SVG_CLIP_RULE_EVEN_ODD;
    } else if (strcmp (str, "nonzero") == 0) {
        node->style.clip_rule = SVG_CLIP_RULE_NONZERO;
    } else {
        return SVG_STATUS_INVALID_VALUE;
    }

    node->style.flags |= SVG_STYLE_FLAG_CLIP_RULE;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_mask (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;
    svg_element_t *element;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            node->style.mask = node->parent->style.mask;
    } else if (strcmp (str, "none") == 0) {
        node->style.mask = NULL;
    } else {
        if (!_svg_is_uri_ref_url (str))
            return SVG_STATUS_INVALID_VALUE;

        status = _svg_resolve_element_url (svg, node->document_uri, node->base_uri, str, &element);
        if (status)
            return status;

        if (element->type != SVG_ELEMENT_TYPE_MASK)
            return SVG_STATUS_WRONG_REF_ELEMENT_TYPE;

        node->style.mask = &element->e.mask.ext_mask;
    }

    node->style.flags |= SVG_STYLE_FLAG_MASK;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_marker_common (const svg_marker_t **marker, svg_dom_node_t *node, svg_t *svg,
                                const char *str)
{
    svg_status_t status;
    svg_element_t *element;

    if (strcmp (str, "none") == 0) {
        *marker = NULL;
    } else {
        if (!_svg_is_uri_ref_url (str))
            return SVG_STATUS_INVALID_VALUE;

        status = _svg_resolve_element_url (svg, node->document_uri, node->base_uri, str, &element);
        if (status)
            return status;

        if (element->type != SVG_ELEMENT_TYPE_MARKER)
            return SVG_STATUS_WRONG_REF_ELEMENT_TYPE;

        *marker = &element->e.marker.ext_marker;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_marker_start (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;
    const svg_marker_t *ext_marker = NULL;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            ext_marker = node->parent->style.marker_start;
    } else {
        status = _svg_style_parse_marker_common (&ext_marker, node, svg, str);
        if (status)
            return status;
    }

    node->style.marker_start = ext_marker;
    node->style.flags |= SVG_STYLE_FLAG_MARKER_START;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_marker_mid (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;
    const svg_marker_t *ext_marker = NULL;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            ext_marker = node->parent->style.marker_mid;
    } else {
        status = _svg_style_parse_marker_common (&ext_marker, node, svg, str);
        if (status)
            return status;
    }

    node->style.marker_mid = ext_marker;
    node->style.flags |= SVG_STYLE_FLAG_MARKER_MID;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_marker_end (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;
    const svg_marker_t *ext_marker = NULL;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL)
            ext_marker = node->parent->style.marker_end;
    } else {
        status = _svg_style_parse_marker_common (&ext_marker, node, svg, str);
        if (status)
            return status;
    }

    node->style.marker_end = ext_marker;
    node->style.flags |= SVG_STYLE_FLAG_MARKER_END;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_parse_marker (svg_dom_node_t *node, svg_t *svg, const char *str)
{
    svg_status_t status;
    const svg_marker_t *ext_marker_start = NULL;
    const svg_marker_t *ext_marker_mid = NULL;
    const svg_marker_t *ext_marker_end = NULL;

    if (strcmp (str, "inherit") == 0) {
        if (node->parent != NULL) {
            ext_marker_start = node->parent->style.marker_start;
            ext_marker_mid = node->parent->style.marker_mid;
            ext_marker_end = node->parent->style.marker_end;
        }
    } else {
        status = _svg_style_parse_marker_common (&ext_marker_start, node, svg, str);
        if (status)
            return status;
        ext_marker_mid = ext_marker_start;
        ext_marker_end = ext_marker_start;
    }

    node->style.marker_start = ext_marker_start;
    node->style.flags |= SVG_STYLE_FLAG_MARKER_START;
    node->style.marker_mid = ext_marker_mid;
    node->style.flags |= SVG_STYLE_FLAG_MARKER_MID;
    node->style.marker_end = ext_marker_end;
    node->style.flags |= SVG_STYLE_FLAG_MARKER_END;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_compute_colors (svg_dom_node_t *node)
{
    svg_style_t *style = &node->style;

    if (style->color.is_current_color) {
        if (node->parent == NULL || node->parent->style.color.is_current_color)
            return SVG_STATUS_INTERNAL_ERROR;

        style->color = node->parent->style.color;
    }

    if (style->stop_color.is_current_color)
        style->stop_color = style->color;

    if (style->fill_paint.type == SVG_PAINT_TYPE_COLOR &&
                style->fill_paint.p.color.is_current_color)
        style->fill_paint.p.color = style->color;

    if (style->stroke_paint.type == SVG_PAINT_TYPE_COLOR &&
                style->stroke_paint.p.color.is_current_color)
        style->stroke_paint.p.color = style->color;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_compute_font_size (svg_dom_node_t *node, svg_render_engine_t *engine, void *closure)
{
    svg_style_t *style = &node->style;
    svg_status_t status;
    double parent_font_size;

    if (style->computed_font_size >= 0)
        return SVG_STATUS_SUCCESS;

    if (!(style->flags & SVG_STYLE_FLAG_FONT_SIZE) &&
        node->parent != NULL && node->parent->style.computed_font_size >= 0)
    {
        style->computed_font_size = node->parent->style.computed_font_size;
        return SVG_STATUS_SUCCESS;
    }

    if (node->parent != NULL) {
        if (node->parent->style.computed_font_size < 0) {
            status = _svg_style_compute_font_size (node->parent, engine, closure);
            if (status)
                return status;
        }

        parent_font_size = node->parent->style.computed_font_size;
    } else {
        parent_font_size = 0;
    }

    status = _svg_engine_measure_font_size (engine, closure, style->font_family, parent_font_size,
                                            &style->font_size, &style->computed_font_size);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}


static const svg_style_parse_map_t SVG_STYLE_PARSE_MAP[] = {
    { "clip",                   _svg_style_parse_clip,                  "auto" },
    { "clip-path",              _svg_style_parse_clip_path,             "none" },
    { "clip-rule",              _svg_style_parse_clip_rule,             "nonzero" },
    { "color",                  _svg_style_parse_color,                 "black" },
/* XXX: { "color-interpolation",_svg_style_parse_color_interpolation,   "sRGB" }, */
/* XXX: { "color-interpolation-filters",_svg_style_parse_color_interpolation_filters,   "linearRGB" }, */
/* XXX: { "color-profile",      _svg_style_parse_color_profile,         "auto" }, */
/* XXX: { "color-rendering",    _svg_style_parse_color_rendering,       "auto" }, */
/* XXX: { "cursor",             _svg_style_parse_cursor,                "auto" }, */
/* XXX: { "direction",          _svg_style_parse_direction,             "ltr" }, */
    { "display",                _svg_style_parse_display,               "inline" },
    { "fill-opacity",           _svg_style_parse_fill_opacity,          "1.0" },
    { "fill",                   _svg_style_parse_fill_paint,            "black" },
    { "fill-rule",              _svg_style_parse_fill_rule,             "nonzero" },
/* XXX: { "font",               _svg_style_parse_font,                  NULL }, */
    { "font-family",            _svg_style_parse_font_family,           "sans-serif" },
    /* XXX: The default is supposed to be "medium" but I'm not parsing that yet */
    { "font-size",              _svg_style_parse_font_size,             "10.0" },
/* XXX: { "font-size-adjust",   _svg_style_parse_font_size_adjust,      "none" }, */
/* XXX: { "font-stretch",       _svg_style_parse_font_stretch,          "normal" }, */
    { "font-style",             _svg_style_parse_font_style,            "normal" },
/* XXX: { "font-variant",       _svg_style_parse_font_variant,          "normal" }, */
    { "font-weight",            _svg_style_parse_font_weight,           "normal" },
/* XXX: { "glyph-orientation-horizontal",       _svg_style_parse_glyph_orientation_horizontal,  "0deg" }, */
/* XXX: { "glyph-orientation-vertical",         _svg_style_parse_glyph_orientation_vertical,    "auto" }, */
/* XXX: { "image-rendering",    _svg_style_parse_image_rendering,       "auto" }, */
/* XXX: { "kerning",            _svg_style_parse_kerning,               "auto" }, */
/* XXX: { "letter-spacing",     _svg_style_parse_letter_spacing,        "normal" }, */
    { "marker",                 _svg_style_parse_marker,                "none" },
    { "marker-end",             _svg_style_parse_marker_end,            "none" },
    { "marker-mid",             _svg_style_parse_marker_mid,            "none" },
    { "marker-start",           _svg_style_parse_marker_start,          "none" },
    { "mask",                   _svg_style_parse_mask,                  "none" },
    { "opacity",                _svg_style_parse_opacity,               "1.0" },
    { "overflow",               _svg_style_parse_overflow,              "hidden" },
/* XXX: { "pointer-events",     _svg_style_parse_pointer_events,        "visiblePainted" }, */
/* XXX: { "shape-rendering",    _svg_style_parse_shape_rendering,       "auto" }, */
    { "stroke-dasharray",       _svg_style_parse_stroke_dash_array,     "none" },
    { "stroke-dashoffset",      _svg_style_parse_stroke_dash_offset,    "0.0" },
    { "stroke-linecap",         _svg_style_parse_stroke_line_cap,       "butt" },
    { "stroke-linejoin",        _svg_style_parse_stroke_line_join,      "miter" },
    { "stroke-miterlimit",      _svg_style_parse_stroke_miter_limit,    "4.0" },
    { "stroke-opacity",         _svg_style_parse_stroke_opacity,        "1.0" },
    { "stroke",                 _svg_style_parse_stroke_paint,          "none" },
    { "stroke-width",           _svg_style_parse_stroke_width,          "1.0" },
    { "text-anchor",            _svg_style_parse_text_anchor,           "start" },
/* XXX: { "text-rendering",     _svg_style_parse_text_rendering,        "auto" }, */
    { "visibility",             _svg_style_parse_visibility,            "visible" },
/* XXX: { "word-spacing",       _svg_style_parse_word_spacing,          "normal" }, */
/* XXX: { "writing-mode",       _svg_style_parse_writing_mode,          "lr-tb" }, */
    { "stop-opacity",           _svg_style_parse_stop_opacity,          "1.0" },
    { "stop-color",             _svg_style_parse_stop_color,            "black" },
};


static svg_status_t
_svg_style_init_defaults (svg_dom_node_t *node, svg_t *svg)
{
    int i;
    svg_status_t status;

    status = _svg_style_init_empty (&node->style);
    if (status)
        return status;

    for (i = 0; i < SVG_ARRAY_SIZE (SVG_STYLE_PARSE_MAP); i++) {
        const svg_style_parse_map_t *map;
        map = &SVG_STYLE_PARSE_MAP[i];

        if (map->default_value) {
            status = (map->parse) (node, svg, map->default_value);
            if (status)
                return status;
        }
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_style_apply_css_properties (svg_dom_node_t *node, svg_t *svg, int do_important)
{
    unsigned int i;
    int j;
    svg_status_t status = SVG_STATUS_SUCCESS;

    for (j = 0; j < node->num_css_properties; j++) {

        if (do_important != node->css_properties[j].important)
            continue;

        for (i = 0; i < SVG_ARRAY_SIZE (SVG_STYLE_PARSE_MAP); i++) {
            if (strcmp (SVG_STYLE_PARSE_MAP[i].name, node->css_properties[j].name) == 0) {
                status = (SVG_STYLE_PARSE_MAP[i].parse) (node, svg, node->css_properties[j].value);
                break;
            }
        }
        if (status == SVG_STATUS_NO_MEMORY || status == SVG_STATUS_UNKNOWN_REF_ELEMENT)
            return status;
    }

    for (j = 0; j < node->num_inline_css_properties; j++) {

        if (do_important != node->inline_css_properties[j].important)
            continue;

        for (i = 0; i < SVG_ARRAY_SIZE (SVG_STYLE_PARSE_MAP); i++) {
            if (strcmp (SVG_STYLE_PARSE_MAP[i].name, node->inline_css_properties[j].name) == 0) {
                status = (SVG_STYLE_PARSE_MAP[i].parse) (node, svg,
                                                         node->inline_css_properties[j].value);
                break;
            }
        }
        if (status == SVG_STATUS_NO_MEMORY || status == SVG_STATUS_UNKNOWN_REF_ELEMENT)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_style_init_empty (svg_style_t *style)
{
    memset (style, 0, sizeof (svg_style_t));

    style->flags = SVG_STYLE_FLAG_NONE;

    style->computed_font_size = -1;

    _svg_length_init_unit (&style->font_size, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&style->stroke_dash_offset, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_OTHER);
    _svg_length_init_unit (&style->stroke_width, 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_OTHER);
    _svg_length_init_unit (&style->clip_rect[0], 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&style->clip_rect[1], 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&style->clip_rect[2], 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&style->clip_rect[3], 0, SVG_LENGTH_UNIT_PX,
                           SVG_LENGTH_ORIENTATION_HORIZONTAL);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_init_copy (svg_style_t *style, svg_style_t *other)
{
    int i;

    style->flags = other->flags;

    style->display = other->display;
    style->visibility = other->visibility;

    style->fill_opacity = other->fill_opacity;
    style->fill_paint = other->fill_paint;
    style->fill_rule = other->fill_rule;

    if (other->font_family != NULL) {
        style->font_family = strdup (other->font_family);
        if (style->font_family == NULL)
            return SVG_STATUS_NO_MEMORY;
    } else {
        style->font_family = NULL;
    }

    style->font_size = other->font_size;
    style->computed_font_size = other->computed_font_size;
    style->font_style = other->font_style;
    style->font_weight = other->font_weight;

    style->opacity = other->opacity;

    style->num_dashes = other->num_dashes;
    if (style->num_dashes > 0) {
        style->stroke_dash_array = malloc (style->num_dashes * sizeof (double));
        if (style->stroke_dash_array == NULL)
            return SVG_STATUS_NO_MEMORY;
        memcpy (style->stroke_dash_array, other->stroke_dash_array,
                style->num_dashes * sizeof (double));
    } else {
        style->stroke_dash_array = NULL;
    }
    style->stroke_dash_offset = other->stroke_dash_offset;

    style->stroke_line_cap = other->stroke_line_cap;
    style->stroke_line_join = other->stroke_line_join;
    style->stroke_miter_limit = other->stroke_miter_limit;
    style->stroke_opacity = other->stroke_opacity;
    style->stroke_paint = other->stroke_paint;
    style->stroke_width = other->stroke_width;

    style->color = other->color;
    style->text_anchor = other->text_anchor;

    style->stop_color = other->stop_color;
    style->stop_opacity = other->stop_opacity;

    style->overflow_hidden = other->overflow_hidden;
    for (i = 0; i < 4; i++)
        style->clip_rect[i] = other->clip_rect[i];

    style->clip_path = other->clip_path;
    style->clip_rule = other->clip_rule;

    style->mask = other->mask;

    style->marker_start = other->marker_start;
    style->marker_mid = other->marker_mid;
    style->marker_end = other->marker_end;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_init_inherit (svg_style_t *style, svg_style_t *parent_style)
{
    int i;
    svg_color_t black_color;

    *style = *parent_style;

    style->flags = SVG_STYLE_FLAG_NONE;

    if (parent_style->font_family != NULL) {
        style->font_family = strdup (parent_style->font_family);
        if (style->font_family == NULL)
            return SVG_STATUS_NO_MEMORY;
    } else {
        style->font_family = NULL;
    }

    if (parent_style->stroke_dash_array != NULL) {
        style->num_dashes = parent_style->num_dashes;
        style->stroke_dash_array = malloc (style->num_dashes * sizeof (double));
        if (style->stroke_dash_array == NULL)
            return SVG_STATUS_NO_MEMORY;
        memcpy (style->stroke_dash_array, parent_style->stroke_dash_array,
                style->num_dashes * sizeof (double));
    } else {
        style->stroke_dash_array = NULL;
    }


    /* properties that are not inherited */

    if (!style->display) {
        style->display = 1;
        style->flags |= SVG_STYLE_FLAG_DISPLAY;
    }

    if (!style->visibility) {
        style->visibility = 1;
        style->flags |= SVG_STYLE_FLAG_VISIBILITY;
    }

    if (style->opacity != 1.0) {
        style->opacity = 1.0;
        style->flags |= SVG_STYLE_FLAG_OPACITY;
    }

    if (!style->overflow_hidden) {
        style->overflow_hidden = 1;
        style->flags |= SVG_STYLE_FLAG_OVERFLOW;
        for (i = 0; i < 4; i++)
            style->clip_rect[i].value = 0;
        style->flags |= SVG_STYLE_FLAG_CLIP;
    }

    if (style->stop_opacity != 1.0) {
        style->stop_opacity = 1.0;
        style->flags |= SVG_STYLE_FLAG_STOP_OPACITY;
    }
    _svg_color_init_from_str (&black_color, "black");
    if (memcmp (&style->stop_color, &black_color, sizeof (svg_color_t)) != 0) {
        style->stop_color = black_color;
        style->flags |= SVG_STYLE_FLAG_STOP_COLOR;
    }

    if (style->clip_path != NULL) {
        style->clip_path = NULL;
        style->flags |= SVG_STYLE_FLAG_CLIP_PATH;
    }
    if (style->mask != NULL) {
        style->mask = NULL;
        style->flags |= SVG_STYLE_FLAG_MASK;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_deinit (svg_style_t *style)
{
    if (style->font_family != NULL)
        free (style->font_family);

    if (style->stroke_dash_array != NULL)
        free (style->stroke_dash_array);

    return SVG_STATUS_SUCCESS;
}

void
_svg_style_reset_display (svg_style_t *style)
{
    style->display = 1;
}

svg_status_t
_svg_style_apply_properties (svg_dom_node_t *node, svg_t *svg)
{
    unsigned int i;
    svg_status_t status;
    const char *str;

    if (node->parent == NULL)
        status = _svg_style_init_defaults (node, svg);
    else
        status = _svg_style_init_inherit (&node->style, &node->parent->style);
    if (status)
        return status;

    for (i = 0; i < SVG_ARRAY_SIZE (SVG_STYLE_PARSE_MAP); i++) {
        const svg_style_parse_map_t *map;
        map = &SVG_STYLE_PARSE_MAP[i];

        _svg_attribute_get_string (node->qattrs, map->name, &str, NULL);

        if (str != NULL) {
            status = (map->parse) (node, svg, str);
            if (status)
                return _svg_style_return_property_error (svg, map->name, status);
        }
    }

    status = _svg_style_apply_css_properties (node, svg, 0);
    if (status)
        return status;

    status = _svg_style_apply_css_properties (node, svg, 1);
    if (status)
        return status;


    status = _svg_style_compute_colors (node);
    if (status)
        return status;


    if (node->override_display_property)
        node->style.display = 1;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_render (svg_dom_node_t *node, svg_render_engine_t *engine, void *closure)
{
    svg_status_t status;
    svg_style_t *style = &node->style;


    if (style->flags & SVG_STYLE_FLAG_VISIBILITY) {
        status = _svg_engine_set_visibility (engine, closure, style->visibility);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_OPACITY) {
        status = _svg_engine_set_opacity (engine, closure, style->opacity);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_COLOR) {
        status = _svg_engine_set_color (engine, closure, &style->color);
        if (status)
            return status;
    }

    status = _svg_style_compute_font_size (node, engine, closure);
    if (status)
        return status;

    if (style->flags & SVG_STYLE_FLAG_FONT_FAMILY) {
        status = _svg_engine_set_font_family (engine, closure, style->font_family);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FONT_SIZE) {
        status = _svg_engine_set_font_size (engine, closure, style->computed_font_size);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FONT_STYLE) {
        status = _svg_engine_set_font_style (engine, closure, style->font_style);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FONT_WEIGHT) {
        status = _svg_engine_set_font_weight (engine, closure, style->font_weight);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FILL_OPACITY) {
        status = _svg_engine_set_fill_opacity (engine, closure, style->fill_opacity);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FILL_PAINT) {
        status = _svg_engine_set_fill_paint (engine, closure, &style->fill_paint);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_FILL_RULE) {
        status = _svg_engine_set_fill_rule (engine, closure, style->fill_rule);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_DASH_ARRAY) {
        /* XXX: How to deal with units of svg_length_t ? */
        status =
            _svg_engine_set_stroke_dash_array (engine, closure, style->stroke_dash_array,
                                               style->num_dashes);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_DASH_OFFSET) {
        status = _svg_engine_set_stroke_dash_offset (engine, closure, &style->stroke_dash_offset);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_LINE_CAP) {
        status = _svg_engine_set_stroke_line_cap (engine, closure, style->stroke_line_cap);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_LINE_JOIN) {
        status = _svg_engine_set_stroke_line_join (engine, closure, style->stroke_line_join);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_MITER_LIMIT) {
        status = _svg_engine_set_stroke_miter_limit (engine, closure, style->stroke_miter_limit);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_OPACITY) {
        status = _svg_engine_set_stroke_opacity (engine, closure, style->stroke_opacity);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_PAINT) {
        status = _svg_engine_set_stroke_paint (engine, closure, &style->stroke_paint);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_STROKE_WIDTH) {
        status = _svg_engine_set_stroke_width (engine, closure, &style->stroke_width);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_TEXT_ANCHOR) {
        status = _svg_engine_set_text_anchor (engine, closure, style->text_anchor);
        if (status)
            return status;
    }


    if (style->flags & SVG_STYLE_FLAG_CLIP_PATH) {
        status = _svg_engine_set_clip_path (engine, closure, style->clip_path);
        if (status)
            return status;

        if (style->flags & SVG_STYLE_FLAG_CLIP_RULE) {
            status = _svg_engine_set_clip_rule (engine, closure, style->clip_rule);
            if (status)
                return status;
        }
    }

    if (style->flags & SVG_STYLE_FLAG_MASK) {
        status = _svg_engine_set_mask (engine, closure, style->mask);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_MARKER_START) {
        status = _svg_engine_set_marker_start (engine, closure, style->marker_start);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_MARKER_MID) {
        status = _svg_engine_set_marker_mid (engine, closure, style->marker_mid);
        if (status)
            return status;
    }

    if (style->flags & SVG_STYLE_FLAG_MARKER_END) {
        status = _svg_engine_set_marker_end (engine, closure, style->marker_end);
        if (status)
            return status;
    }

    status = _svg_engine_end_style (engine, closure);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_style_render_viewport_clipping_path (svg_style_t *style,
                                          svg_render_engine_t *engine, void *closure)
{
    svg_status_t status;

    if (style->overflow_hidden) {
        status = _svg_engine_viewport_clipping_path (engine, closure, &style->clip_rect[0],
                                                     &style->clip_rect[1], &style->clip_rect[2],
                                                     &style->clip_rect[3]);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

double
_svg_style_get_opacity (svg_style_t *style)
{
    return style->opacity;
}

int
_svg_style_get_display (svg_style_t *style)
{
    return style->display;
}

