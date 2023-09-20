/*
* Copyright (C) 2020-2022 Artinchip Technology Co. Ltd
*
*  author: qi.xu@artinchip.com
*  Desc: mpp_decoder interface
*/

#ifndef _MPP_DECODER_H_
#define _MPP_DECODER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpp_dec_type.h"

struct mpp_decoder;

/**
 * struct decode_config - decode congig
 * @pix_fmt: pixel format of output frame
 * @bitstream_buffer_size: buffer size of bitstream
 * @packet_count: packet number
 * @extra_frame_num: extra frame number for smoothing playing
 */
struct decode_config {
	enum mpp_pixel_format pix_fmt;	// output pixel format
	int bitstream_buffer_size;	// bitstream buffer size in pm
	int packet_count;		// packet number in pm
	int extra_frame_num;		// extra frame number in fm
};

/**
 * mpp_decoder_create - create decoder (h264/jpeg/png ...)
 * @type: decoder type
 */
struct mpp_decoder* mpp_decoder_create(enum mpp_codec_type type);

/**
 * mpp_decoder_destory - destory decoder
 * @decoder: mpp_decoder context
 */
void mpp_decoder_destory(struct mpp_decoder* decoder);

/**
 * mpp_decoder_init - init decoder
 * @decoder: mpp_decoder context
 * @config: configuration of decoder
 */
int mpp_decoder_init(struct mpp_decoder *decoder, struct decode_config *config);

/**
 * mpp_decoder_decode - decode one packet
 * @decoder: mpp_decoder context
 */
int mpp_decoder_decode(struct mpp_decoder* decoder);

/**
 * mpp_decoder_get_packet - get an empty packet from decoder
 * @decoder: mpp_decoder context
 * @packet: the packet from mpp_decoder
 * @size: data size of this packet
 */
int mpp_decoder_get_packet(struct mpp_decoder* decoder, struct mpp_packet* packet, int size);

/**
 * mpp_decoder_put_packet - put the packet to decoder
 * @decoder: mpp_decoder context
 * @packet: the packet filled by application
 */
int mpp_decoder_put_packet(struct mpp_decoder* decoder, struct mpp_packet* packet);

/**
 * mpp_decoder_get_frame - get a display frame from decoder
 * @decoder: mpp_decoder context
 * @frame: one decoded frame to be display, defined in linux/include/uapi/video/mpp_types.h
 */
int mpp_decoder_get_frame(struct mpp_decoder* decoder, struct mpp_frame* frame);

/**
 * mpp_decoder_put_frame - return the frame to decoder
 * @decoder: mpp_decoder context
 * @frame: displayed frame to be return, defined in linux/include/uapi/video/mpp_types.h
 */
int mpp_decoder_put_frame(struct mpp_decoder* decoder, struct mpp_frame* frame);

/**
 * mpp_decoder_control - send a control command (like, set/get parameter) to mpp_decoder
 * @decoder: mpp_decoder context
 * @cmd: command name, see mpp_dec_type.h
 * @param: command data
 */
int mpp_decoder_control(struct mpp_decoder* decoder, int cmd, void* param);

/**
 * mpp_decoder_reset - reset mpp_decoder
 * @decoder: mpp_decoder context
 */
int mpp_decoder_reset(struct mpp_decoder* decoder);

#ifdef __cplusplus
}
#endif

#endif
