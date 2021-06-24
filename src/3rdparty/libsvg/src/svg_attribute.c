/* svg_attribute.c: SVG attribute helper functions

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


static svg_status_t
_svg_attribute_find (const svg_qattrs_t *attributes, const char *name,
                     const svg_qattr_t **attribute)
{
    int i;

    if (attributes == NULL)
        return SVG_STATUS_ATTRIBUTE_NOT_FOUND;

    for (i = 0; i < attributes->num; i++) {
        if (_svg_compare_qname_svg (&attributes->atts[i].name, name) == 0) {
            *attribute = &attributes->atts[i];
            return SVG_STATUS_SUCCESS;
        }
    }

    return SVG_STATUS_ATTRIBUTE_NOT_FOUND;
}

static svg_status_t
_svg_attribute_find_ns (const svg_qattrs_t *attributes, int ns_index, const char *name,
                        const svg_qattr_t **attribute)
{
    int i;

    if (attributes == NULL)
        return SVG_STATUS_ATTRIBUTE_NOT_FOUND;

    for (i = 0; i < attributes->num; i++) {
        if (_svg_compare_qname_2 (&attributes->atts[i].name, ns_index, name) == 0) {
            *attribute = &attributes->atts[i];
            return SVG_STATUS_SUCCESS;
        }
    }

    return SVG_STATUS_ATTRIBUTE_NOT_FOUND;
}

/* section "3.3.3 Attribute-Value Normalization" of the XML 1.0 specification says
the XML processor must normalize the attribute. Expat doesn't do this so we do basic
normalization here, i.e. trim leading and trailing whitespace. Note that the value
is modified here if there is trailing whitespace */
static char *
_svg_trim_whitespace (char *value)
{
    char *start = value;
    char *end, *beyond;

    while (_svg_ascii_isspace (*start))
        start++;

    end = start;
    beyond = start;
    while (*beyond != '\0') {
        if (!_svg_ascii_isspace (*beyond))
            end = beyond;
        beyond++;
    }
    if (beyond != end)
        *(end + 1) = '\0';

    return start;
}




svg_status_t
_svg_attribute_get_double (const svg_qattrs_t *attributes, const char *name, double *value,
                           double default_value)
{
    svg_status_t status;
    const svg_qattr_t *attribute;
    char *trimmed_value;
    const char *end;

    *value = default_value;

    status = _svg_attribute_find (attributes, name, &attribute);
    if (status)
        return status;

    trimmed_value = _svg_trim_whitespace (attribute->value);
    *value = _svg_ascii_strtod (trimmed_value, &end);
    if (end == trimmed_value)
        return SVG_STATUS_PARSE_ERROR;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_attribute_get_string (const svg_qattrs_t *attributes, const char *name, const char **value,
                           const char *default_value)
{
    svg_status_t status;
    const svg_qattr_t *attribute;

    *value = default_value;

    status = _svg_attribute_find (attributes, name, &attribute);
    if (status)
        return status;

    *value = _svg_trim_whitespace (attribute->value);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_attribute_get_string_ns (const svg_qattrs_t *attributes, int ns_index, const char *name,
                              const char **value, const char *default_value)
{
    svg_status_t status;
    const svg_qattr_t *attribute;

    *value = default_value;

    status = _svg_attribute_find_ns (attributes, ns_index, name, &attribute);
    if (status)
        return status;

    *value = _svg_trim_whitespace (attribute->value);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_attribute_get_length (const svg_qattrs_t *attributes, const char *name, svg_length_t *value,
                           const char *default_value)
{
    svg_status_t status;
    const svg_qattr_t *attribute;

    status = _svg_length_init_from_str (value, default_value);
    if (status)
        return status;

    status = _svg_attribute_find (attributes, name, &attribute);
    if (status)
        return status;

    status = _svg_length_init_from_str (value, _svg_trim_whitespace (attribute->value));
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

