/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
/*
 * Definitions for the ArtinChip Graphic Engine driver
 *
 * Copyright (C) 2020-2021 ArtinChip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#ifndef _UAPI__ARTINCHIP_GE_H_
#define _UAPI__ARTINCHIP_GE_H_

#include <linux/ioctl.h>
#include "mpp_types.h"

#if defined(__cplusplus)
extern "C" {
#endif

/*
 * enum ge_pd_mode - graphics engine Porter/Duff alpha blending rules
 *
 * pixel = (source * fs + destination * fd)
 * sa = source alpha
 * da = destination alpha
 *
 * @GE_PD_NONE:           fs: sa      fd: 1.0-sa (defaults)
 * @GE_PD_CLEAR:          fs: 0.0     fd: 0.0
 * @GE_PD_SRC:            fs: 1.0     fd: 0.0
 * @GE_PD_SRC_OVER:       fs: 1.0     fd: 1.0-sa
 * @GE_PD_DST_OVER:       fs: 1.0-da  fd: 1.0
 * @GE_PD_SRC_IN:         fs: da      fd: 0.0
 * @GE_PD_DST_IN:         fs: 0.0     fd: sa
 * @GE_PD_SRC_OUT:        fs: 1.0-da  fd: 0.0
 * @GE_PD_DST_OUT:        fs: 0.0     fd: 1.0-sa
 * @GE_PD_SRC_ATOP:       fs: da      fd: 1.0-sa
 * @GE_PD_DST_ATOP:       fs: 1.0-da  fd: sa
 * @GE_PD_ADD:            fs: 1.0     fd: 1.0
 * @GE_PD_XOR:            fs: 1.0-da  fd: 1.0-sa
 * @GE_PD_DST:            fs: 0.0     fd: 1.0
 */
enum ge_pd_rules {
	GE_PD_NONE           =  0,
	GE_PD_CLEAR          =  1,
	GE_PD_SRC            =  2,
	GE_PD_SRC_OVER       =  3,
	GE_PD_DST_OVER       =  4,
	GE_PD_SRC_IN         =  5,
	GE_PD_DST_IN         =  6,
	GE_PD_SRC_OUT        =  7,
	GE_PD_DST_OUT        =  8,
	GE_PD_SRC_ATOP       =  9,
	GE_PD_DST_ATOP       = 10,
	GE_PD_ADD            = 11,
	GE_PD_XOR            = 12,
	GE_PD_DST            = 13,
};

/**
 * struct ge_ctrl - ge ctrl functions
 * @alpha_en
 *  0: enable Porter/Duff alpha blending
 *  1: disable Porter/Duff alpha blending
 * @alpha_rules: Porter/Duff alpha blending rules
 * @src_alpha_mode: source alpha mode
 *  0: pixel alpha mode(src_alpha = src_pixel_alpha)
 *  1: global alpha mode(src_alpha = src_global_alpha)
 *  2: mixded alpha mode(src_alpha = src_pixel_alpha * src_global_alpha / 255)
 * @src_global_alpha: source global alpha value (0~255)
 *  used by global alpha mode and mixded alpha mode
 * @dst_alpha_mode: destination alpha mode
 *  0: pixel alpha mode(dst_alpha = dst_pixel_alpha)
 *  1: global alpha mode(dst_alpha = dst_global_alpha)
 *  2: mixded alpha mode(dst_alpha = dst_pixel_alpha * dst_global_alpha / 255)
 * @dst_global_alpha: destination global alpha value (0~255)
 *  used by global alpha mode and mixed alpha mode
 * @ck_en
 *  0: disable color key
 *  1: enable color key
 * @ck_value: rgb value of color key to match the source pixels
 *  bit[31:24]: reserved
 *  bit[23:16]: R value
 *  bit[15:8]: G value
 *  bit[7:0]: B value
 * @dither_en(Not supported by IOC_GE_ROTATE)
 *  0: disable dither
 *  1: enable dither
 * @flags: the flags of some functions, such as scan order, src H/V flip
 *         and src 90/180/270 degree rotation, the H flip, V flip
 *         and rotation can be enabled at the same time, the effect
 *         of flip is in front of rotation, only supported by IOC_GE_BITBLT
 *         the flags was defined in mpp_types.h
 */
struct ge_ctrl {
	unsigned int       alpha_en;
	enum ge_pd_rules   alpha_rules;
	unsigned int       src_alpha_mode;
	unsigned int       src_global_alpha;
	unsigned int       dst_alpha_mode;
	unsigned int       dst_global_alpha;
	unsigned int       ck_en;
	unsigned int       ck_value;
	unsigned int       dither_en;
	unsigned int       flags;
};

/*
 * enum ge_fillrect_type - the ge fill rectangle types:
 *
 * GE_NO_GRADIENT: No gradient is used, only use start_color to
 *                  fill rectangle, ignore end_color
 * GE_H_LINEAR_GRADIENT: Interpolates colors between start_color
 *                  and end_color in the horizontal direction
 *                  form left to right
 * GE_V_LINEAR_GRADIENT: Interpolates colors between start_color and
 *                  end_color in the vertical direction from top to
 *                  buttom
 */
enum ge_fillrect_type {
	GE_NO_GRADIENT         = 0,
	GE_H_LINEAR_GRADIENT   = 1,
	GE_V_LINEAR_GRADIENT   = 2,
};

/**
 * struct ge_fillrect - ge fill rectangle
 * @type: fill rect type
 * @start_color: start color(32 bits)
 * bit[31:24] alpha value
 * bit[23:16] r value
 * bit[15:8]  g value
 * bit[7:0]   b value
 * @end_color: end color(32 bits)
 * bit[31:24] alpha value
 * bit[23:16] r value
 * bit[15:8]  g value
 * bit[7:0]   b value
 * @dst_buf: the destination buffer
 * @ctrl: ge ctrl functions
 */
struct ge_fillrect {
	enum ge_fillrect_type  type;
	unsigned int           start_color;
	unsigned int           end_color;
	struct mpp_buf         dst_buf;
	struct ge_ctrl         ctrl;
};

/**
 * struct ge_bitblt - ge bitblt
 * @src_buf: the source buffer
 * @dst_buf: the destination buffer
 * @ctrl: ge ctrl functions
 */
struct ge_bitblt {
	struct mpp_buf   src_buf;
	struct mpp_buf   dst_buf;
	struct ge_ctrl   ctrl;
};

/**
 * struct ge_rotation - ge rotation
 * @src_buf: the source buffer
 * @dst_buf: the destination buffer
 * @src_rot_center: left-top x/y coordinate of src center
 * @dst_rot_center: left-top x/y coordinate of dst center
 * @angle_sin: 2.12 fixed point, the sin value of rotation angle
 * @angle_cos: 2.12 fixed point, the cos value of rotation angle
 * @ctrl: ge ctrl functions
 */
struct ge_rotation {
	struct mpp_buf        src_buf;
	struct mpp_buf        dst_buf;
	struct mpp_point      src_rot_center;
	struct mpp_point      dst_rot_center;
	int                   angle_sin;
	int                   angle_cos;
	struct ge_ctrl        ctrl;
};


enum ge_mode {
	GE_MODE_NORMAL,
	GE_MODE_CMDQ,
};

#define IOC_TYPE_GE                'G'

#define IOC_GE_VERSION             _IOR(IOC_TYPE_GE, 0x00, unsigned int)

#define IOC_GE_MODE                _IOR(IOC_TYPE_GE, 0x01, enum ge_mode)

#define IOC_GE_FILLRECT            _IOW(IOC_TYPE_GE, 0x02, \
					struct ge_fillrect)

#define IOC_GE_BITBLT              _IOW(IOC_TYPE_GE, 0x03, \
					struct ge_bitblt)

#define IOC_GE_ROTATE              _IOW(IOC_TYPE_GE, 0x04, \
					struct ge_rotation)

#define IOC_GE_SYNC                _IO(IOC_TYPE_GE, 0x10)

#define IOC_GE_CMD_BUF_SIZE        _IOR(IOC_TYPE_GE, 0x11, unsigned int)

#define IOC_GE_ADD_DMA_BUF         _IOWR(IOC_TYPE_GE, 0x12, \
					struct dma_buf_info)

#define IOC_GE_RM_DMA_BUF          _IOW(IOC_TYPE_GE, 0x13, \
					struct dma_buf_info)

#if defined(__cplusplus)
}
#endif

#endif /* _UAPI__ARTINCHIP_GE_H_ */

