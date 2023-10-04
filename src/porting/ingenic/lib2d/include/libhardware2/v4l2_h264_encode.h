#ifndef _V4L2_H264_ENCODE_H_
#define _V4L2_H264_ENCODE_H_

#include <stdbool.h>
#include <linux/videodev2.h>
#include <linux/v4l2-controls.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct v4l2_h264_encoder_config {
    const char *video_path;
    unsigned int width;
    unsigned int height;
    unsigned int line_length;
    unsigned int input_fmt;
    unsigned int gop_size;
    unsigned int bitrate;
};

struct v4l2_h264_encoder;

struct v4l2_h264_encoder *v4l2_h264_encoder_open(struct v4l2_h264_encoder_config *config);

void *v4l2_h264_encoder_work(struct v4l2_h264_encoder *encoder, void *input_mem,
                unsigned int *ouput_size);

/**
 * 要求input_mem需要物理上连续，否则与使用v4l2_h264_encoder_work一致
 **/
void *v4l2_h264_encoder_work_phys(struct v4l2_h264_encoder *encoder,
                void *input_mem, unsigned int *ouput_size);

void *v4l2_h264_encoder_work_separate(struct v4l2_h264_encoder *encoder,
                void *y_mem, void *uv_mem, unsigned int *ouput_size);

/**
 * 要求y_mem和uv_mem分别需要物理上连续，否则与使用v4l2_h264_encoder_work_separate一致
 **/
void *v4l2_h264_encoder_work_phys_separate(struct v4l2_h264_encoder *encoder,
                void *y_mem, void *uv_mem, unsigned int *ouput_size);

/**
 * 与 v4l2_h264_encoder_work 区别:
 * 底层直接使用inputmem的内存作为VPU输入源,内存占用更少.(但要求mem必须是物理上连续的)
 * */
void *v4l2_h264_encoder_work_by_phy_mem(struct v4l2_h264_encoder *encoder,
                void *input_mem, unsigned long phy_mem, unsigned int *ouput_size);


/*获取最新编出来的是否为关键帧 0：p/b 帧 1： i帧*/
int v4l2_h264_encoder_get_whether_keyframe(struct v4l2_h264_encoder *encoder);

int v4l2_h264_encoder_close(struct v4l2_h264_encoder *encoder);

void v4l2_h264_encoder_set_keyframe(struct v4l2_h264_encoder *encoder);

int v4l2_h264_encoder_get_stream_param(struct v4l2_h264_encoder *encoder, struct v4l2_streamparm *param);

#ifdef  __cplusplus
}
#endif

#endif /* _V4L2_H264_ENCODE_H_ */
