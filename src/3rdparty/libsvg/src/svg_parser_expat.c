/* svg_parser_expat.c: Expat SAX-based parser for SVG documents

   Copyright (C) 2000 Eazel, Inc.
   Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
   Copyright (C) 2002 USC/Information Sciences Institute
   Copyright (C) 2005 Phil Blundell <pb@handhelds.org>
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
#include "svg_hash.h"

#include <limits.h>


static const char g_ns_separator = ' ';


static svg_status_t
_svg_parser_convert_expat_error (enum XML_Error error)
{
    switch (error) {
    case XML_ERROR_NONE:
        return SVG_STATUS_SUCCESS;
    case XML_ERROR_NO_MEMORY:
        return SVG_STATUS_NO_MEMORY;
    case XML_ERROR_NO_ELEMENTS:
        return SVG_STATUS_EMPTY_DOCUMENT;
    case XML_ERROR_TAG_MISMATCH:
        return SVG_STATUS_XML_TAG_MISMATCH;
    case XML_ERROR_UNBOUND_PREFIX:
        return SVG_STATUS_XML_UNBOUND_PREFIX;
    case XML_ERROR_INVALID_TOKEN:
        return SVG_STATUS_XML_INVALID_TOKEN;
    default:
        return SVG_STATUS_XML_PARSE_ERROR;
    }
}




svg_status_t
_svg_parser_init (svg_parser_t *parser, svg_t *svg)
{
    memset (parser, 0, sizeof (svg_parser_t));

    parser->svg = svg;

    parser->status = _svg_dom_init (&parser->dom);
    if (parser->status)
        return parser->status;

    parser->status = _svg_create_css_stylesheet (&parser->stylesheet);
    if (parser->status) {
        _svg_dom_deinit (&parser->dom);
        return parser->status;
    }

    parser->status = SVG_STATUS_SUCCESS;

    return parser->status;
}

svg_status_t
_svg_parser_deinit (svg_parser_t *parser)
{
    svg_parser_state_t *old_state;
    svg_parser_deferred_element_t *old_element;

    _svg_dom_deinit (&parser->dom);

    _svg_destroy_css_stylesheet (parser->stylesheet);

    while (parser->deferred_elements != NULL) {
        old_element = parser->deferred_elements;
        parser->deferred_elements = parser->deferred_elements->next;
        free (old_element);
    }

    while (parser->state != NULL) {
        old_state = parser->state;
        parser->state = parser->state->next;
        free (old_state);
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_parser_begin_dom_parsing (svg_parser_t *parser)
{
    if (parser->ctxt != NULL) {
        parser->status = SVG_STATUS_INVALID_CALL;
        return parser->status;
    }

    parser->status = SVG_STATUS_SUCCESS;

    parser->ctxt = XML_ParserCreateNS (NULL, g_ns_separator);
    if (parser->ctxt == NULL) {
        parser->status = SVG_STATUS_NO_MEMORY;
        return parser->status;
    }

    XML_SetUserData (parser->ctxt, parser);

    XML_SetStartElementHandler (parser->ctxt, _svg_parser_sax_start_element);
    XML_SetEndElementHandler (parser->ctxt, _svg_parser_sax_end_element);
    XML_SetCharacterDataHandler (parser->ctxt, _svg_parser_sax_characters);
    XML_SetProcessingInstructionHandler (parser->ctxt, _svg_parser_sax_proc_instr);

    return parser->status;
}

svg_status_t
_svg_parser_parse_chunk (svg_parser_t *parser, const char *buf, size_t count)
{
    size_t total_count;
    int expat_count;

    if (parser->status)
        return parser->status;

    if (parser->ctxt == NULL)
        return SVG_STATUS_INVALID_CALL;

    /* parse chunk whilst working around possible max value difference between size_t and int */
    total_count = 0;
    while (total_count < count) {
        if (count - total_count > (size_t) INT_MAX)
            expat_count = INT_MAX;
        else
            expat_count = (int) (count - total_count);

        if (XML_Parse (parser->ctxt, &buf[total_count], expat_count, 0) != XML_STATUS_OK) {
            if (parser->status == SVG_STATUS_SUCCESS) {
                parser->status = _svg_parser_convert_expat_error (XML_GetErrorCode (parser->ctxt));
                _svg_parser_set_error (parser, _svg_parser_get_line_number (parser),
                                       parser->status);
            }
            break;
        }

        total_count += expat_count;
    }

    return parser->status;
}

svg_status_t
_svg_parser_end_dom_parsing (svg_parser_t *parser)
{
    if (parser->ctxt == NULL)
        return SVG_STATUS_INVALID_CALL;

    if (XML_Parse (parser->ctxt, NULL, 0, 1) != XML_STATUS_OK) {
        if (parser->status == SVG_STATUS_SUCCESS) {
            parser->status = _svg_parser_convert_expat_error (XML_GetErrorCode (parser->ctxt));
            _svg_parser_set_error (parser, _svg_parser_get_line_number (parser), parser->status);
        }
    }

    XML_ParserFree (parser->ctxt);

    parser->ctxt = NULL;

    return parser->status;
}

svg_status_t
_svg_parser_stop (svg_parser_t *parser)
{
    if (XML_StopParser (parser->ctxt, 0) != XML_STATUS_OK)
        return SVG_STATUS_XML_PARSE_ERROR;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_parser_copy_xml_name (svg_dom_t *dom, const xmlChar *xml_name, svg_qname_t **qname)
{
    char *nonconst_xml_name = (char *) xml_name;
    char *sep;
    const char *local_name;
    const char *uri;
    int ns_index = 0;
    svg_status_t status;

    sep = strchr (nonconst_xml_name, g_ns_separator);
    if (sep == NULL) {
        ns_index = NO_NAMESPACE_INDEX;
        local_name = nonconst_xml_name;
    } else {
        uri = nonconst_xml_name;
        *sep = '\0';
        status = _svg_dom_register_uri (dom, uri, &ns_index);
        *sep = g_ns_separator;
        if (status)
            return status;

        local_name = sep + 1;
    }

    status = _svg_dom_create_qname (ns_index, local_name, qname);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_parser_copy_xml_attributes (svg_dom_t *dom, int element_ns_index, const xmlChar **xml_atts,
                                 svg_qattrs_t **qattrs)
{
    char **nonconst_xml_atts = (char **) xml_atts;
    const char *local_name;
    const char *uri;
    const char *value;
    char *sep;
    int ns_index = 0;
    int i;
    svg_qattrs_t *new_qattrs = NULL;
    int num_qattrs;
    svg_status_t status;

    num_qattrs = 0;
    while (xml_atts[num_qattrs] != NULL)
        num_qattrs++;
    num_qattrs = (num_qattrs + 1) / 2;

    if (num_qattrs == 0) {
        *qattrs = NULL;
        return SVG_STATUS_SUCCESS;
    }

    status = _svg_dom_create_qattrs (num_qattrs, &new_qattrs);
    if (status)
        return status;

    for (i = 0; i < new_qattrs->num; i++) {
        sep = strchr (nonconst_xml_atts[2 * i], g_ns_separator);
        if (sep != NULL) {
            uri = nonconst_xml_atts[2 * i];

            *sep = '\0';
            status = _svg_dom_register_uri (dom, uri, &ns_index);
            *sep = g_ns_separator;
            if (status) {
                _svg_dom_destroy_qattrs (new_qattrs);
                return status;
            }

            local_name = sep + 1;
        } else {
            ns_index = element_ns_index;
            local_name = nonconst_xml_atts[2 * i];
        }
        value = nonconst_xml_atts[2 * i + 1];

        status = _svg_dom_init_qattr (&new_qattrs->atts[i], ns_index, local_name, value);
        if (status) {
            _svg_dom_destroy_qattrs (new_qattrs);
            return status;
        }
    }

    *qattrs = new_qattrs;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_parser_parse_pseudo_proc_instr (svg_dom_t *dom, const char *buf, size_t count)
{
    XML_Parser ctxt;
    enum XML_Status status;

    ctxt = XML_ParserCreate (NULL);
    if (ctxt == NULL)
        return SVG_STATUS_NO_MEMORY;

    XML_SetUserData (ctxt, dom);

    XML_SetStartElementHandler (ctxt, _svg_parser_pseudo_proc_instr_start_element);

    status = XML_Parse (ctxt, buf, (int) count, 1);

    XML_ParserFree (ctxt);

    if (status == XML_STATUS_ERROR)
        return SVG_STATUS_PARSE_ERROR;

    return SVG_STATUS_SUCCESS;
}

long
_svg_parser_get_line_number (svg_parser_t *parser)
{
    return (long) XML_GetCurrentLineNumber (parser->ctxt);
}

