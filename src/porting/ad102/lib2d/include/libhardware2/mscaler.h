#ifndef __LIBHARDWARE2_MSCALER_H__
#define __LIBHARDWARE2_MSCALER_H__

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * 31    23    15    7    0
 *     A     R    G    B
 * MSCALER_FORMAT_BGRA_8888
 * 当输出格式为BRGA格式时，每四个字节组成一个像素点，每个像素点的组成
 * 关系为字节0：B，1：G，2：R，3：A
*/

enum mscaler_fmt{
    MSCALER_FORMAT_NV12         = 0,
    MSCALER_FORMAT_NV21         = 1,

    MSCALER_FORMAT_BGRA_8888    = (0<<2)+2,
    MSCALER_FORMAT_GBRA_8888    = (1<<2)+2,
    MSCALER_FORMAT_RBGA_8888    = (2<<2)+2,
    MSCALER_FORMAT_BRGA_8888    = (3<<2)+2,
    MSCALER_FORMAT_GRBA_8888    = (4<<2)+2,
    MSCALER_FORMAT_RGBA_8888    = (5<<2)+2,

    MSCALER_FORMAT_ABGR_8888    = (8<<2)+2,
    MSCALER_FORMAT_AGBR_8888    = (9<<2)+2,
    MSCALER_FORMAT_ARBG_8888    = (10<<2)+2,
    MSCALER_FORMAT_ABRG_8888    = (11<<2)+2,
    MSCALER_FORMAT_AGRB_8888    = (12<<2)+2,
    MSCALER_FORMAT_ARGB_8888    = (13<<2)+2,

    MSCALER_FORMAT_BGR_565      = (0<<2)+3,
    MSCALER_FORMAT_GBR_565      = (1<<2)+3,
    MSCALER_FORMAT_RBG_565      = (2<<2)+3,
    MSCALER_FORMAT_BRG_565      = (3<<2)+3,
    MSCALER_FORMAT_GRB_565      = (4<<2)+3,
    MSCALER_FORMAT_RGB_565      = (5<<2)+3,
};

struct mscaler_frame_part
{
    /* 用户空间虚拟地址 */
    void                        *mem;

    /* 分量大小 cache_line对齐 */
    unsigned int                mem_size;

    /* 实际物理地址,必须 cache_line 对齐       */
    unsigned long               phys_addr;

    /* 行字节对齐要求，必须同时满足MSCALER_ALIGN对齐 */
    unsigned int                stride;
};

struct mscaler_frame
{
    /* 输入图像格式只支持 nv12/nv21 输出图像格式全支持mscaler_fmt */
    enum mscaler_fmt            fmt;

    /* 图像宽 */
    unsigned int                xres;

    /* 图像高 */
    unsigned int                yres;

    /* mscaler_fmt全格式使用，rgb/rgba等格式只需要使用y分量即可 */
    struct mscaler_frame_part   y;

    /* NV12,NV21 格式时使用,uv分量 */
    struct mscaler_frame_part   uv;
};

struct mscaler_device_info
{
    /* 一行数据需要对齐的大小，单位： 字节 */
    unsigned int stride_align;

    /* 一帧数据需要对齐的大小，单位： 字节 */
    unsigned int frame_align;

    /* 申请的物理内存地址 */
    unsigned long phys_addr;

    /* 申请的内存空间对齐之后的大小 */
    unsigned int mem_align_size;

    /* 申请的用户空间内存地址 */
    void *mapped_mem;
};

/**
 * @brief 获得mscaler设备句柄
 * @param info 需要初始化的mscaler信息结构体,不能为NULL，如果成功,则stride_align，frame_align被赋值
 * @return 成功返回设备句柄,失败返回-1
 */
int mscaler_open(struct mscaler_device_info *info);

/**
 * @brief 关闭mscaler设备句柄
 * @param fd mscaler设备句柄,由mscaler_open()函数获得
 * @return 成功返回0,失败返回-1
 */
int mscaler_close(int fd);

/**
 * @brief 由mscaler申请需要的空间大小（如果图像数据有自己的内存空间，无需申请）
 * @param fd mscaler设备句柄,由mscaler_open()函数获得
 * @param info mscaler的信息,如果内存申请成功,则phys_addr，mem_align_size，mapped_mem被赋值
 * @param mem_size 需要申请内存空间的大小
 * @return 成功返回0,失败返回-1
 */
int mscaler_alloc_mem(int fd, struct mscaler_device_info *info, int mem_size);

/**
 * @brief 释放mscaler_alloc_mem申请的内存空间
 * @param fd mscaler设备句柄,由mscaler_open()函数获得
 * @param info mscaler的信息,释放指定的内存空间
 * @return 成功返回0,失败返回-1
 */
int mscaler_free_mem(int fd, struct mscaler_device_info *info);

/**
 * @brief 图像缩放和格转换
 * @param fd mscaler设备句柄,由mscaler_open()函数获得
 * @param src 图像数据源参数
 * @param dst 图像数据目标参数
 * @return 成功返回0,失败返回-1
 */
int mscaler_convert(int fd, struct mscaler_frame *src, struct mscaler_frame *dst);

#ifdef  __cplusplus
}
#endif

#endif//__LIBHARDWARE2_MSCALER_H__
