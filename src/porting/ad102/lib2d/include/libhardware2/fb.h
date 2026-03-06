#ifndef __LIBHARDWARE2_FB_H__
#define __LIBHARDWARE2_FB_H__

#include <linux/fb.h>
#include <sys/mman.h>

#ifdef  __cplusplus
extern "C" {
#endif

enum fb_fmt {
    fb_fmt_RGB555,
    fb_fmt_RGB565,
    fb_fmt_RGB888,
    fb_fmt_ARGB8888,
    fb_fmt_NV12,
    fb_fmt_NV21,
    fb_fmt_yuv422,
};

struct fb_device_info {
    unsigned int xres;
    unsigned int yres;
    enum fb_fmt fb_fmt;
    unsigned int line_length;
    unsigned int bits_per_pixel;
    unsigned int frame_size;
    unsigned int frame_nums;
    void *mapped_mem;

    struct fb_fix_screeninfo fix;
    struct fb_var_screeninfo var;
};

/* 计算每个像素点所占字节数 */
unsigned int fb_bytes_per_pixel(enum fb_fmt fb_fmt);

/**
 * @brief 获得fb设备句柄
 * @param dev_path fb设备路径,如/dev/fb0
 * @param info 需要初始化的fb信息结构体,不能为NULL
 * @return 成功返回设备句柄,失败返回-1
 */
int fb_open(const char *dev_path, struct fb_device_info *info);

/**
 * @brief 关闭fb设备句柄
 * @param fd fb设备句柄,由fb_open()函数获得
 * @param info fb的信息,由fb_open()函数初始化
 * @return 成功返回0,失败返回-1
 */
int fb_close(int fd, struct fb_device_info *info);

/**
 * @brief 使能fb设备,上电
 * @param fd fb设备句柄,由fb_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int fb_enable(int fd);

/**
 * @brief 关闭fb设备,掉电
 * @param fd fb设备句柄,由fb_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int fb_disable(int fd);

/**
 * @brief 显示/刷新fb指定的帧
 * @param fd fb设备句柄,由fb_open()函数获得
 * @param info fb的信息,由fb_open()函数初始化
 * @param frame_index 要显示的帧的序号,0 到 (info.frame_nums - 1)
 * @return 成功返回0,失败返回负数
 */
int fb_pan_display(int fd, struct fb_device_info *info, unsigned int frame_index);


/*
 * x1830 提供layer叠加的功能,以及NV12格式的显示
 */

enum lcdc_layer_order {
    lcdc_layer_top,
    lcdc_layer_bottom,
    lcdc_layer_0,
    lcdc_layer_1,
    lcdc_layer_2,
    lcdc_layer_3,
};

struct lcdc_layer {
    /* layer 的格式 */
    enum fb_fmt fb_fmt;

    /* layer 的大小 */
    unsigned int xres;
    unsigned int yres;

    /* layer 在屏幕上的偏移 */
    unsigned int xpos;
    unsigned int ypos;

    /* layer 的所在的层级 */
    enum lcdc_layer_order layer_order;

    /* 是否使能 layer */
    int layer_enable;

    /* rgb 格式时使用 */
    struct {
        void *mem;
        unsigned int stride; // 单位： 字节
    } rgb;

    /* NV12,NV21 格式时使用 */
    struct {
        void *mem;
        unsigned int stride; // 单位： 字节
    } y;

    /* NV12,NV21 格式时使用 */
    struct {
        void *mem;
        unsigned int stride; // 单位： 字节
    } uv;

    struct {
        unsigned char enable;
        unsigned char value;
    } alpha;

    struct {
        unsigned char enable;
        unsigned int xres;
        unsigned int yres;
    } scaling;
};

/**
 * @brief pan display 时选择用户的配置
 * @param fd fb设备句柄,由fb_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int fb_pan_display_enable_user_cfg(int fd);

/**
 * @brief pan display 时选择驱动的配置
 * @param fd fb设备句柄,由fb_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int fb_pan_display_disable_user_cfg(int fd);

/**
 * @brief 设置用户的配置
 * @param fd fb设备句柄,由fb_open()函数获得
 * @param cfg lcd 控制器layer的设置
 * @return 成功返回0,失败返回负数
 */
int fb_pan_display_set_user_cfg(int fd, struct lcdc_layer *cfg);

/**
 * @brief 读slcd寄存器的值
 * @param fd fb设备句柄,由fb_open()函数获得
 * @param reg 要读数据地址
 * @param buffer 接收读到的数据
 * @param count 读取次数
 * @return 成功返回0,失败返回负数
 */
int fb_slcd_read_reg_8(int fd, int reg, char *buffer, int count);


#ifdef  __cplusplus
}
#endif

#endif /* __LIBHARDWARE2_FB_H__ */