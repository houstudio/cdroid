#ifndef __LIBHARDWARE2_KEYBOARD_H__
#define __LIBHARDWARE2_KEYBOARD_H__

#ifdef  __cplusplus
extern "C" {
#endif

struct key_event {
    int key_type;
    int is_press;
};

/**
 * @brief 查找并打开多个按键的设备
 * @return 成功返回：句柄
 *         失败返回：-1
 */
long keys_open(void);

/**
 * @brief 关闭按键设备
 * @param handle 句柄，由keys_open()获取
 */
void keys_close(long handle);

/**
 * @brief 读取按键事件
 * @param handle 句柄，由keys_open()获取
 * @param key_event 按键值和状态的结构体
 * @param timeout 阻塞时间(-1：一直阻塞     0：不阻塞    大于0：阻塞时间（毫秒级）)
 * @return 小于0：读取按键事件出错 / poll错误
 *             1：成功读取到按键事件
 *             0：读取按键时间超时
 */
int read_key_event(long handle, struct key_event *key_event, int timeout);

#ifdef  __cplusplus
}
#endif

#endif

