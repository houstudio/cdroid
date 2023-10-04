#ifndef _GPIO_SYS_H_
#define _GPIO_SYS_H_

#ifdef  __cplusplus
extern "C" {
#endif

#define GPIO_PA(n)      (32*0 + n)
#define GPIO_PB(n)      (32*1 + n)
#define GPIO_PC(n)      (32*2 + n)
#define GPIO_PD(n)      (32*3 + n)
#define GPIO_PE(n)      (32*4 + n)

typedef enum _gpio_irq_mode {
    GPIO_NONE,
    GPIO_RISING,
    GPIO_FALLING,
    GPIO_BOTH,
} gpio_irq_mode;

/**
 * @bref 申请gpio
 * @param gpio   gpio pin
 * @return 返回0表示成功，失败返回负数
 */
int gpio_sys_request(unsigned int gpio);

/**
 * @bref 释放gpio
 * @param gpio   gpio pin
 * @return 返回0表示成功，失败返回负数
 */
int gpio_sys_free(unsigned int gpio);

/**
 * @bref 读取gpio值
 * @param gpio   gpio pin
 * @return 成功返回0或1，表示gpio的值，失败返回负数
 */
int gpio_sys_get_value(unsigned int gpio);

/**
 * @bref 设置gpio的值
 * @param gpio   gpio pin
 * @param val    val=[0,1]
 * @return 成功返回0，失败返回负数
 */
int gpio_sys_set_value(unsigned int gpio, int val);

/**
 * @bref gpio为输入模式
 * @param gpio   gpio pin
 * @return 成功返回0，失败返回负数
 */
int gpio_sys_direction_input(unsigned int gpio);

/**
 * @bref gpio输出电平
 * @param gpio   gpio pin
 * @param value  value=[0,1]
 * @return 成功返回0，失败返回负数
 */
int gpio_sys_direction_output(unsigned int gpio, int value);

/**
 * @bref 使能gpio中断
 * @param gpio   gpio pin
 * @param irq_mode  中断模式
 * @return 成功返回0，失败返回负数
 */
int gpio_sys_enable_irq(unsigned int gpio, gpio_irq_mode irq_mode);

/**
 * @bref 等gpio中断触发
 * @param gpio   gpio pin
 * @param timeout_ms  超时时间
 * @return 成功返回0或1，表示gpio的值，失败返回负数
 */
int gpio_sys_irq_wait_timeout(
        unsigned int gpio, int timeout_ms);

/**
 * @bref 等gpio中断触发
 * @param gpio   gpio pin
 * @return 成功返回0或1，表示gpio的值，失败返回负数
 */
int gpio_sys_irq_wait(unsigned int gpio);

#ifdef  __cplusplus
}
#endif

#endif
