#ifndef _LIBHARDWARE2_ALSA_H_
#define _LIBHARDWARE2_ALSA_H_

#include <alsa/asoundlib.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @brief alsa 设备参数结构体
 */
struct alsa_params {
    /* 调用alsa_pcm_set_params 时设置
     * rate: pcm 采样率, 如 44100, 48000, 16000,,,
     * channels: pcm 单个采样点的通道数, 如 1, 2, 4,,,
     * format: 参考 <alsa/pcm.h>, 如 SND_PCM_FORMAT_S16_LE
     */
    unsigned int rate;
    unsigned int channels;
    snd_pcm_format_t format;

    /* 调用alsa_pcm_set_params 时设置, 调用后会返回真实的结果
     * 0 表示自动设置
     * buffer_time: pcm 接收/发送时缓冲区的时长,单位 us, 如300*1000表示300毫秒
     * period_time: pcm 数据切片周期的时长,单位 us,如50*1000表示50毫秒
     */
    unsigned int buffer_time;
    unsigned int period_time;

    /* 调用alsa_pcm_set_params 之后获得结果
     * frame_bytes 每个采样点的大小,单位字节
     * period_frames 一个数据切片周期的采样点数
     * buffer_frames 缓冲区的最大采样点数
     */
    unsigned int frame_bytes;
    unsigned int period_frames;
    unsigned int buffer_frames;
};

/**
 * @brief 音频录音/播放设备句柄
 */
struct alsa_pcm;

/**
 * @brief 打开录音设备
 * @param device 设备名,如"hw:1,0" "plughw:1,0" "defualt"
 * @return 返回设备句柄,NULL表示不成功
 */
struct alsa_pcm *alsa_pcm_open_capture_device(const char *device);

/**
 * @brief 打开播放设备
 * @param device 设备名,如"hw:1,0" "plughw:1,0" "defualt"
 * @return 返回设备句柄,NULL表示不成功
 */
struct alsa_pcm *alsa_pcm_open_playback_device(const char *device);

/**
 * @brief 关闭alsa设备
 * @param alsa 设备句柄,由 alsa_pcm_open_capture/playback_device 函数获得
 */
void alsa_pcm_close(struct alsa_pcm *alsa);

/**
 * @brief 设置播放/录音设备参数
 * @param alsa 设备句柄
 * @param param 设备参数 @see struct alsa_params
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_pcm_set_params(struct alsa_pcm *alsa, struct alsa_params *param);

/**
 * @brief 设置播放/录音设备参数
 * @param alsa 设备句柄
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_pcm_free_params(struct alsa_pcm *alsa);

/**
 * @brief 从录音设备中读取音频数据
 * @param alsa 设备句柄
 * @param buffer 音频数据指针
 * @param frames 音频数据采样点数
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_pcm_read(struct alsa_pcm *alsa, void *buffer, int frames);

/**
 * @brief 往播放设备中写入音频数据
 * @param alsa 设备句柄
 * @param buffer 音频数据指针
 * @param frames 音频数据采样点数
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_pcm_write(struct alsa_pcm *alsa, void *buffer, int frames);

/**
 * @brief 将当前设备的缓冲区中的数据扔掉
 * @param alsa 设备句柄
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_pcm_drop(struct alsa_pcm *alsa);

/**
 * @brief 当前设备缓冲区中可以用读/写的数据数
 * @param alsa 设备句柄
 * @return 可用数据的采样点数
 */
int alsa_pcm_avil(struct alsa_pcm *alsa);

/**
 * @brief 将当前播放设备的缓冲区的数据全部刷新到硬件,确保剩余数据播放被播放
 * @param alsa 设备句柄
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_pcm_drain(struct alsa_pcm *alsa);

/**
 * @brief 启动当前的录音设备
 * @param alsa 设备句柄
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_pcm_start(struct alsa_pcm *alsa);

/**
 * @brief 音频控制器句柄
 */
struct alsa_ctl;

/**
 * @brief 打开音频控制器
 * @param card_name 控制器所在声卡名,如"hw:1" "plughw:1" "defualt"
 * @param ctl_name 控制器的名字,参考具体声卡的定义
 *                 查找时优先全词匹配,其次当包涵在某个控制器名字中也可以匹配到
 * @return 返回设备句柄,NULL表示不成功
 */
struct alsa_ctl *alsa_ctl_open(const char *card_name, const char *ctl_name);

/**
 * @brief 关闭音频控制器
 * @param ctl 音频控制器句柄, 由 alsa_ctl_open 获得
 */
void alsa_ctl_close(struct alsa_ctl *ctl);

/**
 * @brief 获得音频控制器的名字
 * @return 返回音频控制器的名字
 */
const char *alsa_ctl_get_name(struct alsa_ctl *ctl);

/**
 * @brief 设置音频控制器的值
 * @param ctl 音频控制器句柄
 * @param value 要设置的值
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_ctl_set_value(struct alsa_ctl *ctl, long value);

/**
 * @brief 获得音频控制器的值
 * @param ctl 音频控制器句柄
 * @param value 获得音频控制器的值的指针
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_ctl_get_value(struct alsa_ctl *ctl, long *value);

/**
 * @brief 获得音频控制器的值的范围
 * @param ctl 音频控制器句柄
 * @param min 最小值的指针
 * @param max 最大值的指针
 * @return 0: 表示成功   负数: 表示失败
 */
int alsa_ctl_get_value_range(struct alsa_ctl *ctl, long *min, long *max);

/**
 * @brief 打印出声卡中的所有控制器
 * @param card_name 声卡的名字,如"hw:1" "plughw:1" "defualt"
 * @return 0 表示成功   负数: 表示失败
 */
int alsa_ctl_list_all(const char *card_name);

#ifdef  __cplusplus
}
#endif

#endif /* _LIBHARDWARE2_ALSA_H_ */
