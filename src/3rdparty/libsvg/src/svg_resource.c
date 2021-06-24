/* svg_resource.c: File and network resource handling functions

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

#include <errno.h>
#if !defined(_WIN32)
#  include <unistd.h>
#endif

#if HAVE_LIBCURL
#  include <curl/curl.h>
#endif


#define FILE_RESOURCE_BUFFER_SIZE       (4 * 1024)


#if HAVE_LIBCURL
typedef struct {
    void *cb_closure;
    svg_resource_read_stream_cb_t *read_cb;
    svg_status_t status;
} svg_curl_write_data_t;
#endif

typedef struct {
    char *buffer;
    size_t buffer_size;
    size_t max_buffer_size;
} svg_read_buffer_data_t;


#if HAVE_LIBCURL
static svg_status_t
_svg_convert_curl_error (int error)
{
    switch (error) {
    case CURLE_OK:
        return SVG_STATUS_SUCCESS;
    case CURLE_OUT_OF_MEMORY:
        return SVG_STATUS_NO_MEMORY;
    case CURLE_URL_MALFORMAT:
        return SVG_STATUS_INVALID_VALUE;
    case CURLE_UNSUPPORTED_PROTOCOL:
    case CURLE_COULDNT_RESOLVE_HOST:
    case CURLE_COULDNT_RESOLVE_PROXY:
    case CURLE_COULDNT_CONNECT:
    case CURLE_REMOTE_FILE_NOT_FOUND:
        return SVG_STATUS_RESOURCE_NOT_FOUND;
    default:
        return SVG_STATUS_IO_ERROR;
    }
}
#endif

static int
_svg_is_base64_char (char c)
{
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z') ||
           (c >= '0' && c <= '9') ||
            c == '+' ||
            c == '/' ||
            c == '=';
}

static char *
_svg_next_base64_char (char *str, char *c)
{
    char *str_ptr = str;

    if (str == NULL) {
        *c = 'A';
        return NULL;
    }

    while (*str_ptr != '\0' && !_svg_is_base64_char (*str_ptr))
        str_ptr++;

    if (*str_ptr == '\0') {
        *c = 'A';
        return NULL;
    }

    *c = *str_ptr;

    str_ptr++;
    return str_ptr;
}

static unsigned char
_svg_decode_base64 (char c)
{
    if (c >= 'A' && c <= 'Z')
        return c - 'A';
    else if (c >= 'a' && c <= 'z')
        return c - 'a' + 26;
    else if (c >= '0' && c <= '9')
        return c - '0' + 52;
    else if (c == '+')
        return 62;
    else
        return 63;
}

static svg_status_t
_svg_decode_data_uri_data (int base64, char *data_in_out, size_t *num_decoded, size_t *data_size)
{
    char *data_in, *decoded_data_in;
    unsigned char *data_out;
    char c1, c2, c3, c4;
    unsigned char b1, b2, b3, b4;
    size_t data_in_len, reduced_data_in_len, unescaped_len;
    svg_status_t status;

    if (base64) {
        data_out = (unsigned char *) data_in_out;
        data_in = data_in_out;
        decoded_data_in = data_in;
        while (data_in != NULL && *data_in != '\0') {
            decoded_data_in = data_in;

            data_in = _svg_next_base64_char (data_in, &c1);
            if (data_in == NULL)
                break;
            data_in = _svg_next_base64_char (data_in, &c2);
            if (data_in == NULL)
                break;
            data_in = _svg_next_base64_char (data_in, &c3);
            if (data_in == NULL)
                break;
            data_in = _svg_next_base64_char (data_in, &c4);
            if (data_in == NULL)
                break;

            decoded_data_in = data_in;

            b1 = _svg_decode_base64 (c1);
            b2 = _svg_decode_base64 (c2);
            b3 = _svg_decode_base64 (c3);
            b4 = _svg_decode_base64 (c4);

            *data_out++ = ((b1 << 2) | (b2 >> 4));
            if (c3 != '=')
                *data_out++ = (((b2 & 0x0f) << 4) | (b3 >> 2));
            if (c4 != '=')
                *data_out++ = (((b3 & 0x03) << 6) | b4);
        }

        *num_decoded = decoded_data_in - data_in_out;
        *data_size = data_out - (unsigned char *) data_in_out;
    } else {
        reduced_data_in_len = 0;
        data_in_len = strlen (data_in_out);
        if (data_in_len > 0 && data_in_out[data_in_len - 1] == '%') {
            data_in_out[data_in_len - 1] = '\0';
            reduced_data_in_len = 1;
        } else if (data_in_len > 1 && data_in_out[data_in_len - 2] == '%') {
            data_in_out[data_in_len - 2] = '\0';
            reduced_data_in_len = 2;
        }

        status = _svg_unescape_uri_string (data_in_out, &unescaped_len);
        if (status)
            return status;

        if (reduced_data_in_len == 1)
            data_in_out[data_in_len - 1] = '%';
        else if (reduced_data_in_len == 2)
            data_in_out[data_in_len - 2] = '%';

        *num_decoded = data_in_len - reduced_data_in_len;
        *data_size = unescaped_len;
    }

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_create_temp_file (char **filename_ref, FILE **file_ref)
{
    char *filename = NULL;
    FILE *file = NULL;
#if !defined(_WIN32)
    int file_id;
#endif

#if defined(_WIN32)
    filename = tmpnam (NULL);
    if (filename == NULL)
        return SVG_STATUS_IO_ERROR;

    file = fopen (filename, "wb");
    if (file == NULL)
        return SVG_STATUS_IO_ERROR;
#else
    filename = strdup ("/tmp/svg2swf_temp.XXXXXX");
    if (filename == NULL)
        return SVG_STATUS_NO_MEMORY;

    file_id = mkstemp (filename);
    if (file_id < 0) {
        free (filename);
        return SVG_STATUS_IO_ERROR;
    }

    file = fdopen (file_id, "wb");
    if (file == NULL) {
        close (file_id);
        free (filename);
        return SVG_STATUS_IO_ERROR;
    }
#endif

    *filename_ref = filename;
    *file_ref = file;

    return SVG_STATUS_SUCCESS;
}

#if HAVE_LIBCURL
static size_t
_svg_curl_write_handler (void *ptr, size_t size, size_t nmemb, void *stream)
{
    svg_curl_write_data_t *data = (svg_curl_write_data_t *) stream;
    svg_status_t status;

    status = (data->read_cb) (data->cb_closure, (char *) ptr, size * nmemb);
    if (status) {
        data->status = status;
        return 0;
    }

    return size * nmemb;
}
#endif

static svg_status_t
_svg_read_buffer_data_cb (void *closure, const char *buffer, size_t buffer_size)
{
    svg_read_buffer_data_t *data = (svg_read_buffer_data_t *) closure;
    char *new_buffer;
    size_t next_buffer_size;

    if (buffer_size == 0)
        return SVG_STATUS_SUCCESS;

    next_buffer_size = data->buffer_size + buffer_size;
    if (next_buffer_size < data->buffer_size || /* overflow */
        (data->max_buffer_size >= 0 && next_buffer_size > data->max_buffer_size))
        return SVG_STATUS_NO_MEMORY;

    new_buffer = realloc (data->buffer, data->buffer_size + buffer_size);
    if (new_buffer == NULL)
        return SVG_STATUS_NO_MEMORY;

    data->buffer = new_buffer;
    memcpy (&data->buffer[data->buffer_size], buffer, buffer_size);
    data->buffer_size += buffer_size;

    return SVG_STATUS_SUCCESS;
}

static svg_status_t
_svg_read_to_temp_file_cb (void *closure, const char *buffer, size_t buffer_size)
{
    FILE *temp_file = (FILE *) closure;
    size_t result;

    if (buffer_size == 0)
        return SVG_STATUS_SUCCESS;

    result = fwrite (buffer, 1, buffer_size, temp_file);
    if (result != buffer_size)
        return SVG_STATUS_IO_ERROR;

    return SVG_STATUS_SUCCESS;
}




svg_status_t
_svg_resource_read_stream (svg_uri_t *uri, void *cb_closure, svg_resource_read_stream_cb_t *read_cb)
{
    svg_status_t status;
    char *filename = NULL;
    size_t num_read;
    FILE *resource_file = NULL;
    char buffer[FILE_RESOURCE_BUFFER_SIZE];
#if HAVE_LIBCURL
    CURL *curl = NULL;
    char *uri_str = NULL;
    CURLcode curl_result;
    svg_curl_write_data_t curl_write_data;
#endif
    int is_temp_file = 0;
    size_t encoded_data_size;
    size_t data_size;
    size_t total_num_read;
    size_t num_decoded;
    const char *uri_data;
    int base64;


    if (_svg_uri_is_data_scheme (uri)) {
        _svg_uri_data (uri, &uri_data, &base64);

        encoded_data_size = strlen (uri_data);
        total_num_read = 0;
        while (total_num_read < encoded_data_size) {
            num_read = FILE_RESOURCE_BUFFER_SIZE - 1;
            if (num_read > encoded_data_size - total_num_read)
                num_read = encoded_data_size - total_num_read;

            strncpy (buffer, uri_data + total_num_read, num_read);
            buffer[num_read] = '\0';

            status = _svg_decode_data_uri_data (base64, buffer, &num_decoded, &data_size);
            if (status)
                return status;

            if (num_decoded == 0)
                break;

            status = (read_cb) (cb_closure, buffer, data_size);
            if (status)
                return status;

            total_num_read += num_decoded;
        }
    }
    /* note: a relative uri is assumed to be a file relative to the current directory */
    else if (_svg_uri_is_file (uri) || _svg_uri_is_relative (uri)) {
        status = _svg_resource_get_access_filename (uri, &filename, &is_temp_file);
        if (status)
            return status;

        resource_file = fopen (filename, "rb");
        if (resource_file == NULL) {
            switch (errno) {
            case ENOMEM:
                status = SVG_STATUS_NO_MEMORY;
                break;
            case ENOENT:
                status = SVG_STATUS_FILE_NOT_FOUND;
                break;
            default:
                status = SVG_STATUS_IO_ERROR;
            }
            goto fail;
        }

        do {
            num_read = fread (buffer, 1, FILE_RESOURCE_BUFFER_SIZE, resource_file);
            if (num_read != FILE_RESOURCE_BUFFER_SIZE) {
                if (!feof (resource_file)) {
                    status = SVG_STATUS_IO_ERROR;
                    goto fail;
                }
            }

            status = (read_cb) (cb_closure, buffer, num_read);
            if (status)
                goto fail;
        }
        while (num_read == FILE_RESOURCE_BUFFER_SIZE);

        fclose (resource_file);
        resource_file = NULL;
        if (is_temp_file)
            remove (filename);
        free (filename);
        filename = NULL;
    }
#if HAVE_LIBCURL
    else {
        curl = curl_easy_init ();
        if (curl == NULL) {
            status = SVG_STATUS_NO_MEMORY;
            goto fail;
        }

        status = _svg_uri_to_string (uri, &uri_str);
        if (status)
            goto fail;

        curl_write_data.cb_closure = cb_closure;
        curl_write_data.read_cb = read_cb;
        curl_write_data.status = SVG_STATUS_SUCCESS;

        curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 1);
        curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1);
        curl_easy_setopt (curl, CURLOPT_WRITEFUNCTION, _svg_curl_write_handler);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, &curl_write_data);
        curl_easy_setopt (curl, CURLOPT_URL, uri_str);

        curl_result = curl_easy_perform (curl);
        if (curl_result) {
            if (curl_write_data.status)
                status = curl_write_data.status;
            else
                status = _svg_convert_curl_error (curl_result);
            goto fail;
        }

        free (uri_str);
        uri_str = NULL;
        curl_easy_cleanup (curl);
        curl = NULL;
    }
#else /* #if HAVE_LIBCURL */
    else {
        status = SVG_STATUS_RESOURCE_NOT_FOUND;
        goto fail;
    }
#endif


    return SVG_STATUS_SUCCESS;

  fail:
    if (resource_file != NULL)
        fclose (resource_file);
    if (filename != NULL) {
        if (is_temp_file)
            remove (filename);
        free (filename);
    }
#if HAVE_LIBCURL
    if (uri_str != NULL)
        free (uri_str);
    if (curl != NULL)
        curl_easy_cleanup (curl);
#endif

    return status;
}

svg_status_t
_svg_resource_read_stream_to_buffer (svg_uri_t *uri, char **buffer_ref, size_t *buffer_size_ref,
                                     size_t max_buffer_size)
{
    svg_read_buffer_data_t read_buffer_data;
    svg_status_t status;

    read_buffer_data.buffer = NULL;
    read_buffer_data.buffer_size = 0;
    read_buffer_data.max_buffer_size = max_buffer_size;

    status = _svg_resource_read_stream (uri, &read_buffer_data, _svg_read_buffer_data_cb);
    if (status) {
        if (read_buffer_data.buffer != NULL)
            free (read_buffer_data.buffer);

        return status;
    }

    *buffer_ref = read_buffer_data.buffer;
    *buffer_size_ref = read_buffer_data.buffer_size;

    return SVG_STATUS_SUCCESS;
}

svg_status_t
_svg_resource_get_access_filename (svg_uri_t *uri, char **filename, int *is_temp_copy)
{
    svg_status_t status;
#if HAVE_LIBCURL
    CURL *curl = NULL;
    CURLcode curl_result;
    char *uri_str;
#endif
    char *temp_filename = NULL;
    FILE *temp_file = NULL;


    if (_svg_uri_is_data_scheme (uri)) {
        status = _svg_create_temp_file (&temp_filename, &temp_file);
        if (status)
            return status;

        status = _svg_resource_read_stream (uri, temp_file, _svg_read_to_temp_file_cb);
        if (status)
            goto fail;

        fclose (temp_file);
        temp_file = NULL;

        *filename = temp_filename;
        *is_temp_copy = 1;
    }
    /* note: a relative uri is assumed to be a file relative to the current directory */
    else if (_svg_uri_is_file (uri) || _svg_uri_is_relative (uri)) {
        status = _svg_uri_to_filename (uri, filename);
        if (status)
            return status;

        *is_temp_copy = 0;
    }
#if HAVE_LIBCURL
    else {
        /* download resource to temporary file */

        curl = curl_easy_init ();
        if (curl == NULL) {
            status = SVG_STATUS_NO_MEMORY;
            goto fail;
        }

        status = _svg_create_temp_file (&temp_filename, &temp_file);
        if (status)
            goto fail;

        status = _svg_uri_to_string (uri, &uri_str);
        if (status)
            goto fail;

        curl_easy_setopt (curl, CURLOPT_NOPROGRESS, 1);
        curl_easy_setopt (curl, CURLOPT_FAILONERROR, 1);
        curl_easy_setopt (curl, CURLOPT_WRITEDATA, temp_file);
        curl_easy_setopt (curl, CURLOPT_URL, uri_str);

        curl_result = curl_easy_perform (curl);
        if (curl_result) {
            status = _svg_convert_curl_error (curl_result);
            goto fail;
        }

        fclose (temp_file);
        temp_file = NULL;
        free (uri_str);
        uri_str = NULL;
        curl_easy_cleanup (curl);
        curl = NULL;

        *filename = temp_filename;
        *is_temp_copy = 1;
    }
#else /* #if HAVE_LIBCURL */
    else {
        status = SVG_STATUS_RESOURCE_NOT_FOUND;
        goto fail;
    }
#endif


    return SVG_STATUS_SUCCESS;

  fail:
    if (temp_filename != NULL) {
        if (temp_file != NULL)
            fclose (temp_file);
        remove (temp_filename);
        free (temp_filename);
    }
#if HAVE_LIBCURL
    if (uri_str != NULL)
        free (uri_str);
    if (curl != NULL)
        curl_easy_cleanup (curl);
#endif

    return status;
}

