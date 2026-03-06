#ifndef __LIBHARDWARE2_ADC_H__
#define __LIBHARDWARE2_ADC_H__

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief 使能指定的 ADC 通道
 * @param channle_id 通道号
 * @return 返回句柄，失败返回-1
 */
long adc_enable(unsigned int channle_id);

/**
 * @brief 失能对应的 ADC 通道
 * @param handle 句柄
 */
void adc_disable(long handle);

/**
 * @brief 设置 ADC 参考电压
 * @param handle 句柄，由adc_enable()获取
 * @param vref ADC参考电压值
 * @return 返回0表示成功，失败返回负数
 */
int adc_set_vref(long handle, unsigned int vref);

/**
 * @brief 获取 ADC 参考电压
 * @param handle 句柄，由adc_enable()获取
 * @return 返回非负数表示成功（即参考电压），失败返回负数
 */
int adc_get_vref(long handle);

/**
 * @brief 获取 ADC 通道采样后的原始值
 * @param handle 句柄，由adc_enable()获取
 * @return 返回非负数表示成功（即对应的原始值），失败返回负数
 */
int adc_get_value(long handle);

/**
 * @brief 获取 ADC 通道采样后的电压值，由采样后的原始值和参考电压计算得到
 * @param handle 句柄，由adc_enable()获取
 * @return 返回非负数表示成功（即对应的电压值），失败返回负数
 */
int adc_get_voltage(long handle);


enum adc_trigger_type {
    adc_trigger_tcu0_half,
    adc_trigger_tcu0_full,
    adc_trigger_tcu1_half,
    adc_trigger_tcu1_full,
    adc_trigger_tcu2_half,
    adc_trigger_tcu2_full,
    adc_trigger_tcu3_half,
    adc_trigger_tcu3_full,
    adc_trigger_tcu4_half,
    adc_trigger_tcu4_full,
    adc_trigger_tcu5_half,
    adc_trigger_tcu5_full,
    adc_trigger_tcu6_half,
    adc_trigger_tcu6_full,
    adc_trigger_tcu7_half,
    adc_trigger_tcu7_full,
    adc_trigger_gpio_rising_edge,
    adc_trigger_gpio_falling_edge,
    adc_trigger_gpio_both_edge,
    adc_trigger_software,
};

struct adc_seq0_config {
    unsigned char channel_cnt;        // adc 采样序列使用的adc 通道总数
    unsigned char channels[4];        // 采样序列依次的通道号
    void (*irq_cb)(void);             // 中断回调函数,当一个转换序列完成之后回调,回调中需要读取adc数据
};

struct adc_seq1_config {
    int continus_clk_div; // 连续采样模式的计数时钟 = adc_clk / continus_clk_div; 最大值 2的24次方
    int delay_clk_div;    // 采样延时计数时钟 = adc_clk / delay_clk_div; 最大值 2的24次方
    enum adc_trigger_type trigger;     // adc 采样触发类型
    unsigned char enable_channel_num;  // 使能采样通道号,保存在数据的12-15 4bit位
    unsigned char channel_cnt;         // adc 采样序列使用的adc 通道总数
    unsigned char channels[16];        // 采样序列依次的通道号
    unsigned short channel_delays[16];         // 采样序列每个通道的延时, 单位是 adc delay clk

    unsigned char group_cnt;  // 连续采样模式下分组的个数,0表示不使能连续采样
    unsigned char groups[8];  // 分组模式依次对应每组通道的个数
    unsigned short group_delays[8]; // 每个分组转换完之后的延时, 单位是 adc cont clk

    long is_enable_irq;     /* 对应驱动的 void (*irq_cb)(void);
                            开启后保存每次采样的数据，可供读取，除了awd模式外，通常都需要使能 */
};

struct adc_seq2_config {
    int delay_clk_div;    // 采样延时计数时钟 = adc_clk / delay_clk_div; 最大值 2的24次方
    enum adc_trigger_type trigger;     // adc 采样触发类型
    unsigned char enable_channel_num;  // 使能采样通道号,保存在数据的12-15 4bit位
    unsigned char channel_cnt;         // adc 采样序列使用的adc 通道总数
    unsigned char channels[8];        // 采样序列依次的通道号
    unsigned char channel_delays[8];         // 采样序列每个通道的延时, 单位是 adc delay clk

    void (*irq_cb)(void); // 中断回调函数,当一个转换序列完成之后回调,回调中需要读取adc数据
};


/**
 * @brief 打开 ADC 的节点，节点必须是 "/dev/jz_adc_seq"
 * @return 返回句柄，失败返回-1
 */
int adc_open(void);

/**
 * @brief 关闭 ADC 的节点
 * @param handle 句柄
 * @return 返回非负数表示成功，失败返回负数
 */
int adc_close(long handle);

/**
 * @brief 使能 ADC 的 seq1 采集序列
 * @param handle 句柄，由adc_open()获取
 * @param adc_seq1 用户可配置的相关参数
 * @return 返回非负数表示成功，失败返回负数
 */
int adc_seq1_enable(long handle, struct adc_seq1_config adc_seq1);

/**
 * @brief 失能 ADC 的 seq1 采集序列
 * @param handle 句柄，由adc_open()获取
 * @param adc_seq1 用户可配置的相关参数
 * @return 返回非负数表示成功，失败返回负数
 */
int adc_seq1_disable(long handle, struct adc_seq1_config adc_seq1);

/**
 * @brief 读取 ADC 的 seq1 采集序列获取到的数据
 * @param handle 句柄，由adc_open()获取
 * @param timeout 读取超时时间
 * @param data 需要传入一个类似data[16]的数组，用户输出读取结果
 * @return 返回实际读取到的个数，对应采集序列输入的个数，失败返回负数
 */
int adc_seq1_read_timeout(long handle, int timeout, unsigned short *data);

/**
 * @brief 使能 ADC 对应通道的 awd 功能，本身不具备采样功能，通常建议使用seq1的连续采集模式触发检测
 * @param handle 句柄，由adc_open()获取
 * @param channel 需要使能的通道号
 * @param low_threshold adc value < low_threshold 触发awd低阈值中断，< 0 为不使能对应通道的低阈值触发功能
 * @param high_threshold adc value > high_threshold 触发awd高阈值中断，< 0 为不使能对应通道的高阈值触发功能
 * @return 返回非负数表示成功，失败返回负数
 */
int adc_awd_enable(long handle, unsigned int channel, int low_threshold, int high_threshold);

/**
 * @brief 失能 ADC 对应通道的 awd 功能
 * @param handle 句柄，由adc_open()获取
 * @return 返回非负数表示成功，失败返回负数
 */
int adc_awd_disable(long handle);

/**
 * @brief 读取 ADC 的 awd 被触发过的通道有哪些
 * @param handle 句柄，由adc_open()获取
 * @param timeout 读取超时时间
 * @param awd_low_flags 低阈值触发的通道
 * @param awd_high_flags 高阈值触发的通道
 * @return 返回非负数表示成功，失败返回负数
 */
int adc_awd_read_timeout(long handle, int timeout, unsigned short *awd_low_flags, unsigned short *awd_high_flags);

#ifdef  __cplusplus
}
#endif

#endif
