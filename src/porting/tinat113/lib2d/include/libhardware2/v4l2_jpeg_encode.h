#ifndef _V4L2_JPEG_ENCODE_H_
#define _V4L2_JPEG_ENCODE_H_

#include <stdbool.h>
#include <linux/videodev2.h>

#ifdef __cplusplus
extern "C" {
#endif

struct v4l2_jpeg_encoder_config {
    const char *video_path;
    unsigned int width;
    unsigned int height;
    unsigned int line_length;
    unsigned int input_fmt;
    unsigned int quality;
};

struct v4l2_jpeg_encoder;

struct v4l2_jpeg_encoder *v4l2_jpeg_encoder_open(struct v4l2_jpeg_encoder_config *config);

void *v4l2_jpeg_encoder_work(struct v4l2_jpeg_encoder *encoder, void *input_mem,
                unsigned int *ouput_size);

int v4l2_jpeg_encoder_close(struct v4l2_jpeg_encoder *encoder);

#ifdef __cplusplus
}
#endif

#endif
