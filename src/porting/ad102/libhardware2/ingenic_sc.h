#ifndef __LIBHARDWARE2_SC_H__
#define __LIBHARDWARE2_SC_H__

#ifdef  __cplusplus
extern "C" {
#endif
/*
 * sc decrypted.
 *    prepare a src_file encrypted.
 *        |---------------|-------------------|                    |
 *        | SC KEY(2048)  |  data encrypted   |
 *        |---------------|-------------------|                    |
 *
 * */

/* size per round must be align with 64, it can be different from kernel,
 * it doesn't matter when more than 2048 or less */
#define SC_MAX_SIZE_PERROUND    2048
/* be same with kernel */
#define SC_SIGNATURE_SIZE       2048

/**
 * @brief 启动sc设备获得sc设备句柄, 请确保在kernel内menuconfig内选中INGENIC_SC
 * @return 返回sc设备句柄
 */
int sc_open(void);

/**
 * @brief 根据传入的设备句柄fd关闭sc设备
 * @param fd sc设备句柄
 * @return 无返回值
 */
void sc_close(int fd);

/**
 * @brief 根据传入的head地址获取信息并设置密钥等签名信息, 获取实际加密的文件大小
 * @param fd sc设备句柄
 * @param head 加密签名过的数据的起始地址, 大小为2048字节
 * @return 成功返回实际加密的文件大小, 失败返回负数
 */
int sc_setup(int fd, unsigned char *head);

/**
 * @brief 解密: 将存放在src位置大小为size的数据解密, 并将解密出的数据存放在dst位置
 * @param fd sc设备句柄
 * @param src 源数据存放地址
 * @param dst 目标数据存放地址
 * @param size 数据大小
 * @return 成功返回0, 失败返回负数
 */
int sc_decrypt(int fd, unsigned char *src, unsigned char *dst, int size);

/**
 * @brief 解密in_file文件内容并将结果保存在out_file内
 * @param fd sc设备句柄
 * @param in_file 需要被解密的文件存放路径, 请确保该文件是被特殊加密工具加密签名过的文件(使用other bin方式勾选上AES及SIGNUTARE)
 * @param out_file 解密完成后的数据存放文件
 * @return 成功返回0, 失败返回负数
 */
int sc_decrypt_file(int fd, unsigned char *in_file, unsigned char *out_file);

#ifdef  __cplusplus
}
#endif

#endif
