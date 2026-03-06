#ifndef __LIBHARDWARE2_AES_H__
#define __LIBHARDWARE2_AES_H__

#ifdef  __cplusplus
extern "C" {
#endif

enum aes_keyl {
    AES128,
    AES192,
    AES256,
};

enum aes_mode {
    ECB_MODE,
    CBC_MODE,
};

enum aes_endian {
    ENDIAN_LITTLE,
    ENDIAN_BIG, /* openssl uses */
};

enum aes_dece {
    ENCRYPTION,
    DECRYPTION,
};

struct aes_config {
    enum aes_keyl keyl;     /* 密钥长度: 0:128 1:196 2:256 */
    enum aes_mode mode;     /* 编解码是否前后相关: 0:ecb 1:cbc */
    enum aes_endian endian; /* 编解码数据输入大小端模式: 0:little 1:big */
    unsigned char ukey[33]; /* 用户传入的密钥 */
    unsigned char iv[17];   /* 初始化向量(cbc模式使用) */
};

/**
 * @brief 获得aes设备句柄, 根据aes配置结构体内的ukey以及keyl转化得出实际密钥, 且申请aes_handle结构体,
 *      并将传入的aes配置结构体config 以及 获得的aes设备句柄fd 和 实际密钥key存放到aes_handle结构体内
 * @param config aes配置结构体, 存放密钥长度,编解码模式,数据输入大小端,用户密钥,初始化向量(cbc模式使用)
 * @return 成功返回aes_handle结构体, 失败返回NULL
 */
struct aes_handle *aes_open(struct aes_config *config);

/**
 * @brief 根据传入的handle内的设备句柄fd关闭aes设备, 释放handle结构体
 * @param handle aes设备句柄结构体
 * @return 无返回值
 */
void aes_close(struct aes_handle *handle);

/**
 * @brief 编码 起始地址为src 长度为len字节 的数据并将结果保存在dst内
 * @param handle aes设备句柄结构体, 存放aes设备句柄fd,
 *      aes配置结构体config(存放密钥长度,模式,大小端,密钥,初始化向量(cbc模式使用))
 * @param src 源数据存放起始地址
 * @param dst 目标数据存放起始地址
 * @param len 编码数据长度
 * @return 成功返回0, 失败返回负数
 */
int aes_encrypt(struct aes_handle *handle, unsigned char *src, unsigned char *dst, unsigned int len);

/**
 * @brief 解码 起始地址为src 长度为len字节 的数据并将结果保存在dst内
 * @param handle aes设备句柄结构体, 存放aes设备句柄fd,
 *      aes配置结构体config(存放密钥长度,模式,大小端,密钥,初始化向量(cbc模式使用))
 * @param src 源数据存放起始地址
 * @param dst 目标数据存放起始地址
 * @param len 解码数据长度
 * @return 成功返回0, 失败返回负数
 */
int aes_decrypt(struct aes_handle *handle, unsigned char *src, unsigned char *dst, unsigned int len);

/**
 * @brief 编码in_file文件内容并将结果保存在out_file内
 * @param handle aes设备句柄结构体, 存放aes设备句柄fd,
 *      aes配置结构体config(存放密钥长度,模式,大小端,密钥,初始化向量(cbc模式使用))
 * @param in_file 需要被编码的数据存放文件路径, 请确保in_file文件大小以16字节对齐
 * @param out_file 编码完成后的数据存放文件路径
 * @return 成功返回0, 失败返回负数
 */
int aes_encrypt_file(struct aes_handle *handle, unsigned char *in_file, unsigned char *out_file);

/**
 * @brief 解码in_file文件内容并将结果保存在out_file内
 * @param handle aes设备句柄结构体, 存放aes设备句柄fd,
 *      aes配置结构体config(存放密钥长度,模式,大小端,密钥,初始化向量(cbc模式使用))
 * @param in_file 需要被解码的数据存放文件路径, 请确保in_file文件大小以16字节对齐
 * @param out_file 解码完成后的数据存放文件路径
 * @return 成功返回0, 失败返回负数
 */
int aes_decrypt_file(struct aes_handle *handle, unsigned char *in_file, unsigned char *out_file);

#ifdef  __cplusplus
}
#endif

#endif
