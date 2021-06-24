/* svg_uri_reference.c: URI reference parsing and utilities

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


static svg_status_t
_svg_create_empty_uri_ref (svg_uri_ref_t **uri_ref)
{
    svg_uri_ref_t *new_uri_ref;

    new_uri_ref = (svg_uri_ref_t *) malloc (sizeof (svg_uri_ref_t));
    if (new_uri_ref == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_uri_ref, 0, sizeof (svg_uri_ref_t));

    *uri_ref = new_uri_ref;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_create_uri_ref (const char *uri_ref_str, svg_uri_t *base_uri, svg_uri_t *document_uri,
                     svg_uri_ref_t **uri_ref)
{
    svg_uri_ref_t *new_uri_ref = NULL;
    svg_status_t status;
    char *fragment_start = NULL;
    const char *element_id_start, *element_id_end;
    size_t element_id_len, unused_len;
    svg_uri_t *abs_uri = NULL;

    status = _svg_create_empty_uri_ref (&new_uri_ref);
    if (status)
        return status;

    fragment_start = strchr (uri_ref_str, '#');

    if (fragment_start != NULL)
        *fragment_start = '\0';
    status = _svg_create_uri (uri_ref_str, &new_uri_ref->uri);
    if (fragment_start != NULL)
        *fragment_start = '#';
    if (status)
        goto fail;

    if (_svg_uri_is_relative (new_uri_ref->uri)) {
        if (_svg_uri_is_empty_path (new_uri_ref->uri)) {
            if (document_uri != NULL) {
                status = _svg_uri_create_absolute (document_uri, new_uri_ref->uri, &abs_uri);
                if (status)
                    goto fail;
            }
        } else if (base_uri != NULL) {
            status = _svg_uri_create_absolute (base_uri, new_uri_ref->uri, &abs_uri);
            if (status)
                goto fail;
        }

        _svg_destroy_uri (new_uri_ref->uri);
        new_uri_ref->uri = abs_uri;
        abs_uri = NULL;
    }

    if (fragment_start != NULL) {
        element_id_start = fragment_start + 1;

        if (strncmp (element_id_start, "xpointer(id(", 12) == 0)
            element_id_start += 12;

        element_id_end = element_id_start;
        while (*element_id_end != '\0' && *element_id_end != ')' &&
               !_svg_ascii_isspace (*element_id_end))
        {
            element_id_end++;
        }

        element_id_len = (size_t) (element_id_end - element_id_start);
        if (element_id_len > 0) {
            new_uri_ref->element_id = (char *) malloc (element_id_len + 1);
            if (new_uri_ref->element_id == NULL) {
                status = SVG_STATUS_NO_MEMORY;
                goto fail;
            }

            memcpy (new_uri_ref->element_id, element_id_start, element_id_len);
            new_uri_ref->element_id[element_id_len] = '\0';

            status = _svg_unescape_uri_string (new_uri_ref->element_id, &unused_len);
            if (status)
                goto fail;
        }
    }

    *uri_ref = new_uri_ref;

    return SVG_STATUS_SUCCESS;

  fail:
    if (new_uri_ref != NULL)
        _svg_destroy_uri_ref (new_uri_ref);

    return status;
}

svg_status_t
_svg_create_uri_ref_url (const char *attribute, svg_uri_t *base_uri, svg_uri_t *document_uri,
                         svg_uri_ref_t **uri_ref)
{
    if (!_svg_is_uri_ref_url (attribute))
        return SVG_STATUS_INVALID_CALL;

    return _svg_create_uri_ref (attribute + 4, base_uri, document_uri, uri_ref);
}

void
_svg_destroy_uri_ref (svg_uri_ref_t *uri_ref)
{
    if (uri_ref == NULL)
        return;

    if (uri_ref->element_id != NULL)
        free (uri_ref->element_id);
    if (uri_ref->uri != NULL)
        _svg_destroy_uri (uri_ref->uri);

    free (uri_ref);
}

int
_svg_is_uri_ref_url (const char *attribute)
{
    return strncmp (attribute, "url(", 4) == 0;
}

void
_svg_uri_ref_print (svg_uri_ref_t *uri_ref)
{
    if (uri_ref->uri != NULL)
        _svg_uri_print (uri_ref->uri);
    if (uri_ref->element_id != NULL)
        printf ("#%s", uri_ref->element_id);
}

