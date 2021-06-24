/* cairo - a vector graphics library with display and print output
 *
 * Copyright © 2012 Intel Corporation
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 *
 * The Original Code is the cairo graphics library.
 *
 * The Initial Developer of the Original Code is Chris Wilson
 *
 * Contributor(s):
 *    Chris Wilson <chris@chris-wilson.co.uk>
 */

#include "cairoint.h"
#include "cairo-cdroid.h"

#include "cairo-clip-private.h"
#include "cairo-compositor-private.h"
#include "cairo-default-context-private.h"
#include "cairo-error-private.h"
#include "cairo-image-surface-inline.h"
#include "cairo-pattern-private.h"
#include "cairo-surface-backend-private.h"
#include "cairo-surface-fallback-private.h"

#include <pixman.h>

#include <cdtypes.h>
#include <cdgraph.h>
#include <cdlog.h>

slim_hidden_proto(cairo_cdroid_surface_create);

typedef struct _cairo_cdroid_surface {
    cairo_image_surface_t image;
    HANDLE  cdsurface;
    DWORD userdata;
} cairo_cdroid_surface_t;

static cairo_content_t
_cdroid_format_to_content (int  format)
{
    cairo_content_t content = 0;

    content|=CAIRO_CONTENT_COLOR_ALPHA;
    assert(content);
    return content;
}

static inline pixman_format_code_t
_cdroid_to_pixman_format (int format)
{
    switch (format) {
    case GPF_UNKNOWN: return 0;
    case GPF_ARGB1555: return PIXMAN_a1r5g5b5;
    case GPF_ARGB: return PIXMAN_a8r8g8b8;
    case GPF_ABGR: return PIXMAN_a8b8g8r8;
    case GPF_ARGB4444: return PIXMAN_a4r4g4b4;
    default:return 0;
    }
    return 0;
}

static cairo_surface_t *
_cairo_cdroid_surface_create_similar (void            *abstract_src,
				   cairo_content_t  content,
				   int              width,
				   int              height)
{
    cairo_cdroid_surface_t *other  = abstract_src;
    cairo_surface_t *surface;
    HANDLE cdsurface;
    INT format;
    if (width <= 0 || height <= 0)
	return _cairo_image_surface_create_with_content (content, width, height);

    switch (content) {
    default:
	ASSERT_NOT_REACHED;
    case CAIRO_CONTENT_COLOR_ALPHA:
	format = GPF_ARGB;
	break;
    case CAIRO_CONTENT_COLOR:
	format = GPF_RGB32;
	break;
//    case CAIRO_CONTENT_ALPHA:
//	format = GPF_A8;
	break;
    }

    if (GFXCreateSurface(&cdsurface,width,height,format,0))//other->dfb->CreateSurface (other->dfb, &dsc, &buffer))
	return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_DEVICE_ERROR));

    surface = cairo_cdroid_surface_create (cdsurface);
    return surface;
}

static cairo_status_t
_cairo_cdroid_surface_finish (void *abstract_surface)
{
    cairo_cdroid_surface_t *surface = abstract_surface;
    LOGV("destroy cdsurface %p",surface->cdsurface);
    GFXDestroySurface(surface->cdsurface);
    return _cairo_image_surface_finish (abstract_surface);
}

static cairo_image_surface_t *
_cairo_cdroid_surface_map_to_image (void *abstract_surface,
				 const cairo_rectangle_int_t *extents)
{
    cairo_cdroid_surface_t *surface = abstract_surface;

    if (surface->image.pixman_image == NULL) {
	pixman_image_t *image;
	void *data;
	int pitch;

	if (GFXLockSurface (surface->cdsurface,&data, &pitch))
	    return _cairo_image_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
	image = pixman_image_create_bits (PIXMAN_a8r8g8b8,//surface->image.pixman_format,
					  surface->image.width,
					  surface->image.height,
					  data, pitch);
    LOGV("cdsurface=%p buffer=%p pitch=%d pixman_img=%p",surface->cdsurface,data,pitch,image);
	if (image == NULL) {
	    GFXUnlockSurface(surface->cdsurface);
	    return _cairo_image_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));
	}
	_cairo_image_surface_init (&surface->image, image, surface->image.pixman_format);
    }

    return _cairo_image_surface_map_to_image (&surface->image.base, extents);
}

static cairo_int_status_t
_cairo_cdroid_surface_unmap_image (void *abstract_surface,
				cairo_image_surface_t *image)
{
    cairo_cdroid_surface_t *surface = abstract_surface;
    GFXUnlockSurface(surface->cdsurface);
    return _cairo_image_surface_unmap_image (&surface->image.base, image);
}

static cairo_status_t
_cairo_cdroid_surface_flush (void *abstract_surface,
			  unsigned flags)
{
    cairo_cdroid_surface_t *surface = abstract_surface;

    if (flags)
	return CAIRO_STATUS_SUCCESS;
    LOGV("cdsurface=%p surface->image.pixman_image=%p",surface->cdsurface,surface->image.pixman_image);
    if (surface->image.pixman_image) {
	//GFXUnlockSurface(surface->cdsurface);//surface->dfb_surface->Unlock (surface->dfb_surface);

	pixman_image_unref (surface->image.pixman_image);
	surface->image.pixman_image = NULL;
	surface->image.data = NULL;
    }

    return CAIRO_STATUS_SUCCESS;
}


BOOL _op_is_supported(cairo_operator_t op){
    return op==CAIRO_OPERATOR_OVER;
}

static cairo_bool_t fill_box (cairo_box_t *box, void *data){
    cairo_cdroid_surface_t *ngs = (cairo_cdroid_surface_t *)data;
    cairo_rectangle_int_t rb;
    _cairo_box_round_to_rectangle(box,(cairo_rectangle_int_t*)&rb);
    LOGD("rect=%d,%d-%d,%d color=%X",rb.x,rb.y,rb.width,rb.height,ngs->userdata);
    GFXFillRect(ngs->cdsurface,(GFXRect*)&rb,ngs->userdata);
    return 1;
}
static cairo_int_status_t
_cairo_cdroid_surface_fill (void *abstract_surface,
                        cairo_operator_t op,
                        const cairo_pattern_t *source,
                        const cairo_path_fixed_t *path,
                        cairo_fill_rule_t fill_rule,
                        double tolerance,
                        cairo_antialias_t antialias,
                        const cairo_clip_t *clip)
{
    int i;
    const char *opstr[] = { "CLEAR","SOURCE","OVER","IN","OUT","ATOP","DEST","DEST_OVER","DEST_IN","DEST_OUT","DEST_ATOP", "XOR","ADD", "SATURATE"  };
    cairo_cdroid_surface_t *ngs = (cairo_cdroid_surface_t *) abstract_surface;

    LOGV("q[%p] fill op:%s \n", abstract_surface,opstr[op]);
    goto fallback;
    if ( _op_is_supported (op) && source->type == CAIRO_PATTERN_TYPE_SOLID){
        cairo_color_t*c = &((cairo_solid_pattern_t*)source)->color;
        UINT color=(c->alpha_short>>8)<<24|(c->red_short>>8)<<16|(c->green_short>>8)<<8|(c->blue_short>>8);
         
        cairo_boxes_t boxes;
        _cairo_boxes_init_with_clip (&boxes,(cairo_clip_t*)clip);
        cairo_int_status_t status=_cairo_path_fixed_fill_rectilinear_to_boxes (path,fill_rule, antialias,&boxes);
        ngs->userdata=color;
        if (likely (status == CAIRO_INT_STATUS_SUCCESS)){
            _cairo_boxes_for_each_box(&boxes,fill_box,ngs);   
        }
        _cairo_boxes_fini (&boxes);
        return CAIRO_INT_STATUS_SUCCESS;

    }else  if(source->type == CAIRO_PATTERN_TYPE_SURFACE){
         cairo_surface_pattern_t *spattern = (cairo_surface_pattern_t*)source;
         cairo_surface_t *surface = spattern->surface;
         cairo_image_surface_t*isurf;
         void*image_extra;
         _cairo_surface_acquire_source_image (surface, &isurf, &image_extra);  
         _cairo_surface_release_source_image(surface,isurf,image_extra);
         LOGV("op=%d size=%dx%d",op,cairo_image_surface_get_width(surface),cairo_image_surface_get_height(surface)); 
         return CAIRO_INT_STATUS_SUCCESS;
    }
fallback:
    return _cairo_surface_fallback_fill (abstract_surface, op, source, path, fill_rule, tolerance, antialias, clip);
}

static cairo_surface_backend_t
_cairo_cdroid_surface_backend = {
    CAIRO_SURFACE_TYPE_CDROID, /*type*/
    _cairo_cdroid_surface_finish, /*finish*/
    _cairo_default_context_create,

    _cairo_cdroid_surface_create_similar,/*create_similar*/
    NULL, /* create similar image */
    _cairo_cdroid_surface_map_to_image,
    _cairo_cdroid_surface_unmap_image,

    _cairo_surface_default_source,
    _cairo_surface_default_acquire_source_image,
    _cairo_surface_default_release_source_image,
    NULL,

    NULL, /* copy_page */
    NULL, /* show_page */

    _cairo_image_surface_get_extents,
    _cairo_image_surface_get_font_options,

    _cairo_cdroid_surface_flush,
    NULL, /* mark_dirty_rectangle */

    _cairo_surface_fallback_paint,
    _cairo_surface_fallback_mask,
    _cairo_surface_fallback_stroke,
    _cairo_cdroid_surface_fill,
    NULL, /* fill-stroke */
    _cairo_surface_fallback_glyphs,
};

cairo_surface_t *cairo_cdroid_surface_create (HANDLE cdsurface)
{
    cairo_cdroid_surface_t *surface;
    pixman_format_code_t pixman_format;
    int width, height;
    int format=GPF_ARGB;
    
    GFXGetSurfaceInfo(cdsurface,&width,&height,&format);
    LOGV("cdsurface=%p,size=%dx%d",cdsurface,width,height);
    pixman_format =_cdroid_to_pixman_format (format);
    if (! pixman_format_supported_destination (pixman_format))
        return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_INVALID_FORMAT));

    surface = calloc (1, sizeof (cairo_cdroid_surface_t));
    if (surface == NULL)
        return _cairo_surface_create_in_error (_cairo_error (CAIRO_STATUS_NO_MEMORY));

    /* XXX dfb -> device */
    _cairo_surface_init (&surface->image.base,
                         &_cairo_cdroid_surface_backend,
			 NULL, /* device */
			 _cdroid_format_to_content (format),
			 FALSE); /* is_vector */

    surface->image.pixman_format = pixman_format;
    surface->image.format = _cairo_format_from_pixman_format (pixman_format);

    surface->image.width = width;
    surface->image.height = height;
    surface->image.depth = PIXMAN_FORMAT_DEPTH(pixman_format);

    surface->cdsurface = cdsurface;

    return &surface->image.base;
}

HANDLE cairo_cdroid_surface_get_surface (cairo_surface_t *surface)
{
    if (surface->backend->type == CAIRO_SURFACE_TYPE_CDROID)
        return ((cairo_cdroid_surface_t*)surface)->cdsurface;

    return 0;
}

slim_hidden_def(cairo_cdroid_surface_create);
