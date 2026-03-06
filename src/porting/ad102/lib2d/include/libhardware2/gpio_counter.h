#ifndef _LIBHARDWARE2_GPIO_COUNTER_H
#define _LIBHARDWARE2_GPIO_COUNTER_H

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief 获得gpio设备句柄
 * @return 成功返回设备句柄,失败返回负数
 */
int gpio_counter_open(void);

/**
 * @brief 关闭gpio设备句柄
 * @param fd gpio_counter设备句柄,gpio_counter_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int gpio_counter_close(int fd);

/**
 * @brief 配置 gpio_counter 参数
 * @param fd gpio_counter设备句柄，由gpio_counter_open()获取
 * @param channel_id 选择gpio_counter捕获通道号
 * @param mode_name  选择捕获模式以及输入时钟源,可通过gpio_counter_get_mode_information获得支持的模式.
 * @return 成功返回0，失败返回负数
 */
int gpio_counter_config(int fd, int channel_id, char *mode_name);

/**
 * @brief 使能gpio_counter设备
 * @param fd gpio_counter设备句柄,由gpio_counter_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int gpio_counter_enable(int fd, unsigned int channel_id);

/**
 * @brief 失能gpio_counter设备
 * @param fd gpio_counter设备句柄,由gpio_counter_open()函数获得
 * @return 成功返回0,失败返回负数
 */
int gpio_counter_disable(int fd, unsigned int channel_id);

/**
 * @brief 获得指定通道计数值
 * @param fd gpio_counter设备句柄,由gpio_counter_open()函数获得
 * @return 成功返回计数值,失败返回负数
 */
int gpio_counter_get_count(int fd, int channel_id);

/**
 * @brief 获得指定通道捕获到的周期和高电平计数值，
 * @param fd gpio_counter设备句柄,由gpio_counter_open()函数获得
 * @param channel_id 选择gpio_counter捕获通道号
 * @param high_level_time 捕获到的高电平计数值
 * @param period_time 捕获到的周期计数值
 * @return 成功返回0,失败返回负数
 */
int gpio_counter_get_capture(int fd, int channel_id, int *high_level_time, int *period_time);

/**
 * @brief 获取当前gpio_counter控制器支持的 捕获模式
 * @param fd gpio_counter设备句柄,由gpio_counter_open()函数获得
 * @param num 获得当前gpio_counter控制器支持的模式数量
 * @return 成功返回:存放有模式名称的指针数组,失败返回NULL.
 */
char **gpio_counter_get_mode_information(int fd, int *num);

/**
 * @brief 回收gpio_counter_get_mode_information申请的内存.
 * @param array gpio_counter_get_mode_information的返回值
 */
void gpio_counter_free_mode_information(char **array);

#ifdef  __cplusplus
}
#endif

#endif