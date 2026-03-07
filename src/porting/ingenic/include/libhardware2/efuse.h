#ifndef __LIBHARDWARE2_EFUSE_H__
#define __LIBHARDWARE2_EFUSE_H__

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * @info:存放efuse每个段信息（名字、起始位、大小）的结构体
 * @seg_start:段的起始位
 * @seg_size:段的大小
 * @segment_name:段的名字
 */
struct efuse_segment_info {
    unsigned int seg_start;
    unsigned int seg_size;
    char *segment_name;
};

/**
 * @brief 写efuse
 * @param name 段的名字
 * @param wr_buf 写入的数据
 * @param start 写入的起始位
 * @param size 写入数据的大小
 * @return 返回，成功返回0，失败返回-1
 */
int efuse_write(char *name, unsigned char *wr_buf, int start, int size);

/**
 * @brief 读efuse
 * @param name 段的名字
 * @param buf 读取数据存放的位置
 * @param start 读取的起始位
 * @param size 读取数据的大小
 * @return 返回，成功返回0，失败返回-1
 */
int efuse_read(char *name, unsigned char *buf, int start, int size);

/**
 * @brief 读efuse某一段的大小
 * @param name 段的名字
 * @return 返回，成功返回段的大小，失败返回-1
 */
int efuse_read_seg_size(char *name);

/**
 * @brief 获取efuse每个段的信息
 * @return 返回，成功返回存放efuse每个段信息的结构体（最后一个结构体的segment_name为NULL），失败返回NULL
 */
struct efuse_segment_info *efuse_get_segment_information(void);

/**
 * @brief 释放存放efuse每个段信息时所申请的空间
 * @param list 存放efuse每个段信息的结构体
 */
void efuse_free_information(struct efuse_segment_info *info);

#ifdef  __cplusplus
}
#endif

#endif
