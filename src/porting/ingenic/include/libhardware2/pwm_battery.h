#ifndef __LIBHARDWARE2_PWM_BATTERY_H__
#define __LIBHARDWARE2_PWM_BATTERY_H__

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief 打开pwm检测电压节点
 * @param 无
 * @return 返回句柄，失败返回-1
 */
int pwm_battery_open(void);

/**
 * @brief 读取电压值(mv)
 * @param 设备句柄
 * @return 返回电压值，失败返回-1
 */
int pwm_battery_read(int fd);

/**
 * @brief 关闭设备
 * @param 设备句柄
 * @return 无
 */
void pwm_battery_close(int fd);


#ifdef  __cplusplus
}
#endif

#endif