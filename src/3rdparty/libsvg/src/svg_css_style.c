/* svg_css_style.c: Parsing and application of CSS stylesheets

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

/* the sac handler used for parsing <style> and external stylesheets was derived
from the Inkscape (http://www.inkscape.org/) implementation in inkscape/src/sp-style-elem.cpp */

#include "svgint.h"


void
_svg_destroy_css_declarations (svg_css_declaration_t *declarations, int num_declarations)
{
    int i;

    if (declarations == NULL)
        return;

    for (i = 0; i < num_declarations; i++) {
        if (declarations[i].name != NULL)
            free (declarations[i].name);
        if (declarations[i].value != NULL)
            free (declarations[i].value);
    }

    free (declarations);
}




#if !HAVE_LIBCROCO


struct svg_css_stylesheet {
    int nothing;
};


static int
_svg_is_css_whitespace (char c)
{
    return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\f';
}

static void
_svg_skip_comment (const char **str)
{
    (*str) += 2;

    while ((**str) != '\0' && *((*str) + 1) != '\0') {
        if ((**str) == '*' && *((*str) + 1) == '/') {
            (*str) += 2;
            return;
        }
        (*str)++;
    }
    /* parse error - end comment not found */
    if ((**str) != '\0')
        (*str)++;
}

static void
_svg_skip_comment_r (const char *start, const char **str)
{
    (*str) -= 2;

    while ((*str) != start && ((*str) - 1) != start) {
        if ((**str) == '*' && *((*str) - 1) == '/') {
            (*str) -= 1;        /* end string is last char + 1 */
            return;
        }
        (*str)--;
    }
    /* parse error - start comment not found */
    if ((*str) != start)
        (*str) = start;
}

static int
_svg_skip_whitespace_and_comments (const char **str)
{
    int have_consumed = 0;

    while ((**str) != '\0') {
        if (_svg_is_css_whitespace (**str)) {
            (*str)++;
            have_consumed = 1;
        } else if ((**str) == '/' && *((*str) + 1) == '*') {
            _svg_skip_comment (str);
            have_consumed = 1;
        } else {
            break;
        }
    }

    return have_consumed;
}

static int
_svg_skip_whitespace_and_comments_r (const char *start, const char **str)
{
    int have_consumed = 0;

    while (((*str) - 1) != start) {
        if (_svg_is_css_whitespace (*((*str) - 1))) {
            (*str)--;
            have_consumed = 1;
        } else if (((*str) - 2) != start && *((*str) - 1) == '/' && *((*str) - 2) == '*') {
            _svg_skip_comment_r (start, str);
            have_consumed = 1;
        } else {
            break;
        }
    }

    return have_consumed;
}

static svg_status_t
_svg_trim_str (const char *start_str, const char *end_str, const char **new_start_ref,
               const char **new_end_ref)
{
    const char *new_start = *new_start_ref;
    const char *new_end = *new_end_ref;

    new_start = start_str;
    new_end = end_str;

    while (new_start != new_end) {
        if (!_svg_skip_whitespace_and_comments (&new_start))
            break;
    }

    while (new_end != new_start) {
        if (!_svg_skip_whitespace_and_comments_r (new_start, &new_end))
            break;
    }

    if (new_start == new_end)
        return SVG_STATUS_CSS_PARSE_ERROR;


    *new_start_ref = new_start;
    *new_end_ref = new_end;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parse_declaration_str (char **str, const char *start_str, const char *end_str)
{
    svg_status_t status;
    const char *start, *end;

    status = _svg_trim_str (start_str, end_str, &start, &end);
    if (status)
        return status;

    if (start == end)
        return SVG_STATUS_CSS_PARSE_ERROR;

    *str = malloc (end - start + 1);
    if (*str == NULL)
        return SVG_STATUS_NO_MEMORY;

    memcpy (*str, start, end - start);
    (*str)[end - start] = '\0';

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parse_declaration (svg_css_declaration_t *declaration, const char *start_decl,
                        const char *end_decl)
{
    static const char *important = "important";
    svg_status_t status;
    const char *start, *end, *priority_start;
    int index;

    /* name before the ':' */

    start = start_decl;
    end = start;
    while (end != end_decl && *end != ':') {
        if (_svg_skip_whitespace_and_comments (&end))
            continue;
        end++;
    }
    if (end == end_decl)
        return SVG_STATUS_CSS_PARSE_ERROR;

    status = _svg_parse_declaration_str (&declaration->name, start, end);
    if (status)
        return status;


    /* value and priority (!important) after the ':' */

    end++;
    if (end == end_decl)
        return SVG_STATUS_CSS_PARSE_ERROR;

    start = end;
    end = end_decl;
    status = _svg_trim_str (start, end, &start, &end);
    if (status)
        return status;

    priority_start = end - 1;
    index = strlen (important) - 1;
    while (index >= 0 && priority_start != start && *priority_start == important[index]) {
        index--;
        priority_start--;
    }
    if (index < 0) {
        while (priority_start != start && _svg_is_css_whitespace (*priority_start))
            priority_start--;
        if (*priority_start == '!') {
            declaration->important = 1;
            end = priority_start;
        }
    }

    status = _svg_parse_declaration_str (&declaration->value, start, end);
    if (status)
        return status;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_create_css_stylesheet (svg_css_stylesheet_t **stylesheet)
{
    svg_css_stylesheet_t *new_stylesheet;

    new_stylesheet = (svg_css_stylesheet_t *) malloc (sizeof (svg_css_stylesheet_t));
    if (new_stylesheet == NULL)
        return SVG_STATUS_NO_MEMORY;

    *stylesheet = new_stylesheet;

    return SVG_STATUS_SUCCESS;
}

void
_svg_destroy_css_stylesheet (svg_css_stylesheet_t *stylesheet)
{
    if (stylesheet == NULL)
        return;

    free (stylesheet);
}

svg_status_t
_svg_parse_css_buffer (svg_css_stylesheet_t *stylesheet,
                       svg_uri_t *base_uri, const char *buffer, size_t count)
{
    (void) stylesheet;
    (void) base_uri;
    (void) buffer;
    (void) count;

    /* not yet implemented */

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_parse_external_css_stylesheet (svg_css_stylesheet_t *stylesheet, svg_uri_t *uri)
{
    (void) stylesheet;
    (void) uri;

    /* not yet implemented */

    return SVG_STATUS_SUCCESS;
}

/* XXX: lex and yacc type parsing and unicode (utf-8) support would improve this
functions precision and completeness */
svg_status_t
_svg_parse_inline_css (const char *data, size_t count,
                       svg_css_declaration_t **declarations_ref, int *num_declarations_ref)
{
    svg_css_declaration_t *declarations = NULL;
    int num_declarations = 0;
    svg_status_t status;
    const char *start, *end;
    int index;
    int have_non_whitespace;

    /* count # css declarations, each separated by a ';' */
    start = data;
    while (*start != '\0') {
        end = start;
        have_non_whitespace = 0;
        while (*end != '\0' && *end != ';') {
            if (_svg_skip_whitespace_and_comments (&end))
                continue;
            have_non_whitespace = 1;
            end++;
        }

        if (have_non_whitespace)
            num_declarations++;

        if (*end == ';')
            end++;
        start = end;
    }

    if (num_declarations == 0) {
        *declarations_ref = NULL;
        *num_declarations_ref = 0;

        return SVG_STATUS_SUCCESS;
    }

    declarations =
        (svg_css_declaration_t *) malloc (num_declarations * sizeof (svg_css_declaration_t));
    if (declarations == NULL)
        return SVG_STATUS_NO_MEMORY;
    memset (declarations, 0, num_declarations * sizeof (svg_css_declaration_t));

    /* parse css declarations, each separated by a ';' */
    index = 0;
    start = data;
    while (*start != '\0') {
        end = start;
        have_non_whitespace = 0;
        while (*end != '\0' && *end != ';') {
            if (_svg_skip_whitespace_and_comments (&end))
                continue;
            have_non_whitespace = 1;
            end++;
        }

        if (have_non_whitespace) {
            status = _svg_parse_declaration (&declarations[index], start, end);
            if (status)
                goto fail;
            index++;
        }

        if (*end == ';')
            end++;
        start = end;
    }

    *declarations_ref = declarations;
    *num_declarations_ref = num_declarations;

    return SVG_STATUS_SUCCESS;

  fail:
    if (declarations != NULL)
        _svg_destroy_css_declarations (declarations, num_declarations);

    return status;
}

svg_status_t
_svg_get_css_stylesheet_declarations (svg_dom_node_t *node, svg_css_stylesheet_t *stylesheet,
                                      svg_css_declaration_t **declarations, int *num_declarations)
{
    (void) node;
    (void) stylesheet;

    /* not yet implemented */

    *declarations = NULL;
    *num_declarations = 0;

    return SVG_STATUS_SUCCESS;
}


#else


#define MAX_CSS_RESOURCE_SIZE           (50 * 1024 * 1024)


#if defined(_MSC_VER)
#include <libcroco.h>
#else
#include <libcroco/libcroco.h>
#endif


struct svg_css_stylesheet {
    CRStyleSheet *cr_stylesheet;
};

typedef enum {
    NO_STMT,
    FONT_FACE_STMT,
    NORMAL_RULESET_STMT
} svg_cr_stmt_t;

typedef struct svg_css_parser {
    svg_css_stylesheet_t *stylesheet;
    svg_uri_t *base_uri;
    svg_cr_stmt_t stmt_type;
    CRStatement *curr_stmt;
} svg_css_parser_t;

typedef struct svg_inline_css_parser {
    CRDeclaration *decl_list;
} svg_inline_css_parser_t;



static svg_status_t
_svg_create_css_parser (svg_css_parser_t **parser, svg_css_stylesheet_t *stylesheet,
                        svg_uri_t *base_uri)
{
    svg_css_parser_t *new_parser;

    new_parser = malloc (sizeof (svg_css_parser_t));
    if (new_parser == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_parser, 0, sizeof (svg_css_parser_t));

    new_parser->stylesheet = stylesheet;
    new_parser->base_uri = base_uri;

    *parser = new_parser;

    return SVG_STATUS_SUCCESS;
}

static void
_svg_destroy_css_parser (svg_css_parser_t *parser)
{
    if (parser == NULL)
        return;

    if (parser->curr_stmt != NULL)
        cr_statement_destroy (parser->curr_stmt);

    free (parser);
}

static void
_svg_cr_start_selector (CRDocHandler *a_handler, CRSelector *a_sel_list)
{
    svg_css_parser_t *parser = (svg_css_parser_t *) a_handler->app_data;
    CRStatement *ruleset;

    ruleset = cr_statement_new_ruleset (parser->stylesheet->cr_stylesheet, a_sel_list, NULL, NULL);
    if (ruleset == NULL)
        return;

    if (parser->curr_stmt != NULL)
        cr_statement_destroy (parser->curr_stmt);

    parser->stmt_type = NORMAL_RULESET_STMT;
    parser->curr_stmt = ruleset;
}

static void
_svg_cr_end_selector (CRDocHandler *a_handler, CRSelector *a_sel_list)
{
    svg_css_parser_t *parser = (svg_css_parser_t *) a_handler->app_data;
    CRStatement *ruleset = parser->curr_stmt;

    if (parser->stmt_type == NORMAL_RULESET_STMT &&
        ruleset && ruleset->type == RULESET_STMT && ruleset->kind.ruleset->sel_list == a_sel_list)
    {
        parser->stylesheet->cr_stylesheet->statements =
            cr_statement_append (parser->stylesheet->cr_stylesheet->statements, ruleset);
    }

    parser->stmt_type = NO_STMT;
    parser->curr_stmt = NULL;
}

static void
_svg_cr_start_font_face (CRDocHandler *a_handler, CRParsingLocation *a_location)
{
    svg_css_parser_t *parser = (svg_css_parser_t *) a_handler->app_data;

    parser->stmt_type = FONT_FACE_STMT;
    parser->curr_stmt = NULL;
}

static void
_svg_cr_end_font_face (CRDocHandler *a_handler)
{
    svg_css_parser_t *parser = (svg_css_parser_t *) a_handler->app_data;

    parser->stmt_type = NO_STMT;
    parser->curr_stmt = NULL;
}

static void
_svg_cr_import_style (CRDocHandler *a_handler, GList *a_media_list, CRString *a_uri,
                      CRString *a_uri_default_ns, CRParsingLocation *a_location)
{
    svg_css_parser_t *parser = (svg_css_parser_t *) a_handler->app_data;
    svg_status_t status;
    svg_uri_t *uri = NULL;
    svg_uri_t *abs_uri = NULL;

    status = _svg_create_uri (cr_string_peek_raw_str (a_uri), &uri);
    if (status)
        return;

    if (_svg_uri_is_relative (uri)) {
        status = _svg_uri_create_absolute (parser->base_uri, uri, &abs_uri);
        if (status)
            goto end;
    } else {
        abs_uri = uri;
        uri = NULL;
    }

    status = _svg_parse_external_css_stylesheet (parser->stylesheet, abs_uri);
    if (status)
        goto end;

  end:
    if (uri != NULL)
        _svg_destroy_uri (uri);
    if (abs_uri != NULL)
        _svg_destroy_uri (abs_uri);
}

static void
_svg_cr_property (CRDocHandler *a_handler, CRString *a_name, CRTerm *a_value, gboolean a_important)
{
    svg_css_parser_t *parser = (svg_css_parser_t *) a_handler->app_data;
    CRStatement *ruleset = parser->curr_stmt;
    CRDeclaration *decl;

    if (parser->stmt_type == FONT_FACE_STMT) {
        /* we currently ignore @font-face descriptors. */
        return;
    }

    decl = cr_declaration_new (ruleset, cr_string_dup (a_name), a_value);
    if (decl == NULL)
        return;

    decl->important = a_important;
    cr_statement_ruleset_append_decl (ruleset, decl);
}

static svg_status_t
_svg_parse_css (CRParser *cr_parser, svg_css_stylesheet_t *stylesheet, svg_uri_t *base_uri)
{
    svg_css_parser_t *parser = NULL;
    svg_status_t status;
    CRDocHandler *sac_handler = NULL;
    enum CRStatus parse_status;

    status = _svg_create_css_parser (&parser, stylesheet, base_uri);
    if (status)
        return SVG_STATUS_NO_MEMORY;

    sac_handler = cr_doc_handler_new ();
    if (sac_handler == NULL) {
        _svg_destroy_css_parser (parser);
        return SVG_STATUS_NO_MEMORY;
    }

    /* XXX: not yet supporting @font-face, @charset, @media and more ... */

    sac_handler->app_data = parser;
    sac_handler->start_selector = _svg_cr_start_selector;
    sac_handler->end_selector = _svg_cr_end_selector;
    sac_handler->start_font_face = _svg_cr_start_font_face;
    sac_handler->end_font_face = _svg_cr_end_font_face;
    sac_handler->import_style = _svg_cr_import_style;
    sac_handler->property = _svg_cr_property;

    cr_parser_set_sac_handler (cr_parser, sac_handler);

    parse_status = cr_parser_parse (cr_parser);
    if (parse_status != CR_OK)
        status = SVG_STATUS_CSS_PARSE_ERROR;
    else
        status = SVG_STATUS_SUCCESS;

    _svg_destroy_css_parser (parser);

    return status;
}

static svg_status_t
_svg_create_inline_css_parser (svg_inline_css_parser_t **parser)
{
    svg_inline_css_parser_t *new_parser;

    new_parser = malloc (sizeof (svg_inline_css_parser_t));
    if (new_parser == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_parser, 0, sizeof (svg_inline_css_parser_t));

    *parser = new_parser;

    return SVG_STATUS_SUCCESS;
}

static void
_svg_destroy_inline_css_parser (svg_inline_css_parser_t *parser)
{
    if (parser == NULL)
        return;

    if (parser->decl_list != NULL)
        cr_declaration_destroy (parser->decl_list);

    free (parser);
}

static void
_svg_cr_inline_property (CRDocHandler *a_handler, CRString *a_name, CRTerm *a_value,
                         gboolean a_important)
{
    svg_inline_css_parser_t *parser = (svg_inline_css_parser_t *) a_handler->app_data;
    CRDeclaration *decl;

    if (parser->decl_list == NULL) {
        decl = cr_declaration_new (NULL, cr_string_dup (a_name), a_value);
        if (decl == NULL)
            return;
        parser->decl_list = decl;
    } else {
        decl = cr_declaration_append2 (parser->decl_list, cr_string_dup (a_name), a_value);
        if (decl == NULL)
            return;
    }

    decl->important = a_important;
}

static CRXMLNodePtr
_svg_node_iface_get_parent (CRXMLNodePtr ptr)
{
    svg_dom_node_t *parent;

    parent = _svg_dom_get_parent ((svg_dom_node_t *) ptr);
    while (parent != NULL && parent->is_deep_clone)
        parent = _svg_dom_get_parent (parent);

    return (CRXMLNodePtr) parent;
}

static CRXMLNodePtr
_svg_node_iface_first_child (CRXMLNodePtr ptr)
{
    svg_dom_node_t *first_child;

    first_child = _svg_dom_get_first_child ((svg_dom_node_t *) ptr);
    while (first_child != NULL && first_child->is_deep_clone)
        first_child = _svg_dom_get_next_sibling (first_child);

    return (CRXMLNodePtr) first_child;
}

static CRXMLNodePtr
_svg_node_iface_get_next_sibling (CRXMLNodePtr ptr)
{
    svg_dom_node_t *next_sibling;

    next_sibling = _svg_dom_get_next_sibling ((svg_dom_node_t *) ptr);
    while (next_sibling != NULL && next_sibling->is_deep_clone)
        next_sibling = _svg_dom_get_next_sibling (next_sibling);

    return (CRXMLNodePtr) next_sibling;
}

static CRXMLNodePtr
_svg_node_iface_get_prev_sibling (CRXMLNodePtr ptr)
{
    svg_dom_node_t *prev_sibling;

    prev_sibling = _svg_dom_get_prev_sibling ((svg_dom_node_t *) ptr);
    while (prev_sibling != NULL && prev_sibling->is_deep_clone)
        prev_sibling = _svg_dom_get_prev_sibling (prev_sibling);

    return (CRXMLNodePtr) prev_sibling;
}

static int
_svg_node_iface_is_element_node (CRXMLNodePtr ptr)
{
    return _svg_dom_is_element_node ((svg_dom_node_t *) ptr);
}

static const char *
_svg_node_iface_get_node_name (CRXMLNodePtr ptr)
{
    return _svg_dom_get_node_local_name ((svg_dom_node_t *) ptr);
}

static char *
_svg_node_iface_copy_attr_value (CRXMLNodePtr ptr, const char *prop_name)
{
    const char *value;

    value = _svg_dom_get_local_attr_value ((svg_dom_node_t *) ptr, prop_name);
    if (value == NULL)
        return NULL;

    return strdup (value);
}

static void
_svg_node_iface_free_attr_value (void *val)
{
    free (val);
}



static CRNodeIface const SVG_CR_NODE_IFACE = {
    _svg_node_iface_get_parent,
    _svg_node_iface_first_child,
    _svg_node_iface_get_next_sibling,
    _svg_node_iface_get_prev_sibling,
    _svg_node_iface_get_node_name,
    _svg_node_iface_copy_attr_value,
    _svg_node_iface_free_attr_value,
    _svg_node_iface_is_element_node
};




svg_status_t
_svg_create_css_stylesheet (svg_css_stylesheet_t **stylesheet)
{
    svg_css_stylesheet_t *new_stylesheet;

    new_stylesheet = malloc (sizeof (svg_css_stylesheet_t));
    if (new_stylesheet == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_stylesheet, 0, sizeof (svg_css_stylesheet_t));

    new_stylesheet->cr_stylesheet = cr_stylesheet_new (NULL);
    if (new_stylesheet->cr_stylesheet == NULL) {
        free (new_stylesheet);
        return SVG_STATUS_NO_MEMORY;
    }

    *stylesheet = new_stylesheet;

    return SVG_STATUS_SUCCESS;
}

void
_svg_destroy_css_stylesheet (svg_css_stylesheet_t *stylesheet)
{
    if (stylesheet == NULL)
        return;

    if (stylesheet->cr_stylesheet != NULL)
        cr_stylesheet_destroy (stylesheet->cr_stylesheet);

    free (stylesheet);
}

svg_status_t
_svg_parse_css_buffer (svg_css_stylesheet_t *stylesheet, svg_uri_t *base_uri, const char *buffer,
                       size_t count)
{
    svg_status_t status;
    CRParser *cr_parser = NULL;

    cr_parser = cr_parser_new_from_buf ((guchar *) buffer, (gulong) count, CR_UTF_8, FALSE);
    if (cr_parser == NULL)
        return SVG_STATUS_NO_MEMORY;

    status = _svg_parse_css (cr_parser, stylesheet, base_uri);

    cr_parser_destroy (cr_parser);

    return status;
}

svg_status_t
_svg_parse_external_css_stylesheet (svg_css_stylesheet_t *stylesheet, svg_uri_t *uri)
{
    char *buffer = NULL;
    size_t buffer_size;
    svg_status_t status;

    status =
        _svg_resource_read_stream_to_buffer (uri, &buffer, &buffer_size, MAX_CSS_RESOURCE_SIZE);
    if (status)
        return status;

    status = _svg_parse_css_buffer (stylesheet, uri, buffer, buffer_size);

    if (buffer != NULL)
        free (buffer);

    return status;
}

svg_status_t
_svg_parse_inline_css (const char *data, size_t count,
                       svg_css_declaration_t **declarations_ref, int *num_declarations_ref)
{
    svg_css_declaration_t *declarations = NULL;
    int num_declarations = 0;
    svg_status_t status;
    svg_inline_css_parser_t *parser = NULL;
    CRParser *cr_parser = NULL;
    CRDocHandler *sac_handler;
    enum CRStatus parse_status;
    CRDeclaration *cr_decl;
    int i;

    status = _svg_create_inline_css_parser (&parser);
    if (status)
        return SVG_STATUS_NO_MEMORY;

    cr_parser = cr_parser_new_from_buf ((guchar *) data, (gulong) count, CR_UTF_8, FALSE);
    if (cr_parser == NULL) {
        status = SVG_STATUS_NO_MEMORY;
        goto fail;
    }

    sac_handler = cr_doc_handler_new ();
    if (sac_handler == NULL) {
        status = SVG_STATUS_NO_MEMORY;
        goto fail;
    }

    sac_handler->app_data = parser;
    sac_handler->property = _svg_cr_inline_property;

    cr_parser_set_sac_handler (cr_parser, sac_handler);

    parse_status = cr_parser_parse_inline_declaration_block (cr_parser);
    if (parse_status != CR_OK) {
        status = SVG_STATUS_CSS_PARSE_ERROR;
        goto fail;
    }

    if (parser->decl_list != NULL) {
        num_declarations = cr_declaration_nr_props (parser->decl_list);

        declarations =
            (svg_css_declaration_t *) malloc (num_declarations * sizeof (svg_css_declaration_t));
        if (declarations == NULL) {
            status = SVG_STATUS_NO_MEMORY;
            goto fail;
        }
        memset (declarations, 0, num_declarations * sizeof (svg_css_declaration_t));

        for (i = 0; i < num_declarations; i++) {
            cr_decl = cr_declaration_get_from_list (parser->decl_list, i);

            declarations[i].name =
                strdup ((const char *) cr_string_peek_raw_str (cr_decl->property));
            if (declarations[i].name == NULL) {
                status = SVG_STATUS_NO_MEMORY;
                goto fail;
            }
            declarations[i].value = (char *) cr_term_to_string (cr_decl->value);
            if (declarations[i].value == NULL) {
                status = SVG_STATUS_NO_MEMORY;
                goto fail;
            }
        }
    }

    cr_parser_destroy (cr_parser);
    _svg_destroy_inline_css_parser (parser);


    *declarations_ref = declarations;
    *num_declarations_ref = num_declarations;

    return SVG_STATUS_SUCCESS;

  fail:
    if (cr_parser != NULL)
        cr_parser_destroy (cr_parser);
    if (parser != NULL)
        _svg_destroy_inline_css_parser (parser);
    if (declarations != NULL)
        _svg_destroy_css_declarations (declarations, num_declarations);

    return status;
}

svg_status_t
_svg_get_css_stylesheet_declarations (svg_dom_node_t *node, svg_css_stylesheet_t *stylesheet,
                                      svg_css_declaration_t **declarations_ref,
                                      int *num_declarations_ref)
{
    svg_css_declaration_t *declarations = NULL;
    int num_declarations = 0;
    CRCascade *cascade = NULL;
    CRSelEng *selector = NULL;
    CRPropList *prop_list = NULL;
    CRPropList *cur_pair = NULL;
    CRDeclaration *cr_decl;
    int index;
    svg_status_t status;
    enum CRStatus cr_status;

    if (stylesheet->cr_stylesheet == NULL)
        return SVG_STATUS_SUCCESS;

    cascade = cr_cascade_new (stylesheet->cr_stylesheet, 0, 0);
    if (cascade == NULL)
        return SVG_STATUS_NO_MEMORY;
    cr_stylesheet_ref (stylesheet->cr_stylesheet);

    selector = cr_sel_eng_new ();
    if (selector == NULL) {
        status = SVG_STATUS_NO_MEMORY;
        goto fail;
    }

    cr_sel_eng_set_node_iface (selector, &SVG_CR_NODE_IFACE);

    cr_status = cr_sel_eng_get_matched_properties_from_cascade (selector, cascade,
                                                                node, &prop_list);
    if (cr_status != CR_OK) {
        status = SVG_STATUS_CSS_PARSE_ERROR;
        goto fail;
    }

    if (prop_list != NULL) {
        num_declarations = cr_prop_list_nr_props (prop_list);

        declarations =
            (svg_css_declaration_t *) malloc (num_declarations * sizeof (svg_css_declaration_t));
        if (declarations == NULL) {
            status = SVG_STATUS_NO_MEMORY;
            goto fail;
        }
        memset (declarations, 0, num_declarations * sizeof (svg_css_declaration_t));

        cur_pair = prop_list;
        index = 0;
        while (cur_pair != NULL) {
            cr_decl = NULL;
            cr_prop_list_get_decl (cur_pair, &cr_decl);
            if (cr_decl != NULL) {
                declarations[index].name =
                    strdup ((const char *) cr_string_peek_raw_str (cr_decl->property));
                if (declarations[index].name == NULL) {
                    status = SVG_STATUS_NO_MEMORY;
                    goto fail;
                }
                declarations[index].value = (char *) cr_term_to_string (cr_decl->value);
                if (declarations[index].value == NULL) {
                    status = SVG_STATUS_NO_MEMORY;
                    goto fail;
                }
                declarations[index].important = cr_decl->important;
            }

            cur_pair = cr_prop_list_get_next (cur_pair);
            index++;
        }

        cr_prop_list_destroy (prop_list);
    }


    cr_sel_eng_destroy (selector);
    cr_cascade_destroy (cascade);

    *declarations_ref = declarations;
    *num_declarations_ref = num_declarations;

    return SVG_STATUS_SUCCESS;

  fail:
    if (prop_list != NULL)
        cr_prop_list_destroy (prop_list);
    if (selector != NULL)
        cr_sel_eng_destroy (selector);
    if (cascade != NULL)
        cr_cascade_destroy (cascade);
    if (declarations != NULL)
        _svg_destroy_css_declarations (declarations, num_declarations);

    return status;
}



#endif

