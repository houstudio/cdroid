/* svg_length.c: Data structures for SVG lengths

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

#include <math.h>


svg_status_t
_svg_length_init (svg_length_t *length, double value)
{
    return _svg_length_init_unit (length, value, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_OTHER);
}

svg_status_t
_svg_length_init_unit (svg_length_t *length, double value, svg_length_unit_t unit,
                       svg_length_orientation_t orientation)
{
    length->value = value;
    length->unit = unit;
    length->orientation = orientation;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_length_init_from_str (svg_length_t *length, const char *str)
{
    double value;
    svg_length_unit_t unit;
    const char *unit_str;

    value = _svg_ascii_strtod (str, &unit_str);

    if (unit_str == str)
        return SVG_STATUS_PARSE_ERROR;

    if (unit_str != NULL) {
        if (strcmp (unit_str, "px") == 0)
            unit = SVG_LENGTH_UNIT_PX;
        else if (strcmp (unit_str, "pt") == 0)
            unit = SVG_LENGTH_UNIT_PT;
        else if (strcmp (unit_str, "in") == 0)
            unit = SVG_LENGTH_UNIT_IN;
        else if (strcmp (unit_str, "cm") == 0)
            unit = SVG_LENGTH_UNIT_CM;
        else if (strcmp (unit_str, "mm") == 0)
            unit = SVG_LENGTH_UNIT_MM;
        else if (strcmp (unit_str, "pc") == 0)
            unit = SVG_LENGTH_UNIT_PC;
        else if (strcmp (unit_str, "em") == 0)
            unit = SVG_LENGTH_UNIT_EM;
        else if (strcmp (unit_str, "ex") == 0)
            unit = SVG_LENGTH_UNIT_EX;
        else if (strcmp (unit_str, "%") == 0)
            unit = SVG_LENGTH_UNIT_PCT;
        else {
            unit = SVG_LENGTH_UNIT_PX;
        }
    } else {
        unit = SVG_LENGTH_UNIT_PX;
    }

    return _svg_length_init_unit (length, value, unit, length->orientation);
}

svg_status_t
_svg_length_init_from_array_str (svg_length_t *length, const char *str, const char **after_unit)
{
    double value;
    svg_length_unit_t unit;
    const char *unit_str;

    value = _svg_ascii_strtod (str, &unit_str);

    if (unit_str == str)
        return SVG_STATUS_PARSE_ERROR;

    if (unit_str != NULL) {
        if (strncmp (unit_str, "px", 2) == 0) {
            unit = SVG_LENGTH_UNIT_PX;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "pt", 2) == 0) {
            unit = SVG_LENGTH_UNIT_PT;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "in", 2) == 0) {
            unit = SVG_LENGTH_UNIT_IN;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "cm", 2) == 0) {
            unit = SVG_LENGTH_UNIT_CM;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "mm", 2) == 0) {
            unit = SVG_LENGTH_UNIT_MM;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "pc", 2) == 0) {
            unit = SVG_LENGTH_UNIT_PC;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "em", 2) == 0) {
            unit = SVG_LENGTH_UNIT_EM;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "ex", 2) == 0) {
            unit = SVG_LENGTH_UNIT_EX;
            *after_unit = unit_str + 2;
        } else if (strncmp (unit_str, "%", 1) == 0) {
            unit = SVG_LENGTH_UNIT_PCT;
            *after_unit = unit_str + 1;
        } else {
            unit = SVG_LENGTH_UNIT_PX;
            *after_unit = unit_str;
        }
    } else {
        unit = SVG_LENGTH_UNIT_PX;
        *after_unit = unit_str;
    }

    return _svg_length_init_unit (length, value, unit, length->orientation);
}




double
svg_convert_length_to_user_units (const svg_length_context_t *context, const svg_length_t *length)
{
    switch (length->unit) {
    case SVG_LENGTH_UNIT_CM:
        return length->value * context->dpi / 2.54;
    case SVG_LENGTH_UNIT_EM:
        return length->value * context->font_size;
    case SVG_LENGTH_UNIT_EX:
        return length->value * context->x_height;
    case SVG_LENGTH_UNIT_IN:
        return length->value * context->dpi;
    case SVG_LENGTH_UNIT_MM:
        return length->value * context->dpi / 25.4;
    case SVG_LENGTH_UNIT_PC:
        return length->value * context->dpi / 6.0;
    case SVG_LENGTH_UNIT_PCT:
        if (context->bbox_user_units) {
            return length->value / 100.0;
        } else {
            if (length->orientation == SVG_LENGTH_ORIENTATION_HORIZONTAL)
                return (length->value / 100.0) * context->viewport_width;
            else if (length->orientation == SVG_LENGTH_ORIENTATION_VERTICAL)
                return (length->value / 100.0) * context->viewport_height;
            else
                return (length->value / 100.0) *
                    sqrt ((context->viewport_width * context->viewport_width +
                           context->viewport_height * context->viewport_height) / 2.0);
        }
    case SVG_LENGTH_UNIT_PT:
        return length->value * context->dpi / 72.0;
    case SVG_LENGTH_UNIT_PX:
    default:
        return length->value;
    }
}

