#ifndef __LIBHARDWARE2_WATCHDOG_H__
#define __LIBHARDWARE2_WATCHDOG_H__

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief 启动看门狗，同时设置最迟喂狗时间
 * @param ms 最迟喂狗时间 单位 ms
 * @return 成功返回0，失败返回负数
 */
int watchdog_start(unsigned long ms);

/**
 * @brief 停止看门狗计数
 * @param NULL
 * @return 成功返回0，失败返回负数
 */
int watchdog_stop(void);

/**
 * @brief 喂狗
 * @param NULL
 * @return 成功返回0，失败返回负数
 */
int watchdog_feed(void);

/**
 * @brief cpu复位重启
 * @param NULL
 * @return 成功返回0，失败返回负数
 */
int watchdog_reset(void);

#ifdef  __cplusplus
}
#endif

#endif
