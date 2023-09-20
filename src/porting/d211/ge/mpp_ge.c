/*
 * Copyright (C) 2022-2023 ArtinChip Technology Co., Ltd.
 * Authors:  Ning Fang <ning.fang@artinchip.com>
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <pthread.h>
#include <artinchip/sample_base.h>
#include "mpp_ge.h"
#include "ge_ops.h"

#define GE_DEV			"/dev/ge"

extern struct ge_ops ge_normal_ops;
extern struct ge_ops ge_cmdq_ops;

struct ge_ops *ge_ops_lists[] =
{
	&ge_normal_ops,
	&ge_cmdq_ops,
	0
};

static enum ge_mode ge_get_mode(int fd)
{
	enum ge_mode mode;

	int ret = ioctl(fd, IOC_GE_MODE, &mode);
	if (ret < 0)
		printf("ioctl() return %d\n", ret);

	return mode;
}

struct mpp_ge *mpp_ge_open()
{
	int ret;
	struct mpp_ge *ge;

	ge = (struct mpp_ge *)malloc(sizeof(struct mpp_ge));
	if (!ge) {
		printf("mpp_ge malloc failed!\n");
		return NULL;
	}

	memset(ge, 0, sizeof(struct mpp_ge));

	ge->dev_fd = open(GE_DEV, O_RDWR);
	if (ge->dev_fd < 0) {
		printf("Failed to open %s. errno: %d[%s]\n",
			GE_DEV, errno, strerror(errno));
		goto EXIT;
	}

	pthread_mutex_init(&ge->lock, NULL);
	ge->mode = ge_get_mode(ge->dev_fd);
	ge->ops = ge_ops_lists[ge->mode];

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->open(ge);
	pthread_mutex_unlock(&ge->lock);

	if (ret < 0) {
		goto EXIT;
	}

	return ge;
EXIT:
	if (ge) {
		if(ge->dev_fd >= 0)
			close(ge->dev_fd);

		free(ge);
	}

	return NULL;
}

void mpp_ge_close(struct mpp_ge *ge)
{
	if (!ge)
		return;

	pthread_mutex_lock(&ge->lock);
	ge->ops->close(ge);
	pthread_mutex_unlock(&ge->lock);

	if (ge->dev_fd >= 0)
		close(ge->dev_fd);

	free(ge);
}

enum ge_mode mpp_ge_get_mode(struct mpp_ge *ge)
{
	return ge->mode;
}

int mpp_ge_add_dmabuf(struct mpp_ge *ge, int dma_fd)
{
	int ret;

	if (!ge)
		return -1;

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->add_dma_buf(ge, dma_fd);
	pthread_mutex_unlock(&ge->lock);

	return ret;
}

int mpp_ge_rm_dmabuf(struct mpp_ge *ge, int dma_fd)
{
	int ret;

	if (!ge)
		return -1;

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->rm_dma_buf(ge, dma_fd);
	pthread_mutex_unlock(&ge->lock);

	return ret;
}

int mpp_ge_fillrect(struct mpp_ge *ge, struct ge_fillrect *fill)
{
	int ret;

	if (!ge)
		return -1;

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->fillrect(ge, fill);
	pthread_mutex_unlock(&ge->lock);

	return ret;
}

int mpp_ge_bitblt(struct mpp_ge *ge, struct ge_bitblt *blt)
{
	int ret;

	if (!ge)
		return -1;

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->bitblt(ge, blt);
	pthread_mutex_unlock(&ge->lock);

	return ret;
}

int mpp_ge_rotate(struct mpp_ge *ge, struct ge_rotation *rot)
{
	int ret;

	if (!ge)
		return -1;

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->rotate(ge, rot);
	pthread_mutex_unlock(&ge->lock);

	return ret;
}

int mpp_ge_emit(struct mpp_ge *ge)
{
	int ret;

	if (!ge)
		return -1;

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->emit(ge);
	pthread_mutex_unlock(&ge->lock);

	return ret;
}

int mpp_ge_sync(struct mpp_ge *ge)
{
	int ret;

	if (!ge)
		return -1;

	pthread_mutex_lock(&ge->lock);
	ret = ge->ops->sync(ge);
	pthread_mutex_unlock(&ge->lock);

	return ret;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
