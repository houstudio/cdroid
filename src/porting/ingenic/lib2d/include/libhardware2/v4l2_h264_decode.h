#ifndef _V4L2_H264_DECODE_H_
#define _V4L2_H264_DECODE_H_


#ifdef  __cplusplus
extern "C" {
#endif

struct v4l2_h264_decoder_config {
    const char *video_path;
    unsigned int width;
    unsigned int height;
    unsigned int output_fmt;
    /*
     * 该参数为传出参数，调用 v4l2_h264_decoder_open 后获取
     * 参数表示经过对齐后宽的大小，例如 x2600 felix 内核驱动中对齐宽度为 128 的整数倍，即 480 向上对齐为 512
     * 对应解码后缓冲区 output_mem[i] 每行的大小
    */
    unsigned int linesize;
};

struct v4l2_h264_decoder;

struct v4l2_h264_decoder *v4l2_h264_decoder_open(struct v4l2_h264_decoder_config *config);

/**
 * H264解码输出为nv12，参数定义为void *out_mem[2]，其中out_mem[0]为y数据，out_mem[1]为uv数据
 **/
int v4l2_h264_decoder_work(struct v4l2_h264_decoder *decoder, void *input_mem,
                unsigned int input_size, void **output_mem);

int v4l2_h264_decoder_work_release(struct v4l2_h264_decoder *decoder);

int v4l2_h264_decoder_close(struct v4l2_h264_decoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif /* _V4L2_H264_DECODE_H_ */
