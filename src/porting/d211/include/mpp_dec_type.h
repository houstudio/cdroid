/*
* Copyright (C) 2020-2022 Artinchip Technology Co. Ltd
*
*  author: <qi.xu@artinchip.com>
*  Desc: mpp_dec_type
*/

#ifndef MPP_DEC_TYPE_H
#define MPP_DEC_TYPE_H

#include <stdint.h>
#include <stddef.h>
// the header file is in dir linux/include/uapi/video
#include <video/mpp_types.h>

#ifndef u8
	typedef uint8_t		u8;
#endif
#ifndef u16
	typedef uint16_t	u16;
#endif
#ifndef u32
	typedef uint32_t	u32;
#endif
#ifndef u64
	typedef uint64_t	u64;
#endif

#ifndef s8
	typedef int8_t		s8;
#endif
#ifndef s16
	typedef int16_t		s16;
#endif
#ifndef s32
	typedef int32_t		s32;
#endif
#ifndef s64
	typedef int64_t		s64;
#endif

#define MPP_MAX(a, b) ((a)>(b)? (a) : (b))
#define MPP_MIN(a, b) ((a)<(b)? (a) : (b))

/* flags for mpp_frame */
#define FRAME_FLAG_EOS		(1 << 0)
#define FRAME_FLAG_ERROR	(1 << 1)

/* packet flag */
#define PACKET_FLAG_EOS		(0x00000001)
#define PACKET_FLAG_EXTRA_DATA	(0x00000002)

/**
 * struct mpp_packet - mpp packet buffer
 * @data: mpp packet virtual address
 * @size: mpp packet buffer size
 * @pts: pts of packet
 * @flags: buffer flags
 */
struct mpp_packet {
	void *data;
	int size;
	long long pts;
	unsigned int flag;
};

/**
 * struct mpp_scale - mpp scale ratio
 * @hor_scale: horizontal scale ratio
 * @ver_scale: vertical scale ration
 * (1- 1/2 scale; 2 - 1/4 scale; 3 - 1/8 scale)
 */
struct mpp_scale_ratio {
	int hor_scale;
	int ver_scale;
};

/**
 * struct mpp_dec_crop_info - crop info
 * @crop_x: start pos in x for crop
 * @crop_y: start pos in y for crop
 * @crop_width: width of crop window
 * @crop_height: height of crop window
 */
struct mpp_dec_crop_info {
	int crop_x;
	int crop_y;
	int crop_width;
	int crop_height;
};

/**
 * struct mpp_dec_output_pos - start pos of output
 * @output_pos_x: start pos in x for output
 * @output_pos_y: start pos in y for output
 */
struct mpp_dec_output_pos {
	int output_pos_x;
	int output_pos_y;
};

enum mpp_codec_type {
	MPP_CODEC_VIDEO_DECODER_H264 = 0x1000,         // decoder
	MPP_CODEC_VIDEO_DECODER_MJPEG,
	MPP_CODEC_VIDEO_DECODER_PNG,

	MPP_CODEC_VIDEO_ENCODER_H264 = 0x2000,         // encoder
};

enum mpp_dec_cmd {
	MPP_DEC_INIT_CMD_SET_EXT_FRAME_ALLOCATOR,            // frame buffer allocator
	MPP_DEC_INIT_CMD_SET_ROT_FLIP_FLAG,
	MPP_DEC_INIT_CMD_SET_SCALE,
	MPP_DEC_INIT_CMD_SET_CROP_INFO,
	MPP_DEC_INIT_CMD_SET_OUTPUT_POS,
};

enum mpp_dec_errno {
	// if mpp_dec_get_packet return DEC_NO_EMPTY_PACKET, we should wait a minute then call again
	// it happen in send bitstream fast than decode
	DEC_NO_EMPTY_PACKET			= 4, // no packet in empty list

	// if decode return DEC_NO_READY_PACKET, we should wait a minute then call again
	// it happen in decode faster than send bitstream
	DEC_NO_READY_PACKET			= 3, //

	// if decode return DEC_NO_EMPTY_FRAME, we should wait a minute then call again
	// it happen in decode faster than render
	DEC_NO_EMPTY_FRAME 			= 2, //

	// if mpp_dec_get_frame return DEC_NO_RENDER_FRAME, we should wait a minute then call again
	// it happen in render faster than decode
	DEC_NO_RENDER_FRAME 		= 1, //

	DEC_OK					= 0,
	// decode
	DEC_ERR_NOT_SUPPORT 			= -1,

	DEC_ERR_NULL_PTR			= -2,

	// if frame manager not create, mpp_dec_get_frame return DEC_ERR_FM_NOT_CREATE.
	// app should wait a minute to get frame
	DEC_ERR_FM_NOT_CREATE			= -3,
};

#endif
