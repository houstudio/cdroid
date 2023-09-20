/*
* Copyright (C) 2020-2022 Artinchip Technology Co. Ltd
*
*  author: qi.xu@artinchip.com
*  Desc: frame allocator
*/

#ifndef FRAME_ALLOCATOR_H
#define FRAME_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "mpp_dec_type.h"

struct frame_allocator {
	struct alloc_ops *ops;
};

struct alloc_ops {
	int (*alloc_frame_buffer)(struct frame_allocator *p, struct mpp_frame *frame,
		int width, int height, enum mpp_pixel_format format);
	int (*free_frame_buffer)(struct frame_allocator *p, struct mpp_frame *frame);
	int (*close_allocator)(struct frame_allocator *p);
};

#ifdef __cplusplus
}
#endif

#endif
