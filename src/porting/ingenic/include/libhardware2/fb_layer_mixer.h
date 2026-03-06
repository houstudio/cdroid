#ifndef __LIBHARDWARE2_FB_LAYER_MIXER_H__
#define __LIBHARDWARE2_FB_LAYER_MIXER_H__

#include "fb.h"
#include <sys/mman.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct fb_layer_mixer_output_cfg {
    int xres;
    int yres;
    enum fb_fmt format;
    void *dst_mem;
    void *dst_mem_virtual;
};

/**
 * @brief 创建fb_layer_mixer
 * @param cfg mixer的配置信息
 * @return 成功返回mixer句柄, 失败返回负数
 */
int fb_layer_mixer_create(struct fb_layer_mixer_output_cfg *cfg);

/**
 * @brief 配置mixer输入图层的参数
 * @param fd mixer的操作句柄，由fb_layer_mixer_create 获得
 * @param layer_id 图层的编号（0~3）
 * @param layer 图层的配置信息
 * @return 成功返回0,失败返回负数
 */
int fb_layer_mixer_set_input_layer(int fd, int layer_id, struct lcdc_layer *layer);

/**
 * @brief 使mixer输出一帧数据
 * @param fd mixer的操作句柄，由fb_layer_mixer_create 获得
 * @return 成功返回0,失败返回负数
 */
int fb_layer_mixer_work_out_one_frame(int fd);


/**
 * @brief 删除mixer
 * @param fd mixer的操作句柄，由fb_layer_mixer_create 获得
 * @return 无返回值
 */
void fb_layer_mixer_delete(int fd);

#ifdef  __cplusplus
}
#endif

#endif