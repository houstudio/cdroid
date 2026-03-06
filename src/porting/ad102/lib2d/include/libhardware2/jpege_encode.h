#ifndef __LIBHARDWARE2_JPEGE_ENCODE_H__
#define __LIBHARDWARE2_JPEGE_ENCODE_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    JPEGE_PIX_FMT_NV12 = 0,
    JPEGE_PIX_FMT_NV21 = 1,
    JPEGE_PIX_FMT_RGBA_8888 = 10,
    JPEGE_PIX_FMT_BGRA_8888 = 12,
    JPEGE_PIX_FMT_RGB_888 = 16,
    JPEGE_PIX_FMT_YUYV = 41,
    JPEGE_PIX_FMT_YUV444 = 45,
} jpege_pixel_fmt;

/* 编码前传入的相关参数 */
struct jpege_encoder_config {
    unsigned int file_size;  // 图像的源文件大小
    int qa;                  // 压缩等级
    int width;               // 图像的宽
    int height;              // 图像的高
    jpege_pixel_fmt in_fmt;  // 编码前图像源格式
    void *input_mem;         // 待编码的源数据地址
};

/* 编码后传出的相关数据 */
struct jpege_encoder_output_data{
    void *mem;               // 编码后的目标数据地址
    int image_size;          // 编码后的目标数据大小
    unsigned long phy_mem;   // dst_buf物理地址
    int buff_size;           // dst_buff的大小

};

/* 将jpeg encoder作为设备打开，返回设备句柄 */
struct jpege_encoder *jpege_encoder_open(void);
/* 关闭jpeg encoder */
void jpege_encoder_close(struct jpege_encoder *encoder);

/**
 * @brief 获取编码产生的数据
 * @param encoder encoder设备句柄
 * @param input   编码前传入的相关参数
 * @param output  编码后传出的相关数据
 * @return 成功返回0,失败返回-1
 */
int jpege_encoder_get(struct jpege_encoder *encoder, struct jpege_encoder_config *input, struct jpege_encoder_output_data *output);
/**
 * @brief 释放编码产生的数据
 * @param encoder encoder设备句柄
 * @param output  编码后传出的相关数据
 * @return 无返回值
 */
void jpege_encoder_put(struct jpege_encoder *encoder , struct jpege_encoder_output_data *output);


#ifdef __cplusplus
}
#endif

#endif
