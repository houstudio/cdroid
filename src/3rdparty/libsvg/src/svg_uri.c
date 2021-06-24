/* svg_uri.c: URI handling functions

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

#include <uriparser/Uri.h>


struct svg_uri {
    char *uri_str;
    UriUriA uri;
    char *media_type;
    int base64;
    size_t data_offset;
};


static svg_status_t
_svg_convert_uriparser_error (int error)
{
    switch (error) {
    case URI_SUCCESS:
        return SVG_STATUS_SUCCESS;
    case URI_ERROR_SYNTAX:
        return SVG_STATUS_PARSE_ERROR;
    default:
        return SVG_STATUS_INVALID_CALL;
    }
}

static svg_status_t
_svg_create_empty_uri (svg_uri_t **uri)
{
    svg_uri_t *new_uri;

    new_uri = (svg_uri_t *) malloc (sizeof (svg_uri_t));
    if (new_uri == NULL)
        return SVG_STATUS_NO_MEMORY;

    memset (new_uri, 0, sizeof (svg_uri_t));

    *uri = new_uri;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_uri_init (const char *uri_str, svg_uri_t *uri)
{
    UriParserStateA state;
    int result;

    memset (uri, 0, sizeof (svg_uri_t));

    uri->uri_str = strdup (uri_str);
    if (uri->uri_str == NULL)
        return SVG_STATUS_NO_MEMORY;

    state.uri = &uri->uri;
    result = uriParseUriA (&state, uri->uri_str);
    if (result) {
        free (uri->uri_str);
        return _svg_convert_uriparser_error (result);
    }

    result = uriNormalizeSyntaxA (&uri->uri);
    if (result) {
        _svg_destroy_uri (uri);
        return _svg_convert_uriparser_error (result);
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_uri_init_filename (const char *filename, svg_uri_t *uri)
{
    char *uri_str;
    size_t uri_str_size;
    svg_status_t status;
    int result;

#if defined(_WIN32)
    uri_str_size = 8 + 3 * strlen (filename) + 1;
#else
    uri_str_size = 7 + 3 * strlen (filename) + 1;
#endif

    uri_str = (char *) malloc (uri_str_size);
    if (uri_str == NULL)
        return SVG_STATUS_NO_MEMORY;


#if defined(_WIN32)
    result = uriWindowsFilenameToUriStringA (filename, uri_str);
#else
    result = uriUnixFilenameToUriStringA (filename, uri_str);
#endif

    if (result) {
        free (uri_str);
        return _svg_convert_uriparser_error (result);
    }

    status = _svg_uri_init (uri_str, uri);

    free (uri_str);

    return status;
}

static svg_status_t
_svg_uri_init_data (const char *data_uri_str, svg_uri_t *uri)
{
    const char *str;
    const char *parameter_str;
    size_t media_type_len = 0;

    if (strncmp (data_uri_str, "data:", 5) != 0)
        return SVG_STATUS_PARSE_ERROR;

    memset (uri, 0, sizeof (svg_uri_t));

    str = data_uri_str + 5;
    parameter_str = str;
    while (*str != '\0') {
        if (*str == ',') {
            if (str - parameter_str == 6 && strncmp (parameter_str, "base64", 6) == 0)
                uri->base64 = 1;
            else if (parameter_str == data_uri_str + 5)
                media_type_len = str - parameter_str;

            uri->data_offset = str - data_uri_str + 1;
            break;
        } else if (*str == ';') {
            if (parameter_str == data_uri_str + 5)
                media_type_len = str - parameter_str;

            parameter_str = str + 1;
        }

        str++;
    }

    if (*str == '\0')
        return SVG_STATUS_PARSE_ERROR;


    uri->uri_str = strdup (data_uri_str);
    if (uri->uri_str == NULL)
        return SVG_STATUS_NO_MEMORY;

    if (media_type_len > 0) {
        uri->media_type = (char *) malloc (media_type_len + 1);
        if (uri->media_type == NULL) {
            free (uri->uri_str);
            return SVG_STATUS_NO_MEMORY;
        }

        strncpy (uri->media_type, uri->uri_str + 5, media_type_len);
        uri->media_type[media_type_len] = '\0';
    }

    return SVG_STATUS_SUCCESS;
}

static void
_svg_uri_deinit (svg_uri_t *uri)
{
    if (_svg_uri_is_data_scheme (uri)) {
        if (uri->media_type != NULL)
            free (uri->media_type);
    } else {
        uriFreeUriMembersA (&uri->uri);
    }

    if (uri->uri_str != NULL)
        free (uri->uri_str);
}




svg_status_t
_svg_create_uri (const char *uri_str, svg_uri_t **uri)
{
    svg_uri_t *new_uri;
    svg_status_t status;

    status = _svg_create_empty_uri (&new_uri);
    if (status)
        return status;

    if (svg_is_data_scheme_uri (uri_str)) {
        status = _svg_uri_init_data (uri_str, new_uri);
        if (status) {
            free (new_uri);
            return status;
        }
    } else {
        status = _svg_uri_init (uri_str, new_uri);
        if (status) {
            free (new_uri);
            return status;
        }
    }

    *uri = new_uri;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_create_file_uri (const char *filename, svg_uri_t **uri)
{
    svg_uri_t *new_uri;
    svg_status_t status;

    status = _svg_create_empty_uri (&new_uri);
    if (status)
        return status;

    status = _svg_uri_init_filename (filename, new_uri);
    if (status) {
        free (new_uri);
        return status;
    }

    *uri = new_uri;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_create_directory_uri (const char *directory, svg_uri_t **uri)
{
    char *closed_directory = NULL;
    size_t len;
    svg_uri_t *new_uri;
    svg_status_t status;

    len = strlen (directory);
    closed_directory = malloc (len + 2);
    strcpy (closed_directory, directory);
#if defined(_WIN32)
    if (closed_directory[len - 1] != '\\') {
        closed_directory[len] = '\\';
        closed_directory[len + 1] = '\0';
    }
#else
    if (closed_directory[len - 1] != '/') {
        closed_directory[len] = '/';
        closed_directory[len + 1] = '\0';
    }
#endif

    status = _svg_create_file_uri (closed_directory, &new_uri);
    if (status) {
        free (closed_directory);
        return status;
    }

    *uri = new_uri;

    free (closed_directory);

    return SVG_STATUS_SUCCESS;
}

void
_svg_destroy_uri (svg_uri_t *uri)
{
    if (uri == NULL)
        return;

    _svg_uri_deinit (uri);

    free (uri);
}

svg_status_t
_svg_uri_clone (svg_uri_t *other, svg_uri_t **uri)
{
    svg_uri_t *new_uri;
    char *uri_str;
    svg_status_t status;

    if (_svg_uri_is_data_scheme (other)) {
        status = _svg_create_uri (other->uri_str, &new_uri);
        if (status)
            return status;
    } else {
        status = _svg_uri_to_string (other, &uri_str);
        if (status)
            return status;

        status = _svg_create_uri (uri_str, &new_uri);

        free (uri_str);

        if (status)
            return status;
    }

    *uri = new_uri;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_uri_create_absolute (svg_uri_t *base_uri, svg_uri_t *uri, svg_uri_t **abs_uri)
{
    svg_status_t status;
    svg_uri_t *temp_new_uri = NULL;
    svg_uri_t *new_uri = NULL;
    int result;
    char *uri_str = NULL;

    if (_svg_uri_is_data_scheme (base_uri) || _svg_uri_is_data_scheme (uri) ||
        _svg_uri_is_relative (base_uri)) {
        return SVG_STATUS_INVALID_CALL;
    }

    if (!_svg_uri_is_relative (uri)) {
        status = _svg_uri_clone (uri, &new_uri);
        if (status)
            return status;
    } else {
        status = _svg_create_empty_uri (&temp_new_uri);
        if (status)
            return status;

        result = uriAddBaseUriA (&temp_new_uri->uri, &uri->uri, &base_uri->uri);
        if (result) {
            status = _svg_convert_uriparser_error (result);
            goto fail;
        }

        status = _svg_uri_to_string (temp_new_uri, &uri_str);
        if (status)
            goto fail;

        status = _svg_create_uri (uri_str, &new_uri);
        if (status)
            goto fail;

        free (uri_str);
        _svg_destroy_uri (temp_new_uri);
    }

    *abs_uri = new_uri;

    return SVG_STATUS_SUCCESS;

  fail:
    if (uri_str != NULL)
        free (uri_str);
    if (temp_new_uri != NULL)
        _svg_destroy_uri (temp_new_uri);
    if (new_uri != NULL)
        _svg_destroy_uri (new_uri);

    return status;
}

svg_status_t
_svg_uri_create_relative (svg_uri_t *base_uri, svg_uri_t *uri, svg_uri_t **rel_uri)
{
    svg_status_t status;
    svg_uri_t *temp_new_uri = NULL;
    svg_uri_t *new_uri = NULL;
    int result;
    char *uri_str = NULL;

    if (_svg_uri_is_data_scheme (base_uri) || _svg_uri_is_data_scheme (uri) ||
        _svg_uri_is_relative (base_uri)) {
        return SVG_STATUS_INVALID_CALL;
    }

    if (_svg_uri_is_relative (uri)) {
        status = _svg_uri_clone (uri, &new_uri);
        if (status)
            return status;
    } else {
        status = _svg_create_empty_uri (&temp_new_uri);
        if (status)
            return status;

        result = uriRemoveBaseUriA (&temp_new_uri->uri, &uri->uri, &base_uri->uri, 0);
        if (result) {
            status = _svg_convert_uriparser_error (result);
            goto fail;
        }

        if (!_svg_uri_is_relative (temp_new_uri)) {
            status = SVG_STATUS_NO_RELATIVE_URI;
            goto fail;
        }

        status = _svg_uri_to_string (temp_new_uri, &uri_str);
        if (status)
            goto fail;

        status = _svg_create_uri (uri_str, &new_uri);
        if (status)
            goto fail;

        free (uri_str);
        _svg_destroy_uri (temp_new_uri);
    }

    *rel_uri = new_uri;

    return SVG_STATUS_SUCCESS;

  fail:
    if (uri_str != NULL)
        free (uri_str);
    if (temp_new_uri != NULL)
        _svg_destroy_uri (temp_new_uri);
    if (new_uri != NULL)
        _svg_destroy_uri (new_uri);

    return status;
}

int
_svg_uri_equals (const svg_uri_t *left, const svg_uri_t *right)
{
    if (left == right)
        return 1;
    else if ((left == NULL || right == NULL) && left != right)
        return 0;
    else if (_svg_uri_is_data_scheme (left) != _svg_uri_is_data_scheme (right))
        return 0;

    if (_svg_uri_is_data_scheme (left))
        return strcmp (left->uri_str, right->uri_str) == 0;
    else
        return uriEqualsUriA (&left->uri, &right->uri);
}

int
_svg_uri_is_data_scheme (const svg_uri_t *uri)
{
    return uri->data_offset > 0;
}

int
_svg_uri_is_file (const svg_uri_t *uri)
{
    return !_svg_uri_is_data_scheme (uri) &&
        uri->uri.scheme.first != NULL && uri->uri.scheme.afterLast != NULL &&
        strncmp (uri->uri.scheme.first, "file",
                 uri->uri.scheme.afterLast - uri->uri.scheme.first) == 0;
}

int
_svg_uri_is_relative (const svg_uri_t *uri)
{
    return !_svg_uri_is_data_scheme (uri) && uri->uri.scheme.first == NULL;
}

int
_svg_uri_is_empty_path (const svg_uri_t *uri)
{
    return !_svg_uri_is_data_scheme (uri) && uri->uri.pathHead == NULL;
}

int
_svg_string_is_abs_filename (const char *uri_str)
{
#if defined(_WIN32)
    return ((uri_str[0] >= 'a' && uri_str[0] <= 'z') || (uri_str[0] >= 'A' && uri_str[0] <= 'Z')) &&
           uri_str[1] == ':';
#else
    return uri_str[0] == '/';
#endif
}

svg_status_t
_svg_uri_to_string (const svg_uri_t *uri, char **uri_str)
{
    char *new_uri_str = NULL;
    int result;
    int uri_str_len;

    if (uri == NULL) {
        new_uri_str = (char *) malloc (1);
        if (new_uri_str == NULL)
            return SVG_STATUS_NO_MEMORY;
        new_uri_str[0] = '\0';
    } else if (_svg_uri_is_data_scheme (uri)) {
        new_uri_str = strdup (uri->uri_str);
        if (new_uri_str == NULL)
            return SVG_STATUS_NO_MEMORY;
    } else {
        result = uriToStringCharsRequiredA (&uri->uri, &uri_str_len);
        if (result)
            return _svg_convert_uriparser_error (result);

        new_uri_str = (char *) malloc (uri_str_len + 1);
        if (new_uri_str == NULL)
            return SVG_STATUS_NO_MEMORY;

        result = uriToStringA (new_uri_str, &uri->uri, uri_str_len + 1, NULL);
        if (result) {
            free (new_uri_str);
            return _svg_convert_uriparser_error (result);
        }
    }

    *uri_str = new_uri_str;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_uri_to_filename (const svg_uri_t *uri, char **filename)
{
    char *new_filename;
    char *uri_str;
    int result;
    svg_status_t status;

    if (!_svg_uri_is_file (uri) && !_svg_uri_is_relative (uri))
        return SVG_STATUS_INTERNAL_ERROR;


    status = _svg_uri_to_string (uri, &uri_str);
    if (status)
        return status;

    new_filename = (char *) malloc (strlen (uri_str) + 1);
    if (new_filename == NULL) {
        free (uri_str);
        return SVG_STATUS_NO_MEMORY;
    }
#if defined(_WIN32)
    result = uriUriStringToWindowsFilenameA (uri_str, new_filename);
#else
    result = uriUriStringToUnixFilenameA (uri_str, new_filename);
#endif
    if (result) {
        free (new_filename);
        free (uri_str);
        return SVG_STATUS_INVALID_VALUE;
    }

    free (uri_str);

    *filename = new_filename;

    return SVG_STATUS_SUCCESS;
}

const char *
_svg_uri_media_type (const svg_uri_t *uri)
{
    return uri->media_type;
}

void
_svg_uri_data (const svg_uri_t *uri, const char **data, int *base64)
{
    if (uri->data_offset == 0) {
        *data = NULL;
        *base64 = 0;
        return;
    }

    *data = uri->uri_str + uri->data_offset;
    *base64 = uri->base64;
}

svg_status_t
_svg_unescape_uri_string (char *uri_str, size_t *len)
{
    const char *term_zero;

    if (uri_str == NULL) {
        *len = 0;
    } else {
        term_zero = uriUnescapeInPlaceA (uri_str);
        *len = term_zero - uri_str;
    }

    return SVG_STATUS_SUCCESS;
}

void
_svg_uri_print (const svg_uri_t *uri)
{
    svg_status_t status;
    char *uri_str;

    status = _svg_uri_to_string (uri, &uri_str);
    if (status)
        return;

    printf ("%s", uri_str);

    free (uri_str);
}




int
svg_is_data_scheme_uri (const char *uri_str)
{
    return uri_str != NULL && strncmp (uri_str, "data:", 5) == 0;
}

svg_status_t
svg_decode_data_scheme_uri (const char *uri_str, char **media_type_ref,
                            unsigned char **buffer_ref, size_t * buffer_size_ref)
{
    svg_uri_t *uri;
    char *media_type = NULL;
    char *buffer;
    size_t buffer_size;
    svg_status_t status;

    if (!svg_is_data_scheme_uri (uri_str))
        return SVG_STATUS_INVALID_CALL;


    status = _svg_create_uri (uri_str, &uri);
    if (status)
        return status;

    status = _svg_resource_read_stream_to_buffer (uri, &buffer, &buffer_size, -1);
    if (status) {
        _svg_destroy_uri (uri);
        return status;
    }

    if (_svg_uri_media_type (uri) != NULL) {
        media_type = strdup (_svg_uri_media_type (uri));
        if (media_type == NULL) {
            _svg_destroy_uri (uri);
            if (buffer != NULL)
                free (buffer);
            return SVG_STATUS_NO_MEMORY;
        }
    }

    _svg_destroy_uri (uri);

    *media_type_ref = media_type;
    *buffer_ref = (unsigned char *) buffer;
    *buffer_size_ref = buffer_size;

    return SVG_STATUS_SUCCESS;
}

