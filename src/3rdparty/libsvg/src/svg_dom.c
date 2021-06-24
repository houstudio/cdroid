/* svg_dom.c: Simple SVG XML DOM implementation

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


/* keep this list in sync with the values defined in svg_known_ns_id_t (not including NO_NAMESPACE_INDEX) */

static const svg_ns_map_t SVG_KNOWN_NS_MAP[] = {
    {"http://www.w3.org/2000/svg", SVG_NAMESPACE_INDEX},
    {"http://www.w3.org/XML/1998/namespace", XML_NAMESPACE_INDEX},
    {"http://www.w3.org/1999/xlink", XLINK_NAMESPACE_INDEX},
};



static svg_status_t
_svg_init_uri_map_entry (svg_ns_map_t *map, const char *uri, int index)
{
    map->uri = strdup (uri);
    if (map->uri == NULL)
        return SVG_STATUS_NO_MEMORY;

    map->index = index;

    return SVG_STATUS_SUCCESS;
}

static void
_svg_deinit_uri_map_entry (svg_ns_map_t *map)
{
    if (map->uri != NULL) {
        free (map->uri);
        map->uri = NULL;
    }
}

static svg_status_t
_svg_attach_new_node (svg_dom_t *dom, svg_dom_node_t *new_node)
{
    svg_dom_node_t *child;

    if (dom->current_node == NULL) {
        if (dom->root_node != NULL)
            return SVG_STATUS_PARSE_ERROR;      /* multiple root nodes are not permitted */

        dom->root_node = new_node;
    } else if (dom->current_node->children == NULL) {
        dom->current_node->children = new_node;
    } else {
        child = dom->current_node->children;
        while (child->next_sibling != NULL)
            child = child->next_sibling;

        child->next_sibling = new_node;
        new_node->prev_sibling = child;
    }

    new_node->doc = dom;
    new_node->parent = dom->current_node;
    dom->current_node = new_node;

    if (new_node->parent != NULL) {
        _svg_dom_node_set_base_uri (new_node, new_node->parent->base_uri, 0);
        _svg_dom_node_set_document_uri (new_node, new_node->parent->document_uri);
        new_node->preserve_space = new_node->parent->preserve_space;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_append_child_node (svg_dom_node_t *parent, svg_dom_node_t *new_child)
{
    svg_dom_node_t *child;

    if (parent->children == NULL) {
        parent->children = new_child;
        new_child->parent = parent;
    } else {
        child = parent->children;
        while (child->next_sibling != NULL)
            child = child->next_sibling;

        child->next_sibling = new_child;
        new_child->prev_sibling = child;
        new_child->parent = parent;
    }

    return SVG_STATUS_SUCCESS;
}

static int
svg_utf8_code_len (const char *utf8, size_t utf8_len)
{
    if (((unsigned char) utf8[0] & 0x80) == 0) {
        return 1;
    } else if (((unsigned char) utf8[0] & 0xe0) == 0xc0) {
        if (utf8_len > 1 &&
            (utf8[1] & 0xc0) == 0x80)
        {
            return 2;
        }
    } else if (((unsigned char) utf8[0] & 0xf0) == 0xe0) {
        if (utf8_len > 2 &&
            ((unsigned char) utf8[1] & 0xc0) == 0x80 && ((unsigned char) utf8[2] & 0xc0) == 0x80)
        {
            return 3;
        }
    } else if (((unsigned char) utf8[0] & 0xf8) == 0xf0) {
        if (utf8_len > 3 &&
            ((unsigned char) utf8[1] & 0xc0) == 0x80 &&
            ((unsigned char) utf8[2] & 0xc0) == 0x80 && ((unsigned char) utf8[3] & 0xc0) == 0x80)
        {
            return 4;
        }
    }

    return 0;
}

static unsigned int
svg_next_utf8_code_point (const char *utf8, size_t utf8_len, int *code_len)
{
    unsigned int code_point;

    *code_len = svg_utf8_code_len (utf8, utf8_len);

    if (*code_len == 0) {
        code_point = 0xd800;    /* invalid code point */
    } else if (*code_len == 1) {
        code_point = (unsigned int) utf8[0];
    } else if (*code_len == 2) {
        code_point = (((unsigned int) utf8[0]) & 0x1f) << 6;
        code_point |= (((unsigned int) utf8[1]) & 0x3f);
    } else if (*code_len == 3) {
        code_point = (((unsigned int) utf8[0]) & 0x0f) << 12;
        code_point |= (((unsigned int) utf8[1]) & 0x3f) << 6;
        code_point |= (((unsigned int) utf8[2]) & 0x3f);
    } else {
        code_point = (((unsigned int) utf8[0]) & 0x07) << 18;
        code_point |= (((unsigned int) utf8[1]) & 0x3f) << 12;
        code_point |= (((unsigned int) utf8[2]) & 0x3f) << 6;
        code_point |= (((unsigned int) utf8[3]) & 0x3f);
    }

    return code_point;
}




svg_status_t
_svg_dom_init (svg_dom_t *dom)
{
    svg_status_t status;
    int i;

    memset (dom, 0, sizeof (svg_dom_t));

    dom->ns_map = (svg_ns_map_t *) malloc (sizeof (SVG_KNOWN_NS_MAP));
    if (dom->ns_map == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (dom->ns_map, 0, sizeof (SVG_KNOWN_NS_MAP));

    dom->num_ns = SVG_ARRAY_SIZE (SVG_KNOWN_NS_MAP);

    for (i = 0; i < dom->num_ns; i++) {
        status = _svg_init_uri_map_entry (&dom->ns_map[i], SVG_KNOWN_NS_MAP[i].uri,
                                          SVG_KNOWN_NS_MAP[i].index);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

void
_svg_dom_deinit (svg_dom_t *dom)
{
    int i;

    if (dom->ns_map != NULL) {
        for (i = 0; i < dom->num_ns; i++)
            _svg_deinit_uri_map_entry (&dom->ns_map[i]);
        free (dom->ns_map);
    }

    if (dom->root_node != NULL)
        _svg_destroy_dom_node (dom->root_node);
}

svg_status_t
_svg_dom_create_qname (int ns_index, const char *local_name, svg_qname_t **qname)
{
    svg_qname_t *new_qname;
    svg_status_t status;

    new_qname = (svg_qname_t *) malloc (sizeof (svg_qname_t));
    if (new_qname == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_qname, 0, sizeof (svg_qname_t));

    status = _svg_dom_init_qname (new_qname, ns_index, local_name);
    if (status) {
        _svg_dom_destroy_qname (new_qname);
        return status;
    }

    *qname = new_qname;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_dom_init_qname (svg_qname_t *qname, int ns_index, const char *local_name)
{
    qname->local_name = strdup (local_name);
    if (qname->local_name == NULL)
        return SVG_STATUS_NO_MEMORY;

    qname->ns_index = ns_index;

    return SVG_STATUS_SUCCESS;
}

void
_svg_dom_deinit_qname (svg_qname_t *qname)
{
    if (qname->local_name != NULL) {
        free (qname->local_name);
        qname->local_name = NULL;
    }
}

void
_svg_dom_destroy_qname (svg_qname_t *qname)
{
    if (qname == NULL)
        return;

    _svg_dom_deinit_qname (qname);

    free (qname);
}

svg_status_t
_svg_dom_create_qattrs (int num_qattrs, svg_qattrs_t **qattrs)
{
    svg_qattrs_t *new_qattrs;

    new_qattrs = (svg_qattrs_t *) malloc (sizeof (svg_qattrs_t));
    if (new_qattrs == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_qattrs, 0, sizeof (svg_qattrs_t));

    if (num_qattrs > 0) {
        new_qattrs->atts = (svg_qattr_t *) malloc (num_qattrs * sizeof (svg_qattr_t));
        if (new_qattrs->atts == NULL) {
            _svg_dom_destroy_qattrs (new_qattrs);
            return SVG_STATUS_NO_MEMORY;
        }
        new_qattrs->num = num_qattrs;

        memset (new_qattrs->atts, 0, num_qattrs * sizeof (svg_qattr_t));
    }

    *qattrs = new_qattrs;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_dom_init_qattr (svg_qattr_t *qattr, int ns_index, const char *local_name, const char *value)
{
    svg_status_t status;

    status = _svg_dom_init_qname (&qattr->name, ns_index, local_name);
    if (status)
        return status;

    qattr->value = NULL;
    if (value != NULL) {
        qattr->value = strdup (value);
        if (qattr->value == NULL)
            return SVG_STATUS_NO_MEMORY;
    }

    return SVG_STATUS_SUCCESS;
}

void
_svg_dom_deinit_qattr (svg_qattr_t *qattr)
{
    _svg_dom_deinit_qname (&qattr->name);

    if (qattr->value != NULL) {
        free (qattr->value);
        qattr->value = NULL;
    }
}

void
_svg_dom_destroy_qattrs (svg_qattrs_t *qattrs)
{
    int i;

    if (qattrs == NULL)
        return;

    if (qattrs->atts != NULL) {
        for (i = 0; i < qattrs->num; i++)
            _svg_dom_deinit_qattr (&qattrs->atts[i]);

        free (qattrs->atts);
    }

    free (qattrs);
}

int
_svg_compare_qname (svg_qname_t *left, svg_qname_t *right)
{
    if (left->ns_index != right->ns_index)
        return left->ns_index - right->ns_index;

    return strcmp (left->local_name, right->local_name);
}

int
_svg_compare_qname_2 (svg_qname_t *left, int ns_index, const char *local_name)
{
    if (left->ns_index != ns_index)
        return left->ns_index - ns_index;

    return strcmp (left->local_name, local_name);
}

int
_svg_compare_qname_svg (svg_qname_t *left, const char *svg_local_name)
{
    return _svg_compare_qname_2 (left, SVG_NAMESPACE_INDEX, svg_local_name);
}

svg_status_t
_svg_dom_register_uri (svg_dom_t *dom, const char *uri, int *index)
{
    int i;
    svg_ns_map_t *new_map;
    svg_status_t status;
    int next_index;

    if (uri == NULL) {
        *index = NO_NAMESPACE_INDEX;
        return SVG_STATUS_SUCCESS;
    }

    for (i = 0; i < dom->num_ns; i++) {
        if (strcmp (dom->ns_map[i].uri, uri) == 0) {
            *index = dom->ns_map[i].index;
            return SVG_STATUS_SUCCESS;
        }
    }

    new_map = realloc (dom->ns_map, (dom->num_ns + 1) * sizeof (svg_ns_map_t));
    if (new_map == NULL)
        return SVG_STATUS_NO_MEMORY;
    dom->ns_map = new_map;

    memset (&dom->ns_map[dom->num_ns], 0, sizeof (svg_ns_map_t));

    if (dom->num_ns == 0)
        next_index = 1;
    else
        next_index = dom->ns_map[dom->num_ns - 1].index + 1;

    status = _svg_init_uri_map_entry (&dom->ns_map[dom->num_ns], uri, next_index);
    if (status)
        return status;

    dom->num_ns++;

    *index = next_index;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_create_dom_node (svg_dom_node_type_t type, svg_dom_node_t **node)
{
    svg_dom_node_t *new_node;
    svg_status_t status;

    new_node = (svg_dom_node_t *) malloc (sizeof (svg_dom_node_t));
    if (new_node == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_node, 0, sizeof (svg_dom_node_t));

    new_node->type = type;
    new_node->line_number = -1;

    status = _svg_style_init_empty (&new_node->style);
    if (status) {
        free (new_node);
        return status;
    }

    *node = new_node;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_destroy_dom_node (svg_dom_node_t *node)
{
    svg_dom_node_t *child, *next_sibling;
    svg_status_t status;

    if (node == NULL)
        return SVG_STATUS_SUCCESS;

    child = node->children;
    while (child != NULL) {
        next_sibling = child->next_sibling;

        status = _svg_destroy_dom_node (child);
        if (status)
            return status;

        child = next_sibling;
    }

    if (node->qname != NULL && node->own_qname)
        _svg_dom_destroy_qname (node->qname);
    if (node->qattrs != NULL && node->own_qattrs)
        _svg_dom_destroy_qattrs (node->qattrs);
    if (node->base_uri != NULL && node->own_base_uri)
        _svg_destroy_uri (node->base_uri);

    if (!node->is_deep_clone) {
        if (node->ch != NULL)
            free (node->ch);
        if (node->css_properties != NULL)
            _svg_destroy_css_declarations (node->css_properties, node->num_css_properties);
        if (node->inline_css_properties != NULL)
            _svg_destroy_css_declarations (node->inline_css_properties,
                                           node->num_inline_css_properties);
    }

    _svg_style_deinit (&node->style);

    free (node);

    return SVG_STATUS_SUCCESS;
}


svg_status_t
_svg_dom_start_element (svg_dom_t *dom, svg_qname_t *qname, svg_qattrs_t *qattrs,
                        svg_dom_node_t **node)
{
    svg_dom_node_t *new_node = NULL;
    svg_status_t status;
    const char *str;
    svg_uri_t *new_base_uri = NULL;
    svg_uri_t *abs_uri;

    if (dom->current_node != NULL && dom->current_node->type == CHARACTER_NODE_TYPE)
        dom->current_node = dom->current_node->parent;

    status = _svg_create_dom_node (ELEMENT_NODE_TYPE, &new_node);
    if (status)
        return status;

    /* assign and take ownership */
    new_node->qname = qname;
    new_node->own_qname = 1;
    new_node->qattrs = qattrs;
    new_node->own_qattrs = 1;

    status = _svg_attach_new_node (dom, new_node);
    if (status)
        goto fail;

    _svg_attribute_get_string_ns (qattrs, XML_NAMESPACE_INDEX, "space", &str, "");
    if (strcmp (str, "preserve") == 0)
        new_node->preserve_space = 1;
    else if (strcmp (str, "default") == 0)
        new_node->preserve_space = 0;

    _svg_attribute_get_string_ns (qattrs, XML_NAMESPACE_INDEX, "base", &str, NULL);
    if (str != NULL) {
        status = _svg_create_uri (str, &new_base_uri);
        if (status)
            goto fail;

        if (_svg_uri_is_relative (new_base_uri)) {
            status = _svg_uri_create_absolute (new_node->base_uri, new_base_uri, &abs_uri);
            if (status)
                goto fail;

            _svg_destroy_uri (new_base_uri);
            new_base_uri = abs_uri;
        }

        _svg_dom_node_set_base_uri (new_node, new_base_uri, 1);
        new_base_uri = NULL;
    }


    *node = new_node;
    return SVG_STATUS_SUCCESS;


  fail:
    if (new_node != NULL) {
        new_node->qname = NULL;
        new_node->qattrs = NULL;
        _svg_destroy_dom_node (new_node);
    }
    if (new_base_uri != NULL)
        _svg_destroy_uri (new_base_uri);

    return status;
}

svg_status_t
_svg_dom_end_element (svg_dom_t *dom, svg_qname_t *qname)
{
    if (dom->current_node == NULL)
        return SVG_STATUS_PARSE_ERROR;

    if (dom->current_node->type == CHARACTER_NODE_TYPE)
        dom->current_node = dom->current_node->parent;

    dom->current_node = dom->current_node->parent;

    _svg_dom_destroy_qname (qname);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_dom_characters (svg_dom_t *dom, const char *ch, size_t len, svg_dom_node_t **node)
{
    svg_dom_node_t *character_node = NULL;
    char *new_ch = NULL;
    svg_status_t status;

    if (dom->current_node == NULL)
        return SVG_STATUS_PARSE_ERROR;

    if (dom->current_node->type != CHARACTER_NODE_TYPE) {
        status = _svg_create_dom_node (CHARACTER_NODE_TYPE, &character_node);
        if (status)
            return status;
    } else {
        character_node = dom->current_node;
    }

    if (len > 0) {
        new_ch = realloc (character_node->ch, character_node->len + len);
        if (new_ch == NULL) {
            status = SVG_STATUS_NO_MEMORY;
            goto fail;
        }
        character_node->ch = new_ch;

        memcpy (&character_node->ch[character_node->len], ch, len);
        character_node->len += len;
    }

    if (dom->current_node->type != CHARACTER_NODE_TYPE) {
        status = _svg_attach_new_node (dom, character_node);
        if (status)
            goto fail;
    }

    *node = character_node;

    return SVG_STATUS_SUCCESS;


  fail:
    if (dom->current_node->type != CHARACTER_NODE_TYPE && character_node != NULL)
        _svg_destroy_dom_node (character_node);

    return status;
}

svg_status_t
_svg_dom_deep_clone_node (svg_dom_node_t *parent, svg_dom_node_t *from_node, svg_dom_node_t **clone)
{
    svg_status_t status;
    svg_dom_node_t *new_node;
    svg_dom_node_t *from_child;
    svg_dom_node_t *new_child_node;

    if (from_node->type == CHARACTER_NODE_TYPE) {
        status = _svg_create_dom_node (CHARACTER_NODE_TYPE, &new_node);
        if (status)
            return status;

        memcpy (new_node, from_node, sizeof (svg_dom_node_t));
        new_node->parent = NULL;
        new_node->next_sibling = NULL;
        new_node->prev_sibling = NULL;
        new_node->children = NULL;

        new_node->is_deep_clone = 1;
        new_node->own_qname = 0;
        new_node->own_qattrs = 0;
        new_node->own_base_uri = 0;
    } else {
        status = _svg_create_dom_node (ELEMENT_NODE_TYPE, &new_node);
        if (status)
            return status;

        memcpy (new_node, from_node, sizeof (svg_dom_node_t));
        new_node->parent = NULL;
        new_node->next_sibling = NULL;
        new_node->prev_sibling = NULL;
        new_node->children = NULL;

        new_node->is_deep_clone = 1;
        new_node->own_qname = 0;
        new_node->own_qattrs = 0;
        new_node->own_base_uri = 0;

        from_child = from_node->children;
        while (from_child != NULL) {
            status = _svg_dom_deep_clone_node (new_node, from_child, &new_child_node);
            if (status)
                goto fail;

            from_child = from_child->next_sibling;
        }
    }

    status = _svg_style_init_empty (&new_node->style);
    if (status)
        goto fail;

    if (parent != NULL) {
        status = _svg_append_child_node (parent, new_node);
        if (status)
            goto fail;
    }

    *clone = new_node;

    return SVG_STATUS_SUCCESS;

  fail:
    if (new_node != NULL)
        _svg_destroy_dom_node (new_node);

    return status;
}

svg_status_t
_svg_dom_node_set_qname (svg_dom_node_t *node, int ns_index, const char *local_name)
{
    svg_status_t status;
    svg_qname_t *new_qname;

    status = _svg_dom_create_qname (ns_index, local_name, &new_qname);
    if (status)
        return status;

    if (node->qname != NULL && node->own_qname)
        _svg_dom_destroy_qname (node->qname);

    node->qname = new_qname;
    node->own_qname = 1;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_dom_node_set_qattr (svg_dom_node_t *node, int ns_index, const char *local_name,
                          const char *value)
{
    svg_status_t status;
    int i;
    int index;
    svg_qattrs_t *new_qattrs;
    svg_qattr_t *new_atts;
    char *new_value;

    if (node->qattrs == NULL) {
        if (value == NULL)
            return SVG_STATUS_SUCCESS;

        status = _svg_dom_create_qattrs (1, &new_qattrs);
        if (status)
            return status;

        status = _svg_dom_init_qattr (&new_qattrs->atts[0], ns_index, local_name, value);
        if (status) {
            _svg_dom_destroy_qattrs (new_qattrs);
            return status;
        }

        node->qattrs = new_qattrs;
        node->own_qattrs = 1;
    } else {
        if (!node->own_qattrs) {
            status = _svg_dom_create_qattrs (node->qattrs->num, &new_qattrs);
            if (status)
                return status;

            for (i = 0; i < node->qattrs->num; i++) {
                status = _svg_dom_init_qattr (&new_qattrs->atts[i],
                                              node->qattrs->atts[i].name.ns_index,
                                              node->qattrs->atts[i].name.local_name,
                                              node->qattrs->atts[i].value);
                if (status) {
                    _svg_dom_destroy_qattrs (new_qattrs);
                    return status;
                }
            }

            node->qattrs = new_qattrs;
            node->own_qattrs = 1;
        }

        index = -1;
        for (i = 0; i < node->qattrs->num; i++) {
            if (_svg_compare_qname_2 (&node->qattrs->atts[i].name, ns_index, local_name) == 0) {
                index = i;
                break;
            }
        }

        if (index < 0) {
            if (value == NULL)
                return SVG_STATUS_SUCCESS;

            new_atts = realloc (node->qattrs->atts, (node->qattrs->num + 1) * sizeof (svg_qattr_t));
            if (new_atts == NULL)
                return SVG_STATUS_NO_MEMORY;

            node->qattrs->atts = new_atts;

            status =
                _svg_dom_init_qattr (&node->qattrs->atts[node->qattrs->num], ns_index, local_name,
                                     value);
            if (status)
                return status;

            node->qattrs->num++;
        } else {
            if (value == NULL) {
                if (node->qattrs->num == 1) {
                    _svg_dom_destroy_qattrs (node->qattrs);
                    node->qattrs = NULL;
                } else {
                    _svg_dom_deinit_qattr (&node->qattrs->atts[index]);
                    if (index != node->qattrs->num - 1)
                        memcpy (&node->qattrs->atts[index],
                                &node->qattrs->atts[node->qattrs->num - 1], sizeof (svg_qattr_t));

                    node->qattrs->num--;
                }
            } else {
                new_value = strdup (value);
                if (new_value == NULL)
                    return SVG_STATUS_NO_MEMORY;

                free (node->qattrs->atts[index].value);
                node->qattrs->atts[index].value = new_value;
            }
        }
    }

    return SVG_STATUS_SUCCESS;
}

void
_svg_dom_set_line_number (svg_dom_node_t *node, long line_number)
{
    if (node->line_number < 0)
        node->line_number = line_number;
}

void
_svg_dom_node_set_base_uri (svg_dom_node_t *node, svg_uri_t *base_uri, int own_base_uri)
{
    if (node->base_uri != NULL && node->own_base_uri)
        _svg_destroy_uri (node->base_uri);

    node->base_uri = base_uri;
    node->own_base_uri = own_base_uri;
}

void
_svg_dom_node_set_document_uri (svg_dom_node_t *node, svg_uri_t *document_uri)
{
    node->document_uri = document_uri;
}

svg_dom_node_t *
_svg_dom_get_current_element_node (svg_dom_t *node)
{
    if (node->current_node == NULL)
        return NULL;

    if (node->current_node->type == ELEMENT_NODE_TYPE)
        return node->current_node;
    else
        return node->current_node->parent;
}

svg_dom_node_t *
_svg_dom_get_current_character_node (svg_dom_t *node)
{
    if (node->current_node == NULL)
        return NULL;

    if (node->current_node->type == CHARACTER_NODE_TYPE)
        return node->current_node;
    else
        return NULL;
}

svg_dom_node_t *
_svg_dom_get_parent (svg_dom_node_t *node)
{
    return node->parent;
}

svg_dom_node_t *
_svg_dom_get_first_child (svg_dom_node_t *node)
{
    return node->children;
}

svg_dom_node_t *
_svg_dom_get_next_sibling (svg_dom_node_t *node)
{
    return node->next_sibling;
}

svg_dom_node_t *
_svg_dom_get_prev_sibling (svg_dom_node_t *node)
{
    return node->prev_sibling;
}

int
_svg_dom_is_element_node (svg_dom_node_t *node)
{
    return node->type == ELEMENT_NODE_TYPE;
}

const char *
_svg_dom_get_node_local_name (svg_dom_node_t *node)
{
    return node->qname->local_name;
}

const char *
_svg_dom_get_local_attr_value (svg_dom_node_t *node, const char *local_name)
{
    int i;

    if (node->qattrs != NULL)
        for (i = 0; i < node->qattrs->num; i++)
            if (strcmp (node->qattrs->atts[i].name.local_name, local_name) == 0)
                return node->qattrs->atts[i].value;

    return NULL;
}

svg_status_t
_svg_normalize_character_data (svg_dom_node_t *node, char **ch_data_ref, size_t *ch_data_offset_ref,
                               size_t *ch_data_len_ref)
{
    size_t i;
    const char *src;
    char *dst;
    int space;
    const char *start, *end;
    svg_dom_node_t *prev_sibling, *next_sibling;
    char *ch_data;
    int code_len;
    int j;
    size_t ch_data_offset, ch_data_len;

    if (node->len == 0) {
        *ch_data_ref = NULL;
        *ch_data_offset_ref = 0;
        *ch_data_len_ref = 0;
        return SVG_STATUS_SUCCESS;
    }

    ch_data = malloc (node->len);
    if (ch_data == NULL)
        return SVG_STATUS_NO_MEMORY;

    if (node->preserve_space) {
        code_len = 0;
        for (src = node->ch, dst = ch_data, i = 0; i < node->len; i += code_len) {
            svg_next_utf8_code_point (src, node->len - i, &code_len);
            if (code_len == 0) {
                free (ch_data);
                return SVG_STATUS_XML_INVALID_TOKEN;
            }

            if (code_len == 1) {
                if (*src == '\n' || *src == '\r' || *src == '\t')
                    *dst = ' ';
                else
                    *dst = *src;
                src++;
                dst++;
            } else {
                for (j = 0; j < code_len; j++) {
                    *dst = *src;
                    dst++;
                    src++;
                }
            }
        }

        start = ch_data;
        end = dst;
    } else {
        dst = ch_data;
        space = 0;
        code_len = 0;
        for (src = node->ch, i = 0; i < node->len; i += code_len) {
            svg_next_utf8_code_point (src, node->len - i, &code_len);
            if (code_len == 0) {
                free (ch_data);
                return SVG_STATUS_XML_INVALID_TOKEN;
            }

            if (code_len == 1) {
                if (*src == '\t' || *src == '\r' || *src == ' ') {
                    if (!space) {
                        *dst = ' ';
                        space = 1;
                        dst++;
                    }
                } else if (*src != '\n') {
                    *dst = *src;
                    space = 0;
                    dst++;
                }
                src++;
            } else {
                for (j = 0; j < code_len; j++) {
                    *dst = *src;
                    dst++;
                    src++;
                }
            }
        }

        start = ch_data;
        end = dst;

        if (start != end) {
            prev_sibling = _svg_dom_get_prev_sibling (node);
            if (!prev_sibling && *start == ' ')
                start++;

            if (start != end) {
                next_sibling = _svg_dom_get_next_sibling (node);
                if (!next_sibling && *(end - 1) == ' ')
                    end--;
            }
        }
    }

    ch_data_offset = (size_t) (start - ch_data);
    ch_data_len = (size_t) (end - start);

    if (ch_data_len == 0) {
        free (ch_data);
        ch_data = NULL;
    }

    *ch_data_ref = ch_data;
    *ch_data_offset_ref = ch_data_offset;
    *ch_data_len_ref = ch_data_len;

    return SVG_STATUS_SUCCESS;
}

