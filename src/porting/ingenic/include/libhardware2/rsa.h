#ifndef __LIBHARDWARE2_RSA_H__
#define __LIBHARDWARE2_RSA_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define RSA_MAX_KEYL 64

enum rsa_mode {
    RSA_1024,
    RSA_2048,
};

struct rsa_key {
    int is_pub_use;
    enum rsa_mode mode;
    unsigned int n[RSA_MAX_KEYL];
    unsigned int e[RSA_MAX_KEYL];
    unsigned int d[RSA_MAX_KEYL];
};

struct rsa_data {
    int len;
    unsigned int *src;
    unsigned int *dst;
};

struct rsa_handle {
    int fd;
    int keyB;
    enum rsa_mode mode;
};

/**
 * @brief 获得rsa设备句柄结构体, 将key密钥文件内的数据解析为rsa数据结构体内的e,d以及n,
 *        密钥长度设置mode, 且根据密钥选择将e或d以及n密钥数据写入对应寄存器并等待perprocess完成
 * @param key rsa_key结构体, 存放密钥选择, 密钥长度mode，密钥e, 密钥d, 密钥n
 * @param key_file 密钥存放文件路径, 请确保key文件为DER格式
 * @return 成功返回rsa设备句柄结构体, 失败返回NULL
 */
struct rsa_handle *rsa_open(struct rsa_key *key, char *key_file);

/**
 * @brief 根据传入的设备句柄fd关闭rsa设备
 * @param handle rsa设备句柄结构体
 * @return 无返回值
 */
void rsa_close(struct rsa_handle *handle);

/**
 * @brief 编解码 起始地址为src 长度为len个word 的数据并将结果保存在dst内
 * @param fd rsa设备句柄
 * @param data rsa数据结构体, 存放src,dst地址,编解码长度len
 * @return 成功返回0, 失败返回负数
 */
int rsa_crypt(int fd, struct rsa_data *data);

/**
 * @brief 编码in文件内容并将结果保存在out内, 数据添加填充方式为openssl的PKCS
 * @param handle rsa设备句柄结构体, 存放rsa设备句柄fd, 密钥mode及长度keyB
 * @param in_file 需要被编码的数据存放文件路径
 * @param out_file 编码完成后的数据存放文件路径
 * @return 成功返回0, 失败返回负数
 */
int rsa_encrypt_file(struct rsa_handle *handle, char *in_file, char *out_file);

/**
 * @brief 解码in文件内容并将结果保存在out内, 数据去除填充方式为openssl的PKCS
 * @param handle rsa设备句柄结构体, 存放rsa设备句柄fd, 密钥mode及长度keyB
 * @param in_file 需要被解码的数据存放文件路径
 * @param out_file 解码完成后的数据存放文件路径
 * @return 成功返回0, 失败返回负数
 */
int rsa_decrypt_file(struct rsa_handle *handle, char *in_file, char *out_file);

#ifdef  __cplusplus
}
#endif

#endif
