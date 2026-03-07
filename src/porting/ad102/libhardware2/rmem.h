#ifndef __LIBHARDWARE2_RMEM_H__
#define __LIBHARDWARE2_RMEM_H__

#ifdef  __cplusplus
extern "C" {
#endif

struct rmem_alloc_data {
    unsigned int size;
    void *mem;
};

int rmem_open(void);
void rmem_close(int fd);
void *rmem_alloc(int fd, unsigned long *phy_addr, int size);
void rmem_free(int fd, void *mmaped_rmem, unsigned long phy_addr, int size);

enum rmem_cache_type {
    rmem_cache_dev_to_mem,
    rmem_cache_mem_to_dev,
};

/**
 * @brief 刷指定大小的cache
 * @param fd rmem 描述符
 * @param mmaped_mem  要刷cache的起始地址
 * @param size  需要刷的区域大小
 * @param type  rmem的cache类型
 * @return 成功返回0
 */
int rmem_cache_sync(int fd, void *mmaped_mem, int size, enum rmem_cache_type type);

/**
 * @brief 刷指定区域内的cache (行与行之间不连续时, 可用于按行刷cache)
 * @param fd rmem 描述符
 * @param mmaped_mem  要刷cache的起始地址
 * @param rect_w  指定区域的宽(按字节)
 * @param rect_h  指定区域的高(按字节)
 * @param stride  实际的一行大小(按字节)
 * @param type  rmem的cache类型
 * @return 成功返回0
 */
int rmem_rect_cache_sync(int fd, void *mmaped_mem,
            int rect_w, int rect_h, int stride, enum rmem_cache_type type);



/**
 * @brief 添加额外的内存
 * @param fd rmem 描述符
 * @param mem 物理内存地址
 * @param size 大小
 * @return 成功返回0
 */
int rmem_add_extra_mem(int fd, unsigned long phymem, int size);

#ifdef  __cplusplus
}
#endif

#endif