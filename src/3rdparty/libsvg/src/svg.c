/* svg.c - Parsing and rendering SVG documents

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

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#if defined(_MSC_VER)
#include "win32_compat.h"
#else
#include <unistd.h>
#include <libgen.h>
#include <sys/param.h>
#endif
#include <zlib.h>


#define SVG_PARSE_BUFFER_SIZE (8 * 1024)


typedef struct {
    svg_t *svg;
    z_stream strm;
    unsigned char buffer[SVG_PARSE_BUFFER_SIZE];
    int first_read;
    unsigned char peek_bytes[2];
    int num_peek_bytes;
    int is_compressed;
} svg_read_data_t;


static const char *SVG_STATUS_STRINGS[] = {
    "success",
    "no memory",
    "i/o error",
    "file not found",
    "resource not found",
    "invalid value",
    "invalid base uri value",
    "duplicate element identifier",
    "missing element identifier in reference",
    "uri is relative when absolute expected",
    "unknown referenced element",
    "wrong type for referenced element",
    "document root is not svg element",
    "missing svg namespace declaration",
    "empty document",
    "attribute not found",
    "invalid call",
    "parse error",
    "xml parse error",
    "xml tag mismatch",
    "unbound xml prefix",
    "invalid xml token",
    "css parse error",
    "no relative uri",
    "external document error",
    "internal error",
    "unexpected error"          /* used if status is outside enum range */
};


static svg_status_t
_svg_init (svg_t *svg)
{
    svg_status_t status;

    memset (svg, 0, sizeof (svg_t));

    svg->dpi = 90.0;

    status = svg_set_base_directory (svg, ".");
    if (status)
        return status;
    svg->user_set_base_uri = 0;

    _svg_parser_init (&svg->parser, svg);

    svg->element_ids = _svg_xml_hash_create (100);

    svg->error_info.line_number = -1;

    return SVG_STATUS_SUCCESS;
}

static void
_svg_deinit (svg_t *svg)
{
    svg_t *old_child;
    int i;

    while (svg->children != NULL) {
        old_child = svg->children;
        svg->children = svg->children->next_sibling;
        svg_destroy (old_child);
    }

    if (svg->base_uri != NULL)
        _svg_destroy_uri (svg->base_uri);
    if (svg->document_uri != NULL)
        _svg_destroy_uri (svg->document_uri);

    if (svg->root_element)
        _svg_element_destroy (svg->root_element);

    _svg_parser_deinit (&svg->parser);

    if (svg->trace != NULL && svg->own_trace)
        _svg_destroy_trace (svg->trace);

    _svg_xml_hash_free (svg->element_ids);

    for (i = 0; i < svg->num_images; i++)
        free (svg->image_uris[i]);
    free (svg->image_uris);
}

static svg_status_t
_svg_set_trace (svg_t *svg, svg_trace_t *trace, int own)
{
    if (svg->trace != NULL && svg->own_trace)
        _svg_destroy_trace (svg->trace);

    svg->trace = trace;
    svg->own_trace = own;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_set_base_uri (svg_t *svg, svg_uri_t *uri, int from_user)
{
    if (_svg_uri_is_relative (uri))
        return SVG_STATUS_RELATIVE_URI;

    if (svg->base_uri != NULL)
        _svg_destroy_uri (svg->base_uri);

    svg->base_uri = uri;
    svg->user_set_base_uri = from_user;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_read_cb (void *closure, const char *buffer, size_t buffer_size)
{
    static int const gz_magic[2] = { 0x1f, 0x8b };      /* gzip magic header */
    svg_read_data_t *data = (svg_read_data_t *) closure;
    int result;
    svg_status_t status;
    int copy_peek_bytes = 0;

    if (buffer_size <= 0)
        return SVG_STATUS_SUCCESS;

    if (data->first_read) {
        copy_peek_bytes = 2 - data->num_peek_bytes;
        if ((size_t) copy_peek_bytes > buffer_size)
            copy_peek_bytes = (int) buffer_size;

        memcpy (&data->peek_bytes[data->num_peek_bytes], buffer, copy_peek_bytes);
        data->num_peek_bytes += copy_peek_bytes;

        if (data->num_peek_bytes != 2)
            return SVG_STATUS_SUCCESS;


        if (data->peek_bytes[0] == gz_magic[0] && data->peek_bytes[1] == gz_magic[1])
            data->is_compressed = 1;

        data->first_read = 0;

        if (copy_peek_bytes < 2) {
            status = _svg_read_cb (closure, (char *) data->peek_bytes, 2 - copy_peek_bytes);
            if (status)
                return status;
        }
    }

    if (data->is_compressed) {
        data->strm.avail_in = (unsigned int) buffer_size;
        data->strm.next_in = (unsigned char *) buffer;

        do {
            data->strm.avail_out = SVG_PARSE_BUFFER_SIZE;
            data->strm.next_out = data->buffer;

            result = inflate (&data->strm, Z_NO_FLUSH);
            if (result != Z_OK && result != Z_STREAM_END) {
                if (result == Z_MEM_ERROR)
                    return SVG_STATUS_NO_MEMORY;
                return SVG_STATUS_IO_ERROR;
            }

            status = svg_parse_chunk (data->svg, (char *) data->buffer,
                                      SVG_PARSE_BUFFER_SIZE - data->strm.avail_out);
            if (status)
                return status;
        }
        while (data->strm.avail_out == 0);
    } else {
        status = svg_parse_chunk (data->svg, buffer, buffer_size);
        if (status)
            return status;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_parse_uri_ref_doc (svg_t *svg, svg_uri_t *uri)
{
    svg_read_data_t data;
    svg_status_t status;
    int result;
    svg_uri_t *abs_uri = NULL;
    svg_uri_t *new_base_uri;

    if (_svg_uri_is_relative (uri)) {
        status = _svg_uri_create_absolute (svg->base_uri, uri, &abs_uri);
        if (status)
            return status;
    } else {
        status = _svg_uri_clone (uri, &abs_uri);
        if (status)
            return status;
    }

    if (!svg->user_set_base_uri) {
        status = _svg_uri_clone (abs_uri, &new_base_uri);
        if (status) {
            _svg_destroy_uri (abs_uri);
            return status;
        }

        if (svg->base_uri != NULL)
            _svg_destroy_uri (svg->base_uri);
        svg->base_uri = new_base_uri;
    }

    svg->document_uri = abs_uri;

    data.svg = svg;
    data.first_read = 1;
    data.num_peek_bytes = 0;
    data.is_compressed = 0;

    data.strm.zalloc = Z_NULL;
    data.strm.zfree = Z_NULL;
    data.strm.opaque = Z_NULL;
    data.strm.avail_in = 0;
    data.strm.next_in = Z_NULL;

    result = inflateInit2 (&data.strm, 15 + 32);        /* + 32 -> enable zlib and gzip */
    if (result != Z_OK)
        return SVG_STATUS_NO_MEMORY;

    status = svg_parse_chunk_begin (svg);
    if (status)
        goto end;

    status = _svg_resource_read_stream (svg->document_uri, &data, _svg_read_cb);
    if (status)
        goto end;

    status = svg_parse_chunk_end (svg);

  end:
    inflateEnd (&data.strm);
    return status;
}

static svg_status_t
_svg_resolve_element_ref (svg_t *svg, svg_t *from_svg, svg_uri_ref_t *abs_uri_ref,
                          svg_element_t **element)
{
    svg_t *ref_svg;
    svg_t *child;
    svg_t *new_child;
    svg_status_t status;

    if (abs_uri_ref->element_id == NULL)
        return SVG_STATUS_MISSING_REF_ELEMENT_ID;


    if (abs_uri_ref->uri != NULL && !_svg_uri_equals (abs_uri_ref->uri, svg->document_uri)) {

        if (svg->parent != NULL && from_svg != svg->parent) {
            status = _svg_resolve_element_ref (svg->parent, svg, abs_uri_ref, element);
            if (status) {
                if ((svgint_status_t)status != SVGINT_STATUS_UNKNOWN_DOCUMENT)
                    return status;
            } else {
                return SVG_STATUS_SUCCESS;
            }
        }

        child = svg->children;
        while (child != NULL) {
            status = _svg_resolve_element_ref (child, svg, abs_uri_ref, element);
            if (status) {
                if ((svgint_status_t)status != SVGINT_STATUS_UNKNOWN_DOCUMENT)
                    return status;
            } else {
                return SVG_STATUS_SUCCESS;
            }

            child = child->next_sibling;
        }

        if (svg != from_svg)
            return SVGINT_STATUS_UNKNOWN_DOCUMENT;


        status = svg_create (&new_child);
        if (status)
            return status;

        if (svg->trace != NULL) {
            status = _svg_set_trace (new_child, svg->trace, 0);
            if (status)
                return status;
        }

        status = _svg_parse_uri_ref_doc (new_child, abs_uri_ref->uri);
        if (status) {
            svg_destroy (new_child);
            _svg_set_external_error_status (svg, status);
            return SVG_STATUS_EXTERNAL_DOCUMENT_ERROR;
        }

        new_child->parent = svg;
        new_child->next_sibling = svg->children;
        svg->children = new_child;

        ref_svg = new_child;
    } else {
        ref_svg = svg;
    }


    *element = _svg_fetch_element_by_id (ref_svg, abs_uri_ref->element_id);
    if (*element == NULL)
        return SVG_STATUS_UNKNOWN_REF_ELEMENT;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_store_element_by_id (svg_t *svg, svg_element_t *element)
{
    int result;

    if (element->node->is_deep_clone)
        return SVG_STATUS_SUCCESS;

    result = _svg_xml_hash_add_entry (svg->element_ids, element->id, element);
    if (result) {
        if (result == 2)
            return SVG_STATUS_DUPLICATE_ELEMENT_ID;
        else
            return SVG_STATUS_NO_MEMORY;
    }

    return SVG_STATUS_SUCCESS;
}

svg_element_t *
_svg_fetch_element_by_id (svg_t *svg, const char *id)
{
    return _svg_xml_hash_lookup (svg->element_ids, id);
}

svg_status_t
_svg_resolve_element_href (svg_t *svg, svg_uri_t *document_uri, svg_uri_t *base_uri,
                           const char *href, svg_element_t **element)
{
    svg_status_t status;
    svg_uri_ref_t *abs_uri_ref;

    status = _svg_create_uri_ref (href, base_uri, document_uri, &abs_uri_ref);
    if (status)
        return status;

    if (abs_uri_ref->element_id == NULL) {
        _svg_destroy_uri_ref (abs_uri_ref);
        return SVG_STATUS_MISSING_REF_ELEMENT_ID;
    }

    status = _svg_resolve_element_ref (svg, svg, abs_uri_ref, element);

    _svg_destroy_uri_ref (abs_uri_ref);

    return status;
}

svg_status_t
_svg_resolve_element_url (svg_t *svg, svg_uri_t *document_uri, svg_uri_t *base_uri, const char *url,
                          svg_element_t **element)
{
    svg_status_t status;
    svg_uri_ref_t *abs_uri_ref;

    status = _svg_create_uri_ref_url (url, base_uri, document_uri, &abs_uri_ref);
    if (status)
        return status;

    if (abs_uri_ref->element_id == NULL) {
        _svg_destroy_uri_ref (abs_uri_ref);
        return SVG_STATUS_MISSING_REF_ELEMENT_ID;
    }

    status = _svg_resolve_element_ref (svg, svg, abs_uri_ref, element);

    _svg_destroy_uri_ref (abs_uri_ref);

    return status;
}

svg_status_t
_svg_register_image_uri (svg_t *svg, svg_uri_t *uri, int *index)
{
    char **new_image_uris;
    char *uri_str;
    int i;
    svg_status_t status;

    status = _svg_uri_to_string (uri, &uri_str);
    if (status)
        return status;

    for (i = 0; i < svg->num_images; i++) {
        if (strcmp (uri_str, svg->image_uris[i]) == 0) {
            free (uri_str);
            *index = i;
            return SVG_STATUS_SUCCESS;
        }
    }

    new_image_uris = (char **) realloc (svg->image_uris, (svg->num_images + 1) * sizeof (char *));
    if (new_image_uris == NULL) {
        free (uri_str);
        return SVG_STATUS_NO_MEMORY;
    }
    svg->image_uris = new_image_uris;

    new_image_uris[svg->num_images] = uri_str;

    *index = svg->num_images;
    svg->num_images++;

    return SVG_STATUS_SUCCESS;
}

const char *
_svg_get_image_uri (svg_t *svg, int index)
{
    if (index >= svg->num_images)
        return NULL;

    return svg->image_uris[index];
}

void
_svg_set_error_status (svg_t *svg, svg_status_t error_status)
{
    if (svg->error_info.error_status) {
        if (error_status == SVG_STATUS_SUCCESS)
            svg->error_info.error_status = error_status;
    } else if (error_status) {
        svg->error_info.error_status = error_status;
    }
}

void
_svg_set_external_error_status (svg_t *svg, svg_status_t error_status)
{
    if (svg->error_info.external_error_status) {
        if (error_status == SVG_STATUS_SUCCESS)
            svg->error_info.external_error_status = error_status;
    } else if (error_status) {
        svg->error_info.external_error_status = error_status;
    }
}

void
_svg_set_error_line_number (svg_t *svg, long line_number)
{
    if (svg->error_info.line_number >= 0) {
        if (line_number < 0)
            svg->error_info.line_number = -1;
    } else if (line_number >= 0) {
        svg->error_info.line_number = line_number;
    }
}

void
_svg_set_error_element_name (svg_t *svg, const svg_qname_t *element_name)
{
    if (svg->error_info.element_name != NULL) {
        if (element_name == NULL) {
            free (svg->error_info.element_name);
            svg->error_info.element_name = NULL;
        }
    } else if (element_name != NULL && element_name->local_name != NULL) {
        svg->error_info.element_name = strdup (element_name->local_name);
    }
}

void
_svg_set_error_attribute_name (svg_t *svg, const char *attribute_name)
{
    if (svg->error_info.attribute_name != NULL) {
        if (attribute_name == NULL) {
            free (svg->error_info.attribute_name);
            svg->error_info.attribute_name = NULL;
        }
    } else if (attribute_name != NULL) {
        svg->error_info.attribute_name = strdup (attribute_name);
    }
}

void
_svg_set_error_property_name (svg_t *svg, const char *property_name)
{
    if (svg->error_info.property_name != NULL) {
        if (property_name == NULL) {
            free (svg->error_info.property_name);
            svg->error_info.property_name = NULL;
        }
    } else if (property_name != NULL) {
        svg->error_info.property_name = strdup (property_name);
    }
}

svg_status_t
_svg_return_error_status (svg_t *svg, svg_status_t error_status)
{
    _svg_set_error_status (svg, error_status);

    return error_status;
}

svg_status_t
_svg_externalize_status (svgint_status_t internal_status, svg_status_t default_status)
{
    if (internal_status > (svgint_status_t)SVG_STATUS_INTERNAL_ERROR)
        return default_status;
    return internal_status;
}


svg_status_t
svg_create (svg_t **svg)
{
    svg_t *new_svg;
    svg_status_t status;

    new_svg = malloc (sizeof (svg_t));
    if (new_svg == NULL)
        return SVG_STATUS_NO_MEMORY;

    status = _svg_init (new_svg);
    if (status) {
        free (new_svg);
        return status;
    }

    *svg = new_svg;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_destroy (svg_t *svg)
{
    _svg_deinit (svg);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_trace_render_engine (svg_t *svg)
{
    svg_trace_t *new_trace;
    svg_status_t status;

    if (svg->trace == NULL) {
        status = _svg_create_trace (&new_trace);
        if (status)
            return status;

        status = _svg_set_trace (svg, new_trace, 1);
        if (status) {
            _svg_destroy_trace (new_trace);
            return status;
        }
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_parse_file (svg_t *svg, FILE *file)
{
    svg_status_t status = SVG_STATUS_SUCCESS;
    gzFile zfile;
    char buf[SVG_PARSE_BUFFER_SIZE];
    int read;

    if (svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;


    zfile = gzdopen (dup (fileno (file)), "r");
    if (zfile == NULL) {
        switch (errno) {
        case ENOMEM:
            return _svg_return_error_status (svg, SVG_STATUS_NO_MEMORY);
        case ENOENT:
            return _svg_return_error_status (svg, SVG_STATUS_FILE_NOT_FOUND);
        default:
            return _svg_return_error_status (svg, SVG_STATUS_IO_ERROR);
        }
    }

    status = svg_parse_chunk_begin (svg);
    if (status)
        goto end;

    while (!gzeof (zfile)) {
        read = gzread (zfile, buf, SVG_PARSE_BUFFER_SIZE);
        if (read > -1) {
            status = svg_parse_chunk (svg, buf, read);
            if (status)
                goto end;
        } else {
            status = SVG_STATUS_IO_ERROR;
            goto end;
        }
    }

    status = svg_parse_chunk_end (svg);

  end:
    gzclose (zfile);

    if (status)
        return _svg_return_error_status (svg, status);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_parse (svg_t *svg, const char *uri_or_filename)
{
    svg_status_t status;
    svg_uri_t *uri = NULL;

    if (svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;


    if (_svg_string_is_abs_filename (uri_or_filename))
        status = _svg_create_file_uri (uri_or_filename, &uri);
    else
        status = _svg_create_uri (uri_or_filename, &uri);
    if (status)
        return _svg_return_error_status (svg, status);

    status = _svg_parse_uri_ref_doc (svg, uri);

    _svg_destroy_uri (uri);

    if (status)
        return _svg_return_error_status (svg, status);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_set_base_directory (svg_t *svg, const char *directory)
{
    char current_dir[MAXPATHLEN];
    svg_uri_t *new_base_uri = NULL;
    svg_uri_t *cwd_uri = NULL;
    svg_uri_t *rel_uri = NULL;
    svg_status_t status;

    if (directory == NULL || directory[0] == '\0' || strcmp (directory, ".") == 0) {
        if (getcwd (current_dir, MAXPATHLEN) == NULL)
            return _svg_return_error_status (svg, SVG_STATUS_IO_ERROR);

        status = _svg_create_directory_uri (current_dir, &new_base_uri);
        if (status)
            return _svg_return_error_status (svg, status);
    } else {
        status = _svg_create_directory_uri (directory, &new_base_uri);
        if (status)
            return _svg_return_error_status (svg, status);

        if (_svg_uri_is_relative (new_base_uri)) {
            rel_uri = new_base_uri;
            new_base_uri = NULL;

            if (getcwd (current_dir, MAXPATHLEN) == NULL) {
                status = SVG_STATUS_IO_ERROR;
                goto fail;
            }

            status = _svg_create_directory_uri (current_dir, &cwd_uri);
            if (status)
                goto fail;

            status = _svg_uri_create_absolute (cwd_uri, rel_uri, &new_base_uri);
            if (status)
                goto fail;

            _svg_destroy_uri (cwd_uri);
            cwd_uri = NULL;
            _svg_destroy_uri (rel_uri);
            rel_uri = NULL;
        }
    }

    status = _svg_set_base_uri (svg, new_base_uri, 1);
    if (status) {
        _svg_destroy_uri (new_base_uri);
        return _svg_return_error_status (svg, status);
    }

    return SVG_STATUS_SUCCESS;


  fail:
    if (new_base_uri != NULL)
        _svg_destroy_uri (new_base_uri);
    if (cwd_uri != NULL)
        _svg_destroy_uri (cwd_uri);
    if (rel_uri != NULL)
        _svg_destroy_uri (rel_uri);

    return _svg_return_error_status (svg, status);
}

svg_status_t
svg_set_base_uri (svg_t *svg, const char *abs_uri_str)
{
    svg_uri_t *new_base_uri;
    svg_status_t status;

    if (abs_uri_str == NULL || abs_uri_str[0] == '\0')
        return _svg_return_error_status (svg, SVG_STATUS_INVALID_BASE_URI);

    status = _svg_create_uri (abs_uri_str, &new_base_uri);
    if (status)
        return _svg_return_error_status (svg, status);

    status = _svg_set_base_uri (svg, new_base_uri, 1);
    if (status) {
        _svg_destroy_uri (new_base_uri);
        return _svg_return_error_status (svg, status);
    }

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_parse_buffer (svg_t *svg, const char *buf, size_t count)
{
    svg_status_t status;

    if (svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;


    status = svg_parse_chunk_begin (svg);
    if (status)
        return _svg_return_error_status (svg, status);

    status = svg_parse_chunk (svg, buf, count);
    if (status)
        return _svg_return_error_status (svg, status);

    status = svg_parse_chunk_end (svg);
    if (status)
        return _svg_return_error_status (svg, status);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_parse_chunk_begin (svg_t *svg)
{
    svg_status_t status;

    if (svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;


    status = _svg_parser_begin_dom_parsing (&svg->parser);
    if (status)
        return _svg_return_error_status (svg, status);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_parse_chunk (svg_t *svg, const char *buf, size_t count)
{
    svg_status_t status;

    if (svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;


    status = _svg_parser_parse_chunk (&svg->parser, buf, count);
    if (status)
        return _svg_return_error_status (svg, status);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_parse_chunk_end (svg_t *svg)
{
    svg_status_t status;

    if (svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;


    status = _svg_parser_end_dom_parsing (&svg->parser);
    if (status)
        return _svg_return_error_status (svg, status);

    status = _svg_parser_process_dom (&svg->parser);
    if (status)
        return _svg_return_error_status (svg, status);

    return SVG_STATUS_SUCCESS;
}

svg_status_t
svg_render (svg_t *svg, svg_render_engine_t *target_engine, void *target_closure)
{
    svg_status_t status;
    svg_render_engine_t *engine;
    void *closure;

    if (svg->error_info.error_status != SVG_STATUS_SUCCESS)
        return SVG_STATUS_INVALID_CALL;

    if (svg->root_element == NULL)
        return SVG_STATUS_EMPTY_DOCUMENT;


    if (svg->trace != NULL) {
        status = _svg_trace_push_target_engine (svg->trace, svg->document_uri, target_engine,
                                                target_closure);
        if (status)
            return status;

        _svg_trace_get_engine (svg->trace, &engine, &closure);
    } else {
        engine = target_engine;
        closure = target_closure;
    }

    status = svg_element_render (svg->root_element, engine, closure);

    if (svg->trace != NULL)
        _svg_trace_pop_target_engine (svg->trace);

    if (status)
        return _svg_return_error_status (svg, status);

    return SVG_STATUS_SUCCESS;
}

double
svg_get_dpi (const svg_t *svg)
{
    return svg->dpi;
}

void
svg_get_size (const svg_t *svg, svg_length_t *width, svg_length_t *height)
{
    if (svg->root_element != NULL) {
        _svg_svg_group_get_size (&svg->root_element->e.svg_group, width, height);
    } else {
        _svg_length_init_unit (width, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_HORIZONTAL);
        _svg_length_init_unit (height, 0, SVG_LENGTH_UNIT_PX, SVG_LENGTH_ORIENTATION_VERTICAL);
    }
}

void
svg_get_image_uris (const svg_t *svg, const char ***image_uris, int *num_images)
{
    *image_uris = (const char **) svg->image_uris;
    *num_images = svg->num_images;
}

svg_status_t
svg_get_error_status (const svg_t *svg)
{
    return svg->error_info.error_status;
}

svg_status_t
svg_get_external_error_status (const svg_t *svg)
{
    return svg->error_info.external_error_status;
}

const char *
svg_get_error_string (svg_status_t status)
{
    if (status >= 0 && status < SVG_ARRAY_SIZE (SVG_STATUS_STRINGS))
        return SVG_STATUS_STRINGS[status];
    else
        return SVG_STATUS_STRINGS[SVG_ARRAY_SIZE (SVG_STATUS_STRINGS) - 1];
}

long
svg_get_error_line_number (const svg_t *svg)
{
    return svg->error_info.line_number;
}

const char *
svg_get_error_element (const svg_t *svg)
{
    return svg->error_info.element_name;
}

const char *
svg_get_error_property (const svg_t *svg)
{
    return svg->error_info.property_name;
}

const char *
svg_get_error_attribute (const svg_t *svg)
{
    return svg->error_info.attribute_name;
}

