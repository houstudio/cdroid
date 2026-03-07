#ifndef __LIBHARDWARE2_HASH_H__
#define __LIBHARDWARE2_HASH_H__

#ifdef  __cplusplus
extern "C" {
#endif

#define CMD_hash_init        _IOWR('h', 0, int)
#define CMD_hash_write       _IOWR('h', 1, unsigned long)
#define CMD_hash_deinit      _IOWR('h', 2, unsigned long)
#define MD5_byte             16
#define SHA1_byte            20
#define SHA224_byte          28
#define SHA256_byte          32

enum encryption_mode {
    MD5,
    SHA1,
    SHA224,
    SHA256,
};
/**
 * NOTE:该库必须按以下流程执行，可以多次执行hash_write，
 *      一旦执行了hash_init，就必须执行完整个流程,若不执行完整个流程就重新执行则会报错。
 * 流程：hash_init -> hash_write -> hash_deinit
 */

/**
 * @brief 初始化hash加密模式并获取文件描述符
 * @param mode 加密模式：MD5,SHA1,SHA224,SHA256
 * @return 返回文件描述符，返回负数表示失败
 */
int hash_init(enum encryption_mode mode);
/**
 * @brief 写入需要加密的字符
 * @param handle 文件描述符
 * @param str 需要加密的字符串的指针
 * @param str_size 需要加密字符串的长度
 * @return 返回0表示写入成功，返回负数表示失败
 */
int hash_write(int handle,unsigned char *str,int str_size);
/**
 * @brief 读取密文同时关闭文件描述符
 * @param handle 文件描述符
 * @param rec 接收密文的指针
 * @param hash_size 接收密文的长度，不允许小于对应加密模式的长度
 * @return 返回0表示获取密文成功，返回负数表示失败
 */
int hash_deinit(int handle,unsigned char *rec,unsigned long hash_size);
#ifdef  __cplusplus
}
#endif

#endif