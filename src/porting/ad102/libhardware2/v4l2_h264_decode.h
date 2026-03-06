#ifndef _V4L2_H264_DECODE_H_
#define _V4L2_H264_DECODE_H_


#ifdef  __cplusplus
extern "C" {
#endif

struct v4l2_h264_decoder_config {
    /* 以下参数为 open 前需要的参数 */
    const char *video_path;
    unsigned int width;
    unsigned int height;
    unsigned int output_fmt;

    /* 以下参数为 open 后返回的参数 */
    unsigned int align_width;   //输出buff需满足的宽
    unsigned int align_height;  //输出buff需满足的高
};

struct v4l2_h264_decoder;

/*
 * 解码输出nv12, 到给定返回地址out_mem[0]，out_mem[1]
*/
struct v4l2_h264_decoder *v4l2_h264_decoder_open(struct v4l2_h264_decoder_config *config);
/*
 * out_mem[0]:y分量数据, out_mem[1]:uv分量数据
 *
 * 注意：out_mem[0] & out_mem[1]所指向的buff需满足
 * 宽高需为config->align_width, config->align_height
 * 起始地址需要4k对齐
*/
int v4l2_h264_decoder_work(struct v4l2_h264_decoder *decoder, void *input_mem, unsigned int input_size,
                            void **output_mem);

int v4l2_h264_decoder_work_release(struct v4l2_h264_decoder *decoder);

int v4l2_h264_decoder_close(struct v4l2_h264_decoder *decoder);



/*
 * 解码输出nv12, 到指定返回地址y_mem，uv_mem
*/
struct v4l2_h264_decoder *v4l2_h264_decoder_direct_open(struct v4l2_h264_decoder_config *config);
/*
 * y_mem:y分量数据, uv_mem:uv分量数据
 *
 * 注意：y_mem & uv_mem所指向的buff需满足
 * 宽高需为config->align_width, config->align_height
 * 起始地址需要4k对齐
*/
int v4l2_h264_decoder_direct_work(struct v4l2_h264_decoder *decoder, void *input_mem, unsigned int input_size,
                            void *y_mem, void *uv_mem);

int v4l2_h264_decoder_direct_close(struct v4l2_h264_decoder *decoder);



/* 解码中的当前帧参数 (底层驱动只支持裁切，不支持偏移) */
struct v4l2_h264_decoder_curr_info {
    unsigned int curr_width;    //当前帧的实际宽
    unsigned int curr_height;   //当前帧的实际高
    unsigned int crop_right;    //当前帧的右侧裁剪的像素数量
    unsigned int crop_bottom;   //当前帧的底部裁剪的像素数量
};

/* 获取当前帧的参数 */
int v4l2_h264_decoder_get_frame_info(struct v4l2_h264_decoder *decoder, struct v4l2_h264_decoder_curr_info *info);

#ifdef  __cplusplus
}
#endif

#endif /* _V4L2_H264_DECODE_H_ */
