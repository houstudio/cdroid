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

int rmem_cache_sync(int fd, void *mmaped_mem, int size, enum rmem_cache_type type);

#ifdef  __cplusplus
}
#endif

#endif