/* svg_gradient.c: Data structures for SVG gradients

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

#include <math.h>


svg_status_t
_svg_gradient_stop_init (svg_gradient_stop_t *stop)
{
    memset (&stop->color, 0, sizeof (svg_color_t));
    stop->offset = 0;
    stop->opacity = 1.0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_stop_init_copy (svg_gradient_stop_t *stop, svg_gradient_stop_t *other)
{
    *stop = *other;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_stop_deinit (svg_gradient_stop_t *stop)
{
    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_stop_apply_attributes (svg_element_t *stop_element, const svg_qattrs_t *attributes)
{
    svg_gradient_stop_t *stop = &stop_element->e.gradient_stop;
    svg_length_t offset_length;
    svg_status_t status;

    status = _svg_attribute_get_length (attributes, "offset", &offset_length, "0");
    if (status)
        return _svg_element_return_property_error (stop_element, "offset", status);

    if (offset_length.unit == SVG_LENGTH_UNIT_PCT)
        stop->offset = offset_length.value / 100.0;
    else
        stop->offset = offset_length.value;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_stop_apply_style (svg_gradient_stop_t *stop, svg_style_t *style)
{
    stop->color = style->stop_color;
    stop->opacity = style->stop_opacity;

    return SVG_STATUS_SUCCESS;
}


svg_status_t
_svg_gradient_init (svg_gradient_int_t *gradient)
{
    svg_gradient_t *ext_gradient = &gradient->ext_gradient;
    svg_status_t status;

    status = _svg_container_init (&gradient->container);
    if (status)
        return status;

    _svg_gradient_set_type (gradient, SVG_GRADIENT_LINEAR);

    ext_gradient->units = SVG_COORD_SPACE_UNITS_BBOX;
    ext_gradient->spread = SVG_GRADIENT_SPREAD_PAD;

    _svg_transform_init (&ext_gradient->transform);

    ext_gradient->stops = NULL;
    ext_gradient->num_stops = 0;
    ext_gradient->stops_size = 0;

    gradient->inherited_stops = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_init_copy (svg_gradient_int_t *gradient, svg_gradient_int_t *other)
{
    svg_gradient_t *ext_gradient = &gradient->ext_gradient;
    svg_gradient_t *ext_other = &other->ext_gradient;
    svg_status_t status;

    /* stop elements are not copied */
    status = _svg_container_init (&gradient->container);
    if (status)
        return status;

    *ext_gradient = *ext_other;

    if (ext_gradient->num_stops > 0)
        gradient->inherited_stops = 1;
    else
        gradient->inherited_stops = 0;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_deinit (svg_gradient_int_t *gradient)
{
    svg_gradient_t *ext_gradient = &gradient->ext_gradient;
    svg_status_t status;

    status = _svg_container_deinit (&gradient->container);
    if (status)
        return status;

    if (ext_gradient->stops != NULL && !gradient->inherited_stops)
        free (ext_gradient->stops);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_set_type (svg_gradient_int_t *gradient, svg_gradient_type_t type)
{
    svg_gradient_t *ext_gradient = &gradient->ext_gradient;

    ext_gradient->type = type;

    if (ext_gradient->type == SVG_GRADIENT_LINEAR) {
        _svg_length_init_unit (&ext_gradient->u.linear.x1, 0, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_HORIZONTAL);
        _svg_length_init_unit (&ext_gradient->u.linear.y1, 0, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_VERTICAL);
        _svg_length_init_unit (&ext_gradient->u.linear.x2, 100, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_HORIZONTAL);
        _svg_length_init_unit (&ext_gradient->u.linear.y2, 0, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_VERTICAL);
    } else {
        _svg_length_init_unit (&ext_gradient->u.radial.cx, 50, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_HORIZONTAL);
        _svg_length_init_unit (&ext_gradient->u.radial.cy, 50, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_VERTICAL);
        _svg_length_init_unit (&ext_gradient->u.radial.fx, 50, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_HORIZONTAL);
        _svg_length_init_unit (&ext_gradient->u.radial.fy, 50, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_VERTICAL);
        _svg_length_init_unit (&ext_gradient->u.radial.r, 50, SVG_LENGTH_UNIT_PCT,
                               SVG_LENGTH_ORIENTATION_OTHER);
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_add_stop (svg_gradient_int_t *gradient, svg_gradient_stop_t *stop)
{
    svg_gradient_t *ext_gradient = &gradient->ext_gradient;
    svg_gradient_stop_t *new_ext_stops, *new_ext_stop;

    if (gradient->inherited_stops) {
        ext_gradient->stops = NULL;
        ext_gradient->num_stops = 0;
        ext_gradient->stops_size = 0;

        gradient->inherited_stops = 0;
    }

    if (ext_gradient->num_stops >= ext_gradient->stops_size) {
        int old_size = ext_gradient->stops_size;
        if (ext_gradient->stops_size)
            ext_gradient->stops_size *= 2;
        else
            ext_gradient->stops_size = 2;       /* Any useful gradient has at least 2 */
        new_ext_stops = realloc (ext_gradient->stops,
                                 ext_gradient->stops_size * sizeof (svg_gradient_stop_t));
        if (new_ext_stops == NULL) {
            ext_gradient->stops_size = old_size;
            return SVG_STATUS_NO_MEMORY;
        }
        ext_gradient->stops = new_ext_stops;
    }

    new_ext_stop = &ext_gradient->stops[ext_gradient->num_stops];
    if (ext_gradient->num_stops == 0 ||
        stop->offset >= ext_gradient->stops[ext_gradient->num_stops - 1].offset)
    {
        new_ext_stop->offset = stop->offset;
    }
    else
    {
        new_ext_stop->offset = ext_gradient->stops[ext_gradient->num_stops - 1].offset;
    }
    new_ext_stop->color = stop->color;
    new_ext_stop->opacity = stop->opacity;

    ext_gradient->num_stops++;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_gradient_apply_attributes (svg_element_t *gradient_element, const svg_qattrs_t *attributes)
{
    svg_gradient_int_t *gradient = &gradient_element->e.gradient;
    svg_gradient_t *ext_gradient = &gradient->ext_gradient;
    svg_status_t status;
    const char *href;
    const char *str;
    svg_gradient_int_t *prototype = NULL;
    svg_length_t h_len, v_len, o_len;

    _svg_length_init_unit (&h_len, 0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_HORIZONTAL);
    _svg_length_init_unit (&v_len, 0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_VERTICAL);
    _svg_length_init_unit (&o_len, 0, SVG_LENGTH_UNIT_PCT, SVG_LENGTH_ORIENTATION_OTHER);


    _svg_attribute_get_string_ns (attributes, XLINK_NAMESPACE_INDEX, "href", &href, NULL);
    if (href != NULL) {
        svg_element_t *ref = NULL;
        svg_gradient_t save_ext_gradient = *ext_gradient;

        status = _svg_resolve_element_href (gradient_element->svg,
                                            gradient_element->node->document_uri,
                                            gradient_element->node->base_uri, href, &ref);
        if (status) {
            if (status == SVG_STATUS_UNKNOWN_REF_ELEMENT)
                return status;  /* don't set error here because next time round it could be resolved */
            else
                return _svg_element_return_property_error (gradient_element, "href", status);
        }

        if (ref->type != SVG_ELEMENT_TYPE_GRADIENT)
            return _svg_element_return_property_error (gradient_element, "href",
                                                       SVG_STATUS_WRONG_REF_ELEMENT_TYPE);

        prototype = &ref->e.gradient;

        _svg_gradient_deinit (gradient);
        _svg_gradient_init_copy (gradient, prototype);

        if (ext_gradient->type != save_ext_gradient.type) {
            ext_gradient->type = save_ext_gradient.type;
            ext_gradient->u = save_ext_gradient.u;
        }
    }

    status = _svg_attribute_get_string (attributes, "gradientUnits", &str, "objectBoundingBox");
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND) {
        if (strcmp (str, "userSpaceOnUse") == 0)
            ext_gradient->units = SVG_COORD_SPACE_UNITS_USER;
        else if (strcmp (str, "objectBoundingBox") == 0)
            ext_gradient->units = SVG_COORD_SPACE_UNITS_BBOX;
        else
            return _svg_element_return_property_error (gradient_element, "gradientUnits",
                                                       SVG_STATUS_INVALID_VALUE);
    }

    status = _svg_attribute_get_string (attributes, "gradientTransform", &str, NULL);
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND) {
        status = _svg_transform_parse_str (&ext_gradient->transform, str);
        if (status)
            return _svg_element_return_property_error (gradient_element, "gradientTransform",
                                                       status);
    }

    status = _svg_attribute_get_string (attributes, "spreadMethod", &str, "pad");
    if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND) {
        if (strcmp (str, "pad") == 0)
            ext_gradient->spread = SVG_GRADIENT_SPREAD_PAD;
        else if (strcmp (str, "reflect") == 0)
            ext_gradient->spread = SVG_GRADIENT_SPREAD_REFLECT;
        else if (strcmp (str, "repeat") == 0)
            ext_gradient->spread = SVG_GRADIENT_SPREAD_REPEAT;
        else
            return _svg_element_return_property_error (gradient_element, "spreadMethod",
                                                       SVG_STATUS_INVALID_VALUE);
    }

    if (ext_gradient->type == SVG_GRADIENT_LINEAR) {
        status = _svg_attribute_get_length (attributes, "x1", &h_len, "0%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "x1", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.linear.x1 = h_len;

        status = _svg_attribute_get_length (attributes, "y1", &v_len, "0%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "y1", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.linear.y1 = v_len;

        status = _svg_attribute_get_length (attributes, "x2", &h_len, "100%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "x2", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.linear.x2 = h_len;

        status = _svg_attribute_get_length (attributes, "y2", &v_len, "0%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "y2", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.linear.y2 = v_len;
    } else {
        status = _svg_attribute_get_length (attributes, "cx", &h_len, "50%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "cx", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.radial.cx = h_len;

        status = _svg_attribute_get_length (attributes, "cy", &v_len, "50%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "cy", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.radial.cy = v_len;

        status = _svg_attribute_get_length (attributes, "r", &o_len, "50%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "r", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.radial.r = o_len;

        status = _svg_attribute_get_length (attributes, "fx", &h_len, "50%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "fx", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.radial.fx = h_len;
        else
            ext_gradient->u.radial.fx = ext_gradient->u.radial.cx;

        status = _svg_attribute_get_length (attributes, "fy", &v_len, "50%");
        if (status && status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            return _svg_element_return_property_error (gradient_element, "fy", status);
        if (status != SVG_STATUS_ATTRIBUTE_NOT_FOUND)
            ext_gradient->u.radial.fy = v_len;
        else
            ext_gradient->u.radial.fy = ext_gradient->u.radial.cy;
    }

    return SVG_STATUS_SUCCESS;
}




void
svg_legalize_radial_gradient_focal_point (double cx, double cy, double r, double fx_in,
                                          double fy_in, double *fx_out, double *fy_out)
{
    double dx, dy;
    double cfr;
    double angle;

    dx = fx_in - cx;
    dy = fy_in - cy;
    cfr = sqrt (dx * dx + dy * dy);

    if (cfr > r) {
        angle = atan2 (dy, dx);

        *fx_out = cx + r * cos (angle);
        *fy_out = cy + r * sin (angle);
    } else {
        *fx_out = fx_in;
        *fy_out = fy_in;
    }
}

