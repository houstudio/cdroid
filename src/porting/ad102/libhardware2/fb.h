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
 * @brief 获得fb设备信息
 * @param fd fb设备句柄,由fb_open()函数获得
 * @param info 需要初始化的fb信息结构体,不能为NULL
 * @return 成功返回0,失败返回负数
 */
int fb_get_info(int fd, struct fb_device_info *info);

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

/**
 * @brief 设置fb设备旋转角度(0/90/180/270°,仅ad100/x2600支持动态旋转)
 * @param fd fb设备句柄,由fb_open()函数获得
 * @param angle 旋转角度,仅支持0/90/180/270°
 * @return 成功返回0,失败返回负数
 */
int fb_set_rot_angle(int fd, unsigned int angle);

/**
 * @brief 获得fb设备旋转角度(0/90/180/270°,仅ad100/x2600支持动态旋转)
 * @param fd fb设备句柄,由fb_open()函数获得
 * @return 成功返回旋转角度(0/90/180/270°),失败返回负数
 */
int fb_get_rot_angle(int fd);

int fb_suspend(int fd);

int fb_resume(int fd);

/**
 * @brief ESD检测判断屏幕是否正常显示 (检测方法及顺序:1.te detect 2.read reg 3.error report)
 * @param fd fb设备句柄,由fb_open()函数获得
 * @return 成功返回 0    (可继续隔一定时长循环执行)
 *         失败返回错误状态 (可调用fb_suspend + fb_resume进行恢复)
 */

#define ESD_MIPI_TFT_TE_ERR             (0x01 << 0)
#define ESD_DISPLAY_ERR                 (0x01 << 1)
#define ESD_NUM_ON_DSI_ERR              (0x01 << 2)
#define ESD_ECC_MULTI_ERR               (0x01 << 3)
#define ESD_CHECKSUM_ERR                (0x01 << 4)
#define ESD_CONTENTION_ERR              (0x01 << 5)
#define ESD_FALSE_CONTROL_ERR           (0x01 << 6)
#define ESD_EOT_SYNC_ERR                (0x01 << 7)

int fb_esd_check(int fd);

/**
 * @brief 获取分割后需要的内存大小
 * @param src_w 源图像的宽
 * @param src_h 源图像的高
 * @param fmt fb设备显示的图像格式
 * @return 返回内存大小
 */
int fb_large_get_buffer_size(int src_w, int src_h, enum fb_fmt fmt);

/**
 * @brief 分割超大图像层以适应硬件缩放限制
 * @param large_cfg 源图像层配置,以及目标配置参数
 * @param small_cfg1 分割之后layer0图层的配置
 * @param small_cfg2 分割之后layer1图层的配置
 * @param split_mem 分割后图像数据的虚拟内存起始地址
 * @param split_phy 分割后图像数据的物理内存起始地址
 * @return 成功返回0,失败返回负数
 */
int fb_large_cfg_split(struct lcdc_layer *large_cfg, struct lcdc_layer *small_cfg1, struct lcdc_layer *small_cfg2,
                        void *split_mem, unsigned long split_phy);

#ifdef  __cplusplus
}
#endif

#endif /* __LIBHARDWARE2_FB_H__ */