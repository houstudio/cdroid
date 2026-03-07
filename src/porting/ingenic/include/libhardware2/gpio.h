/*
 *  Copyright (C) 2020 Ingenic Semiconductor Co., Ltd.
 *
 *  Ingenic library hardware version2
 *
 */
#ifndef __HARDWARE2_GPIO_H__
#define __HARDWARE2_GPIO_H__

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief 获得gpio设备句柄
 * @return 成功返回设备句柄,失败返回负数
 */
int gpio_open(void);

/**
 * @brief 关闭gpio设备句柄
 * @param fd gpio设备句柄,由gpio_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int gpio_close(int fd);

/**
 * @brief 设置gpio功能
 * @param fd gpio设备句柄,由gpio_open()函数获得
 * @param gpio 描述对应的gpio引脚，如"PB08"
 * @param funcs 描述对应的gpio功能，如char *funcs = {"input", "pull_up"};
 * @param func_count funcs 数组的长度，即一次设置的功能的个数
 * @return 成功返回0,失败返回负数
 */
int gpio_set_func(int fd, const char *gpio, char *funcs[], unsigned int func_count);

/**
 * @brief 获得gpio当前的功能
 * @param fd gpio设备句柄,由gpio_open()函数获得
 * @param gpio 描述对应的gpio引脚，如"PB08"
 * @param buf 用于获取gpio的功能
 * @param buf_size 给定的buf的大小
 * @return 成功返回0,失败返回负数
 */
int gpio_get_func(int fd, const char *gpio, char *buf, int buf_size);

/**
 * @brief 获得gpio当前的功能，此函数数与 gpio_set_func 相对应
 * @param fd gpio设备句柄,由gpio_open()函数获得
 * @param gpio 描述对应的gpio引脚，如"PB08"
 * @param buf buf 数组,每个成员作为一个buf获取gpio的一个功能
 * @param buf_count buf 数组中成员的个数，即buf的个数
 * @param buf_size buf 数组中每个buf的大小,每个buf大小都一样大
 * @return 成功返回获取到的功能的个数,失败返回负数
 */
int gpio_get_func2(int fd, const char *gpio, char *buf[], int buf_count, int buf_size);

/**
 * @brief 获得gpio输入的值
 * @param fd gpio设备句柄,由gpio_open()函数获得
 * @param gpio 描述对应的gpio引脚，如"PB08"
 * @return 成功返回0或1,失败返回负数
 */
int gpio_get_value(int fd, const char *gpio);

/**
 * @brief 获得gpio输入的值
 * @param fd gpio设备句柄,由gpio_open()函数获得
 * @param gpio 描述对应的gpio引脚，如"PB08"
 * @param value 0 输出低 1 输出高
 * @return 成功返回0或1,失败返回负数
 */
int gpio_set_value(int fd, const char *gpio, int value);

/**
 * @brief 获得gpio的帮助信息
 * @param fd gpio设备句柄,由gpio_open()函数获得
 * @param buf 用于获取gpio的帮助信息
 * @param buf_size 给定的buf的大小
 * @return 成功返回0或1,失败返回负数
 */
int gpio_get_help(int fd, char *buf, unsigned int buf_size);

#ifdef  __cplusplus
}
#endif

#endif /* __HARDWARE2_GPIO_H__ */
