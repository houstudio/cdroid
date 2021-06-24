/* libsvg-cairo - Render SVG documents using the cairo library
 *
 * Copyright © 2002 University of Southern California
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Carl D. Worth <cworth@isi.edu>
 */

#include <stdlib.h>
#include <string.h>

#include "svg-cairo-internal.h"


svg_cairo_status_t
_svg_cairo_state_create (svg_cairo_state_t **state)
{
    *state = malloc (sizeof (svg_cairo_state_t));
    if (*state == NULL)
	return SVG_CAIRO_STATUS_NO_MEMORY;

    _svg_cairo_state_init (*state);

    return SVG_CAIRO_STATUS_SUCCESS;
}

svg_cairo_status_t
_svg_cairo_state_init (svg_cairo_state_t *state)
{
    /* trust libsvg to set all of these to reasonable defaults:
    state->fill_paint;
    state->stroke_paint;
    state->fill_opacity;
    state->stroke_opacity;
    */
    state->child_surface = NULL;
    state->saved_cr = NULL;

#if HAVE_PANGOCAIRO
    state->font_description = pango_font_description_new ();
#else
    state->font_family = SVG_CAIRO_FONT_FAMILY_DEFAULT;

    state->font_size = 1.0;
    state->font_style = SVG_FONT_STYLE_NORMAL;
    state->font_weight = 400;
    state->font_dirty = 1;
#endif

    state->dash = NULL;
    state->num_dashes = 0;
    state->dash_offset = 0;

    state->opacity = 1.0;

    state->bbox = 0;

    state->text_anchor = SVG_TEXT_ANCHOR_START;

    state->next = NULL;

    return SVG_CAIRO_STATUS_SUCCESS;
}

svg_cairo_status_t
_svg_cairo_state_init_copy (svg_cairo_state_t *state, const svg_cairo_state_t *other)
{
    _svg_cairo_state_deinit (state);

    if (other == NULL)
	return _svg_cairo_state_init (state);

    *state = *other;

    /* We don't need our own child_surface or saved cr at this point. */
    state->child_surface = NULL;
    state->saved_cr = NULL;

#if HAVE_PANGOCAIRO
    state->font_description = pango_font_description_copy (other->font_description);
#endif

    state->viewport_width = other->viewport_width;
    state->viewport_height = other->viewport_height;
    state->view_box_width = other->view_box_width;
    state->view_box_height = other->view_box_height;

    return SVG_CAIRO_STATUS_SUCCESS;
}

svg_cairo_status_t
_svg_cairo_state_deinit (svg_cairo_state_t *state)
{
    if (state->child_surface) {
	cairo_surface_destroy(state->child_surface);
	state->child_surface = NULL;
    }

    if (state->saved_cr) {
	cairo_destroy(state->saved_cr);
	state->saved_cr = NULL;
    }

#if HAVE_PANGOCAIRO
    if (state->font_description) {
	pango_font_description_free (state->font_description);
	state->font_description = NULL;
    }
#endif

    state->next = NULL;

    return SVG_CAIRO_STATUS_SUCCESS;
}

svg_cairo_status_t
_svg_cairo_state_destroy (svg_cairo_state_t *state)
{
    _svg_cairo_state_deinit (state);

    free (state);

    return SVG_CAIRO_STATUS_SUCCESS;
}

svg_cairo_state_t *
_svg_cairo_state_push (svg_cairo_state_t *state)
{
    svg_cairo_state_t *new;

    _svg_cairo_state_create (&new);
    if (new == NULL)
	return NULL;

    _svg_cairo_state_init_copy (new, state);

    new->next = state;

    return new;
}

svg_cairo_state_t *
_svg_cairo_state_pop (svg_cairo_state_t *state)
{
    svg_cairo_state_t *next;

    if (state == NULL)
	return NULL;

    next = state->next;

    _svg_cairo_state_destroy (state);

    return next;
}
