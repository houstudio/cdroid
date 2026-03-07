#ifndef _LIBHARDWARE2_CAMERA_COMMON_H_
#define _LIBHARDWARE2_CAMERA_COMMON_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <libhardware2/camera_pixel_format.h>

struct sensor_dbg_register {
    unsigned long long reg;
    unsigned long long val;
    unsigned int size;      /* val size, unit:byte */
};

struct camera_info {
    char name[64];

    /* 每行像素数 */
    unsigned int width;

    /* 行数 */
    unsigned int height;

    /* camera 帧率 */
    unsigned int fps;

    /* camera 帧数据格式 */
    camera_pixel_fmt data_fmt;

    /* 一行的长度,单位字节
     * 对于 nv12,nv21, 表示y数据一行的长度
     * 另外由此可以算出uv数据偏移 line_length*height
     */
    unsigned int line_length;

    /* 一帧数据经过对齐之前的大小 */
    unsigned int frame_size;

    /* 帧缓冲总数 */
    unsigned int frame_nums;

    /* 帧缓冲的物理基地址 */
    unsigned long phys_mem;

    /* mmap 后的帧缓冲基地址 */
    void *mapped_mem;

    /*帧对齐大小 */
    unsigned int frame_align_size;
};

/* 图像帧信息 */
struct frame_info {
    unsigned int index;             /* 帧缓存编号 */
    unsigned int sequence;          /* 帧序列号 */

    unsigned int width;             /* 帧宽 */
    unsigned int height;            /* 帧高 */
    unsigned int pixfmt;            /* 帧的图像格式 */
    unsigned int size;              /* 帧所占用空间大小 */
    void *vaddr;                    /* 帧的虚拟地址 */
    unsigned long paddr;            /* 帧的物理地址 */

    unsigned long long timestamp;   /* 帧的时间戳，单位微秒，单调时间 */

    unsigned int isp_timestamp;     /* isp时间戳，在vic通过isp clk计数换算得出，单位微秒，
                                       最大值10000秒左右（最大值和isp clk相关），大于最大值重新清零计时 */
    unsigned int shutter_count;     /* 曝光计数 */
};


#ifdef  __cplusplus
}
#endif

#endif /* _LIBHARDWARE2_CAMERA_COMMON_H_ */
