#ifndef _LIBHARDWARE2_SSLV_H_
#define _LIBHARDWARE2_SSLV_H_

#include <stdio.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct sslv_config_data {
    unsigned int id;        /* SSLV 控制器 ID */
    char *name;             /* name 仅作为一个标识 */

    /* bits_per_word 代表数据的位宽.
        例如:bits_per_word = 32 时,SSLV传输过程中的最小数据单位为32bit. */
    unsigned int bits_per_word;

    /**
     * 极性 sslv_pol :
     * 当sslv_pol=0，在时钟空闲即无数据传输时,clk电平为低电平
     * 当sslv_pol=1，在时钟空闲即无数据传输时,clk电平为高电平
     * 相位 sslv_pha :
     * 当sslv_pha=0，表示在第一个跳变沿开始传输数据，下一个跳变沿完成传输
     * 当sslv_pha=1，表示在第二个跳变沿开始传输数据，下一个跳变沿完成传输
     */
    unsigned int sslv_pol;
    unsigned int sslv_pha;

    unsigned int loop_mode; /* 循环模式 */
};

/**
 * @brief 获得SSLV设备句柄
 * @param sslv_dev_path sslv设备路径(/dev/sslv0)
 * @return 成功返回sslv设备句柄, 失败返回-1
 */
int sslv_open(char *sslv_dev_path);

/**
 * @brief 关闭SSLV设备
 * @param fd sslv设备句柄
 * @return 无返回值
 */
void sslv_close(int fd);

/**
 * @brief 使能SSLV设备
 * @param fd sslv设备句柄
 * @return 成功返回0, 失败返回-1
 */
int sslv_enable(int fd);

/**
 * @brief 失能SSLV设备
 * @param fd sslv设备句柄
 * @return 成功返回0, 失败返回-1
 */
int sslv_disable(int fd);

/**
 * @brief 接收SPI Master发过来的长度为Size的数据
 * @param fd sslv设备句柄
 * @param buf 用于存放接收到的size长度的数据缓冲区
 * @param size 接收数据的长度(sizeof(buf))
 * @return 成功返回接收成功的字节数, 失败返回-1
 */
int sslv_receive(int fd, void *buf, int size);

/**
 * @brief 向SPI Master发送一串长度为Size的数据
 * @param fd sslv设备句柄
 * @param buf 存放需要发送出去的一串数据
 * @param size 发送数据长度
 * @param add_zero 是否在发送完成之后多发送一个数据0(1发 0不发)
 * @return 成功返回数据字节数, 失败返回-1
 */
int sslv_send(int fd, void *buf, int size, char add_zero);

/**
 * @brief 获取SSLV注册结构体信息
 * @param fd sslv设备句柄
 * @param info 结构体信息存放地址
 * @return 无返回值
 */
void sslv_get_info(int fd, struct sslv_config_data *info);

/**
 * @brief 设置SSLV传输模式
 * @param fd sslv设备句柄
 * @param mode 选择sslv传输模式(pol | pha)
 * @return 成功返回0, 失败返回-1
 */
int sslv_set_mode(int fd, int mode);

/**
 * @brief 设置SSLV最小传输单位
 * @param fd sslv设备句柄
 * @param bits 传输最小单位(8/16/32)
 * @return 成功返回0, 失败返回-1
 */
int sslv_set_bits(int fd, int bits);

#ifdef  __cplusplus
}
#endif

#endif /* _LIBHARDWARE2_SSLV_H_ */