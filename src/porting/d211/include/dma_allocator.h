/*
* Copyright (C) 2020-2022 Artinchip Technology Co. Ltd
*
*  author: qi.xu@artinchip.com
*  Desc: dma-buf allocator
*/

#ifndef DMA_ALLOCATOR_H
#define DMA_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <video/mpp_types.h>

enum dma_buf_sync_flag {
	// clean. check dirty bit of cacheline. if the dirty bit is 1,
	// write the cacheline to dram, then set dirty bit 0
	CACHE_CLEAN = 0,

	// invalid. set the cacheline invalid
	CACHE_INVALID = 1,

	// flush. clean then invalid
	CACHE_FLUSH = 2,
};

/**
 * dmabuf_device_open - open dma-buf heap device
 *
 * return: fd of dma-buf heap device
 */
int dmabuf_device_open();

/**
 * dmabuf_device_close - close dma-buf heap device
 * @dma_fd: fd of dma-buf heap device
 */
void dmabuf_device_close(int dma_fd);

/**
 * dmabuf_device_destroy - destroy dma-buf heap
 * @dma_fd: fd of dma-buf heap device
 */
void dmabuf_device_destroy(int dma_fd);

/**
 * dmabuf_alloc - alloc a dma-buf
 * @dma_fd: fd of dma-buf heap device
 * @size: size of dma-buf
 *
 * return: error if < 0; else return fd of dma-buf
 */
int dmabuf_alloc(int dma_fd, int size);

/**
 * dmabuf_free - free a dma-buf
 * @buf_fd: fd of dma-buf
 */
void dmabuf_free(int buf_fd);

/**
 * dmabuf_mmap - mmap dma-buf to virtual space
 * @buf_fd: fd of dma-buf
 * @size: size of dma-buf
 * return virtual address of dma-buf
 */
unsigned char* dmabuf_mmap(int buf_fd, int size);

/**
 * dmabuf_munmap - munmap dma-buf from virtual space
 * @addr: virtual address of dma-buf
 * @size: size of dma-buf
 */
void dmabuf_munmap(unsigned char* addr, int size);

/**
 * dmabuf_sync - sync data for dma-buf
 * @buf_fd: fd of dma-buf
 * @flag: cache sync flag
 */
int dmabuf_sync(int buf_fd, enum dma_buf_sync_flag flag);

/**
 * dmabuf_sync - sync range data for dma-buf
 * @buf_fd: fd of dma-buf
 * @start_addr: the virtual address of the buffer need flush cache
 * @size: the size of the buffer need flush cache
 * @flag: cache sync flag
 */
int dmabuf_sync_range(int buf_fd, unsigned char* start_addr, int size, enum dma_buf_sync_flag flag);

/**
 * mpp_buf_alloc - alloc a mpp-buf, dma-buf size is stride[i]*height in struct mpp_buf
 * @dma_fd: fd of dma-buf heap device
 * @buf: mpp_buf
 */
int mpp_buf_alloc(int dma_fd, struct mpp_buf* buf);

/**
 * mpp_buf_free - free a mpp-buf
 * @buf: mpp_buf
 */
void mpp_buf_free(struct mpp_buf* buf);

#ifdef __cplusplus
}
#endif

#endif /* DMA_ALLOCATOR_H */
