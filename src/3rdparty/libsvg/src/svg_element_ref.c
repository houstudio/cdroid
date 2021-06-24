/* svg_element_ref.c: Data structures for SVG graphics element references

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
_svg_create_element_ref (svg_element_t *element, svg_element_ref_t **element_ref)
{
    svg_element_ref_t *new_element_ref;

    new_element_ref = (svg_element_ref_t *) malloc (sizeof (svg_element_ref_t));
    if (new_element_ref == NULL)
        return SVG_STATUS_NO_MEMORY;

    new_element_ref->element = element;

    *element_ref = new_element_ref;

    return SVG_STATUS_SUCCESS;
}

void
_svg_destroy_element_ref (svg_element_ref_t *element_ref)
{
    if (element_ref == NULL)
        return;

    free (element_ref);
}

svg_status_t
svg_element_ref_render (svg_element_ref_t *element_ref, svg_render_engine_t *target_engine,
                        void *target_closure)
{
    svg_status_t status;
    uint64_t style_flags;
    svg_render_engine_t *engine;
    void *closure;

    if (element_ref->element->svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;


    if (element_ref->element->svg->trace != NULL) {
        status = _svg_trace_push_target_engine (element_ref->element->svg->trace,
                                                element_ref->element->node->document_uri,
                                                target_engine, target_closure);
        if (status)
            return status;

        _svg_trace_get_engine (element_ref->element->svg->trace, &engine, &closure);
    } else {
        engine = target_engine;
        closure = target_closure;
    }


    switch (element_ref->element->type) {
    case SVG_ELEMENT_TYPE_PATTERN:
        element_ref->element->e.pattern.enable_render = 1;
        break;
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        element_ref->element->e.clip_path.enable_render = 1;
        break;
    case SVG_ELEMENT_TYPE_MASK:
        element_ref->element->e.mask.enable_render = 1;
        break;
    case SVG_ELEMENT_TYPE_MARKER:
        element_ref->element->e.marker.enable_render = 1;
        break;
    default:
        break;
    }


    style_flags = element_ref->element->node->style.flags;
    element_ref->element->node->style.flags = (uint64_t) (-1);

    status = svg_element_render (element_ref->element, engine, closure);

    element_ref->element->node->style.flags = style_flags;


    switch (element_ref->element->type) {
    case SVG_ELEMENT_TYPE_PATTERN:
        element_ref->element->e.pattern.enable_render = 0;
        break;
    case SVG_ELEMENT_TYPE_CLIP_PATH:
        element_ref->element->e.clip_path.enable_render = 0;
        break;
    case SVG_ELEMENT_TYPE_MASK:
        element_ref->element->e.mask.enable_render = 0;
        break;
    case SVG_ELEMENT_TYPE_MARKER:
        element_ref->element->e.marker.enable_render = 0;
        break;
    default:
        break;
    }


    if (element_ref->element->svg->trace != NULL)
        _svg_trace_pop_target_engine (element_ref->element->svg->trace);


    return status;
}

