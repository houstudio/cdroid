#ifndef _LIBHARDWARE2_CAMERA_H_
#define _LIBHARDWARE2_CAMERA_H_

#ifdef  __cplusplus
extern "C" {
#endif

#include <libhardware2/camera_common.h>


/* 计算每个像素点所占字节数 */
unsigned int cam_bytes_per_pixel(camera_pixel_fmt cam_fmt);

/**
 * @brief 获得camera设备句柄
 * @param info 用于存放camera的信息,不能为NULL
 * @param device_path 设备节点
 * @return 成功返回设备句柄,失败返回-1
 */
int camera_open(struct camera_info *info, const char* device_path);

/**
 * @brief 关闭camera设备句柄
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param info camera的信息,由camera_open()函数初始化
 * @return 成功返回0,失败返回-1
 */
int camera_close(int fd, struct camera_info *info);

/**
 * @brief 使能camera设备,上电
 * @param fd camera设备句柄,由camera_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int camera_power_on(int fd);

/**
 * @brief 关闭camera设备,掉电
 * @param fd camera设备句柄,由camera_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int camera_power_off(int fd);

/**
 * @brief 开始camera图像录制
 * @param fd camera设备句柄,由camera_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int camera_stream_on(int fd);

/**
 * @brief 结束camera图像录制
 * @param fd camera设备句柄,由camera_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int camera_stream_off(int fd);

/**
 * @brief 获取一帧录制的图像数据
 * @param fd camera设备句柄,由camera_open()函数获得
 * @return 成功返回图像缓冲区指针,失败返回NULL
 */
void *camera_wait_frame(int fd);

/**
 * @brief 获取一帧录制的图像数据
 * @param fd camera设备句柄,由camera_open()函数获得
 * @return 成功返回图像缓冲区指针,失败返回NULL
 */
void *camera_get_frame(int fd);

/**
 * @brief 释放一帧图像缓冲区
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param mem 缓冲区指针,由camera_wait_frame()函数获得
 * @return 成功返回0,失败返回负数
 */
int camera_put_frame(int fd, void *mem);

/**
 * @brief 获取一帧录制的图像数据(不等待)
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param frame 获取的帧信息指针
 * @return 成功返回0,失败返回负数
 */
int camera_dqbuf(int fd, struct frame_info *frame);

/**
 * @brief 获取一帧录制的图像数据(未获取有效数据继续等待3s)
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param frame 获取的帧信息指针
 * @return 成功返回0,失败返回负数
 */
int camera_dqbuf_wait(int fd, struct frame_info *frame);

/**
 * @brief 释放一帧图像缓冲区
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param frame 释放的帧信息指针
 * @return 成功返回0,失败返回负数
 */
int camera_qbuf(int fd, struct frame_info *frame);

/**
 * @brief 获取已录制的图像数据帧数
 * @param fd camera设备句柄,由camera_open()函数获得
 * @return 成功返回帧数,失败返回负数
 */
int camera_get_avaliable_frame_count(int fd);

/**
 * @brief 丢弃已录制的图像数据帧
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param frames 需要丢弃的帧数
 * @return 成功返回帧数,失败返回负数
 */
int camera_drop_frames(int fd, unsigned int frames);

/**
 * @brief 获取sensor寄存器
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param reg sensor寄存器信息
 * @return 成功返回0,失败返回负数
 */
int camera_get_sensor_reg(int fd, struct sensor_dbg_register *reg);

/**
 * @brief 设置sensor寄存器
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param reg sensor寄存器信息
 * @return 成功返回0,失败返回负数
 */
int camera_set_sensor_reg(int fd, struct sensor_dbg_register *reg);

/**
 * @brief 获取sensor帧率
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param fps_num 帧率分子的指针
 * @param fps_den 帧率分母的指针
 * @return 成功返回0,失败返回负数
 */
int camera_get_fps(int fd, unsigned int *fps_num, unsigned int *fps_den);

/**
 * @brief 设置sensor帧率
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param fps_num 帧率分子
 * @param fps_den 帧率分母
 * @return 成功返回0,失败返回负数
 */
int camera_set_fps(int fd, unsigned int fps_num, unsigned int fps_den);

/**
 * camera功能开关
 */
typedef enum {
    CAMERA_OPS_MODE_DISABLE = 0,    /* 不使能该模块功能 */
    CAMERA_OPS_MODE_ENABLE,         /* 使能该模块功能 */
} camera_ops_mode;

/**
 * @brief 获取sensor水平镜像使能模式
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param mode 使能模式
 * @return 成功返回0,失败返回负数
 */
int camera_get_hflip(int fd, camera_ops_mode *mode);

/**
 * @brief 设置sensor水平镜像使能模式
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param mode 使能模式
 * @return 成功返回0,失败返回负数
 */
int camera_set_hflip(int fd, camera_ops_mode mode);

/**
 * @brief 获取sensor垂直翻转使能模式
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param mode 使能模式
 * @return 成功返回0,失败返回负数
 */
int camera_get_vflip(int fd, camera_ops_mode *mode);

/**
 * @brief 设置sensor垂直翻转使能模式
 * @param fd camera设备句柄,由camera_open()函数获得
 * @param mode 使能模式
 * @return 成功返回0,失败返回负数
 */
int camera_set_vflip(int fd, camera_ops_mode mode);

/**
 * @brief 重设camera格式
 * @param info 用于存放camera的信息,是否能为NULL取决于sensor驱动
 * @return 成功返回设备句柄,失败返回-1
 */
int camera_reset_fmt(int fd, struct camera_info *info);

#ifdef  __cplusplus
}
#endif

#endif /* _LIBHARDWARE2_CAMERA_H_ */
