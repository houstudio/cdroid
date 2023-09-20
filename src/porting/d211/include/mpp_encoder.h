/*
* Copyright (C) 2020-2023 ArtInChip Technology Co. Ltd
*
*  author: <qi.xu@artinchip.com>
*  Desc: mpp encoder
*/

#ifndef __MPP_ENCODER_H__
#define __MPP_ENCODER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <video/mpp_types.h>

/**
 * mpp_encode_jpeg - encode one jpeg frame
 * @frame: the frame need be encoded
 * @quality: encode quality, 1~100
 * @dma_buf_fd: fd of output dma_buf to save jpeg data
 * @buf_len: the length of output buffer
 * @len: the length of encoded jpeg data
 */
int mpp_encode_jpeg(struct mpp_frame* frame, int quality, int dma_buf_fd, int buf_len, int *len);

#ifdef __cplusplus
}
#endif

#endif
