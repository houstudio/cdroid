#ifndef __LIBHARDWARE2_SPI_H__
#define __LIBHARDWARE2_SPI_H__

#include <linux/spi/spidev.h>

#ifdef  __cplusplus
extern "C" {
#endif

struct spi_info {
    unsigned char spi_mode;
    unsigned int spi_speed;
    unsigned char spi_bits;
    unsigned char spi_lsb;
};

struct spidev_register_data {
    int busnum;
    char *cs_gpio;
    char spidev_path[20];
};

/**
 * @brief 获得spi设备句柄
 * @param spi_dev_path spi设备路径,如/dev/spidev0.0
 * @return 成功返回设备句柄,失败返回-1
 */
int spi_open(char *spi_dev_path);

/**
 * @brief 关闭spi设备
 * @param spi_fd spi设备句柄
 * @return 无返回值
 */
void spi_close(int spi_fd);

/**
 * @brief spi设备数据传输（同时发送和接收）
 * @param spi_fd spi设备句柄
 * @param spi_tr spi设备传输需要配置的结构体
 * @param num 传输结构体的个数
 * @return 成功返回0,失败返回-1
 */
int spi_transfer(int spi_fd, struct spi_ioc_transfer *spi_tr, int num);

/**
 * @brief spi设备数据传输（只发送不接收）
 * @param spi_fd spi设备句柄
 * @param tx_buf 要发送数据的缓冲区
 * @param len 发送的长度（单位：字节）
 * @return 成功返回0,失败返回-1
 */
int spi_write(int spi_fd, char *tx_buf, int len);

/**
 * @brief spi设备数据传输（只接收不发送）
 * @param spi_fd spi设备句柄
 * @param rx_buf 接收数据的缓冲区
 * @param len 接收的长度（单位：字节）
 * @return 成功返回0,失败返回-1
 */
int spi_read(int spi_fd, char *rx_buf, int len);

/**
 * @brief 设置spi传输时的模式
 * @param spi_fd spi设备句柄
 * @param mode 传输时的模式
 * @return 成功返回0,失败返回-1
 */
int spi_set_mode(int spi_fd, int mode);

/**
 * @brief 设置spi传输时的速度
 * @param spi_fd spi设备句柄
 * @param speed 传输时的速度
 * @return 成功返回0,失败返回-1
 */
int spi_set_speed(int spi_fd, int speed);

/**
 * @brief 设置spi传输时的位数
 * @param spi_fd spi设备句柄
 * @param bits_per_word 传输时的位数
 * @return 成功返回0,失败返回-1
 */
int spi_set_bits(int spi_fd, int bits_per_word);

/**
 * @brief 设置spi传输时数据的高低位发送顺序
 * @param spi_fd spi设备句柄
 * @param lsb 1表示低位在前，0表示高位在前
 * @return 成功返回0,失败返回-1
 */
int spi_set_lsb(int spi_fd, int lsb_first);

/**
 * @brief 获取spi传输的配置信息
 * @param spi_fd spi设备句柄
 * @param spi 存放信息的结构体
 * @无返回值
 */
void spi_get_info(int spi_fd, struct spi_info *spi);

/**
 * @brief 添加一个spi设备
 * @param data 注册添加一个spi设备需要准备的结构体
 * @return 成功返回0， 失败返回-1
 */
int spi_add_device(struct spidev_register_data *data);

/**
 * @brief 删除一个spi设备
 * @param spidev_path 设备节点的路径,如：/dev/spidev0.0
 * @return 成功返回0,失败返回-1
 */
int spi_del_device(char *spidev_path);

#ifdef  __cplusplus
}
#endif

#endif