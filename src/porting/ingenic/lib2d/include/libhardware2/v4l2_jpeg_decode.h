#ifndef _V4L2_JPEG_DECODE_H_
#define _V4L2_JPEG_DECODE_H_


#ifdef  __cplusplus
extern "C" {
#endif

struct v4l2_jpeg_decoder_config {
    const char *video_path;
    unsigned int width;
    unsigned int height;
    unsigned int output_fmt;    // 参考 <linux/videodev2.h> -> V4L2_PIX_FMT_NV12( v4l2_fourcc('N', 'V', '1', '2') )
};

struct v4l2_jpeg_decoder;

struct v4l2_jpeg_decoder *v4l2_jpeg_decoder_open(struct v4l2_jpeg_decoder_config *config);

/**
 * JPEG解码输出为nv12，参数定义为void *out_mem[2]，其中out_mem[0]为y数据，out_mem[1]为uv数据
 **/
int v4l2_jpeg_decoder_work(struct v4l2_jpeg_decoder *decoder, void *input_mem, unsigned int input_size, void **output_mem);

int v4l2_jpeg_decoder_close(struct v4l2_jpeg_decoder *decoder);

#ifdef  __cplusplus
}
#endif

#endif /* _V4L2_JPEG_DECODE_H_ */
