/* svg_symbol.c: Data structures for SVG symbol elements

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
_svg_symbol_init (svg_symbol_t *symbol)
{
    svg_status_t status;

    status = _svg_container_init (&symbol->container);
    if (status)
        return status;

    _svg_view_box_init (&symbol->view_box);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_symbol_init_copy (svg_symbol_t *symbol, svg_symbol_t *other)
{
    svg_status_t status;

    status = _svg_container_init_copy (&symbol->container, &other->container);
    if (status)
        return status;

    symbol->view_box = other->view_box;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_symbol_deinit (svg_symbol_t *symbol)
{
    svg_status_t status;

    status = _svg_container_deinit (&symbol->container);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_symbol_apply_attributes (svg_element_t *symbol_element, const svg_qattrs_t *attributes)
{
    svg_symbol_t *symbol = &symbol_element->e.symbol;
    const char *view_box_str, *aspect_ratio_str;
    svg_status_t status;

    _svg_style_reset_display (&symbol_element->node->style);

    _svg_attribute_get_string (attributes, "viewBox", &view_box_str, NULL);
    if (view_box_str != NULL) {
        status = _svg_parse_view_box (view_box_str, &symbol->view_box);
        if (status)
            return _svg_element_return_property_error (symbol_element, "viewBox", status);

        _svg_attribute_get_string (attributes, "preserveAspectRatio", &aspect_ratio_str,
                                   "xMidYMid meet");
        status = _svg_view_box_parse_aspect_ratio (aspect_ratio_str, &symbol->view_box);
        if (status)
            return _svg_element_return_property_error (symbol_element, "preserveAspectRatio",
                                                       status);
    }

    return SVG_STATUS_SUCCESS;
}

