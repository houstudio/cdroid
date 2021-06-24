/* svg_view_box.c: Data structures for SVG viewbox attributes

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
_svg_view_box_init (svg_view_box_t *view_box)
{
    view_box->box.x = 0;
    view_box->box.y = 0;
    view_box->box.width = 0;
    view_box->box.height = 0;

    view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_UNKNOWN;
    view_box->meet_or_slice = SVG_MEET_OR_SLICE_UNKNOWN;

    return SVG_STATUS_SUCCESS;
}

int
_svg_view_box_is_null (svg_view_box_t *view_box)
{
    return view_box->aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_UNKNOWN;
}

svg_status_t
_svg_parse_view_box (const char *view_box_str, svg_view_box_t *view_box)
{
    const char *s;
    const char *end;

    s = view_box_str;
    view_box->box.x = _svg_ascii_strtod (s, &end);
    if (end == s)
        return SVG_STATUS_PARSE_ERROR;

    s = end;
    _svg_str_skip_space_or_char (&s, ',');
    view_box->box.y = _svg_ascii_strtod (s, &end);
    if (end == s)
        return SVG_STATUS_PARSE_ERROR;

    s = end;
    _svg_str_skip_space_or_char (&s, ',');
    view_box->box.width = _svg_ascii_strtod (s, &end);
    if (end == s)
        return SVG_STATUS_PARSE_ERROR;
    if (view_box->box.width < 0)
        return SVG_STATUS_INVALID_VALUE;

    s = end;
    _svg_str_skip_space_or_char (&s, ',');
    view_box->box.height = _svg_ascii_strtod (s, &end);
    if (end == s)
        return SVG_STATUS_PARSE_ERROR;
    if (view_box->box.height < 0)
        return SVG_STATUS_INVALID_VALUE;


    if (view_box->aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_UNKNOWN) {
        view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMIDYMID;
        view_box->meet_or_slice = SVG_MEET_OR_SLICE_MEET;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_view_box_parse_aspect_ratio (const char *aspect_ratio_str, svg_view_box_t *view_box)
{
    const char *s = aspect_ratio_str;

    if (strncmp (s, "defer", 5) == 0) {
        view_box->defer = 1;
        s += 5;
    } else {
        view_box->defer = 0;
    }

    _svg_str_skip_space (&s);

    if (strncmp (aspect_ratio_str, "none", 4) == 0) {
        view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_NONE;
        s += 4;
    } else {
        if (strncmp (aspect_ratio_str, "xMinYMin", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMINYMIN;
        else if (strncmp (aspect_ratio_str, "xMidYMin", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN;
        else if (strncmp (aspect_ratio_str, "xMaxYMin", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN;
        else if (strncmp (aspect_ratio_str, "xMinYMid", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMINYMID;
        else if (strncmp (aspect_ratio_str, "xMidYMid", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMIDYMID;
        else if (strncmp (aspect_ratio_str, "xMaxYMid", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMAXYMID;
        else if (strncmp (aspect_ratio_str, "xMinYMax", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMINYMAX;
        else if (strncmp (aspect_ratio_str, "xMidYMax", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX;
        else if (strncmp (aspect_ratio_str, "xMaxYMax", 8) == 0)
            view_box->aspect_ratio = SVG_PRESERVE_ASPECT_RATIO_XMAXYMAX;
        else
            return SVG_STATUS_PARSE_ERROR;

        s += 8;
    }

    if (view_box->aspect_ratio != SVG_PRESERVE_ASPECT_RATIO_NONE) {
        _svg_str_skip_space (&s);

        if (strncmp (s, "meet", 4) == 0)
            view_box->meet_or_slice = SVG_MEET_OR_SLICE_MEET;
        else if (strncmp (s, "slice", 5) == 0)
            view_box->meet_or_slice = SVG_MEET_OR_SLICE_SLICE;
        else
            return SVG_STATUS_PARSE_ERROR;
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_view_box_transform (const svg_view_box_t *view_box,
                         double viewport_width, double viewport_height, svg_transform_t *transform)
{
    double x_scale, y_scale, scale;
    double vbox_x_min, vbox_x_mid, vbox_x_max;
    double vbox_y_min, vbox_y_mid, vbox_y_max;
    double vport_x_min, vport_x_mid, vport_x_max;
    double vport_y_min, vport_y_mid, vport_y_max;

    if (view_box->box.width == 0 || view_box->box.height == 0 ||
        viewport_width == 0 || viewport_height == 0)
    {
        _svg_transform_init_scale (transform, 0, 0);
        return SVG_STATUS_SUCCESS;
    }


    x_scale = viewport_width / view_box->box.width;
    y_scale = viewport_height / view_box->box.height;

    _svg_transform_init (transform);

    if (view_box->aspect_ratio == SVG_PRESERVE_ASPECT_RATIO_NONE) {
        _svg_transform_add_scale (transform, x_scale, y_scale);
        _svg_transform_add_translate (transform, -view_box->box.x, -view_box->box.y);
    } else {
        if (view_box->meet_or_slice == SVG_MEET_OR_SLICE_MEET) {
            if (x_scale < y_scale)
                scale = x_scale;
            else
                scale = y_scale;
        } else {
            if (x_scale < y_scale)
                scale = y_scale;
            else
                scale = x_scale;
        }

        _svg_transform_add_scale (transform, scale, scale);

        vbox_x_min = view_box->box.x;
        vbox_x_max = view_box->box.x + view_box->box.width;
        vbox_x_mid = vbox_x_min + (vbox_x_max - vbox_x_min) / 2;
        vbox_y_min = view_box->box.y;
        vbox_y_max = view_box->box.y + view_box->box.height;
        vbox_y_mid = vbox_y_min + (vbox_y_max - vbox_y_min) / 2;

        vport_x_min = 0;
        vport_x_max = viewport_width / scale;
        vport_x_mid = vport_x_min + (vport_x_max - vport_x_min) / 2;
        vport_y_min = 0;
        vport_y_max = viewport_height / scale;
        vport_y_mid = vport_y_min + (vport_y_max - vport_y_min) / 2;

        switch (view_box->aspect_ratio) {
        case SVG_PRESERVE_ASPECT_RATIO_XMINYMIN:
            _svg_transform_add_translate (transform, vport_x_min - vbox_x_min,
                                          vport_y_min - vbox_y_min);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMIDYMIN:
            _svg_transform_add_translate (transform, vport_x_mid - vbox_x_mid,
                                          vport_y_min - vbox_y_min);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMAXYMIN:
            _svg_transform_add_translate (transform, vport_x_max - vbox_x_max,
                                          vport_y_min - vbox_y_min);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMINYMID:
            _svg_transform_add_translate (transform, vport_x_min - vbox_x_min,
                                          vport_y_mid - vbox_y_mid);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMIDYMID:
            _svg_transform_add_translate (transform, vport_x_mid - vbox_x_mid,
                                          vport_y_mid - vbox_y_mid);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMAXYMID:
            _svg_transform_add_translate (transform, vport_x_max - vbox_x_max,
                                          vport_y_mid - vbox_y_mid);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMINYMAX:
            _svg_transform_add_translate (transform, vport_x_min - vbox_x_min,
                                          vport_y_max - vbox_y_max);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMIDYMAX:
            _svg_transform_add_translate (transform, vport_x_mid - vbox_x_mid,
                                          vport_y_max - vbox_y_max);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_XMAXYMAX:
            _svg_transform_add_translate (transform, vport_x_max - vbox_x_max,
                                          vport_y_max - vbox_y_max);
            break;
        case SVG_PRESERVE_ASPECT_RATIO_UNKNOWN:
        case SVG_PRESERVE_ASPECT_RATIO_NONE:
            break;
        }
    }

    return SVG_STATUS_SUCCESS;
}




void
svg_complete_image_viewbox (svg_view_box_t *view_box, double image_width, double image_height)
{
    view_box->box.width = image_width;
    view_box->box.height = image_height;
}

svg_status_t
svg_get_viewbox_transform (const svg_view_box_t *view_box,
                           double viewport_width, double viewport_height,
                           svg_transform_t *transform)
{
    svg_status_t status;

    status = _svg_view_box_transform (view_box, viewport_width, viewport_height, transform);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

