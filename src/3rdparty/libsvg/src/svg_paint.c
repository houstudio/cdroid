/* svg_path.c: Data structures for SVG paths

   Copyright  2002 USC/Information Sciences Institute

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
_svg_paint_init (svg_paint_t *paint, svg_t *svg, svg_uri_t *document_uri, svg_uri_t *base_uri,
                 const char *str)
{
    svg_status_t status;
    svg_element_t *element = NULL;

    if (strcmp (str, "none") == 0) {
        paint->type = SVG_PAINT_TYPE_NONE;
        return SVG_STATUS_SUCCESS;
    }

    if (_svg_is_uri_ref_url (str)) {
        status = _svg_resolve_element_url (svg, document_uri, base_uri, str, &element);
        if (status)
            return status;

        switch (element->type) {
        case SVG_ELEMENT_TYPE_GRADIENT:
            paint->type = SVG_PAINT_TYPE_GRADIENT;
            paint->p.gradient = &element->e.gradient.ext_gradient;
            break;
        case SVG_ELEMENT_TYPE_PATTERN:
            paint->type = SVG_PAINT_TYPE_PATTERN;
            paint->p.pattern = &element->e.pattern.ext_pattern;
            break;
        default:
            return SVG_STATUS_WRONG_REF_ELEMENT_TYPE;
        }

        return SVG_STATUS_SUCCESS;
    }

    status = _svg_color_init_from_str (&paint->p.color, str);
    if (status)
        return status;
    paint->type = SVG_PAINT_TYPE_COLOR;

    return SVG_STATUS_SUCCESS;
}

