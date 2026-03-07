#ifndef _VIDEO_AVPU_H264_ENCODE_H_
#define _VIDEO_AVPU_H264_ENCODE_H_

#include <stdbool.h>
#include <stdint.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct avpu_h264_encoder_config {
    unsigned int width;
    unsigned int height;
    unsigned int frame_rate;
    unsigned int input_fmt;
    unsigned int gop_size;
    unsigned int bitrate;
};

struct avpu_h264_encoder;

/*
 * 获取AVPU编码器申请的帧大小 NV12格式 (宽 * 高[16像素对齐] * 3 / 2)
 * encoder : 打开AVPU编码器句柄
 * 返回值: AVPU申请帧大小
 */
uint32_t avpu_h264_encoder_get_frame_size(struct avpu_h264_encoder *encoder);

/*
 * 获取AVPU编码器存放UV数据偏移 (宽 * 高[16像素对齐])
 * encoder : 打开AVPU编码器句柄
 * 返回值: UV数据偏移
 */
uint32_t avpu_h264_encoder_get_uv_offset(struct avpu_h264_encoder *encoder);



/*
 * 初始化AVPU 编码器为h264模式
 */
struct avpu_h264_encoder *avpu_h264_encoder_open(struct avpu_h264_encoder_config *config);


/*
 * 将一帧NV12数据编码为h264格式
 * encoder   : 打开AVPU编码器句柄
 * map_vaddr : 应用层操作的虚拟地址. 必须为video_avpu_encoder_alloc 申请获得
 * out_length: 该数据帧h264编码处理后的大小
 * out_frame : 存放h264编码后的数据, 可由malloc申请获得该地址
 * 返回值     : =0 编码成功
 *             <0 编码失败
 */
int avpu_h264_encoder_work(struct avpu_h264_encoder *encoder, void *map_vaddr, uint32_t *out_length, void *out_frame);


/*
 * 关闭AVPU 编码器
 */
int avpu_h264_encoder_close(struct avpu_h264_encoder *encoder);


/*
 * AVPU编码器初始化
 */
int avpu_encoder_init(void);

/*
 * AVPU编码器去初始化
 */
void avpu_encoder_deinit(void);

/*
 * 从AVPU编码器中申请内存
 * 返回值为: mmap后应用层操作虚拟地址
 * size: 为申请内存大小, 256对齐
 */
void *avpu_encoder_alloc(int size);

/*
 * 从AVPU编码器中释放内存
 * 返回值为: vaddr是mmap后应用层的虚拟地址
 */
void avpu_encoder_free(void *vaddr);

/*
 * 查找VPU编码器中申请的虚拟地址对应的物理地址
 * 返回值为: vaddr是mmap后应用层的地址
 */
void *avpu_encoder_vitr_to_phys(void *vaddr);

/*
 * 查找VPU编码器中申请的物理地址对应 应用层操作的虚拟地址
 * 返回值为: vaddr是mmap后应用层的地址
 */
void *avpu_encoder_phys_to_virt(void *paddr);


#ifdef  __cplusplus
}
#endif

#endif /* _VIDEO_AVPU_H264_ENCODE_H_ */
