#ifndef __LIBHARDWARE2_PWM_H__
#define __LIBHARDWARE2_PWM_H__

#ifdef  __cplusplus
extern "C" {
#endif

enum pwm_shutdown_mode {
    /**
     * pwm停止输出时,尽量保证pwm的信号结尾是一个完整的周期
     */
    PWM_graceful_shutdown,
    /**
     * pwm停止输出时,立刻将pwm设置成空闲时电平
     */
    PWM_abrupt_shutdown,
};

enum pwm_idle_level {
    /**
     * pwm 空闲时电平为低
     */
    PWM_idle_low,
    /**
     * pwm 空闲时电平为高
     */
    PWM_idle_high,
};

enum pwm_accuracy_priority {
    /**
     * 优先满足pwm的目标频率的精度,级数可能不准确
     */
    PWM_accuracy_freq_first,
    /**
     * 优先满足pwm的级数设置,pwm频率可能不准确
     */
    PWM_accuracy_levels_first,
};

struct pwm_config_data {
    enum pwm_shutdown_mode shutdown_mode;
    enum pwm_idle_level idle_level;
    enum pwm_accuracy_priority accuracy_priority;
    unsigned long freq;
    unsigned long levels;
    unsigned int id;
};

/**
 * @brief 同时使能多个PWM通道
 * @param channels 具体需要申请的PWM通道
 * @return 成功返回0，失败返回-1
 */
int pwm_enable_channels(unsigned int channels);

/**
 * @brief 同时失能多个PWM通道
 * @param channels 具体需要失能的PWM通道
 * @return 成功返回0，失败返回-1
 */
int pwm_disable_channels(unsigned int channels);

/**
 * @brief 设置PWM不是真正的使能
 * @param handle 句柄，由pwm_request()获取
 * @return 成功返回0，失败返回-1
 */
int pwm_not_really_enable(long handle);

/**
 * @brief 设置PWM不是真正的失能
 * @param handle 句柄，由pwm_request()获取
 * @return 成功返回0，失败返回-1
 */
int pwm_not_really_disable(long handle);

/**
 * @brief 申请指定 PWM 通道
 * @param gpio_name pwm对应的gpio引脚，如，"pb17"、"pc11"...
 * @return 返回句柄，失败返回-1
 */
long pwm_request(const char *gpio_name);

/**
 * @brief 释放指定的 PWM 通道
 * @param handle 句柄，由pwm_request()获取
 */
void pwm_release(long handle);

/**
 * @brief 设置 pwm 参数，参数由 pwm_config_data 结构体传入
 * @param handle 句柄，由pwm_request()获取
 * @param cfg 类型为 pwm_config_data 结构体指针，其成员参考struct pwm_config_data结构体的定义
 * @return 成功返回0，失败返回负数
 */
int pwm_config(long handle, struct pwm_config_data *cfg);

/**
 * @brief 设置 PWM 调制的级数
 * @param handle 句柄，由pwm_request()获取
 * @param level pwm调制的级数,即一个周期内非空闲电平的长度
 * @return 成功返回0，失败返回负数
 */
int pwm_set_level(long handle, unsigned int level);

/* ---- dma mode ---- */

enum pwm_dma_start_level {
    PWM_start_low,   /* pwm dma模式的起始电平为低 */
    PWM_start_high,  /* pwm dma模式的起始电平为高 */
};

/*
    注意: high和low不能为零
        时间单位由pwm2_dma_init返回
 */
struct pwm_data {
    /* 低电平个数 */
    unsigned low:16;
    /* 高电平个数 */
    unsigned high:16;
};

/**
 * @brief 初始化pwm dma模式
 * @param handle 句柄，由pwm_request()获取
 * @return 成功返回pwm频率，失败返回负数
 */
int pwm_dma_init(long handle, enum pwm_idle_level idle_level, enum pwm_dma_start_level start_level);

/**
 * @brief 使用dma数据更新pwm
 * @param handle 句柄，由pwm_request()获取
 * @param data dma数据
 * @param data_count 数据个数
 * @return 成功返回0，失败返回负数
 */
int pwm_dma_update(long handle, struct pwm_data *data, unsigned int data_count);

/**
 * @brief 使用dma发送指定个数的电平
 * @param handle 句柄，由pwm_request()获取
 * @param high 高电平个数
 * @param low 底电平个数
 * @param data_count 数据的个数
 * @return 成功返回0，失败返回负数
 */
int pwm_dma_send(long handle, unsigned short high, unsigned short low, unsigned int data_count);

/**
 * @brief 使能dma的循环模式
 * @param handle 句柄，由pwm_request()获取
 * @param data dma数据
 * @param data_count 数据个数
 * @return 成功返回0，失败返回负数
 */
int pwm_dma_enable_loop(long handle, struct pwm_data *data, unsigned int data_count);

/**
 * @brief 关闭dma循环
 * @param handle 句柄，由pwm_request()获取
 * @return 成功返回0，失败返回负数
 */
int pwm_dma_disable_loop(long handle);

#ifdef  __cplusplus
}
#endif

#endif
