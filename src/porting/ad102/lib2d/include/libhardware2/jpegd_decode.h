#ifndef __LIBHARDWARE2_JPEGD_DECODE_H__
#define __LIBHARDWARE2_JPEGD_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JPEGD_PIX_FMT_NV12 = 0,
    JPEGD_PIX_FMT_NV21 = 1,
    JPEGD_PIX_FMT_BGRA_8888 = 12,
    JPEGD_PIX_FMT_RGB_888 = 16,
    JPEGD_PIX_FMT_YUYV = 41,
    JPEGD_PIX_FMT_YUV444 = 45,
} jpegd_pixel_fmt;

/* 解码前传入的相关参数 */
struct jpegd_decoder_config {
    unsigned int file_size;  // jpg图片文件大小
    int width;               // jpg图片的宽
    int height;              // jpg图片的高
    jpegd_pixel_fmt out_fmt;    // 解码后图像目标格式
    void *input_mem;         // 待解码的源数据地址
};

/* 解码后传出的相关数据 */
struct jpegd_decoder_output_data{
    void *mem;               // 解码后的目标数据地址
    int image_size;          // 解码后的目标数据大小
    int image_width;         // 解码后图像的宽
    int image_height;        // 解码后图像的宽
    unsigned long phy_mem;   // dst_buf物理地址
    int buff_size;           // dst_buff的大小
};

/* 将jpeg decoder作为设备打开，返回设备句柄 */
struct jpegd_decoder *jpegd_decoder_open(void);
/* 关闭jpeg decoder */
void jpegd_decoder_close(struct jpegd_decoder *decoder);

/**
 * @brief 获取解码产生的数据
 * @param decoder decoder设备句柄
 * @param input   解码前传入的相关参数
 * @param output  解码后传出的相关数据
 * @return 成功返回0,失败返回-1
 */
int jpegd_decoder_get(struct jpegd_decoder *decoder, struct jpegd_decoder_config *input, struct jpegd_decoder_output_data *output);
/**
 * @brief 释放解码产生的数据
 * @param decoder decoder设备句柄
 * @param output  解码后传出的相关数据
 * @return 无返回值
 */
void jpegd_decoder_put(struct jpegd_decoder *decoder , struct jpegd_decoder_output_data *output);


#ifdef __cplusplus
}
#endif

#endif
